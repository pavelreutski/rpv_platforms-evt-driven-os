#include <assert.h>

#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "kernel.h"
#include "kernel_exec.h"
#include "kernel_signal.h"

#include "kernel_sevice.h"
#include "kernel_svchost.h"

#include "private/service.h"

#define USER_EVENTS_POOL               (50)
#define MAX_EVENTS                     (USER_EVENTS_POOL * 2)

#define MAX_QUEUE_EVENTS			   (10)
#define MAX_EVT_SUBSCRIBERS            (10)

#define EVT_PROCESS_FINISHED           (0xff)

struct evt_s {

	uint8_t evtId;
	evt_data_t evtData;

};

struct evt_sub_token_s {

	uint8_t evt_id;

	union {
		uint8_t a8;
		struct {
			uint8_t proc_sub : 1;
			uint8_t krnl_sub : 1;
		} flags;
	} attrib;

	evt_subscriber_t evt_sub;
};

struct proc_s {

	uint16_t id;

	sigset_t flags;
	sigset_t flagMask;

	int argc;
	const char **argv;
	int (*callee_addr)(int argc, const char **argv);
};

typedef struct proc_s process_t;

typedef struct evt_s evt_t;
typedef struct evt_sub_token_s evt_subscription_token_t;

static uint8_t evt_head                                 = 0;
static uint8_t evt_tail                                 = 0;

static uint8_t free_events                              = MAX_QUEUE_EVENTS;
static uint8_t free_evtSubs                             = MAX_EVT_SUBSCRIBERS;

static uint8_t proc_exec                                = 0;

static process_t proc;
static process_t *rt_proc                               = NULL;

static evt_t events[MAX_QUEUE_EVENTS];
static evt_subscription_token_t evt_tokens[MAX_EVT_SUBSCRIBERS];

static sigset_t kernel_flags                            = 0;
static sigset_t kernel_flagMask                         = 0;

static sigset_t* context_flags(void);
static sigset_t* context_flagMask(void);

static void subscribe_evt(uint8_t evtId, evt_subscriber_t sub);

static void run_proc(void);
static void unsubscribe_proc(void);

static void exec_evts(void);
static void enque_evt(uint8_t evtId, evt_data_t* evtData);

extern service_t __svc_table[];
extern service_t __end_svc_table[];

void _kernel_pubEvt(uint8_t id, evt_data_t* data) {

	if ((rt_proc != NULL) &&
			(id < USER_EVENTS_POOL)) { 
				return; // process tries publish kernel events
	}

	enque_evt(id, data);
}

void _kernel_subEvt(uint8_t id, evt_subscriber_t subscriber) {

	if ((rt_proc != NULL) && (id < USER_EVENTS_POOL)) {
		return; // subscribing kernel events by the process is not allowed
	}

	subscribe_evt(id, subscriber);
}

void _kernel_pipeline() {

	_kernel_svcPipeline(); // execute services pipeline
	exec_evts();           // execute events (subscribers)	
	
	if (proc_exec == 0) {  // if a process exec is requested
		return;
	}

	if ((rt_proc != NULL) &&
			((rt_proc -> id) == proc_exec)) return; // when kernel pipeline is invoked from the current process

	run_proc(); // not a current process context -> run process

	proc_exec--;
	rt_proc = NULL;
}

// ------------------------------------------ kenel svc -----------------------------------------------------------

void _kernel_svchost() {
}

void *_kernel_service(void const* svc, char const** svc_name) {
	
	if (svc_name == NULL) {
		return NULL;
	}

	service_t *prev_svc = (service_t *) svc;
	
	service_t *svc_table = (service_t *) &__svc_table;
	service_t *end_svc_table = (service_t *) &__end_svc_table;

	service_t *next_svc = 
		(prev_svc == NULL) ? svc_table : (++prev_svc);

	if (next_svc == end_svc_table || 
			(next_svc < svc_table || next_svc > end_svc_table)) {
		return NULL;
	}

	*svc_name = next_svc -> s_name;

	return next_svc;
}

void _kernel_svcPipeline() {

	for (service_t *service = (service_t *) &__svc_table; 
			service != (service_t *) &__end_svc_table; service++) {
		service -> svc_main();
	}
}

// ------------------------------------------ kexec -------------------------------------------------------------

