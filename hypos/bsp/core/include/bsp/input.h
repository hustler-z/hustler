/**
 * Hustler's Project
 *
 * File:  input.h
 * Date:  2024/06/07
 * Usage:
 */

#ifndef _BSP_INPUT_H
#define _BSP_INPUT_H
// --------------------------------------------------------------
#include <bsp/device.h>

#define KEY_RESERVED		0
#define KEY_ESC			    1
#define KEY_1			    2
#define KEY_2			    3
#define KEY_3			    4
#define KEY_4			    5
#define KEY_5			    6
#define KEY_6			    7
#define KEY_7			    8
#define KEY_8			    9
#define KEY_9			    10
#define KEY_0			    11
#define KEY_MINUS		    12
#define KEY_EQUAL		    13
#define KEY_BACKSPACE		14
#define KEY_TAB			    15
#define KEY_Q			    16
#define KEY_W			    17
#define KEY_E			    18
#define KEY_R			    19
#define KEY_T			    20
#define KEY_Y			    21
#define KEY_U			    22
#define KEY_I			    23
#define KEY_O			    24
#define KEY_P			    25
#define KEY_LEFTBRACE		26
#define KEY_RIGHTBRACE		27
#define KEY_ENTER		    28
#define KEY_LEFTCTRL		29
#define KEY_A			    30
#define KEY_S			    31
#define KEY_D			    32
#define KEY_F			    33
#define KEY_G			    34
#define KEY_H			    35
#define KEY_J			    36
#define KEY_K			    37
#define KEY_L			    38
#define KEY_SEMICOLON		39
#define KEY_APOSTROPHE		40
#define KEY_GRAVE		    41
#define KEY_LEFTSHIFT		42
#define KEY_BACKSLASH		43
#define KEY_Z			    44
#define KEY_X			    45
#define KEY_C			    46
#define KEY_V			    47
#define KEY_B			    48
#define KEY_N			    49
#define KEY_M			    50
#define KEY_COMMA		    51
#define KEY_DOT			    52
#define KEY_SLASH		    53
#define KEY_RIGHTSHIFT		54
#define KEY_KPASTERISK		55
#define KEY_LEFTALT		    56
#define KEY_SPACE		    57
#define KEY_CAPSLOCK		58
#define KEY_F1			    59
#define KEY_F2			    60
#define KEY_F3			    61
#define KEY_F4			    62
#define KEY_F5			    63
#define KEY_F6			    64
#define KEY_F7			    65
#define KEY_F8			    66
#define KEY_F9			    67
#define KEY_F10			    68
#define KEY_NUMLOCK		    69
#define KEY_SCROLLLOCK		70
#define KEY_KP7			    71
#define KEY_KP8			    72
#define KEY_KP9			    73
#define KEY_KPMINUS		    74
#define KEY_KP4			    75
#define KEY_KP5			    76
#define KEY_KP6			    77
#define KEY_KPPLUS		    78
#define KEY_KP1			    79
#define KEY_KP2			    80
#define KEY_KP3			    81
#define KEY_KP0			    82
#define KEY_KPDOT		    83

#define KEY_ZENKAKUHANKAKU	85
#define KEY_102ND		    86
#define KEY_F11			    87
#define KEY_F12			    88
#define KEY_RO			    89
#define KEY_KATAKANA		90
#define KEY_HIRAGANA		91
#define KEY_HENKAN		    92
#define KEY_KATAKANAHIRAGANA	93
#define KEY_MUHENKAN		94
#define KEY_KPJPCOMMA		95
#define KEY_KPENTER		    96
#define KEY_RIGHTCTRL		97
#define KEY_KPSLASH		    98
#define KEY_SYSRQ		    99
#define KEY_RIGHTALT		100
#define KEY_LINEFEED		101
#define KEY_HOME		    102
#define KEY_UP			    103
#define KEY_PAGEUP		    104
#define KEY_LEFT		    105
#define KEY_RIGHT		    106
#define KEY_END			    107
#define KEY_DOWN		    108
#define KEY_PAGEDOWN		109
#define KEY_INSERT		    110
#define KEY_DELETE		    111
#define KEY_MACRO		    112
#define KEY_MUTE		    113
#define KEY_VOLUMEDOWN		114
#define KEY_VOLUMEUP		115
#define KEY_POWER		    116	/* SC System Power Down */
#define KEY_KPEQUAL		    117
#define KEY_KPPLUSMINUS		118
#define KEY_PAUSE		    119
#define KEY_SCALE		    120	/* AL Compiz Scale (Expose) */

#define KEY_KPCOMMA		    121
#define KEY_HANGEUL		    122
#define KEY_HANGUEL		    KEY_HANGEUL
#define KEY_HANJA		    123
#define KEY_YEN			    124
#define KEY_LEFTMETA		125
#define KEY_RIGHTMETA		126
#define KEY_COMPOSE		    127
#define KEY_FN			    0x1d0
// --------------------------------------------------------------

enum {
	INPUT_MAX_MODIFIERS	= 4,
	INPUT_BUFFER_LEN	= 16,
};

enum {
	/* Keyboard LEDs */
	INPUT_LED_SCROLL	= 1 << 0,
	INPUT_LED_NUM		= 1 << 1,
	INPUT_LED_CAPS		= 1 << 2,
};

struct input_key_xlate {
	/* keycode of the modifiers which select this table, -1 if none */
	int left_keycode;
	int right_keycode;
	const unsigned char *xlate;	    /* keycode to ASCII table */
	int num_entries;	            /* number of entries in this table */
};

struct input_config {
	struct hypos_device *dev;
	unsigned char fifo[INPUT_BUFFER_LEN];
	int fifo_in, fifo_out;

	/* Which modifiers are active (1 bit for each MOD_... value) */
	unsigned char modifiers;
	unsigned char flags;		    /* active state keys (FLAGS_...) */
	unsigned char leds;		        /* active LEDs (INPUT_LED_...) */
	unsigned char leds_changed;	    /* LEDs that just changed */
	unsigned char num_tables;	    /* number of modifier tables */
	int prev_keycodes[INPUT_BUFFER_LEN];	/* keys held last time */
	int num_prev_keycodes;	        /* number of prev keys */
	struct input_key_xlate table[INPUT_MAX_MODIFIERS];

	int (*read_keys)(struct input_config *config);
	bool allow_repeats;		        /* Don't filter out repeats */
	unsigned int next_repeat_ms;	/* Next time we repeat a key */
	unsigned int repeat_delay_ms;	/* Time before autorepeat starts */
	unsigned int repeat_rate_ms;	/* Autorepeat rate in ms */
};

struct stdio_dev;

int input_send_keycodes(struct input_config *config, int keycode[], int count);
int input_add_keycode(struct input_config *config, int new_keycode,
          bool release);
int input_add_table(struct input_config *config, int left_keycode,
        int right_keycode, const unsigned char *xlate, int num_entries);
int input_tstc(struct input_config *config);
int input_getc(struct input_config *config);
int input_stdio_register(struct stdio_dev *dev);
void input_set_delays(struct input_config *config, int repeat_delay_ms,
       int repeat_rate_ms);
void input_allow_repeats(struct input_config *config, bool allow_repeats);
int input_leds_changed(struct input_config *config);
int input_add_tables(struct input_config *config, bool german);
int input_init(struct input_config *config, int leds);

// --------------------------------------------------------------
#endif /* _BSP_INPUT_H */
