#include <string.h>

#include "events.h"
#include "console.h"

#include "kernel.h"
#include "kernel_stdio.h"
#include "kernel_context.h"

static console_key_t key_Buffer;

static void onKernel_keyPressed(evt_data_t *evtData);

void _kernel_stdio(void) {

	set_console(CON_VGA);
	_kernel_subscribe_kernel_evt(
				EVT_USERCON_KEY, onKernel_keyPressed);
}

void _kernel_outLn(void) {
	out_Ln();
}

void _kernel_outTab(void) {
	out_TAB();
}

void _kernel_outInt(const int num) {
	out_Int(num);
}

void _kernel_outFloat(const float num) {
	out_Float(num);
}

void _kernel_outByte(const uint8_t byte) {
	out_Byte(byte);
}

void _kernel_outChar(const char code) {
	out_Char(code);
}

void _kernel_outString(const char* str) {
	out_String(str);
}

void _kernel_gotoXY(uint16_t x, uint16_t y) {
	out_XY(x, y);
}

bool _kernel_tryGetKey(console_key_t *key) {

	memset(key, 0, sizeof(console_key_t));

	uint16_t *flags;
	_kernel_context_flags(KEYB_INPUT_PENDING, &flags);

	_kernel_pipeline();

	bool isConKeyEvent =
			(*flags & KEYB_INPUT_PENDING) ^
						KEYB_INPUT_PENDING;

	if (isConKeyEvent)
		memcpy(key, &key_Buffer, sizeof(console_key_t));

	return isConKeyEvent;
}

void _kernel_getKey(console_key_t *key) {

	memset(key, 0, sizeof(console_key_t));

	uint16_t *flags;
	_kernel_context_flags(
			KEYB_INPUT_PENDING, &flags);

	while(*flags &
			KEYB_INPUT_PENDING)
		_kernel_pipeline();

	memcpy(key, &key_Buffer, sizeof(console_key_t));
}

static void onKernel_keyPressed(evt_data_t *evtData) {
	
    memcpy(&key_Buffer, (console_key_t *) evtData, sizeof(console_key_t));
	_kernel_clear_context_flags(KEYB_INPUT_PENDING);
}
