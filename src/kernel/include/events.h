#pragma once

typedef struct {

	char code;
	uint8_t scan_code;

} keyb_key_t;

typedef union {

	keyb_key_t key;

} evt_data_t;

typedef void (*evt_subscriber)(evt_data_t* evtData);