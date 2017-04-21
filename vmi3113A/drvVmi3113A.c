/* drvVmi3113A.c */

/*********************************************************************

EPICS driver for VMI-3113A scanning 12-bit, 64-channel Analog to
Digital Converter board

Copyright (c) 2001,2003,2005 Duke University
Author:         Steven Hartman
                Duke Free-Electron Laser Laboratory
                <hartman@fel.duke.edu>

Version:        1.2
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

#include "drvVmi3113A.h"

#define NUM_CHANNELS 64	/* number of channels in board */

/* these can be changed in st.cmd before iocInit if necessary */
unsigned int numVmi3113Acards = 2;	/* number of cards in crate */
unsigned int vmi3113A_base = 0x8500;	/* base address of card 0 */

/* set card 0 jumpers J9:A08-A15 to 0x8500 */
/* set card 1 jumpers J9:A08-A15 to 0x8600 */
/* set J9:17,18 and J9:19,20 to open for short supervisory access */
/* All other jumbers to factory configuration */

struct {
	long		number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvVmi3113A={
	2,
	vmi3113A_report,
	vmi3113A_init};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvVmi3113A);
#endif

static unsigned short **pai_vmi3113A;
static int vmi3113A_addr;

/* initialize the 3113A analog input */
long vmi3113A_init(void)
{	
	unsigned short **boards_present;
	short		shval;
	int		status;
	register short	i;
	vmic3113A * board;

	pai_vmi3113A = (unsigned short **)
		calloc(numVmi3113Acards,sizeof(*pai_vmi3113A));
	if(!pai_vmi3113A){
		return ERROR;
	}

	boards_present = pai_vmi3113A;

	if ((status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)(long)vmi3113A_base, (char **)&vmi3113A_addr)) != OK){
		printf("Addressing error in vmi3113A driver");
		return ERROR;
	}

	board = ((vmic3113A * )((int)vmi3113A_addr));

	for ( i = 0; i < numVmi3113Acards; i++, board ++, boards_present ++ ) {
		if (vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*boards_present = (unsigned short *)board;
			board->csr = ( SOFTWARE_RESET ); /* clear the board */
			board->csr = ( FAIL_LED_OFF | AUTOSCANNING ); /* begin autoscanning mode */
		}
		else {
			*boards_present = 0;
		}	
	}
	return(0);
}

/* 3113A analog input driver */
int vmi3113A_driver(
	unsigned short card,
	unsigned int signal,
	unsigned long *pval
) {

	register vmic3113A *paiVMI;
	if ((paiVMI= (vmic3113A *)pai_vmi3113A[card]) == 0)
		return(-1);
	*pval = paiVMI->chan[signal];

	return(0);
}

/* dbior */
long vmi3113A_report(int level)
{
	int i, j;
	vmic3113A *paiVMI;
	unsigned long value;

	for (i = 0; i < numVmi3113Acards; i++) {
		if (pai_vmi3113A[i]) {
			printf("  AI: VMIVME-3113A ADC: card %d is present.\n",i);
			if ( level > 0 ){
				paiVMI = (vmic3113A *)pai_vmi3113A[i];
				printf("    Address = %p, Board ID = 0x%x, CSR = 0x%x, CFR = 0x%x\n",paiVMI,paiVMI->bid,paiVMI->csr,paiVMI->cfr);
				if ( level > 1 ) {
					for ( j = 0; j < NUM_CHANNELS; j++) {
						vmi3113A_driver(i,j,&value);
						printf("    Signal %.2d = 0x%.4lx\n",j,value);
					}
				}
			}
		}
	}
	return(0);
}
