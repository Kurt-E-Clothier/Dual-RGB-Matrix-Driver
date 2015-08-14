/***************************************************************************
* 
* File              : matrixRGB_main-vxx.c
*
* Author			: Kurt E. Clothier
* Date				: July 17, 2015
* Modified			: July 30, 2015
*
* Description       : RGM Matrix Driver
*
* Compiler			: AVR-GCC
* Hardware			: ATMEGA328p; Ext 16MHz Osc
* Fuse Settings		: E:07, H:D9, L:E7
*
* More Information	: http://www.projectsbykec.com/
*					: http://www.pubnub.com
*
****************************************************************************/
 #define FW_VERSION	0x31	// Firmware Version 3.1 - I2C Communication
 #define HW_VERSION	0x20	// Hardwave Version 2.0 - See matrixRGB-vX.X.sch
 /***************************************************************************
  Completed
	- Individual Color Control of Every RGB in 2 Matrices
	- RGB Color Resolution: [0, 3] (64 possible colors)
	- I2C Communication
	- Quadrant Control Via I2C

  Working On
	- Better loop delays (to avoid delay in quadrant control)
	- Disable servos (set IO pin HI) during Screen saver time
	- Initial Chase Sequence Setup Phase
	- Chase Sequences

  Additional Notes

***************************************************************************/
 
/**************************************************************************
	Definitions for Conditional Code
***************************************************************************/

/**************************************************************************
	Included Header Files
***************************************************************************/
#include "modules/avr.h"
#include "definitions.h"
#include "modules/macros/color_8bit.h"
#include "modules/twi/twi.h"
#include <util/delay.h>

/**************************************************************************
	Definitions for Testing Purposes Only
***************************************************************************/

/**************************************************************************
	Global Variables
***************************************************************************/
// Using these unused I/O registers is much more efficient than variables

#define chase_sequence 	PCMSK0	// actuve chase sequence
#define column			PCMSK1	// Current active column [0, 7]
#define TWI_msgBuf		PCMSK2	// Message Buffer for TWI bus

#define OCR0A_cnt		EEARH	// Counter for the OCR0A timer
//#define EEDR
//#define EEARL	

#define stat_flags		GPIOR0	// Status flags
#define	quad_flags		GPIOR1	// 1 bit for each quadrant of 2 matrices
//#define GPIOR2

static volatile bool	TWI_isBusy = false;

// 1 bit for each RGB in each column
static volatile uint8_t leds[MATRICES][COLUMNS][COLORS];

// the actual color of each RGB (see color_8bit.h)
static volatile uint8_t	colors[MATRICES][COLUMNS][LEDS];

/**************************************************************************
    Local Function Prototypes
***************************************************************************/
static void initialize_AVR(void);
static void set_quadrant(const uint8_t mtrx, const uint8_t quad, const uint8_t color);
static void set_quadrants(const uint8_t color);
static void set_column(const uint8_t mtrx, const uint8_t col, const uint8_t color);
static void set_row(const uint8_t mtrx, const uint8_t row, const uint8_t color);
static void set_led(const uint8_t mtrx, const uint8_t row, const uint8_t col, const uint8_t color);
static void set_matrix(const uint8_t mtrx, const uint8_t color);
static void turn_off_matrices(void);

