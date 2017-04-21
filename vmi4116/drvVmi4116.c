/* drvVmi4116.c */

/*********************************************************************

EPICS driver for VMIC 4116 16-bit, 8-channel Digital to Analog 
Converter board

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

#include <vxWorks.h>
#include <stdlib.h>
#include <stdio.h>
#include <vxLib.h>
#include <sysLib.h>
#include <vme.h>

#include <dbDefs.h>
#include <drvSup.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvVmi4116.h"

#define NUM_CHANNELS 8

/* these can be changed in st.cmd before iocInit if necessary */
unsigned int numVmi4116cards = 6;	/* number of cards in crate */
unsigned int vmi4116_base = 0x0060;	/* base address of card 0 */

struct {
	long		number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvVmi4116 = {
	2,
	vmi4116_report,
	vmi4116_init };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvVmi4116);
#endif

static unsigned short **pao_vmi4116;
static int vmi4116_addr;

long vmi4116_init(void)
{
	unsigned short **boards_present;
	short shval;
	int status;
	register short i;
	vmic4116 * board;

	pao_vmi4116 = (unsigned short **)
		calloc(numVmi4116cards,sizeof(*pao_vmi4116));
	if ( !pao_vmi4116 )
		return ERROR;
	
	boards_present = pao_vmi4116;

	if ((status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)(long)vmi4116_base, (char **)&vmi4116_addr)) != OK ) {
		printf("drvVmi4116: Addressing error in vmi4116_init.\n");
		return ERROR;
	}

	board = ((vmic4116 *)((int)vmi4116_addr));

	for ( i = 0; i < numVmi4116cards; i++, board++, boards_present++ ) {
		if(vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*boards_present = (unsigned short *)board;
			board->csr = (LED_OFF | AO_P3);
		}
		else {
			*boards_present = 0;
		}
	}
	return(0);
}

int vmi4116_write(unsigned short card, unsigned int signal, unsigned long *pval)
{
	register vmic4116 *paoVMI;
	if ((paoVMI = (vmic4116 *)pao_vmi4116[card]) == 0 )
		return ERROR;
	
	paoVMI->chan[signal] = *pval;

	return(0);
}

long vmi4116_report(int level)
{
	int i;
	vmic4116 *paoVMI;

	for ( i = 0; i < numVmi4116cards; i++ ) {
		if ( pao_vmi4116[i]) {
			printf("  AO: VMI4116 DAC: card %d is present.\n",i);
			if ( level > 0 ) {
				paoVMI = (vmic4116 *)pao_vmi4116[i];
				printf("    Address = %p, CSR = 0x%x\n",paoVMI,paoVMI->csr);
				if ( level > 1 ) {
					printf("    DAC registers are write only\n");
				}
			}
		}
	}
	return(0);
}

