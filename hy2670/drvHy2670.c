/* drvHy2670.c */

/*********************************************************************

EPICS driver for Hytec VDD 2670 18-bit, 2-channel Digital to
Analog Converter board

Copyright (c) 2003,2005 Duke University
Author:         Steven Hartman
                Duke Free-Electron Laser Laboratory
                <hartman@fel.duke.edu>

Version:        1.1
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

********************************************************************/


/* Note: The board range selection register does not work appropriately
 * on our board. The range setting at the end of the init routine has 
 * therefore been commented out. The board defaults to 0 to 10 V if the 
 * range register is not touched. 
 */


#include <vxWorks.h>
#include <sysLib.h>
#include <stdlib.h>
#include <stdio.h>
#include <vxLib.h>
#include <vme.h>

#include <dbDefs.h>
#include <drvSup.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvHy2670.h"

static long hy2670_io_report(int level);
static long hy2670_init(void);

/* these can be redefined in st.cmd before iocInit if needed */
int hy2670_base = 0xE000;	/* default address for card as delivered */
int numHy2670cards = 2;		/* number of cards in crate */

#define DAC_A_RANGE ZERO_TO_TEN
#define DAC_B_RANGE ZERO_TO_TEN

struct {
	long number;
	DRVSUPFUN report;
	DRVSUPFUN init;
} drvHy2670={
	2,
	hy2670_io_report,
	hy2670_init};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvHy2670);
#endif

static unsigned short **paoHy2670;
static int hy2670_addr;

static long hy2670_init(void)
{
	short shval;
	unsigned short **boards_present;
	int status, i;
	register struct hytec_2670 *board;

	paoHy2670 = (unsigned short **)calloc(numHy2670cards,sizeof(*paoHy2670));
	if (!paoHy2670) 
		return(ERROR);
	
	boards_present = paoHy2670;

	if ( (status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)(long)hy2670_base, (char **)&hy2670_addr) ) != OK ) {
		printf("Addressing error in Hy2670 driver\n");
		return(ERROR);
	}

	board = (struct hytec_2670 *)((int) hy2670_addr);

	for ( i = 0; i < numHy2670cards; i++, board++, boards_present++) {
		if ( vxMemProbe( (char *)(&board->dac_a),READ,sizeof(short),(char *)&shval) == OK ) {
			*boards_present = (unsigned short *)board;
			/*
			 * board->range = ( (DAC_A_RANGE << 2) | DAC_B_RANGE );
			*/
		}
	}

	return(OK);
}

long hy2670_write(unsigned short card, unsigned short chan, unsigned long *pval) {
	register struct hytec_2670 *pHy2670;

	if ( (pHy2670 = (struct hytec_2670 *)paoHy2670[card]) == 0) {
		return(ERROR);
	}
	if ( chan == 0 ) 
		pHy2670->dac_a = *pval;
	else if ( chan == 1 ) 
		 pHy2670->dac_b = *pval;
	else return(ERROR);

	return(0);
}

static long hy2670_io_report(int level)
{
	int i;
	struct hytec_2670 *pHy2670;

	for (i = 0; i < numHy2670cards; i++) {
		if (paoHy2670[i]) {
			printf("  AO: Hytec 2670 card number %d is present.\n",i);
			if ( level > 0 ) {
				pHy2670 = (struct hytec_2670 *)paoHy2670[i];
				printf("  Address = %p\n",pHy2670);
			}
				
		}
	}

	return(0);
}
