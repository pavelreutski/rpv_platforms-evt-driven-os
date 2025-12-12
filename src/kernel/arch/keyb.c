#include <stdint.h>
#include <string.h>

#include "print.h"
#include "events.h"

#include "kernel_events_aggregate.h"

#define MAX_KEYS_QUEUE  (10)

typedef enum {

	KB_SCROLL_LED = 0x01,
	KB_NUM_LED    = 0x02,
	KB_CAPS_LED   = 0x04

} kb_led_flags;

typedef enum {

	KB_LSHIFT_HOLDS = 0x01,
	KB_RSHIFT_HOLDS = 0x02,
	KB_CTRL_HOLDS   = 0x04,
	KB_KEY_BREAK    = 0x08

} kb_keys_flags;

const uint8_t Make[] = { 0xF0,  0x1C,	0x32,	0x21,	0x23,	0x24,	0x2B,	0x34,	0x33,	0x43,	0x3B,	0x42,	0x4B,
				0x3A,	0x31,	0x44,	0x4D,	0x15,	0x2D,	0x1B,	0x2C,	0x3C,	0x2A,	0x1D,	0x22,	0x35,	0x1A,	0x45,	0x16,
				0x1E,	0x26,	0x25,	0x2E,	0x36,	0x3D,	0x3E,	0x46,	0x0E,	0x4E,	0x55,	0x5D,	0x66,	0x29,	0x0D,	0x58,
				0x12,	0x14,	0x11,	0x59,	0x5A,	0x76,	0x05,	0x06,	0x04,	0x0C,	0x03,	0x0B,	0x83,	0x0A,	0x01,	0x09,
				0x78,	0x07,	0x7E,	0x54,	0x5B,	0x77,	0x7C,	0x71,	0x70,	0x69,	0x72,	0x7A,	0x6B,	0x73,	0x74,	0x6C,
				0x75,	0x7D,	0x7B,	0x79,	0x4C,	0x52,	0x41,	0x49,	0x4A };

const char * Keys[] = {	"brk",	"a",	"b",	"c",	"d",	"e",	"f",	"g",	"h",	"i",	"j",	"k",	"l",
				"m",	"n",	"o",	"p",	"q",	"r",	"s",	"t",	"u",	"v",	"w",	"x",	"y",	"z",	"0",	"1",
				"2",	"3",	"4", 	"5",	"6",	"7",	"8",	"9",	"`",	"-",	"=",	"\\",  "\b", " ", "\t", "CAPS",
			"LSHFT", "LCTRL", "LALT", "RSHFT", "\n", "ESC",	"F1",	"F2",	"F3",	"F4",	"F5",	"F6",	"F7",	"F8",	"F9",	"F10",
				"F11", "F12", "SCROLL", "[",	"]", 	"NUM",	"KP*",	"KP.",	"KP0",	"KP1",	"KP2",	"KP3",	"KP4",	"KP5",	"KP6",	"KP7",
				"KP8",	"KP9",	"KP-",	"KP+",	";",	"'",	",",	".",	"/",	"A",	"B",	"C",	"D",	"E",	"F",	"G",
				"H",	"I",	"J",    "K",	"L",	"M",	"N",	"O",	"P",	"Q",	"R",	"S",	"T",	"U",	"V",	"W",
				"X",	"Y",	"Z", 	"0",	"1",	"2",	"3",	"4", 	"5",	"6",	"7",	"8",	"9",	"~",	"_",	"+",
			  "|",  "\b", " ", "\t", "CAPS", "LSHFT", "LCTRL", "LALT", "RSHFT", "\n", "ESC", "F1",	"F2",	"F3",	"F4",	"F5",
				"F6",	"F7",	"F8",	"F9",	"F10", "F11", "F12",  "SCROLL", "[",	"]", 	"NUM",	"KP*",	"KP.",	"KP0",	"KP1",	"KP2",
				"KP3",	"KP4",	"KP5",	"KP6",	"KP7", "KP8",  "KP9",	"KP-",	"KP+",	":",	"\"",	"<",	">",	"?" };

static uint8_t KeybState;

static uint8_t keybScanCodeDecode(uint8_t);

/// @brief kernel new keyboard scan callback
/// @param scan_code a next received scan code 
void __attribute__((sysv_abi)) _kernel_onKeybScan(uint8_t scan_code) {

    switch(scan_code) {

        case 0xf0: { // Break - key released
            KeybState |= KB_KEY_BREAK; } break;
        default: {

            if (KeybState & KB_KEY_BREAK) {

                KeybState ^= KB_KEY_BREAK;

                switch(scan_code) {
                    case 0x12: // LShift
                        KeybState ^= KB_LSHIFT_HOLDS; break;
                    case 0x59: // RShift
                        KeybState ^= KB_RSHIFT_HOLDS; break;
                    case 0x14: // LCtrl/RCtrl
                        KeybState ^= KB_CTRL_HOLDS; break;
                    default: break;
                }

                break;
            }

            switch(scan_code) {
                case 0x12: //// LShift
                    KeybState |= KB_LSHIFT_HOLDS; break;
                case 0x59: //// RShift
                    KeybState |= KB_RSHIFT_HOLDS; break;
                case 0x14: //// LCtrl/RCtrl
                    KeybState |= KB_CTRL_HOLDS; break;
                default: {

                    uint8_t index =
                            keybScanCodeDecode(scan_code);

                    if (!(index ^ 0xff)) break;

                    const char* decoded = Keys[index +
                                            85 * ((KeybState &
                                                    (KB_LSHIFT_HOLDS | KB_RSHIFT_HOLDS)) ? 1 : 0)];

                    uint8_t key_code =
                            strlen(decoded) > 1 ? 0: *decoded;

                    console_key_t key = { 
                        code: key_code,
                        scan_code: scan_code
                     };

                    _kernel_publish_evt(
                            EVT_USERCON_KEY,  (evt_data_t *) &key);

                    if (key_code != 0) {
                        print_char(key.code);
                    }
                }
            }
        }
    }
}

static uint8_t keybScanCodeDecode(uint8_t scanCode) {

	uint8_t index = 0;

	while(index < sizeof(Make))
		if (Make[index] == scanCode) return index;
		else index++;

	return 0xff;
}