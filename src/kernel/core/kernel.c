#include <string.h>
#include <stdint.h>

#include "kernel.h"
#include "kernel_context.h"

#include "kernel_exec.h"

#include "service_host.h"
#include "events_service.h"

#define USER_EVENTS_POOL               (50)
#define MAX_EVENTS                     (USER_EVENTS_POOL * 2)

#define MAX_QUEUE_EVENTS			   (10)
#define MAX_EVT_SUBSCRIBERS            (10)

#define MAX_PROCCESSES                 (10)
#define MAX_EXEC_PROC_ALLOWED          (1)

#define EVT_PROG_PROCESS_FINISHED      (0xff)

typedef struct {

	uint8_t evtId;
	evt_data_t evtData;

} evt_t;

typedef struct {

	uint8_t nUserEvtSubscribers;
	uint8_t nKernelEvtSubscribers;

	evt_subscriber user_subscribers[MAX_EVT_SUBSCRIBERS];
	evt_subscriber kernel_subscribers[MAX_EVT_SUBSCRIBERS];

} evt_subscription_token_t;

typedef struct {

	int argc;
	const char **argv;
	int (*exec_callee_addr)(int argc, const char **argv);

	evt_subscription_token_t *evtTokens[USER_EVENTS_POOL];

	uint16_t flags;
	uint16_t id;
	uint16_t subscriptions;

} exec_proccess_t;

static uint8_t EvtHead                                  = 0;
static uint8_t EvtTail                                  = 0;

static evt_t *RecvEvt                                   = NULL;
static uint8_t FreeEvents                               = MAX_QUEUE_EVENTS;

static evt_t Events[MAX_QUEUE_EVENTS];
static evt_subscription_token_t EvtTokens[MAX_EVENTS];

static uint8_t ExecProc                                 = 0;
static uint16_t KernelFlags                             = 0;

static exec_proccess_t *Process                         = NULL;
static exec_proccess_t Processes[MAX_PROCCESSES];

static void runExecCtx(void);

static void unsubscribeExecCtx(void);
static void makeProcessExecWhenRequested(void);

static uint16_t *getContextFlags(void);
static void enqueEvt(uint8_t evtId, evt_data_t* evtData);

void _evt_service() { }

service_status_codes_t _service_push_events() {

	if (FreeEvents == MAX_QUEUE_EVENTS)
		return SERVICE_OK;

	do {

		// Pull next event

		RecvEvt =
				&Events[EvtHead % MAX_QUEUE_EVENTS];

		EvtHead++;
		FreeEvents++;

		// Invoke Kernel/User subscribers

		evt_subscription_token_t *token = &EvtTokens[RecvEvt -> evtId];

		for(uint8_t counter = 0;
				counter < token -> nKernelEvtSubscribers; counter++)
			(token -> kernel_subscribers[counter])(&(RecvEvt -> evtData));

		if (token -> nUserEvtSubscribers)
			(token -> user_subscribers[token -> nUserEvtSubscribers - 1])(&(RecvEvt -> evtData));

	} while(FreeEvents < MAX_QUEUE_EVENTS);

	return SERVICE_OK;
}

void _kernel_publish_evt(uint8_t evtId, evt_data_t* evtData) {

	if (Process &&
			(evtId < USER_EVENTS_POOL)) return; // Process tries publish kernel events

	enqueEvt(evtId, evtData);
}

void _kernel_subscribe_evt(uint8_t evt, evt_subscriber subscriber) {

	if (evt < USER_EVENTS_POOL) return;  // Only user events subscriptions allowed with this call for both Kernel and User

	evt_subscription_token_t* token = &EvtTokens[evt];

	if (token -> nUserEvtSubscribers == MAX_EVT_SUBSCRIBERS) return;

	if (!((size_t) token -> user_subscribers[
								token -> nUserEvtSubscribers - 1] ^ (size_t) subscriber)) return;

	uint8_t processEvtToken =
			evt % USER_EVENTS_POOL;

	// When process subscription but already subscribed

	if (Process &&
			Process -> evtTokens[processEvtToken]) {

		token ->
			user_subscribers[
			      token -> nUserEvtSubscribers - 1] = subscriber; // Override with new subscriber

		return;
	}

	token -> user_subscribers[
				token -> nUserEvtSubscribers++] = subscriber;

	// When not a process subscription
	if (!Process) return;

	Process -> evtTokens[processEvtToken] = token;
	Process -> subscriptions++;
}

