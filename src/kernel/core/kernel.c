#include <assert.h>

#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <limits.h>

#include "kernel.h"
#include "kernel_exec.h"
#include "kernel_signal.h"

#include "kernel_svc.h"

#define USER_EVENTS_POOL               (50)
#define MAX_EVENTS                     (USER_EVENTS_POOL * 2)

#define MAX_QUEUE_EVENTS			   (10)
#define MAX_EVT_SUBSCRIBERS            (10)

#define MAX_PROCCESSES                 (1)
#define MAX_EXEC_PROC_ALLOWED          (1)

#define EVT_PROCESS_FINISHED           (0xff)

struct evt_s {

	uint8_t evtId;
	evt_data_t evtData;

};

struct evt_sub_token_s {

	uint8_t user_evtSubs;
	uint8_t kernel_evtSubs;

	evt_subscriber user_subscribers[MAX_EVT_SUBSCRIBERS];
	evt_subscriber kernel_subscribers[MAX_EVT_SUBSCRIBERS];

};

struct proc_s {

	int argc;
	const char **argv;
	int (*callee_addr)(int argc, const char **argv);

	struct evt_sub_token_s *evt_tokens[USER_EVENTS_POOL];

	uint16_t id;
	uint16_t subscriptions;

	sigset_t flags;
	sigset_t flagMask;
};

typedef struct evt_s evt_t;
typedef struct evt_sub_token_s evt_subscription_token_t;

typedef struct proc_s process_t;

static uint8_t evt_head                                 = 0;
static uint8_t evt_tail                                 = 0;

static uint8_t free_events                              = MAX_QUEUE_EVENTS;

static evt_t events[MAX_QUEUE_EVENTS];
static evt_subscription_token_t evt_tokens[MAX_EVENTS];
   
static sigset_t kernel_flags                            = 0;
static sigset_t kernel_flagMask                         = 0;

static uint8_t proc_exec                                = 0;

static process_t *process                               = NULL;
static process_t processes[MAX_PROCCESSES];

static sigset_t* context_flags(void);
static sigset_t* context_flagMask(void);

static void run_proc(void);
static void unsubscribe_proc(void);

static void exec_evts(void);
static void enque_evt(uint8_t evtId, evt_data_t* evtData);

void _kernel_pubEvt(uint8_t evtId, evt_data_t* evtData) {

	if ((process != NULL) &&
			(evtId < USER_EVENTS_POOL)) return; // Process tries publish kernel events

	enque_evt(evtId, evtData);
}

void _kernel_subEvt(uint8_t evt, evt_subscriber subscriber) {

	if (evt < USER_EVENTS_POOL) return;  // Only user events subscriptions allowed with this call for both Kernel and User

	evt_subscription_token_t* token = &evt_tokens[evt];

	if (token -> user_evtSubs == MAX_EVT_SUBSCRIBERS) return;

	if (!((size_t) token -> user_subscribers[
								token -> user_evtSubs - 1] ^ (size_t) subscriber)) return;

	uint8_t processEvtToken =
			evt % USER_EVENTS_POOL;

	// When process subscription but already subscribed

	if ((process != NULL) &&
			process -> evt_tokens[processEvtToken]) {

		token ->
			user_subscribers[
			      token -> user_evtSubs - 1] = subscriber; // Override with new subscriber

		return;
	}

	token -> user_subscribers[
				(token -> user_evtSubs)++] = subscriber;

	// When not a process subscription
	if (process == NULL) return;

	process -> evt_tokens[processEvtToken] = token;
	process -> subscriptions++;
}

void _kernel_subkEvt(uint8_t evt, evt_subscriber subscriber) {

	if (process != NULL) return; // Subscribing kernel events by the Process is not allowed

	evt_subscription_token_t* token = &evt_tokens[evt];
	if (!((token -> kernel_evtSubs) ^ MAX_EVT_SUBSCRIBERS)) return;

	token -> kernel_subscribers[(token -> kernel_evtSubs)++] = subscriber;
}

void _kernel_pipeline() {

	exec_evts();           // execute events (subscribers)
	_kernel_svcPipeline(); // execute services pipeline
	
	if (proc_exec == 0) {  // if a process exec is requested
		return;
	}

	if ((process != NULL) &&
			((process -> id) == proc_exec)) return; // when kernel pipeline is invoked from the current process

	run_proc(); // not a current process context -> run process

	proc_exec--;
	process = (proc_exec > 0) ? &processes[proc_exec - 1] : NULL; // pop previouse process or NULL when the bottom most stack process is finished
}

// ------------------------------------------ kenel svc -----------------------------------------------------------

void _kernel_svchost() {
}

void _kernel_svcPipeline() {
}

// ------------------------------------------ kexec -------------------------------------------------------------

void _kernel_exec_f(
		int (*exec_callee_addr)(int argc, const char **argv),
		int argc, const char **argv) {

	if (proc_exec == MAX_EXEC_PROC_ALLOWED)
		return; // [DEBUG]: Running more than one user process isn`t allowed yet

	process_t *proc = &processes[proc_exec];

	proc -> argc = argc;
	proc -> argv = argv;

	proc -> callee_addr = exec_callee_addr;

	proc_exec++;
}

// ------------------------------------------ signals -------------------------------------------------------------