/**************************************************************************
    Main
***************************************************************************/
int main (void) 
{
	uint8_t color = UNIQUE_COLORS -1;
	uint8_t row = 0;
	uint8_t col = 0;
	uint8_t quad = 0;
	uint8_t binary_cnt = 0;
	uint8_t wdt_cnt = 0;
	uint8_t update_cnt = 0;
	
	initialize_AVR();

	SET_FLAG(SET_LEDS);
	chase_sequence = SMILEY;
	DISABLE_SERVOS();

	quad_flags = 0xFF;

	turn_off_matrices();

	// Turn On TWI
	TWI_RESET_WITH_ACK();


	/**********************************************
	 *	MAIN LOOP
	 *	- Handle Chase Sequences
	 **********************************************/
	for(;;)
	{
		//-------------------------
		// Handle Color Loops
		//-------------------------
		if (FLAG_IS_SET(INCREMENT_COLOR)) {
			if (++color == UNIQUE_COLORS)
				color = 1;
			CLEAR_FLAG(INCREMENT_COLOR);
		}
		else if (FLAG_IS_SET(DECREMENT_COLOR)) {
			if (--color == 0)
				color = UNIQUE_COLORS -1;
			CLEAR_FLAG(DECREMENT_COLOR);
		}

		//-------------------------
		// Reset Chase Sequence
		//-------------------------
		if (FLAG_IS_SET(RESET_CHASE)) {
			wdt_cnt = 0;
			chase_sequence = LOOP_QUAD;
			CLEAR_FLAG(RESET_CHASE);
			CLEAR_FLAG(PASSIVE_MODE);
			ENABLE_SERVOS();
		}

		//-------------------------
		// Handle Chase Sequences
		//-------------------------
		switch (chase_sequence) {

			//-------------------------
			// Constant On at the last color
			//-------------------------
			case ALL_CONSTANT:
				break;

			//-------------------------
			// Loop through all colors
			//-------------------------
			case LOOP_ALL:
				SET_FLAG(DECREMENT_COLOR);
				set_matrix(0, color);
				set_matrix(1, color);
				_delay_ms(100);
				break;
			
			//-------------------------
			// Color wheel - one color transistion per revolution
			//-------------------------
			case QUAD_WHEEL:
				if (quad == 0) {
					set_quadrant(0, 3, COL_BLACK);
					set_quadrant(1, 3, COL_BLACK);
				}
				else {
					set_quadrant(0, quad-1, COL_BLACK);
					set_quadrant(1, quad-1, COL_BLACK);
				}
				set_quadrant(0, quad, color);
				set_quadrant(1, quad, color);
				if (++quad == QUADS) {
					SET_FLAG(DECREMENT_COLOR);
					quad = 0;
				}
				_delay_ms(50);
				break;

			//-------------------------
			// Color wheel - change colors with quadrants
			//-------------------------
			case QUAD_WHEEL2:
				SET_FLAG(DECREMENT_COLOR);
				if (quad == 0) {
					set_quadrant(0, 3, COL_BLACK);
					set_quadrant(1, 3, COL_BLACK);
				}
				else {
					set_quadrant(0, quad-1, COL_BLACK);
					set_quadrant(1, quad-1, COL_BLACK);
				}
				set_quadrant(0, quad, color);
				set_quadrant(1, quad, color);
				if (++quad == QUADS) {
					quad = 0;
				}
				_delay_ms(50);
				
				break;

			//-------------------------
			// Loop through all colors in Quadrant(s)
			//-------------------------
			case LOOP_QUAD:
				SET_FLAG(DECREMENT_COLOR);
				set_quadrants(color);
				_delay_ms(100);
				break;

			//-------------------------
			// Binary Counter - by Columns
			//-------------------------
			case BINARY_COLS:
				for (col = 0; col < COLUMNS; ++col) {
					if (binary_cnt & _BV(col))
						set_column(0, col, COL_BLUE);
					else
						set_column(0, col, COL_BLACK);
				}
				++binary_cnt;
				_delay_ms(250);
				break;

			//-------------------------
			// Binary Counter - by Row
			//-------------------------
			case BINARY_ROWS:
				for (row = 0; row < ROWS; ++row) {
					if (binary_cnt & _BV(row))
						set_row(0, row, COL_RED);
					else
						set_row(0, row, COL_BLACK);
				}
				++binary_cnt;
				_delay_ms(250);
				break;

			//-------------------------
			// Display all colors
			//-------------------------
			case ALL_COLORS:
				if (FLAG_IS_SET(SET_LEDS)) {
					for (col = 0; col < COLUMNS; ++col) {
						for (row = 0; row < ROWS; ++row) {
							if (color < UNIQUE_COLORS) {
								set_led(0, col, row, color);
								++color;
							}
							else
								set_led(0, col, row, COL_BLACK);
						}
					}
					CLEAR_FLAG(SET_LEDS);
				}
				break;

			//-------------------------
			// Smiley Faces
			//-------------------------
			case SMILEY:
				if (FLAG_IS_SET(SET_LEDS)) {
					turn_off_matrices();

					// Border
					#define FACE_COLOR	COL_OLIVE
					set_row(0, 0, FACE_COLOR);
					set_row(0, 7, FACE_COLOR);
					set_column(0, 0, FACE_COLOR);
					set_column(0, 7, FACE_COLOR);

					set_row(1, 0, FACE_COLOR);
					set_row(1, 7, FACE_COLOR);
					set_column(1, 0, FACE_COLOR);
					set_column(1, 7, FACE_COLOR);

					// Eyes
					set_led(0, 1, 1, COL_WHITE);
					set_led(0, 1, 2, COL_WHITE);
					set_led(0, 2, 1, COL_WHITE);
					set_led(0, 2, 2, COL_BLUE);

					set_led(0, 1, 5, COL_WHITE);
					set_led(0, 1, 6, COL_WHITE);
					set_led(0, 2, 5, COL_WHITE);
					set_led(0, 2, 6, COL_BLUE);

					set_led(1, 1, 1, COL_WHITE);
					set_led(1, 1, 2, COL_WHITE);
					set_led(1, 2, 2, COL_WHITE);
					set_led(1, 2, 1, COL_GREEN);

					set_led(1, 1, 5, COL_WHITE);
					set_led(1, 1, 6, COL_WHITE);
					set_led(1, 2, 6, COL_WHITE);
					set_led(1, 2, 5, COL_GREEN);

					// Nose
					set_led(0, 3, 4, FACE_COLOR);
					set_led(0, 4, 3, FACE_COLOR);
					set_led(0, 4, 4, FACE_COLOR);

					set_led(1, 3, 3, FACE_COLOR);
					set_led(1, 4, 3, FACE_COLOR);
					set_led(1, 4, 4, FACE_COLOR);

					// Mouth
					set_led(0, 5, 1, COL_CORAL);
					set_led(0, 6, 2, COL_CORAL);
					set_led(0, 6, 3, COL_CORAL);
					set_led(0, 6, 4, COL_CORAL);
					set_led(0, 6, 5, COL_CORAL);
					set_led(0, 5, 6, COL_CORAL);

					set_led(1, 5, 1, COL_CORAL);
					set_led(1, 6, 2, COL_CORAL);
					set_led(1, 6, 3, COL_CORAL);
					set_led(1, 6, 4, COL_CORAL);
					set_led(1, 6, 5, COL_CORAL);
					set_led(1, 5, 6, COL_CORAL);
					CLEAR_FLAG(SET_LEDS);
					SET_FLAG(UPDATE_LEDS);
				}

					// Look back and forth
				if (FLAG_IS_SET(UPDATE_LEDS)) {
					set_led(0, 2, 2, COL_WHITE);
					set_led(0, 2, 1, COL_BLUE);
					set_led(0, 2, 6, COL_WHITE);
					set_led(0, 2, 5, COL_BLUE);
					set_led(1, 2, 1, COL_WHITE);
					set_led(1, 2, 2, COL_GREEN);
					set_led(1, 2, 5, COL_WHITE);
					set_led(1, 2, 6, COL_GREEN);
					_delay_ms(200);
					set_led(0, 2, 1, COL_WHITE);
					set_led(0, 2, 2, COL_BLUE);
					set_led(0, 2, 5, COL_WHITE);
					set_led(0, 2, 6, COL_BLUE);
					set_led(1, 2, 2, COL_WHITE);
					set_led(1, 2, 1, COL_GREEN);
					set_led(1, 2, 6, COL_WHITE);
					set_led(1, 2, 5, COL_GREEN);
					update_cnt = SMILEY_EYE_DELAY;
					CLEAR_FLAG(UPDATE_LEDS);
				}

				break;

			//-------------------------
			// Set matrix to white
			//-------------------------
			case ALL_WHITE:
				if (FLAG_IS_SET(SET_LEDS)) {
					set_matrix(0, COL_WHITE);
					CLEAR_FLAG(SET_LEDS);
				}
				break;

			//-------------------------
			// Turn off all LEDs
			//-------------------------
			case ALL_OFF:
			default:
				if (FLAG_IS_SET(SET_LEDS)) {
					turn_off_matrices();
					CLEAR_FLAG(SET_LEDS);
				}
				break;
			
			//-------------------------
			// Test - Test corner LEDs
			//-------------------------
			case TEST_CORNERS:
				set_led(0, 7, 0, COL_BLACK);
				set_led(0, 0, 0, COL_RED);
				set_led(0, 4, 4, COL_RED);
				set_led(1, 7, 7, COL_BLACK);
				set_led(1, 0, 7, COL_RED);
				set_led(1, 4, 3, COL_RED);
				_delay_ms(200);
				set_led(0, 0, 0, COL_BLACK);
				set_led(0, 0, 7, COL_BLUE);
				set_led(0, 4, 4, COL_BLUE);
				set_led(1, 0, 7, COL_BLACK);
				set_led(1, 0, 0, COL_BLUE);
				set_led(1, 4, 3, COL_BLUE);
				_delay_ms(200);
				set_led(0, 0, 7, COL_BLACK);
				set_led(0, 7, 7, COL_YELLOW);
				set_led(0, 4, 4, COL_YELLOW);
				set_led(1, 0, 0, COL_BLACK);
				set_led(1, 7, 0, COL_YELLOW);
				set_led(1, 4, 3, COL_YELLOW);
				_delay_ms(200);
				set_led(0, 7, 7, COL_BLACK);
				set_led(0, 7, 0, COL_GREEN);
				set_led(0, 4, 4, COL_GREEN);
				set_led(1, 7, 0, COL_BLACK);
				set_led(1, 7, 7, COL_GREEN);
				set_led(1, 4, 3, COL_GREEN);
				_delay_ms(200);
				break;
		}

		//-------------------------
		// Update Timer - 10ms increments
		//-------------------------
		if (update_cnt > 0) {
			_delay_ms(10);
			if (--update_cnt == 0) {
				SET_FLAG(UPDATE_LEDS);
			}
		}


		//-------------------------
		// WatchDog Timer
		//-------------------------
		if (wdt_cnt < WDT_MAX) {
			if (++wdt_cnt == WDT_MAX) {
				DISABLE_SERVOS();
				SET_FLAG(SET_LEDS);
				SET_FLAG(PASSIVE_MODE);
				chase_sequence = SMILEY;
			}
		}
		

	}	// End of Main Loop
}	// End of Main

