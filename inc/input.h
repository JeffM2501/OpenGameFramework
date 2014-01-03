// input.cpp
// base SDL input class

#ifndef _INPUT_H_
#define _INPUT_H_

//common defines for the standard devices
#define _KEYBOARD "KEYBOARD"
#define _MOUSE	  "MOUSE"
#define _JOYSTICK "JOYSTICK1"


// now the nasty
// this is just a remap of the SDL keys
// but we need to do it, just in case somone DOSN"T use SDL.

typedef enum {
	/* The keyboard syms have been cleverly chosen to map to ASCII */
	KEY_UNKNOWN		= 0,
	KEY_FIRST		= 0,
	KEY_BACKSPACE		= 8,
	KEY_TAB		= 9,
	KEY_CLEAR		= 12,
	KEY_RETURN		= 13,
	KEY_PAUSE		= 19,
	KEY_ESCAPE		= 27,
	KEY_SPACE		= 32,
	KEY_EXCLAIM		= 33,
	KEY_QUOTEDBL		= 34,
	KEY_HASH		= 35,
	KEY_DOLLAR		= 36,
	KEY_AMPERSAND		= 38,
	KEY_QUOTE		= 39,
	KEY_LEFTPAREN		= 40,
	KEY_RIGHTPAREN		= 41,
	KEY_ASTERISK		= 42,
	KEY_PLUS		= 43,
	KEY_COMMA		= 44,
	KEY_MINUS		= 45,
	KEY_PERIOD		= 46,
	KEY_SLASH		= 47,
	KEY_0			= 48,
	KEY_1			= 49,
	KEY_2			= 50,
	KEY_3			= 51,
	KEY_4			= 52,
	KEY_5			= 53,
	KEY_6			= 54,
	KEY_7			= 55,
	KEY_8			= 56,
	KEY_9			= 57,
	KEY_COLON		= 58,
	KEY_SEMICOLON		= 59,
	KEY_LESS		= 60,
	KEY_EQUALS		= 61,
	KEY_GREATER		= 62,
	KEY_QUESTION		= 63,
	KEY_AT			= 64,
	/* 
	Skip uppercase letters
	*/
	KEY_LEFTBRACKET	= 91,
	KEY_BACKSLASH		= 92,
	KEY_RIGHTBRACKET	= 93,
	KEY_CARET		= 94,
	KEY_UNDERSCORE		= 95,
	KEY_BACKQUOTE		= 96,
	KEY_a			= 97,
	KEY_b			= 98,
	KEY_c			= 99,
	KEY_d			= 100,
	KEY_e			= 101,
	KEY_f			= 102,
	KEY_g			= 103,
	KEY_h			= 104,
	KEY_i			= 105,
	KEY_j			= 106,
	KEY_k			= 107,
	KEY_l			= 108,
	KEY_m			= 109,
	KEY_n			= 110,
	KEY_o			= 111,
	KEY_p			= 112,
	KEY_q			= 113,
	KEY_r			= 114,
	KEY_s			= 115,
	KEY_t			= 116,
	KEY_u			= 117,
	KEY_v			= 118,
	KEY_w			= 119,
	KEY_x			= 120,
	KEY_y			= 121,
	KEY_z			= 122,
	KEY_DELETE		= 127,
	/* End of ASCII mapped keysyms */

	/* International keyboard syms */
	KEY_WORLD_0		= 160,		/* 0xA0 */
	KEY_WORLD_1		= 161,
	KEY_WORLD_2		= 162,
	KEY_WORLD_3		= 163,
	KEY_WORLD_4		= 164,
	KEY_WORLD_5		= 165,
	KEY_WORLD_6		= 166,
	KEY_WORLD_7		= 167,
	KEY_WORLD_8		= 168,
	KEY_WORLD_9		= 169,
	KEY_WORLD_10		= 170,
	KEY_WORLD_11		= 171,
	KEY_WORLD_12		= 172,
	KEY_WORLD_13		= 173,
	KEY_WORLD_14		= 174,
	KEY_WORLD_15		= 175,
	KEY_WORLD_16		= 176,
	KEY_WORLD_17		= 177,
	KEY_WORLD_18		= 178,
	KEY_WORLD_19		= 179,
	KEY_WORLD_20		= 180,
	KEY_WORLD_21		= 181,
	KEY_WORLD_22		= 182,
	KEY_WORLD_23		= 183,
	KEY_WORLD_24		= 184,
	KEY_WORLD_25		= 185,
	KEY_WORLD_26		= 186,
	KEY_WORLD_27		= 187,
	KEY_WORLD_28		= 188,
	KEY_WORLD_29		= 189,
	KEY_WORLD_30		= 190,
	KEY_WORLD_31		= 191,
	KEY_WORLD_32		= 192,
	KEY_WORLD_33		= 193,
	KEY_WORLD_34		= 194,
	KEY_WORLD_35		= 195,
	KEY_WORLD_36		= 196,
	KEY_WORLD_37		= 197,
	KEY_WORLD_38		= 198,
	KEY_WORLD_39		= 199,
	KEY_WORLD_40		= 200,
	KEY_WORLD_41		= 201,
	KEY_WORLD_42		= 202,
	KEY_WORLD_43		= 203,
	KEY_WORLD_44		= 204,
	KEY_WORLD_45		= 205,
	KEY_WORLD_46		= 206,
	KEY_WORLD_47		= 207,
	KEY_WORLD_48		= 208,
	KEY_WORLD_49		= 209,
	KEY_WORLD_50		= 210,
	KEY_WORLD_51		= 211,
	KEY_WORLD_52		= 212,
	KEY_WORLD_53		= 213,
	KEY_WORLD_54		= 214,
	KEY_WORLD_55		= 215,
	KEY_WORLD_56		= 216,
	KEY_WORLD_57		= 217,
	KEY_WORLD_58		= 218,
	KEY_WORLD_59		= 219,
	KEY_WORLD_60		= 220,
	KEY_WORLD_61		= 221,
	KEY_WORLD_62		= 222,
	KEY_WORLD_63		= 223,
	KEY_WORLD_64		= 224,
	KEY_WORLD_65		= 225,
	KEY_WORLD_66		= 226,
	KEY_WORLD_67		= 227,
	KEY_WORLD_68		= 228,
	KEY_WORLD_69		= 229,
	KEY_WORLD_70		= 230,
	KEY_WORLD_71		= 231,
	KEY_WORLD_72		= 232,
	KEY_WORLD_73		= 233,
	KEY_WORLD_74		= 234,
	KEY_WORLD_75		= 235,
	KEY_WORLD_76		= 236,
	KEY_WORLD_77		= 237,
	KEY_WORLD_78		= 238,
	KEY_WORLD_79		= 239,
	KEY_WORLD_80		= 240,
	KEY_WORLD_81		= 241,
	KEY_WORLD_82		= 242,
	KEY_WORLD_83		= 243,
	KEY_WORLD_84		= 244,
	KEY_WORLD_85		= 245,
	KEY_WORLD_86		= 246,
	KEY_WORLD_87		= 247,
	KEY_WORLD_88		= 248,
	KEY_WORLD_89		= 249,
	KEY_WORLD_90		= 250,
	KEY_WORLD_91		= 251,
	KEY_WORLD_92		= 252,
	KEY_WORLD_93		= 253,
	KEY_WORLD_94		= 254,
	KEY_WORLD_95		= 255,		/* 0xFF */

	/* Numeric keypad */
	KEY_KP0		= 256,
	KEY_KP1		= 257,
	KEY_KP2		= 258,
	KEY_KP3		= 259,
	KEY_KP4		= 260,
	KEY_KP5		= 261,
	KEY_KP6		= 262,
	KEY_KP7		= 263,
	KEY_KP8		= 264,
	KEY_KP9		= 265,
	KEY_KP_PERIOD		= 266,
	KEY_KP_DIVIDE		= 267,
	KEY_KP_MULTIPLY	= 268,
	KEY_KP_MINUS		= 269,
	KEY_KP_PLUS		= 270,
	KEY_KP_ENTER		= 271,
	KEY_KP_EQUALS		= 272,

	/* Arrows + Home/End pad */
	KEY_UP			= 273,
	KEY_DOWN		= 274,
	KEY_RIGHT		= 275,
	KEY_LEFT		= 276,
	KEY_INSERT		= 277,
	KEY_HOME		= 278,
	KEY_END		= 279,
	KEY_PAGEUP		= 280,
	KEY_PAGEDOWN		= 281,

	/* Function keys */
	KEY_F1			= 282,
	KEY_F2			= 283,
	KEY_F3			= 284,
	KEY_F4			= 285,
	KEY_F5			= 286,
	KEY_F6			= 287,
	KEY_F7			= 288,
	KEY_F8			= 289,
	KEY_F9			= 290,
	KEY_F10		= 291,
	KEY_F11		= 292,
	KEY_F12		= 293,
	KEY_F13		= 294,
	KEY_F14		= 295,
	KEY_F15		= 296,

	/* Key state modifier keys */
	KEY_NUMLOCK		= 300,
	KEY_CAPSLOCK		= 301,
	KEY_SCROLLOCK		= 302,
	KEY_RSHIFT		= 303,
	KEY_LSHIFT		= 304,
	KEY_RCTRL		= 305,
	KEY_LCTRL		= 306,
	KEY_RALT		= 307,
	KEY_LALT		= 308,
	KEY_RMETA		= 309,
	KEY_LMETA		= 310,
	KEY_LSUPER		= 311,		/* Left "Windows" key */
	KEY_RSUPER		= 312,		/* Right "Windows" key */
	KEY_MODE		= 313,		/* "Alt Gr" key */
	KEY_COMPOSE		= 314,		/* Multi-key compose key */

	/* Miscellaneous function keys */
	KEY_HELP		= 315,
	KEY_PRINT		= 316,
	KEY_SYSREQ		= 317,
	KEY_BREAK		= 318,
	KEY_MENU		= 319,
	KEY_POWER		= 320,		/* Power Macintosh power key */
	KEY_EURO		= 321,		/* Some european keyboards */
	KEY_UNDO		= 322,		/* Atari keyboard has Undo */

	/* Add any other keys here */

	KEY_LAST
} KEYTypes;

