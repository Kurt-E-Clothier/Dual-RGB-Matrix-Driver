/***************************************************************************
* 
* File              : color_8bit.h
*
* Author			: Kurt E. Clothier
* Date				: July 24, 2015
* Modified			: July 27, 2015
*
* Description       : 8 Bit Colors
*
* Compiler			: AVR-GCC
*
* More Information	: http://www.projectsbykec.com/
*
****************************************************************************/

#ifndef _COLOR_8BIT_
#define _COLOR_8BIT_

/***************************************************************************
	Macros
 ***************************************************************************/

#define COLOR_MAX_RESOLUTION	3		// [0, 255] mapped to [0, 3]
#define MAX_COLOR_RESOLUTION	COLOR_MAX_RESOLUTION
#define COLOR_MASK				0x3F	// Mask the lower 6 bits

#define RED_LEVEL	0
#define GREEN_LEVEL	1
#define BLUE_LEVEL	2
#define RGB_LEVELS	3

/***************************************************************************
	Available Colors - Array Indexing
 ***************************************************************************/
#define COL_BLACK			0
#define COL_DARK_GREY		1
#define COL_PALE_PURPLE		2
#define COL_DEEP_FUCHSIA	3
#define COL_MAGENTA			4
#define COL_DARK_MAGENTA	5
#define COL_VIOLET			6
#define COL_PURPLE			7
#define COL_INDIGO			8
#define COL_NAVY			9
#define COL_BLUE			10
#define COL_POWDER_BLUE		11
#define COL_SKY_BLUE		12
#define COL_JADE			13
#define COL_CYAN			14
#define COL_TURQUOISE		15
#define COL_SPRING_GREEN	16
#define COL_DARK_GREEN		17
#define COL_GREEN			18
#define COL_LIME			19
#define COL_LAWN_GREEN		20
#define COL_GREEN_YELLOW	21
#define COL_YELLOW			22
#define COL_GOLD			23
#define COL_OLIVE			24
#define COL_DARK_ORANGE		25
#define COL_ORANGE			26
#define COL_ORANGE_RED		27
#define COL_BRICK			28
#define COL_MAROON			29
#define COL_RED				30
#define COL_BRIGHT_PINK		31
#define COL_DEEP_PINK		32
#define COL_DARK_PINK		33
#define COL_FUCHSIA			34
#define COL_PINK			35
#define COL_CORAL			36
#define COL_PEACH			37
#define COL_SALMON			38
#define COL_KHAKI			39
#define COL_CREAM			40
#define COL_WHITE			41
#define COL_GREY			42
#define COL_LIGHT_GREY		43

#define UNIQUE_COLORS		44


/***************************************************************************
	R G & B Levels for each available color
 ***************************************************************************/
static const unsigned char COLOR_LEVELS[UNIQUE_COLORS][RGB_LEVELS] = {
	{0,0,0},	// black (off)
	//{1,2,1},	
	//{1,2,2},	
	//{1,2,3},	
	//{1,1,3},	
	{1,1,2},	// dark grey
	{2,1,3},	// pale purple
	{2,0,3},	// deep fuchsia
	{3,0,3},	// magenta
	{1,0,1},	// dark magenta
	{1,0,2},	// violet
	{1,0,3},	// purple
	{0,0,1},	// indigo`
	{0,0,2},	// navy
	{0,0,3},	// blue
	{0,1,3},	// powder blue
	//{0,1,2},	
	{0,2,3},	// sky blue
	{0,2,2},	// jade
	//{0,2,1},	
	//{1,3,3},	
	{0,3,3},	// cyan
	{0,3,2},	// turquoise
	{0,3,1},	// spring green
	//{0,1,1},	
	{0,1,0},	// dark green
	{0,2,0},	// green
	{0,3,0},	// lime
	//{2,3,0},	
	{1,3,0},	// lawn green
	//{1,3,1},
	//{1,3,2},
	{1,2,0},	// green yellow
	{3,3,0},	// yellow
	{2,2,0},	// gold
	{1,1,0},	// olive
	{2,1,0},	// dark orange
	{3,2,0},	// orange 
	{3,1,0},	// orange red
	{1,0,0},	// brick
	{2,0,0},	// maroon
	{3,0,0},	// red
	{3,0,2},	// bright pink
	{3,0,1},	// deep pink
	{2,0,1},	// dark pink
	{2,0,2},	// fuchsia
	{3,1,3},	// pink
	//{2,1,2},
	{3,1,2},	// coral
	{3,1,1},	// peach
	//{2,1,1},	
	{3,2,1},	// salmon
	//{3,2,2},	
	//{3,2,3},	
	//{1,2,3},	
	//{2,2,1},	
	//{2,3,1},	
	//{2,3,2},	
	{3,3,1},	// khaki
	{3,3,2},	// cream
	{3,3,3},	// white
	//{2,3,3},	
	{2,2,2},	// grey
	{1,1,1}		// light grey
};

static const unsigned char TEST_LEVELS[16][RGB_LEVELS] = {
	{0,0,0},	// black
	{3,0,3},	// magenta
	{1,0,2},	// purple
	{0,0,2},	// navy
	{0,0,3},	// blue
	{0,1,2},	// teal
	{0,3,3},	// cyan
	{0,3,0},	// green
	{1,3,0},	// lime
	{1,1,0},	// olive
	{3,3,0},	// yellow
	{3,2,1},	// salmon
	{3,2,0},	// orange
	{2,0,0},	// maroon
	{3,0,0},	// red
	{3,3,3},	// white
};

#endif	// _COLOR_8BIT_