/**************************************************************************
	UTILITIES
***************************************************************************/

/**
 * Set an LED to a color.
 *
 * @param col	column of the matrix [0, 7]
 * @param row	row of the matrix [0, 7]
 * @param color	color to set
 */
static void set_led(const uint8_t mtrx, const uint8_t row, const uint8_t col, const uint8_t color)
{
	colors[mtrx][col][row] = color;
}

/**
 * Turn off both matrices
 */
static void turn_off_matrices(void)
{
	uint8_t col = 0;
	uint8_t led = 0;
	for (col = 0; col < COLUMNS; ++col) {
		for (led = 0; led < LEDS; ++led) {
			colors[0][col][led] = COL_BLACK;
			colors[1][col][led] = COL_BLACK;
		}
	}
}

/**
 * Set the matrix to a color.
 *
 * @param color	color to set
 */
static void set_matrix(const uint8_t mtrx, const uint8_t color)
{
	uint8_t col = 0;
	uint8_t led = 0;
	for (col = 0; col < COLUMNS; ++col) {
		for (led = 0; led < LEDS; ++led) {
			colors[mtrx][col][led] = color;
		}
	}
}

/**
 * Set a column to a color.
 *
 * @param col	column of the matrix [0, 7]
 * @param color	color to set
 */
static void set_column(const uint8_t mtrx, const uint8_t col, const uint8_t color)
{
	uint8_t led = 0;
	for (led = 0; led < LEDS; ++led)
		colors[mtrx][col][led] = color;
}