void _kernel_exec_f(
		int (*exec_callee_addr)(int argc, const char **argv),
		int argc, const char **argv) {

	if (rt_proc != NULL) {
		return; // [DEBUG]: Running more than one user process isn`t allowed yet;
	}

	proc.argc = argc;
	proc.argv = argv;

	proc.callee_addr = exec_callee_addr;

	proc_exec++;
}

// ------------------------------------------ signals -------------------------------------------------------------

int	_kernel_raise(int sgl) {

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

int _kernel_sigwait(const sigset_t *set, int *sgl) {

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

int _kernel_sigpending(sigset_t *set) {

	static_assert(((sigset_t) - 1) > 0, "sigset_t must be unsigned");

	sigset_t pending_set = 0;
	sigset_t *ctx_flags = context_flags();

	_kernel_pipeline();
	pending_set = *ctx_flags;

	*set = pending_set;
	return 0;
}

int _kernel_sigprocmask(int what, const sigset_t *set, sigset_t *oldset) {
	
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
	return (rt_proc != NULL) ?
			&(rt_proc -> flags) : &kernel_flags;
}

static sigset_t* context_flagMask(void) {
	return (rt_proc != NULL) ?
		&(rt_proc -> flagMask) : &kernel_flagMask;
}

static __attribute__((noinline)) void exec_evts(void) {

	if (free_events == MAX_QUEUE_EVENTS) {
		return;
	}

	do {

		// pull next event

		evt_t *recv_evt =
				&events[evt_head % MAX_QUEUE_EVENTS];

		evt_head++;
		free_events++;

		// call subscribers

		size_t i;
		evt_subscription_token_t *token;

		uint8_t recv_evtId = recv_evt -> evtId;

		for (i = 0, token = evt_tokens; 
				i < MAX_EVT_SUBSCRIBERS; i++, token++) {

			if (token -> evt_id == recv_evtId) {
				(token -> evt_sub)(&(recv_evt -> evtData));				
			}
		}

	} while(free_events < MAX_QUEUE_EVENTS);
}

static __attribute__((noinline)) void subscribe_evt(uint8_t evtId, evt_subscriber_t sub) {

	if (free_evtSubs == 0) {
		return;
	}

	size_t i;
	evt_subscription_token_t *token;

	for (i = (evtId % MAX_EVT_SUBSCRIBERS), 
			token = &evt_tokens[i]; token -> evt_sub != NULL; 
				i++, i %= MAX_EVT_SUBSCRIBERS, token = &evt_tokens[i]) { }	

	bool proc_sub = (rt_proc != NULL);

	token -> evt_id = evtId;
	token -> evt_sub = sub;

	token -> attrib.flags.proc_sub = proc_sub;
	token -> attrib.flags.krnl_sub = !proc_sub;

	free_evtSubs--;
}

static __attribute__((noinline)) void enque_evt(uint8_t evtId, evt_data_t* evtData) {

	if (free_events == 0) { 
		return;
	}

	evt_t *evt =
			&events[evt_tail % MAX_QUEUE_EVENTS];

	evt -> evtId = evtId;

	if (evtData != NULL) {
		memcpy(&evt -> evtData, evtData, sizeof(evt_data_t));
	}

	evt_tail++;
	free_events--;
}

static __attribute__((noinline)) void unsubscribe_proc() {

	if (rt_proc == NULL) {
		return;
	}

	size_t i = 0;
	evt_subscription_token_t *token;
	
	for (i = 0, token = evt_tokens; 
			i < MAX_EVT_SUBSCRIBERS; i++, token++) {

		if (token -> attrib.flags.proc_sub) {

			token -> attrib.a8 = 0;
			token -> evt_sub = NULL;
		}
	}
}

static __attribute__((noinline)) void run_proc() {

	rt_proc = &proc;

	rt_proc -> id = proc_exec;

	uint8_t argc = rt_proc -> argc;
	const char **argv = (const char **) rt_proc -> argv;

	int (*callee_addr)(int argc,
			const char **argv) = rt_proc -> callee_addr;

	callee_addr(argc, argv);

	// unsubscribe just finished process
	unsubscribe_proc();

	// publish process finished event
	enque_evt(EVT_PROCESS_FINISHED, NULL);
}