/* Enumeration of valid key mods (possibly OR'd together) */
typedef enum {
	KEYMOD_NONE  = 0x0000,
	KEYMOD_LSHIFT= 0x0001,
	KEYMOD_RSHIFT= 0x0002,
	KEYMOD_LCTRL = 0x0040,
	KEYMOD_RCTRL = 0x0080,
	KEYMOD_LALT  = 0x0100,
	KEYMOD_RALT  = 0x0200,
	KEYMOD_LMETA = 0x0400,
	KEYMOD_RMETA = 0x0800,
	KEYMOD_NUM   = 0x1000,
	KEYMOD_CAPS  = 0x2000,
	KEYMOD_MODE  = 0x4000,
	KEYMOD_RESERVED = 0x8000
} KEYMod;

#define KEYMOD_CTRL	(KEYMOD_LCTRL|KEYMOD_RCTRL)
#define KEYMOD_SHIFT	(KEYMOD_LSHIFT|KEYMOD_RSHIFT)
#define KEYMOD_ALT	(KEYMOD_LALT|KEYMOD_RALT)
#define KEYMOD_META	(KEYMOD_LMETA|KEYMOD_RMETA)

// now the code

class CInputDevice
{
public:
	CInputDevice();
	virtual ~CInputDevice();

	virtual bool Init ( void );
	virtual bool Release ( void );