/**
 * Set a row to a color.
 *
 * @param row	row of the matrix [0, 7]
 * @param color	color to set
 */
static void set_row(const uint8_t mtrx, const uint8_t row, const uint8_t color)
{
	uint8_t col = 0;
	for (col = 0; col < COLUMNS; ++col)
		colors[mtrx][col][row] = color;
}

/**
 * Set a quadrant to a color, based on values in quad_flags.
 * Using unrolled loops to save time.
 */
static void set_quadrants(const uint8_t color)
{
	uint8_t col = 0;
	turn_off_matrices();

	for (col = 0; col < 4; ++col) {
		if (quad_flags & QUAD00) {
			colors[0][col][0] = color;
			colors[0][col][1] = color;
			colors[0][col][2] = color;
			colors[0][col][3] = color;
		}
		if (quad_flags & QUAD03) {
			colors[0][col][4] = color;
			colors[0][col][5] = color;
			colors[0][col][6] = color;
			colors[0][col][7] = color;
		}
		if (quad_flags & QUAD10) {
			colors[1][col][0] = color;
			colors[1][col][1] = color;
			colors[1][col][2] = color;
			colors[1][col][3] = color;
		}
		if (quad_flags & QUAD13) {
			colors[1][col][4] = color;
			colors[1][col][5] = color;
			colors[1][col][6] = color;
			colors[1][col][7] = color;
		}
	}
	for (col = 4; col < COLUMNS; ++col) {
		if (quad_flags & QUAD01) {
			colors[0][col][0] = color;
			colors[0][col][1] = color;
			colors[0][col][2] = color;
			colors[0][col][3] = color;
		}
		if (quad_flags & QUAD02) {
			colors[0][col][4] = color;
			colors[0][col][5] = color;
			colors[0][col][6] = color;
			colors[0][col][7] = color;
		}
		if (quad_flags & QUAD11) {
			colors[1][col][0] = color;
			colors[1][col][1] = color;
			colors[1][col][2] = color;
			colors[1][col][3] = color;
		}
		if (quad_flags & QUAD12) {
			colors[1][col][4] = color;
			colors[1][col][5] = color;
			colors[1][col][6] = color;
			colors[1][col][7] = color;
		}
	}
}

