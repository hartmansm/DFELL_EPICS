/* drvHighlandV85x.c */

/*
 EPICS driver for Highland V850 / V851 or Berkeley Nucleonics 
 B950 / B951 Digital Delay Generator.

 Copyright (c) 2006 Duke University
 Author:        Steven Hartman
                Duke Free Electron Laser Laboratory
                <hartman@fel.duke.edu>
 
 Version:       1.0
 Please check <www.fel.duke.edu/epics> for the most recent version.


 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 USA
*/


#include <vxWorks.h>
#include <vme.h>
#include <stdioLib.h>
#include <stdlib.h>
#include <sysLib.h>
#include <vxLib.h>
#include <sys/types.h>

#include <dbDefs.h>
#include <drvSup.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvHighlandV85x.h"

#define NUMCHANS 6	/* max number of channels per card. note: 850 is 4 */
#define DEFAULTMODE 0x0000; /* default board settings */

/* these can be changed in st.cmd before iocInit if necessary */
unsigned int ddg85x_num_cards = 4;	/* max cards in crate */
unsigned int ddg85x_base = 0xc000;

long ddg85x_report(int);
long ddg85x_init(void);

struct {
	long	number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvHighlandV85x = {
	2,
	ddg85x_report,
	ddg85x_init };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvHighlandV85x);
#endif

static unsigned short **pddg85x;
static int ddg85x_addr;

/* initialize board */
long ddg85x_init(void)
{
	unsigned short **boards_present;
	short shval;
	int status;
	int i;
	ddg85x *board;

	pddg85x = (unsigned short **)
		calloc(ddg85x_num_cards,sizeof(*pddg85x));
	if ( !pddg85x ) {
		printf("drvHighlandV85x.c: Failed to allocate space.\n");
		return ERROR;
	}
	
	boards_present = pddg85x;

	if ((status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)(long)ddg85x_base, (char **)&ddg85x_addr)) != OK ) {
		printf("drvHighlandV85x.c: Addressing error in ddg85x_init().\n");
		return ERROR;
	}

	board = ((ddg85x *)((int)ddg85x_addr));

	for ( i = 0; i < ddg85x_num_cards; i++, board++, boards_present++ ) {
		if ( (vxMemProbe((char *)board, VX_READ, sizeof(unsigned short), (char *) &shval) ) == OK ) {
			*boards_present = (unsigned short *)board;
			if (( (board->vximfr) !=  MFR_ID ) || (board->vxitype) != TYPE_ID ) {
				printf("drvHighlandV85x.c: Incorrect module type found at %p for card %d.\n",board,i);
				return ERROR;
			}
			board->control = DEFAULTMODE; 
			board->actions = DEFAULTMODE;
		}
		else {
			*boards_present = 0;
		}
	}
	return 0;
}

/* set delay, value is in nanoseconds, note: channel index starts at zero */
long ddg85x_delay(unsigned short card, unsigned short chan, double val)
{
	ddg85x *board;
	unsigned long delay;

	if ( (board = (ddg85x *)pddg85x[card]) == 0 ) {
		return ERROR;
	}

	if ( val < INSERTDELAY ) {
		delay = 0;
	}
	else {
		delay = (unsigned long)( (val - INSERTDELAY) * NSECFACTOR + 0.5);
	}

	switch (chan) {
	case (0):
		board->dly1hi = (unsigned short)( (delay & 0xffff0000) >> 0x10 );
		board->dly1lo = (unsigned short)( (delay & 0xffff) );
		break;
	case (1):
		board->dly2hi = (unsigned short)( (delay & 0xffff0000) >> 0x10 );
		board->dly2lo = (unsigned short)( (delay & 0xffff) );
		break;
	case (2):
		board->dly3hi = (unsigned short)( (delay & 0xffff0000) >> 0x10 );
		board->dly3lo = (unsigned short)( (delay & 0xffff) );
		break;
	case (3):
		board->dly4hi = (unsigned short)( (delay & 0xffff0000) >> 0x10 );
		board->dly4lo = (unsigned short)( (delay & 0xffff) );
		break;
	case (4):
		board->dly5hi = (unsigned short)( (delay & 0xffff0000) >> 0x10 );
		board->dly5lo = (unsigned short)( (delay & 0xffff) );
		break;
	case (5):
		board->dly6hi = (unsigned short)( (delay & 0xffff0000) >> 0x10 );
		board->dly6lo = (unsigned short)( (delay & 0xffff) );
		break;
	default:
		return ERROR;
	}

	/* jam-load timing registers */
	board->actions = FEOD | XFR; /* force end of delay and transfer new timing registers */

	return 0;
}		

/* set waveform mode, sets mode for pairs of channels (0-1, 2-3, or 4-5). 
 * note: channel index starts at zero */
long ddg85x_wave(unsigned short card, unsigned chan, unsigned char mode1, unsigned char mode2)
{
	ddg85x *board;

	if ( (board = (ddg85x *)pddg85x[card]) == 0 ) {
		return ERROR;
	}

	switch (chan) {
	case(0):
	case(1):
		board->wave12 = (unsigned short)( (mode1 & 0xf) | (mode2 << 0x4) ); 
		break;
	case(2):
	case(3):
		board->wave34 = (unsigned short)( (mode1 & 0xf) | (mode2 << 0x4) ); 
		break;
	case(4):
	case(5):
		board->wave56 = (unsigned short)( (mode1 & 0xf) | (mode2 << 0x4) ); 
		break;
	default:
		return ERROR;
	}

	return 0;

}

/* disable card */
long ddg85x_disable(unsigned short card, unsigned char mode)
{
	ddg85x *board;

	if ( (board = (ddg85x *)pddg85x[card]) == 0 ) {
		return ERROR;
	}

	if (mode) 
		board->control = DISARM;
	else
		board->control = DEFAULTMODE;
	
	return 0;
}

/* set output voltage, levels are in volts */
long ddg85x_vout(unsigned short card, double lowlevel, double highlevel)
{
	ddg85x *board;

	if ( (board = (ddg85x *)pddg85x[card]) == 0 ) {
		return ERROR;
	}

	board->voutlo = -1 * lowlevel * VLOW_FACTOR;
	board->vouthi = highlevel * VHIGH_FACTOR;

	return 0;
}

/* set trigger voltage, triglevel is in volts */
long ddg85x_triglev(unsigned short card, double triglevel)
{
	ddg85x *board;

	if ( (board = (ddg85x *)pddg85x[card]) == 0 ) {
		return ERROR;
	}

	board->trglvl = (2.5 - triglevel) * TRGLVL_FACTOR;

	return 0;
}


long ddg85x_report(int level)
{
	int i;
	ddg85x *board;

	for ( i = 0; i < ddg85x_num_cards; i++, board++) {
		if ( pddg85x[i] ) {
			printf("Highland/BNC V85x Digital Delay Generator: card %d is present\n",i);
			if (level > 0) {
				board = (ddg85x *)pddg85x[i];
				printf("    Address = %p, Manufacturer ID = 0x%x, Type = 0x%x, Status Register = 0x%x\n", board, board->vximfr, board->vxitype, board->vxists); 
			}

			if (level > 1) {
				printf("    All other registers are write-only.\n");
			}
		}
	}

	return 0;
}

