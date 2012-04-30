/*
	Data for "tapeeject" Image
*/

#include <exec/types.h>
#include <intuition/intuition.h>

UWORD __chip ejectData[48] =
{
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0018,0x0000,
	0x003C,0x0000,0x007E,0x0000,0x00FF,0x0000,0x01FF,0x8000,
	0x03FF,0xC000,0x07FF,0xE000,0x0FFF,0xF000,0x1FFF,0xF800,
	0x3FFF,0xFC00,0x3FFF,0xFC00,0x3FFF,0xFC00,0x0000,0x0000,
	0x0000,0x0000,0x3FFF,0xFC00,0x3FFF,0xFC00,0x3FFF,0xFC00,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

struct Image eject =
{
	0, 0,		/* LeftEdge, TopEdge */
	24, 24, 1,	/* Width, Height, Depth */
	ejectData,	/* ImageData */
	0x0001, 0x0000,	/* PlanePick, PlaneOnOff */
	NULL		/* NextImage */
};