/**
 * Set a quadrant to a color.
 *
 * @param quad	quadrant of the matrix [0, 3]
 * @param color	color to set the quadrant
 */
static void set_quadrant(const uint8_t mtrx, const uint8_t quad, const uint8_t color)
{
	uint8_t col = 0;
	uint8_t led = 0;

	switch (mtrx) {

	case (0):
		switch (quad) {
			case (0):
				for (col = 4; col < COLUMNS; ++col)
					for (led = 0; led < 4; ++led)
						colors[0][col][led] = color;
				break;
			case (1):
				for (col = 0; col < 4; ++col)
					for (led = 0; led < 4; ++led)
						colors[0][col][led] = color;
				break;
			case (2):
				for (col = 0; col < 4; ++col)
					for (led = 4; led < LEDS; ++led)
						colors[0][col][led] = color;
				break;
			case (3):
				for (col = 4; col < COLUMNS; ++col)
					for (led = 4; led < LEDS; ++led)
						colors[0][col][led] = color;
				break;
			default:
				break;
		}
	case (1):
		switch (quad) {
			case (0):
				for (col = 0; col < 4; ++col)
					for (led = 0; led < 4; ++led)
						colors[1][col][led] = color;
				break;
			case (1):
				for (col = 4; col < COLUMNS; ++col)
					for (led = 0; led < 4; ++led)
						colors[1][col][led] = color;
				break;
			case (2):
				for (col = 4; col < COLUMNS; ++col)
					for (led = 4; led < LEDS; ++led)
						colors[1][col][led] = color;
				break;
			case (3):
				for (col = 0; col < 4; ++col)
					for (led = 4; led < LEDS; ++led)
						colors[1][col][led] = color;
				break;
			default:
				break;
		}
		break;
	default:
		break;

	}
}

/**************************************************************************
	INTERRUPT HANDLERS
***************************************************************************/
// Catch-All Default for Unexpected Interrupts
//ISR(BADISR_vect){}

/***************************************************************
 * TWI Interrupt Service Routine - Communication with PubNub Client
 *
 *	This ISR will control the communication between this chip 
 *	and the PubNub client. The client is acts as the bus master
 *	and controls the coloration of the matrices.
 ***************************************************************/
