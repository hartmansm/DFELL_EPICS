/* drvVmi2120.c */

/*
 * EPICS driver for VMIVME-2120 
 * 64-Channel High Voltage Digital Output Board
 *
 */

/*
 Copyright (C) 2004,2005  Duke University
 Author: Steven Hartman <hartman@fel.duke.edu>, Duke FEL Lab

 Version: 1.1
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
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#include "drvVmi2120.h"

/* these can be overidden in st.cmd before iocInit if necessary */
unsigned int numVmi2120cards = 1;	/* number of cards in crate */
unsigned int vmi2120_base = 0xff00;	/* base address of card 0 */
				/* set csr address to base + 0x08 */

struct {
	long		number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvVmi2120 = {
	2,
	vmi2120_report,
	vmi2120_init };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvVmi2120);
#endif

static unsigned short **pvmi2120;
static int vmi2120_addr;

long vmi2120_init(void)
{
	unsigned short **boards_present;
	short shval;
	long status;
	short i;
	vmic2120 * board;

	pvmi2120 = (unsigned short **)
		calloc(numVmi2120cards, sizeof(*pvmi2120));
	if(!pvmi2120) {
		return ERROR;
	}

	boards_present = pvmi2120;

	if ((status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)(long)vmi2120_base, (char **)&vmi2120_addr)) != OK ) {
		printf("drvVmi2120: Addressing error in vmi2120_init\n");
		return ERROR;
	}

	board = ((vmic2120 * ) ((int)vmi2120_addr));

	for ( i = 0; i < numVmi2120cards; i++, board ++, boards_present ++ ) {
		if (vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*boards_present = (unsigned short *)board;
			board->csr = ACTIVE;
		}
			else *boards_present = 0;
	}

	return(0);
}

int vmi2120_write(
	unsigned short card, 
	unsigned short signal,
	unsigned char *pval) {

	unsigned short word;
	register vmic2120 *pboVMI;
	if ( (pboVMI = (vmic2120 *)pvmi2120[card]) == 0)
		return(-1);

	word = signal/8;
	switch (word) {
	case 7 :
		if ( *pval )  
			pboVMI->dr0 |= ( 1 << ( signal - 56 ) );
		else 
			pboVMI->dr0 &= ~( 1 << ( signal - 56 ) );
		break;
	case 6 :
		if ( *pval )  
			pboVMI->dr1 |= ( 1 << ( signal - 48 ) );
		else 
			pboVMI->dr1 &= ~( 1 << ( signal - 48 ) );
		break;
	case 5 :
		if ( *pval )  
			pboVMI->dr2 |= ( 1 << ( signal - 40 ) );
		else 
			pboVMI->dr2 &= ~( 1 << ( signal - 40 ) );
		break;
	case 4 :
		if ( *pval )  
			pboVMI->dr3 |= ( 1 <<  (signal - 32 ) );
		else 
			pboVMI->dr3 &= ~( 1 << (signal - 32 ) );
		break;
	case 3 :
		if ( *pval )  
			pboVMI->dr4 |= ( 1 << ( signal - 24 ) );
		else 
			pboVMI->dr4 &= ~( 1 << ( signal - 24 ) );
		break;
	case 2 :
		if ( *pval )  
			pboVMI->dr5 |= ( 1 << ( signal - 16 ) );
		else 
			pboVMI->dr5 &= ~( 1 << ( signal - 16 ) );
		break;
	case 1 :
		if ( *pval )  
			pboVMI->dr6 |= ( 1 << ( signal - 8 ) );
		else 
			pboVMI->dr6 &= ~( 1 << ( signal - 8 ) );
		break;
	case 0 :
		if ( *pval )  
			pboVMI->dr7 |= ( 1 << signal );
		else 
			pboVMI->dr7 &= ~( 1 << signal );
		break;
	}

	return(0);
}

long vmi2120_report(int level) {
	int i;
	vmic2120 *pVMI;

	for (i = 0; i < numVmi2120cards; i++) {
		if (pvmi2120[i]) {
			printf("  bo: VMIVME-2120 card %d is present.\n",i);
			if (level > 0) {
				pVMI = (vmic2120 *)pvmi2120[i];
				printf("    Address = %p\n",pVMI);
				printf("    Data Register 0 = 0x%.2x\n    Data Register 1 = 0x%.2x\n    Data Register 2 = 0x%.2x\n    Data Register 3 = 0x%.2x\n    Data Register 4 = 0x%.2x\n    Data Register 5 = 0x%.2x\n    Data Register 6 = 0x%.2x\n    Data Register 7 = 0x%.2x\n",pVMI->dr0,pVMI->dr1,pVMI->dr2,pVMI->dr3,pVMI->dr4,pVMI->dr5,pVMI->dr6,pVMI->dr7);
			}

		}
	}

	return(0);
}
