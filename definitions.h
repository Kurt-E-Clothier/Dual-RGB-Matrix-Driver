/***************************************************************************
* 
* File              : definitions.h
*
* Author			: Kurt E. Clothier
* Date				: July 17, 2015
* Modified			: July 30, 2015
*
* Description       : Header file for matrixRGB
*
* Compiler			: AVR-GCC
* Hardware			: ATMEGA328p; Ext 16MHz Osc
* Fuse Settings		: E:07, H:D9, L:E7
*
* More Information	: http://www.projectsbykec.com/
*
****************************************************************************/

/***************************************************************************
	Constants & Status Flags
****************************************************************************/
// stat_flag
//#define stat_flag		GPIOR0
#define SET_LEDS		0x01
#define UPDATE_LEDS		0x02
#define TWI_DONE		0x08
#define PASSIVE_MODE	0x10
#define RESET_CHASE		0x20
#define INCREMENT_COLOR	0x40
#define DECREMENT_COLOR	0x80

#define FLAG_IS_SET(FLAG)	(stat_flags & (FLAG))
#define FLAG_IS_CLEAR(FLAG)	!FLAG_IS_SET(FLAG)
#define SET_FLAG(FLAG)		stat_flags |= (FLAG)
#define CLEAR_FLAG(FLAG)	stat_flags &= ~(FLAG)

#define TWI_IS_DONE			(stat_flag & TWI_DONE)
#define TWI_NOT_DONE		!TWI_IS_DONE

#define WDT_MAX		70

// Color Control
#define COLORS		3
#define BLUE_LEDS	2
#define GREEN_LEDS	1
#define RED_LEDS	0

#define MATRICES	2
#define QUADS		4
#define COLUMNS		8
#define LEDS		8
#define ROWS		LEDS

// Quadrant Flags for 2 matrices
#define QUAD00		0x01
#define QUAD01		0x02
#define QUAD02		0x04
#define QUAD03		0x08
#define QUAD10		0x10
#define QUAD11		0x20
#define QUAD12		0x40
#define QUAD13		0x80

/***************************************************************************
	TWI Macros
****************************************************************************/
#define TWI_SLAVE_ADDRESS	0x47
#define TWI_MSG_SIZE		2

/***************************************************************************
	Chase Sequences - 255 possible
****************************************************************************/
// Full Matrix
#define ALL_CONSTANT		0x00
#define LOOP_ALL			0x01
#define ALL_WHITE			0x02
#define ALL_COLORS			0x03

// Row Sequences
#define BINARY_ROWS			0x10

// Column Sequences
#define BINARY_COLS			0x20
#define COLOR_SLIDE_COLS	0x21

// Quadrant Sequences
#define QUAD_WHEEL			0x30
#define QUAD_WHEEL2			0x31
#define QUAD_WHEEL3			0x32
#define LOOP_QUAD			0x40

// Miscellaneous
#define SMILEY				0xE0
#define SMILEY_EYE_DELAY	150		// * 10 ms

// Tests
#define TEST_CORNERS		0xF0
#define ALL_OFF				0xFF

/***************************************************************************
	Firmware Version Constants
****************************************************************************/
#ifndef FW_VERSION

#error "FW_VERSION must be defined in PROJECT_main-vX.X.c"

//---------------------
// FW_VERISON 2.x
//---------------------
//#elif FW_VERSION == 0x2x

#else

#define MAX_LED_RESOLUTION 10	// Full [0, 255] scale mapped to [0, 10] by steps of 25

#endif

/***************************************************************************
	Hardware Definitions and Associated Macros
****************************************************************************/
#ifndef HW_VERSION

#error "HW_VERSION must be defined in PROJECT_main-vX.X.c"

//---------------------
// HW_VERISON 2.0
//---------------------
#elif HW_VERSION == 0x20

// SCK
#define SCK_PORT		PORTB
#define SCK				_BV(PB5)

#define SET_SCK_HI()	SCK_PORT |= SCK
#define SET_SCK_LO()	SCK_PORT &= ~SCK

// Latch
#define LATCH_PORT		PORTB
#define LATCH			_BV(PB4)

#define LATCH_LEDS()	do { LATCH_PORT |= LATCH; LATCH_PORT &= ~LATCH; } while(0)

// LED Data Out Lines
#define SDI_PORT0		PORTB
#define SDI_R0			_BV(PB0)
#define SDI_G0			_BV(PB1)
#define SDI_B0			_BV(PB2)

#define SDI_PORT1		PORTC
#define SDI_R1			_BV(PC1)
#define SDI_G1			_BV(PC2)
#define SDI_B1			_BV(PC3)

#define SET_HI(COLOR, MATRIX)	SDI_PORT ## MATRIX |= (SDI_ ## COLOR ## MATRIX)
#define SET_LO(COLOR, MATRIX)	SDI_PORT ## MATRIX &= ~(SDI_ ## COLOR ## MATRIX)

// LED Output Enable
#define OE_PORT			PORTB
#define LED_OE			_BV(PB3)

#define ENABLE_LEDS()	OE_PORT &= ~LED_OE	// Active LO
#define DISABLE_LEDS()	OE_PORT |= LED_OE

// Transistor Control Register
#define CLK_PORT		PORTD
#define CLK				_BV(PD4)

#define SET_CLK_HI()	CLK_PORT |= CLK
#define SET_CLK_LO()	CLK_PORT &= ~CLK

// Transistor Control Reg Data, use SCK line (no latches = no data transfer)
#define DATA_PORT		PORTD
#define REG_DATA		_BV(PD5)

#define SET_DATA_HI()	PORTD |= REG_DATA
#define SET_DATA_LO()	PORTD &= ~REG_DATA

// Servo Enable Control
#define SERVO_CTRL_PORT	PORTD
#define SERVO_CTRL		_BV(PD3)

#define ENABLE_SERVOS()		SERVO_CTRL_PORT &= ~SERVO_CTRL
#define DISABLE_SERVOS()	SERVO_CTRL_PORT |= SERVO_CTRL

//---------------------
// HW_VERISON 1.0
//---------------------
#elif HW_VERSION == 0x10

#define _U_SPI_PORT		PORTB
#define _U_DO			_BV(PB3)
#define _U_DI			_BV(PB4)
#define _U_SCK			_BV(PB5)

#define LATCH_PORT		PORTB
#define RED_LATCH		_BV(PB0)
#define GREEN_LATCH		_BV(PB1)
#define BLUE_LATCH		_BV(PB2)

#define LATCH_RED()		do { LATCH_PORT |= RED_LATCH; LATCH_PORT &= ~RED_LATCH;} while(0)	
#define LATCH_BLUE()	do { LATCH_PORT |= BLUE_LATCH; LATCH_PORT &= ~BLUE_LATCH;} while(0)	
#define LATCH_GREEN()	do { LATCH_PORT |= GREEN_LATCH; LATCH_PORT &= ~GREEN_LATCH;} while(0)	

#define LATCH_ALL()		do { LATCH_PORT |= (RED_LATCH | BLUE_LATCH | GREEN_LATCH); \
							 LATCH_PORT &= ~(RED_LATCH | BLUE_LATCH | GREEN_LATCH); } while(0)

#else

#error "Unknown HW_VERSION!"

#endif	// HW_VERSION
