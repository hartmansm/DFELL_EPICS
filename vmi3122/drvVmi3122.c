/* drvVmi3122.c */

/* Copyright (c) 2005 Duke University
 *
 * Steve Hartman <hartman@fel.duke.edu>, Duke FEL Laboratory
 * Version 1.0
 * Please check <www.fel.duke.edu/epics> for the most recent version.
 *
 * This code provides EPICS support for the VMIVME-3122 High-Performance
 * 16-bit ADC board.
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
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 USA
*/

#include <vxWorks.h>
#include <sysLib.h>
#include <vxLib.h>
#include <types.h>
#include <stdioLib.h>
#include <stdlib.h>
#include <vme.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include <dbDefs.h>
#include <drvSup.h>

#include <drvVmi3122.h>

#define GAIN	GAIN_1X
#define SCAN	AUTO_SCAN 

#define NUM_CHANNELS 64 /* number of channels in board */

/* these can be changed in st.cmd before iocInit if necessary */
unsigned int numVmi3122cards = 2;        /* number of cards in crate */
unsigned int vmi3122_base = 0xC00000;    /* base address of card 0 */

long vmi3122_init(void);
long vmi3122_report(int);

struct {
	long	number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvVmi3122={
	2,
	vmi3122_report,
	vmi3122_init};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvVmi3122);
#endif

static int vmi3122_addr;
static unsigned short **pai_vmi3122;

/* initialize the 3122 analog input card */
long vmi3122_init(void)
{
	unsigned short **boards_present;
	short shval;
	long status;
	int i;
	vmic3122 *board;

	pai_vmi3122 = (unsigned short **)calloc(numVmi3122cards,sizeof(*pai_vmi3122));
	if(!pai_vmi3122) {
		printf("Vmi3122Config: failed to allocate space\n");
		return(ERROR);
	}

	boards_present = pai_vmi3122;

	if ( (status = sysBusToLocalAdrs(VME_AM_STD_SUP_DATA,(char *)vmi3122_base,(char **)&vmi3122_addr)) != OK){
		printf("Addressing error in Vmi3122Config\n");
		return(ERROR);
	}

	board = (( vmic3122 *)((int)vmi3122_addr));

	for ( i = 0; i < numVmi3122cards; i++, board++, boards_present++) {

		if (vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*boards_present = (unsigned short *)board;

			board->src = 1; /* arbitrary data to initiate sofware reset */
			board->ccr =  0x5800 | GAIN | SCAN;	

			/* control and status register */
			board->csr = LED_OFF | OFFSET_BIN;

		}
	}
	return(OK);
}

int vmi3122_read(unsigned short card, unsigned int signal, unsigned long *pval) {
	register vmic3122 *paiVMI;
	if (( paiVMI = ( vmic3122 *)pai_vmi3122[card]) == 0)
		return(ERROR);
	*pval = paiVMI->data[signal];

	return(OK);
}

long vmi3122_report(int level)
{
	int i;
	vmic3122 *paiVMI;

	for (i = 0; i < numVmi3122cards; i++) {
		if (pai_vmi3122[i]) {
			printf("  ai: VMIVME-3122 card number %d is present\n",i);
			if ( level > 0 ) {
				paiVMI = (vmic3122 *)pai_vmi3122[i];
				printf("  Address = %p, Board ID = 0x%x, CSR = 0x%x, CCR = 0x%x, RCR = 0x%x, ICR = 0x%x, IVR = 0x%x\n",paiVMI,paiVMI->bir,paiVMI->csr,paiVMI->ccr,paiVMI->rcr,paiVMI->icr,paiVMI->ivr);
			}

		}
	}
	return(OK);
}