	virtual bool Update ( void );

	virtual int Bind ( const char *command, const char *item );
	virtual float GetValue ( const char *command );
	virtual float GetValue ( int command );

	// used to send in init data or whatnot ( stick number, etc );
	virtual bool SetItem ( const char *key, int item );

	virtual bool Active ( void );

	virtual const char* GetName ( void );

	virtual const char* GetNextAction ( void );
};

#include "singleton.h"

class CInputMananger: public Singleton<CInputMananger>
{
protected:
	friend class Singleton<CInputMananger>;
	CInputMananger();
	~CInputMananger();

public:
	void Init ( void );
	void Release ( void );

	void ReadConfig ( const char *file );

	int Bind ( const char *command, const char *device, const char *item );
	int GetCommandID ( const char *command );
	void Unbind ( const char *command );
	void Unbind ( int command );

	void Update ( void );

	void SetPollCommand ( const char* command );
	void SetPollCommand ( int command );
	float PollState ( const char *command );
	float PollState ( int command );

	int GetEvents ( void );
	bool GetNextEvent ( void );
	const char *GetEventName ( void );
	int GetEventID ( void );
	float GetEventValue ( void );

	int GetNumDevices ( void );
	CInputDevice* GetDevice ( const char *device );
	CInputDevice* GetDevice ( int device );
	
	int RegisterDevice ( const char * name, CInputDevice *device );
	
	// global utils that deal with input
	void HideOSCursor ( bool hide );

	// set the mouse to relitive mode
	void SetMouseRelitiveMode ( bool mode = true );

	// interface style mouse and keyboard funcs
	void GetKeysState ( unsigned char ** keys );
	bool GetKeyState ( KEYTypes key );
	void GetMousePos ( int &iX, int &iY, bool bInc );
	void GetMouseButtons ( bool &b1, bool &b2, bool &b3);

	// call this right before you call update if you want text
	const char* GetEnteredText( void );