ISR(TWI_vect)
{
	switch (TWSR)
	{
		//--------------------------------------
		// Receive Data
		//--------------------------------------

		// Received: SLA + W; ACK returned
		case TWI_SRX_ADR_ACK:
			TWI_isBusy = true;
			TWI_ENABLE_ACK();
			break;

		// Received: Data after SLA+W; ACK returned
		case TWI_SRX_ADR_DATA_ACK:
		// Received: Data after SLA+W; NACK returned
		case TWI_SRX_ADR_DATA_NACK:
			TWI_msgBuf = TWDR;
			TWI_ENABLE_ACK();
			break;

		//--------------------------------------
		// STOP or Repeated START
		//--------------------------------------
		case TWI_SRX_STOP_RESTART:
			SET_FLAG(RESET_CHASE);
			quad_flags = TWI_msgBuf;
			TWI_isBusy = false;
			TWI_ENABLE_ACK();
			break;

		//--------------------------------------
		// Transmit Data
		//--------------------------------------

		// Received: SLA + R; ACK returned
		case TWI_STX_ADR_ACK:
		// Transmitted TWDR; ACK received, Done
		case TWI_STX_DATA_ACK_LAST_BYTE:
		// Transmitted TWDR; ACK received
		case TWI_STX_DATA_ACK:          
		// Transmitted TWDR; NACK received
		case TWI_STX_DATA_NACK:  

		//--------------------------------------
		// General Call
		//--------------------------------------

		// Received: General Call; ACK returned
		case TWI_SRX_GEN_ACK:
		// Received: Data after Gen Call; ACK returned
		case TWI_SRX_GEN_DATA_ACK:
		// Received: Data after Gen Call; NACK returned
		case TWI_SRX_GEN_DATA_NACK:

		//--------------------------------------
		// Error States
		//--------------------------------------

		// Bus Error: Illegal START or STOP
		case TWI_BUS_ERROR:
		// And anything else...
		default:     
			TWI_isBusy = false;
			TWI_RECOVER();
			TWI_ENABLE_ACK();
			break;
	}
}

/***************************************************************
 * Timer/Counter0 Compare Match A
 *
 * This ISR is the core timing mechanism of the LED control.
 * It fires at every possible LED turn off time. A count of 
 * zero signifies the start of a new RGB period. At all counts
 * after that, the turn off times for each LED are compared 
 * to the current time. If they are equal an LED is turned off.
 *
 * In this way, the code is simulating a PWM channel for every
 * R, G, & B LED in two entire matrices (2 x 8 x 8 x 3 total LEDs).
 * The max resolution for the LEDs is set in color_8bit.h.
 ***************************************************************/