void _kernel_subscribe_kernel_evt(uint8_t evt, evt_subscriber subscriber) {

	if (Process) return; // Subscribing kernel events by the Process is not allowed

	evt_subscription_token_t* token = &EvtTokens[evt];
	if (!(token -> nKernelEvtSubscribers ^ MAX_EVT_SUBSCRIBERS)) return;

	token -> kernel_subscribers[token -> nKernelEvtSubscribers++] = subscriber;
}

void _kernel_exec_f(
		int (*exec_callee_addr)(int argc, const char **argv),
		int argc, const char **argv) {

	if (!(ExecProc ^ MAX_EXEC_PROC_ALLOWED))
		return; // [DEBUG]: Running more than one user process isn`t allowed yet

	exec_proccess_t *proc =
			(Processes + ExecProc);

	proc -> argc = argc;
	proc -> argv = argv;

	proc -> exec_callee_addr = exec_callee_addr;

	ExecProc++;
}

void _kernel_pipeline() {

	_services_pipeline();
	makeProcessExecWhenRequested();
}

void _kernel_clear_context_flags(uint16_t clrFlags) {

	uint16_t *ctxFlags = getContextFlags();
	*ctxFlags &= (~clrFlags);
}

void _kernel_context_flags(uint16_t setFlags, uint16_t **ctxFlags) {

	*ctxFlags = getContextFlags();
	**ctxFlags |= setFlags;
}

// -------------------------------------------------------------------------------------------------------------------------------------------

static uint16_t *getContextFlags() {
	return Process ?
			&(Process -> flags) : &KernelFlags;
}

static void enqueEvt(uint8_t evtId, evt_data_t* evtData) {

	if (!FreeEvents) return;

	evt_t *evt =
			&Events[EvtTail % MAX_QUEUE_EVENTS];

	evt -> evtId = evtId;

	if (evtData)
		memcpy(&evt -> evtData, evtData, sizeof(evt_data_t));

	EvtTail++;
	FreeEvents--;
}

static void unsubscribeExecCtx() {

	for (uint16_t c1 = 0; c1 < USER_EVENTS_POOL; c1++) {

		evt_subscription_token_t *token =
				Process -> evtTokens[c1];

		if (!token) continue;

		token -> nUserEvtSubscribers--;
		Process -> evtTokens[c1] = NULL;
	}

	Process -> subscriptions ^=
			Process -> subscriptions;
}

static void makeProcessExecWhenRequested() {

	if (!ExecProc) return;
	if (Process &&
			!((Process -> id) ^ ExecProc)) return; // When OS pipeline is invoked from the current process

	runExecCtx();

	ExecProc--;
	Process = ExecProc ? Process + (ExecProc - 1) : NULL; // Pop previouse process or NULL when the bottom most stack process is finished
}

static void runExecCtx() {

	Process =
			Processes + (ExecProc - 1);

	Process -> id = ExecProc;

	Process -> subscriptions ^=
			Process -> subscriptions;

	uint8_t argc = Process -> argc;
	const char **argv = (const char **) Process -> argv;

	int (*exec_callee_addr)(int argc,
			const char **argv) = Process -> exec_callee_addr;

	exec_callee_addr(argc, argv);

	// Unsubscribe just finished process

	if (Process -> subscriptions)
		unsubscribeExecCtx();

	// Publish process finished event
	enqueEvt(EVT_PROG_PROCESS_FINISHED, NULL);
}