	// called by event handalers
	int Event(unsigned char event, void* message);

private:
	bool InitStandardDevices ( void );

	bool relitiveMouseMode;
	struct trInputInfo;
	trInputInfo	*info;
};

KEYTypes GetCurrentKey ( void ); 


//----------------- key names ----------------------//
	/*
	UNKNOWN				= KEY_UNKNOWN
	BACKSPACE			= KEY_BACKSPACE
	TAB						= KEY_TAB
	CLEAR					= KEY_CLEAR
	RETURN				= KEY_RETURN
	PAUSE					= KEY_PAUSE
	ESCAPE				= KEY_ESCAPE
	SPACE					= KEY_SPACE
	EXCLAIM				= KEY_EXCLAIM
	!							= KEY_EXCLAIM
	QUOTEDBL			= KEY_QUOTEDBL
	"							= KEY_QUOTEDBL
	HASH					= KEY_HASH
	#							= KEY_HASH
	DOLLAR				= KEY_DOLLAR
	$							= KEY_DOLLAR
	AMPERSAND			= KEY_AMPERSAND
	&							= KEY_AMPERSAND
	QUOTE					= KEY_QUOTE
	'							= KEY_QUOTE
	LEFTPAREN			= KEY_LEFTPAREN
	(							= KEY_LEFTPAREN
	RIGHTPAREN		= KEY_RIGHTPAREN
	)							= KEY_RIGHTPAREN
	ASTERISK			= KEY_ASTERISK
	*							= KEY_ASTERISK
	PLUS					= KEY_PLUS
	+							= KEY_PLUS
	COMMA					= KEY_COMMA
	,							= KEY_COMMA
	MINUS					= KEY_MINUS
	-							= KEY_MINUS
	PERIOD				= KEY_PERIOD
	.							= KEY_PERIOD
	SLASH					= KEY_SLASH
	/							= KEY_SLASH
	COLON					= KEY_COLON
	:							= KEY_COLON
	SEMICOLON			= KEY_SEMICOLON
	;							= KEY_SEMICOLON
	LESS					= KEY_LESS
	<							= KEY_LESS
	EQUALS				= KEY_EQUALS
	=							= KEY_EQUALS
	GREATER				= KEY_GREATER
	>							= KEY_GREATER
	QUESTION			= KEY_QUESTION
	?							= KEY_QUESTION
	AT						= KEY_AT
	@							= KEY_AT
	LEFTBRACKET		= KEY_LEFTBRACKET
	[							= KEY_LEFTBRACKET
	BACKSLASH			= KEY_BACKSLASH
	\\						= KEY_BACKSLASH
	RIGHTBRACKET  = KEY_RIGHTBRACKET
	]							= KEY_RIGHTBRACKET
	CARET					= KEY_CARET
	^							= KEY_CARET
	UNDERSCORE		= KEY_UNDERSCORE
	_							= KEY_UNDERSCORE
	BACKQUOTE			= KEY_BACKQUOTE
	BACKTICK			= KEY_BACKQUOTE
	`							= KEY_BACKQUOTE
	DELETE				= KEY_DELETE

	// numbers
	0 						 = KEY_0
	1 						 = KEY_1
	2 						 = KEY_2
	3 						 = KEY_3
	4 						 = KEY_4
	5 						 = KEY_5
	6 						 = KEY_6
	7 						 = KEY_7
	8 						 = KEY_8
	9 						 = KEY_9

	// leters
	A  							= KEY_a
	B  							= KEY_b
	C  							= KEY_c
	D  							= KEY_d
	E  							= KEY_e
	F  							= KEY_f
	G  							= KEY_g
	H  							= KEY_h
	I  							= KEY_i
	J  							= KEY_j
	K  							= KEY_k
	L  							= KEY_l
	M  							= KEY_m
	N  							= KEY_n
	O  							= KEY_o
	P  							= KEY_p
	Q  							= KEY_q
	R  							= KEY_r
	S  							= KEY_s
	T  							= KEY_t
	U  							= KEY_u
	V  							= KEY_v
	W  							= KEY_w
	X  							= KEY_x
	Y  							= KEY_y
	Z  							= KEY_z

	// Numeric keypad
	KP0 						 = KEY_KP0
	KP1 						 = KEY_KP1
	KP2 						 = KEY_KP2
	KP3 						 = KEY_KP3
	KP4 						 = KEY_KP4
	KP5 						 = KEY_KP5
	KP6 						 = KEY_KP6
	KP7 						 = KEY_KP7
	KP8 						 = KEY_KP8
	KP9 						 = KEY_KP9
	KEYPAD0  					= KEY_KP0
	KEYPAD1  					= KEY_KP1
	KEYPAD2  					= KEY_KP2
	KEYPAD3  					= KEY_KP3
	KEYPAD4  					= KEY_KP4
	KEYPAD5  					= KEY_KP5
	KEYPAD6  					= KEY_KP6
	KEYPAD7  					= KEY_KP7
	KEYPAD8  					= KEY_KP8
	KEYPAD9  					= KEY_KP9
	KEYPAD 0  				= KEY_KP0
	KEYPAD 1  				= KEY_KP1
	KEYPAD 2  				= KEY_KP2
	KEYPAD 3  				= KEY_KP3
	KEYPAD 4  				= KEY_KP4
	KEYPAD 5  				= KEY_KP5
	KEYPAD 6  				= KEY_KP6
	KEYPAD 7  				= KEY_KP7
	KEYPAD 8  				= KEY_KP8
	KEYPAD 9  				= KEY_KP9
	KP_PERIOD					= KEY_KP_PERIOD
	KP_DIVIDE					= KEY_KP_DIVIDE
	KP_MULTIPLY				= KEY_KP_MULTIPLY
	KP_MINUS					= KEY_KP_MINUS
	KP_PLUS						= KEY_KP_PLUS
	KP_ENTER					= KEY_KP_ENTER
	KP_EQUALS					= KEY_KP_EQUALS
	KEYPAD PERIOD			= KEY_KP_PERIOD
	KEYPAD .					= KEY_KP_PERIOD
	KEYPAD DIVIDE			= KEY_KP_DIVIDE
	KEYPAD /					= KEY_KP_DIVIDE
	KEYPAD MULTIPLY		= KEY_KP_MULTIPLY
	KEYPAD *					= KEY_KP_MULTIPLY
	KEYPAD MINUS			= KEY_KP_MINUS
	KEYPAD -					= KEY_KP_MINUS
	KEYPAD PLUS				= KEY_KP_PLUS
	KEYPAD +					= KEY_KP_PLUS
	KEYPAD ENTER			= KEY_KP_ENTER
	KEYPAD EQUALS			= KEY_KP_EQUALS
	KEYPAD =					= KEY_KP_EQUALS

	//Arrows + Home/End pad 
	UP						= KEY_UP
	DOWN					= KEY_DOWN
	RIGHT					= KEY_RIGHT
	LEFT					= KEY_LEFT
	INSERT				= KEY_INSERT
	HOME					= KEY_HOME
	END						= KEY_END
	PAGEUP				= KEY_PAGEUP
	PAGEDOWN			= KEY_PAGEDOWN
	PAGE UP				= KEY_PAGEUP
	PAGE DOWN			= KEY_PAGEDOWN

	// Function keys
	F1  					= KEY_F1
	F2  					= KEY_F2
	F3  					= KEY_F3
	F4  					= KEY_F4
	F5  					= KEY_F5
	F6  					= KEY_F6
	F7  					= KEY_F7
	F8  					= KEY_F8
	F9  					= KEY_F9
	F10  					= KEY_F10
	F11  					= KEY_F11
	F12  					= KEY_F12
	F13  					= KEY_F13
	F14  					= KEY_F14
	F15  					= KEY_F15

	// Key state modifier keys
	NUMLOCK				= KEY_NUMLOCK
	CAPSLOCK			= KEY_CAPSLOCK
	SCROLLOCK			= KEY_SCROLLOCK
	RSHIFT				= KEY_RSHIFT
	LSHIFT				= KEY_LSHIFT
	RCTRL					= KEY_RCTRL
	LCTRL					= KEY_LCTRL
	RALT					= KEY_RALT
	LALT					= KEY_LALT
	RMETA					= KEY_RMETA
	LMETA					= KEY_LMETA
	LSUPER				= KEY_LSUPER		//Left "Windows" key
	RSUPER				= KEY_RSUPER		//Right "Windows" key
	MODE					= KEY_MODE			//"Alt Gr" key
	COMPOSE				= KEY_COMPOSE		//Multi-key compose key

	// Miscellaneous function keys
	HELP					= KEY_HELP
	PRINT					= KEY_PRINT
	SYSREQ				= KEY_SYSREQ
	BREAK					= KEY_BREAK
	MENU					= KEY_MENU
	POWER					= KEY_POWER			//Power Macintosh power key
	EURO					= KEY_EURO			//Some european keyboards
	UNDO					= KEY_UNDO;			//Atari keyboard has Undo
*/
#endif //_INPUT_H_
