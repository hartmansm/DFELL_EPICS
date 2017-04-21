/* drvVmi1182.c */

/*
 * EPICS driver for VMIVME-1182 
 * 64-Channel Optically-Coupled Digital Input Board
 *
 * Copyright (C) 2004,2005 Duke University 
 * Author:	Steven Hartman
 *		Duke Free Electron Laser Laboratory
 *		<hartman@fel.duke.edu>
 * Version: 1.1
 * Please check <www.fel.duke.edu/epics> for the most recent version.
 *
 */

/*
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
#include <intLib.h>
#include <iv.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include <dbDefs.h>
#include <dbScan.h>
#include <drvSup.h>

#include "drvVmi1182.h"

#define NUM_CHANNELS 64
#define MAX_VMI1182 2	/* set to max number of cards in crate */

long vmi1182_init(void);
int vmi1182_read(unsigned short card, unsigned short word, unsigned short *pval)
;
long vmi1182_report(int level);

/* these can be overidden in st.cmd before iocInit if necessary */
int vmi1182_base = 0x100000;	/* base address of card 0 in A24 */
int vmi1182_irq_level = 6;
int vmi1182_ivec_base = 0x60;

struct {
	long		number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvVmi1182 = {
	2,
	vmi1182_report,
	vmi1182_init };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvVmi1182);
#endif

static unsigned short **pvmi1182;
static int vmi1182_addr;

IOSCANPVT vmi1182_ioscanpvt[MAX_VMI1182];

static void int_service(IOSCANPVT ioscanpvt) {
	scanIoRequest(ioscanpvt);
}


long vmi1182_init(void)
{
	unsigned short **boards_present;
	short shval;
	long status;
	short i,j;
	vmic1182 * board;

	pvmi1182 = (unsigned short **)
		calloc(MAX_VMI1182, sizeof(*pvmi1182));
	if(!pvmi1182) {
		return ERROR;
	}

	boards_present = pvmi1182;

	if ((status = sysBusToLocalAdrs(VME_AM_STD_SUP_DATA,(char *)(long)vmi1182_base, (char **)&vmi1182_addr)) != OK ) {
		printf("drvVmi1182: Addressing error in vmi1182_init\n");
		return ERROR;
	}

	board = ((vmic1182 * ) ((int)vmi1182_addr));

	for ( i = 0; i < MAX_VMI1182; i++, board ++, boards_present ++ ) {
		if (vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*boards_present = (unsigned short *)board;
			if ( board->str == SELFTEST_OK ) {
				board->csr =  LED_OFF | TESTMODE_OFF;
			}
			else {
				printf("drvVmi1182: Board %d is not present or did not pass self test.\nSelf Test Results Register = 0x%x\n",i,board->str);
				return ERROR;
			}

			/* interrupt on change of state */
			for ( j = 0; j < NUM_CHANNELS; j++ ) {
				/* 
				 * COS interrupt on both rising and falling 
				 * edges, debounce time of 40 ms 
				 */
				board->dsr[j] = COS_BOTH | 0x0040;
			}
			scanIoInit(&vmi1182_ioscanpvt[i]);
			if (intConnect(INUM_TO_IVEC(vmi1182_ivec_base+i),(VOIDFUNCPTR)int_service,(int)vmi1182_ioscanpvt[i]) != OK) {
				printf("vmi1182_init: intConnect failed\n");
				return(ERROR);
			}

			if ( sysIntEnable(vmi1182_irq_level) != OK) {
				printf("vmi1182_init: intLevel out of range (%d)\n",vmi1182_irq_level);
				return(ERROR);
			}
			board->ivr = ( (vmi1182_irq_level << 8) | (vmi1182_ivec_base + i) ); 
		}
		else *boards_present = 0;
	}

	return(0);
}

int vmi1182_read( 
	unsigned short card, 
	unsigned short word,
	unsigned short *pval) {

	register vmic1182 *pbiVMI;

	if ( (pbiVMI = (vmic1182 *)pvmi1182[card]) == 0 )
		return(-1);
	
	*pval = pbiVMI->data[word];
	return(0);
}

long vmi1182_report(int level) {
	int i, j;
	vmic1182 *pVMI;

	for (i = 0; i < MAX_VMI1182; i++) {
		if (pvmi1182[i]) {
			printf("  bi: VMIVME-1182 card %d is present.\n",i);
			if (level > 0) {
				pVMI = (vmic1182 *)pvmi1182[i];
				printf("    Address = %p, Board ID = 0x%.4x, CSR = 0x%.4x\n",pVMI,pVMI->bid,pVMI->csr);
				for (j = 0; j < 4; j++) { 
					printf("    Input Data Register %d = 0x%.4x\n",j,pVMI->data[j]);
				}
			}

		}
	}

	return(0);
}