int	raise(int sgl) {

	int flag = sgl;

	flag--;
	if ((flag < 0) || 
			((size_t) flag > (sizeof(sigset_t) * CHAR_BIT))) {
		return -1;
	}

	sigset_t *ctx_flags = context_flags();
	*ctx_flags |= (((sigset_t) 1) << flag);

	return 0;
}

int sigwait(const sigset_t *set, int *sgl) {

	static_assert(((sigset_t) - 1) > 0, "sigset_t must be unsigned");

	sigset_t sgls = 0;
	sigset_t wait_set = *set;

	sigset_t *ctx_flags = context_flags();

	do {		
		_kernel_pipeline();
		sgls = ((*ctx_flags) & wait_set);
	} while(sgls == 0);

	int s;
	sigset_t i;

	for (i = 0, s = (sgls & 1); 
			(s == 0) && (i < (sizeof(sigset_t) * CHAR_BIT)); 
					i++, sgls >>= 1, s = (sgls & 1)) { }

	*sgl = (i + 1);
	*ctx_flags &= (~(((sigset_t) 1) << i));

	return 0;
}

int sigpending(sigset_t *set) {

	static_assert(((sigset_t) - 1) > 0, "sigset_t must be unsigned");

	sigset_t pending_set = 0;
	sigset_t *ctx_flags = context_flags();

	_kernel_pipeline();
	pending_set = *ctx_flags;

	*set = pending_set;
	return 0;
}

int sigprocmask(int what, const sigset_t *set, sigset_t *oldset) {
	
	static_assert(((sigset_t) - 1) > 0, "sigset_t must be unsigned");

	sigset_t *ctx_flagMask = context_flagMask();

	if (oldset != NULL) {
		*oldset = *ctx_flagMask;
	}

	if (set == NULL) {
		return 0;
	}

	switch(what) {

		case SIG_UNBLOCK: {
						
			*ctx_flagMask &= ~(*set);

			sigset_t *ctx_flags = context_flags();
			*ctx_flags &= *(ctx_flagMask);
		} break;

		case SIG_SETMASK: { *ctx_flagMask = *set; } break;
		case SIG_BLOCK: { *ctx_flagMask |= (*set); } break;

		default:
			return -1;
	}

	return 0;
}

int _kernel_sigemptyset(sigset_t *set) {
	*set = 0;
	return 0;
}

int _kernel_sigaddset(sigset_t *set, const int sgl) {

	int flag = sgl;

	flag--;
	if ((flag < 0) ||
			((size_t) flag > sizeof(sigset_t) * CHAR_BIT)) {
		return -1;
	}

	*set |= (((sigset_t) 1) << flag);

	return 0;
}

bool _kernel_sigismember(sigset_t *set, const int sgl) {

	int flag = sgl;

	flag--;
	if ((set == NULL) || (flag < 0) || 
			((size_t) flag > (sizeof(sigset_t) * CHAR_BIT))) {
		return false;
	}

	return ((*set) & ((((sigset_t) 1) << flag)));
}

// -------------------------------------------- internal private --------------------------------------------------

static sigset_t* context_flags(void) {
	return (process != NULL) ?
			&(process -> flags) : &kernel_flags;
}

static sigset_t* context_flagMask(void) {
	return (process != NULL) ?
		&(process -> flagMask) : &kernel_flagMask;
}

static __attribute__((noinline)) void exec_evts(void) {

	if (free_events == MAX_QUEUE_EVENTS) {
		return;
	}

	do {

		// Pull next event

		evt_t *recv_evt =
				&events[evt_head % MAX_QUEUE_EVENTS];

		evt_head++;
		free_events++;

		// Invoke Kernel/User subscribers

		evt_subscription_token_t *token = &evt_tokens[recv_evt -> evtId];

		for(uint8_t counter = 0;
				counter < token -> kernel_evtSubs; counter++)
			(token -> kernel_subscribers[counter])(&(recv_evt -> evtData));

		if (token -> user_evtSubs)
			(token -> user_subscribers[token -> user_evtSubs - 1])(&(recv_evt -> evtData));

	} while(free_events < MAX_QUEUE_EVENTS);
}

static __attribute__((noinline)) void enque_evt(uint8_t evtId, evt_data_t* evtData) {

	if (!free_events) return;

	evt_t *evt =
			&events[evt_tail % MAX_QUEUE_EVENTS];

	evt -> evtId = evtId;

	if (evtData)
		memcpy(&evt -> evtData, evtData, sizeof(evt_data_t));

	evt_tail++;
	free_events--;
}

static __attribute__((noinline)) void unsubscribe_proc() {

	for (size_t i = 0; i < USER_EVENTS_POOL; i++) {

		evt_subscription_token_t *token;

		if ((token = process -> evt_tokens[i]) != NULL) {

			(token -> user_evtSubs)--;
			process -> evt_tokens[i] = NULL;
		}
	}

	process -> subscriptions ^=
			process -> subscriptions;
}

static __attribute__((noinline)) void run_proc() {

	process = &processes[proc_exec - 1];

	process -> id = proc_exec;
	process -> subscriptions ^=
			process -> subscriptions;

	uint8_t argc = process -> argc;
	const char **argv = (const char **) process -> argv;

	int (*callee_addr)(int argc,
			const char **argv) = process -> callee_addr;

	callee_addr(argc, argv);

	// Unsubscribe just finished process

	if (process -> subscriptions)
		unsubscribe_proc();

	// Publish process finished event
	enque_evt(EVT_PROCESS_FINISHED, NULL);
}
