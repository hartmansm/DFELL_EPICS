/* drvVmi4132.c */

/*
 EPICS driver for VMIVME-4132 32-Channel, 12-bit Analog Output Board (DAC)

 Copyright (c) 2003,2005 Duke University
 Author:	Steven Hartman
		Duke Free Electron Laser Laboratory
		<hartman@fel.duke.edu>
	
 Version:	1.1
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

#include "drvVmi4132.h"

#define NUM_CHANNELS 32

/* these can be changed in st.cmd before iocInit if necessary */
unsigned int numVmi4132cards = 2;	/* number of cards in crate */
unsigned int vmi4132_base = 0x8E80;	/* base address of card 0 */

struct {
	long		number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvVmi4132 = {
	2,
	vmi4132_report,
	vmi4132_init };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvVmi4132);
#endif

static unsigned short **pao_vmi4132;
static int vmi4132_addr;

long vmi4132_init(void)
{
	unsigned short **boards_present;
	short shval;
	int status;
	register short i;
	vmic4132 * board;

	pao_vmi4132 = (unsigned short **)
		calloc(numVmi4132cards,sizeof(*pao_vmi4132));
	if ( !pao_vmi4132 )
		return ERROR;
	
	boards_present = pao_vmi4132;

	if ((status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)(long)vmi4132_base, (char **)&vmi4132_addr)) != OK ) {
		printf("drvVmi4132: Addressing error in vmi4132_init.\n");
		return ERROR;
	}

	board = ((vmic4132 *)((int)vmi4132_addr));

	for ( i = 0; i < numVmi4132cards; i++, board++, boards_present++ ) {
		if(vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*boards_present = (unsigned short *)board;
			board->csr = (LED_OFF | ONLINE | BINARY_OFFSET);
		}
		else {
			*boards_present = 0;
		}
	}
	return(0);
}

int vmi4132_write(unsigned short card, unsigned int signal, unsigned long *pval)
{
	register vmic4132 *paoVMI;
	if ((paoVMI = (vmic4132 *)pao_vmi4132[card]) == 0 )
		return ERROR;
	
	paoVMI->chan[signal] = *pval;

	return(0);
}

long vmi4132_report(int level)
{
	int i, j;
	vmic4132 *paoVMI;

	for ( i = 0; i < numVmi4132cards; i++ ) {
		if ( pao_vmi4132[i]) {
			printf("AO: VMI4132 DAC: card %d is present.\n",i);
			if ( level > 0 ) {
				paoVMI = (vmic4132 *)pao_vmi4132[i];
				printf("    Address = %p, Board ID = 0x%x, CSR = 0x%x\n",paoVMI,paoVMI->bid,paoVMI->csr);
				if ( level > 1 ) {
					for ( j = 0; j < NUM_CHANNELS; j++) {
						printf("    Signal %.2d = 0x%.3x\n",j,paoVMI->chan[j]);
					}
				}
			}
		}
	}
	return(0);
}