ISR(TIMER0_COMPA_vect)
{
	uint8_t data_red0 = 0;
	uint8_t data_green0 = 0;
	uint8_t data_blue0 = 0;
	uint8_t data_red1 = 0;
	uint8_t data_green1 = 0;
	uint8_t data_blue1 = 0;
	uint8_t this_color = 0;
	uint8_t bit = 8;
	uint8_t led = 7;

	switch (OCR0A_cnt) {

		//--------------------------
		// Max Resolution, don't do anything!
		// At this point, LEDs should stay on
		//--------------------------
		case MAX_COLOR_RESOLUTION:
			break;

		//--------------------------
		// Start of LED Period
		//--------------------------
		case 0:

			// Start a positive pulse to shift through the control register
			if (++ column == 8) {
				SET_DATA_HI();
				column = 0;
			}
			else {
				SET_DATA_LO();
			}
			// Turn all LEDs back on
			leds[0][column][RED_LEDS] = 0xFF;
			leds[0][column][GREEN_LEDS] = 0xFF;
			leds[0][column][BLUE_LEDS] = 0xFF;
			leds[1][column][RED_LEDS] = 0xFF;
			leds[1][column][GREEN_LEDS] = 0xFF;
			leds[1][column][BLUE_LEDS] = 0xFF;
			// continue

		//--------------------------
		// All Other times
		//	- Loop through all colors for all leds
		//	- Turn individual LEDs off at the right time
		//--------------------------
		default:
			// Find LEDs to turn off
			do {
				// Matrix 0
				this_color = colors[0][column][led];
				if (OCR0A_cnt == COLOR_LEVELS[this_color][RED_LEVEL])
					leds[0][column][RED_LEDS] &= ~(_BV(led));
				if (OCR0A_cnt == COLOR_LEVELS[this_color][GREEN_LEVEL])
					leds[0][column][GREEN_LEDS] &= ~(_BV(led));
				if (OCR0A_cnt == COLOR_LEVELS[this_color][BLUE_LEVEL])
					leds[0][column][BLUE_LEDS] &= ~(_BV(led));
				// Matrix 1
				this_color = colors[1][column][led];
				if (OCR0A_cnt == COLOR_LEVELS[this_color][RED_LEVEL])
					leds[1][column][RED_LEDS] &= ~(_BV(led));
				if (OCR0A_cnt == COLOR_LEVELS[this_color][GREEN_LEVEL])
					leds[1][column][GREEN_LEDS] &= ~(_BV(led));
				if (OCR0A_cnt == COLOR_LEVELS[this_color][BLUE_LEVEL])
					leds[1][column][BLUE_LEDS] &= ~(_BV(led));
			} while (led-- > 0);

			data_red0 = leds[0][column][RED_LEDS];
			data_green0 = leds[0][column][GREEN_LEDS];
			data_blue0 = leds[0][column][BLUE_LEDS];
			data_red1 = leds[1][column][RED_LEDS];
			data_green1 = leds[1][column][GREEN_LEDS];
			data_blue1 = leds[1][column][BLUE_LEDS];

			// Shift pulse to next column
			if (OCR0A_cnt == 0) {
				DISABLE_LEDS();
				SET_CLK_HI();
				SET_CLK_LO();
			}
			// Transfer LED level data to Current Drivers
			do {  
				// Set Data
				if(data_red0 & 0x80) SET_HI(R, 0);
				else SET_LO(R, 0);

				if(data_green0 & 0x80) SET_HI(G, 0);
				else SET_LO(G, 0);
	
				if(data_blue0 & 0x80) SET_HI(B, 0);
				else SET_LO(B, 0);

				if(data_red1 & 0x80) SET_HI(R, 1);
				else SET_LO(R, 1);

				if(data_green1 & 0x80) SET_HI(G, 1);
				else SET_LO(G, 1);
	
				if(data_blue1 & 0x80) SET_HI(B, 1);
				else SET_LO(B, 1);

				// Clock Rising Edge
				SET_SCK_HI();
	
				// Shift data to next bit
				data_red0 <<= 1;
				data_green0 <<= 1;
				data_blue0 <<= 1;
				data_red1 <<= 1;
				data_green1 <<= 1;
				data_blue1 <<= 1;
	
				// Clock Falling Edge
				SET_SCK_LO();
			} while (--bit > 0);

			// Enable new LED data
			LATCH_LEDS();
			ENABLE_LEDS();
			break;
	}
	// Restart the count		
	if (++OCR0A_cnt > MAX_COLOR_RESOLUTION)
		OCR0A_cnt = 0;
}

/**************************************************************************
	INITIALIZATION ROUTINES AND POWER MODES
***************************************************************************/

/****** Initialize the ATmega328p microcontroller *****/
static void initialize_AVR(void)
{ 
	cli();	// Turn off interrupts

	// Set up AVR I/O Pins - Mostly all Outputs
	DDRB = 0xFF;
	DDRC = (0xFF & ~(_BV(PC0)));
	DDRD = (_BV(PD3) | _BV(PD4) | _BV(PD5));

	// Set Pullups on Unused Inputs
	PORTC = _BV(PC0);
	PORTD = ~(_BV(PD3) | _BV(PD4) | _BV(PD5));

	DISABLE_LEDS();

	// Power Reduction Register - Enable Modules as Used
	PRR = 
		//_BV(PRTWI) |		// Disable TWI Clock
		_BV(PRSPI) |		// Disable SPI Clock
		_BV(PRTIM2) |		// Disable Timer2 Clock
		_BV(PRTIM1) |		// Disable Timer1 Clock
		//_BV(PRTIM0) |		// Disable Timer0 Clock
		_BV(PRUSART0) |		// Disable USART0 CLock
		_BV(PRADC);			// Disable ADC Clock

	// Timer 0 - LED Control
	TCCR0A = 
		_BV(WGM01);			// CTC Mode, TOP = OCR0A
	TCCR0B =				
		_BV(CS00) |			// Prescaler = 1024
		_BV(CS02);			
	OCR0A = 2;
	TIMSK0 = _BV(OCIE0A);		// Enable Compare Match A Interrupt

	// TWI - Communication with PubNub Client (bus master)
	TWAR =	 
		(TWI_SLAVE_ADDRESS << 1);	// TWI Slave Address
									// Do Not Recognize General Call
	TWCR = _BV(TWEN);				// Enable TWI
	sei();	// Turn on interrupts
}



