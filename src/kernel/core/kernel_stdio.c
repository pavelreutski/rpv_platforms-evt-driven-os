#include <string.h>
#include <stdarg.h>

#include "events.h"
#include "console.h"

#include "kernel.h"
#include "kernel_stdio.h"
#include "kernel_context.h"

static console_key_t key_Buffer;

static void onKernel_keyPressed(evt_data_t *evtData);

static void out_console(void **ctx, char const* s);
static void out_strbuffer(void **ctx, char const* s);

static void format_int(void (*out)(void **, char const*), void **ctx, int value);
static void format_core(void (*out)(void **, char const*), void **ctx, char const* fmt, va_list args);

void _kernel_stdio(void) {

	set_con();
	_kernel_subscribe_kernel_evt(
				EVT_USERCON_KEY, onKernel_keyPressed);
}

void _kernel_outLn(void) {
	con_ln();
}

void _kernel_outTab(void) {
	con_tab();
}

void _kernel_outChar(const char code) {
	con_char(code);
}

void _kernel_outString(const char* str) {
	con_string(str);
}

void _kernel_outXY(uint16_t x, uint16_t y) {
	con_xy(x, y);
}

void _kernel_stringFormat(char *s, char const* fmt, ...) {

	va_list args;
	va_start(args, fmt);

	format_core(out_strbuffer, (void **) &s, fmt, args);

	va_end(args);
}

void _kernel_outStringFormat(const char *fmt, ...) {

	va_list args;
	va_start(args, fmt);

	format_core(out_console, (void **) NULL, fmt, args);

	va_end(args);
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

static void out_console(void **ctx, char const* s) {

	(void) ctx;
	con_string(s);
}

static void out_strbuffer(void **ctx, char const* s) {

	const char *str = s;
	char *buffer = *((char **) ctx);

	while (*str) {
		*(buffer++) = *(str++);
	}

	*buffer = '\0';
	*((char **) ctx) = buffer;
}

static void format_int(void (*out)(void **, char const*), void **ctx, int value) {

	char str_buf[12];
	char *s = str_buf + sizeof(str_buf);

	*(--s) = '\0';
	
	unsigned d = 
		(value < 0) ? (~((unsigned) value) + 1) : (unsigned) value;

	do {
		*(--s) = '0' + (d % 10);
		d /= 10;
	} while(d > 0);

	if (value < 0) {
		*(--s) = '-';
	}

	out(ctx, s);
}

static void format_core(void (*out)(void **, char const*), void **ctx, char const* fmt, va_list args) {

	while(*fmt) {

		char fmt_cs[] = {*fmt, '\0' };

		if (*fmt != '%') {

			out(ctx, fmt_cs);
			fmt++;

			continue;
		}

		const char fmt_c = *(fmt + 1);
		
		fmt += 2;
		*fmt_cs = fmt_c;

		switch(fmt_c) {

		case '\0':  {
			return;
		} break;

		case 's':  { 
			char const* s = va_arg(args, char const*);			
			out(ctx, s);
		} break;

		case 'd': {

			const int value = va_arg(args, const int);
			format_int(out, ctx, value);
			
		} break;

		default: { out(ctx, fmt_cs); } break;

		}
	}
}
