#pragma once

/// @brief defines user events in kernel (extendable by last member)
typedef enum : uint8_t {

	EVT_USERCON_KEY = 50,	
	EVT_SYS_USER_EVENTS

} user_events_t;

/// @brief define kernel events (not accessible for user subscriptions)
typedef enum : uint8_t {

	EVT_ONESHOT_TICK,
	EVT_PERIODIC_TICK,
	EVT_KERNEL_EVENTS

} kernel_events_t;

typedef struct {

	char code;   // ASCII key code
	uint8_t scan_code;

} console_key_t;

typedef union {

	console_key_t con_key;

} evt_data_t;

typedef void (*evt_subscriber)(evt_data_t* evtData);