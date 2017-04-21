/* drvVmi2536.c */

/*
 * EPICS driver for VMIVME-2536 
 * 32-Channel Optically-Coupled Digital I/O Board
 *
 * Version:	1.1
 * Please check www.fel.duke.edu/epics for most recent version.
 *
 * Copyright (C) 2003,2005  Duke University
 * Steven Hartman <hartman@fel.duke.edu>
 * Duke Free Electron Laser Laboratory
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#include "drvVmi2536.h"

/* these can be overidden in st.cmd before iocInit if necessary */
unsigned int numVmi2536cards = 1;	/* number of cards in crate */
unsigned int vmi2536_base = 0xff00;	/* base address of card 0 */

struct {
	long		number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvVmi2536 = {
	2,
	vmi2536_report,
	vmi2536_init };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvVmi2536);
#endif

static unsigned short **pvmi2536;
static int vmi2536_addr;

long vmi2536_init(void)
{
	unsigned short **pcards_present;
	short shval;
	long status;
	short i;
	vmic2536 * board;

	pvmi2536 = (unsigned short **)
		calloc(numVmi2536cards, sizeof(*pvmi2536));
	if(!pvmi2536) {
		return ERROR;
	}

	pcards_present = pvmi2536;

	if ((status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)(long)vmi2536_base, (char **)&vmi2536_addr)) != OK ) {
		printf("drvVmi2536: Addressing error in vmi2536_init\n");
		return ERROR;
	}

	board = ((vmic2536 * ) ((int)vmi2536_addr));

	for ( i = 0; i < numVmi2536cards; i++, board ++, pcards_present ++ ) {
		if (vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*pcards_present = (unsigned short *)board;
			board->csr = ( LED_OFF | OUTPUTS_ON | INPUTS_ON );
		}
			else *pcards_present = 0;
	}

	return(0);
}

int vmi2536_read(
	unsigned short card, 
	unsigned int signal,
	unsigned long *pval) {

	register vmic2536 *pbiVMI;
	if ( (pbiVMI = (vmic2536 *)pvmi2536[card]) == 0 )
		return(-1);

	if (pbiVMI->input & (1 << signal) )
		*pval = 1;
	else
		*pval = 0;

	return(0);
}

int vmi2536_write(
	unsigned short card, 
	unsigned int signal,
	unsigned long *pval) {

	register vmic2536 *pboVMI;
	if ( (pboVMI = (vmic2536 *)pvmi2536[card]) == 0)
		return(-1);

	if ( *pval )  
		pboVMI->output |= ( 1 << signal );
	else 
		pboVMI->output &= ~( 1 << signal );

	return(0);
}

long vmi2536_report(int level) {
	int i;
	vmic2536 *pVMI;

	for (i = 0; i < numVmi2536cards; i++) {
		if (pvmi2536[i]) {
			printf("VMIVME-2536 card %d is present.\n",i);
			if (level > 0) {
				pVMI = (vmic2536 *)pvmi2536[i];
				printf("    Board ID = 0x%.4x, CSR = 0x%.4x\n",pVMI->bid,pVMI->csr);
				printf("    32-bit Input Register = 0x%.8lx, 32-bit Output Register = 0x%.8lx\n",pVMI->input,pVMI->output);
			}

		}
	}

	return(0);
}
