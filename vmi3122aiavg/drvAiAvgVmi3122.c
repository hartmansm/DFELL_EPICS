/* drvAvgVmi3122.c */

/* Copyright (c) 2003,2005 Duke University
 *
 * Steve Hartman <hartman@fel.duke.edu>, Duke FEL Laboratory
 * Version 1.1
 * Please check <www.fel.duke.edu/epics> for the most recent version.
 *
 * This code provides EPICS support for the VMIVME-3122 High-Performance
 * 16-bit ADC board.
 * 
 * The board is run in autoscan mode until it's internal data buffer is full.
 * At this point an interrupt is triggered which causes the data for 
 * each channel to be averaged over the number of readings and this
 * average is written to the scratch memory of the card. The device support
 * reads this averaged value from the scratch pad register.
 *
 * The number of points averaged is determined by the number of channels
 * which are scanned (user determined). The buffer has a size sufficient
 * for 1024 samples (i.e. 16 samples to average if all 64 channels are in
 * use. 
 *
 * The routine Vmi3122Config() should be called from st.cmd before iocInit
 * to initialize the card with the proper sampling rate, number of channels,
 * offset address, etc.
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
#include <string.h>
#include <iv.h>
#include <intLib.h>
#include <vme.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include <dbDefs.h>
#include <dbAccess.h>
#include <drvSup.h>
#include <recSup.h>
#include <recGbl.h>
#include <devSup.h>
#include <link.h>
#include <alarm.h>
#include <dbScan.h>
#include <aiRecord.h>
#include <drvVmi3122.h>

#define GAIN	GAIN_1X
#define SCAN	AUTO_SCAN 

long vmi3122_report(int level);
int vmi3122_driver(unsigned short card, unsigned int data, unsigned long *pval);
int vmi3122_trig(unsigned short card);

int numVmi3122cards;
unsigned int vmi3122_blocksize;

static long init_record();
static long read_ai();
static long special_linconv();
static int vmi3122_addr;

struct {
	long	number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvAvgVmi3122={
	2,
	vmi3122_report,
	NULL};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvAvgVmi3122);
#endif

static int vmi3122_addr;
static unsigned short **pai_vmi3122;
static void average_data(int);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_ai;
	DEVSUPFUN	special_linconv;
} devAiVmi3122={
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	read_ai,
	special_linconv};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAiVmi3122);
#endif


/* This must be called one time from st.cmd before iocInit */
int Vmi3122Config(int numcards,unsigned long baseoffset, unsigned int blocksize, unsigned long rate, unsigned int irq_level, unsigned char ivec_base)
{
	unsigned short **boards_present;
	short shval;
	long status;
	int i;
	vmic3122 *board;

	numVmi3122cards = numcards;
	
	if ( (baseoffset & 0xFFFF000) != baseoffset) {
		printf("Vmi3122Config: Illegal offset (0x%lx)\n",baseoffset);
		return(ERROR);
	}
	

	pai_vmi3122 = (unsigned short **)calloc(numVmi3122cards,sizeof(*pai_vmi3122));
	if(!pai_vmi3122) {
		printf("Vmi3122Config: failed to allocate space\n");
		return(ERROR);
	}

	boards_present = pai_vmi3122;

	if ( (status = sysBusToLocalAdrs(VME_AM_STD_SUP_DATA,(char *)baseoffset,(char **)&vmi3122_addr)) != OK){
		printf("Addressing error in Vmi3122Config\n");
		return(ERROR);
	}

	board = (( vmic3122 *)((int)vmi3122_addr));

	for ( i = 0; i < numVmi3122cards; i++, board++, boards_present++) {

		if (vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*boards_present = (unsigned short *)board;

			board->src = 1; /* arbitrary data to initiate sofware reset */

			/* user input for block size */
			/* using 1024 words for buffer size and
			   always use the lowest channel block */
			switch (blocksize) {
			case 1 :
				board->ccr =  0xD800 | GAIN | SCAN;	
				break;
			case 8 :
				board->ccr =  0xE000 | GAIN | SCAN;	
				break;
			case 16 :
				board->ccr =  0xE800 | GAIN | SCAN;	
				break;
			case 32 :
				board->ccr =  0xF000 | GAIN | SCAN;	
				break;
			case 64 :
				board->ccr =  0xF800 | GAIN | SCAN;	
				break;
			default :
				printf("Vmi3122Config: Invalid block size (%d)\n",blocksize);
				return(ERROR);
			}
			vmi3122_blocksize = blocksize;


			/* control and status register */
			board->csr = LED_OFF | OFFSET_BIN | SOFT_TRIG | END_BUFFER;

			/* scan rate */
			if ( rate > 100000 || rate < 1 ) {
				printf("Vmi3122Config: Rate out of range (%ld)\n",rate);
				return(ERROR);
			}
			board->rcr = (12.5e6 / rate ) - 3;

			/* interrupts */
			if (intConnect(INUM_TO_IVEC(ivec_base+i),(VOIDFUNCPTR)average_data,i) != OK) {
				printf("Vmi3122Config: intConnect failed\n");
				return(ERROR);
			}

			if ( sysIntEnable(irq_level) != OK) {
				printf("Vmi3122Config: intLevel out of range (%d)\n",irq_level);
				return(ERROR);
			}

			board->icr = 0xFF00 | irq_level;
			board->ivr = 0xFF00 | (ivec_base + i);

			/* start trigger */
			board->stc = 1; /* arbitrary data */

		}
		else {
			printf("No Vmi3122 card present\n");
			*boards_present = 0;
		}
	}
	return(OK);
}

static long init_record(pai)
struct aiRecord	*pai;
{

	/* ai.inp must be an VME_IO */
	switch (pai->inp.type) {
	case (VME_IO) :
		pai->udf = FALSE;
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pai,
		    "devAiVmi3122 (init_record) Illegal INP field");
		return(S_db_badField);
	}

	/* set linear conversion slope*/
	pai->eslo = (pai->eguf - pai->egul)/65535.0; /* 16-bit */

	return(0);
}

int vmi3122_driver(unsigned short card, unsigned int signal, unsigned long *pval) {
	register vmic3122 *paiVMI;
	if (( paiVMI = ( vmic3122 *)pai_vmi3122[card]) == 0)
		return(ERROR);
	*pval = paiVMI->scratch[signal];

	return(OK);
}

void average_data(int board) {
	
	register vmic3122 *paiVMI;
	unsigned long value[64];
	unsigned int count;
	int i,j;

	if (( paiVMI = ( vmic3122 *)pai_vmi3122[board]) == 0)
		return;
	
	count = 1024 / vmi3122_blocksize;

	for ( i = 0; i < vmi3122_blocksize; i++ ) { 
		value[i] = 0;
		for (j=0; j < count; j++ ) { 
			value[i] += paiVMI->data[(j*vmi3122_blocksize)+i];
		}

	}

	for ( i = 0; i < vmi3122_blocksize; i++ ) {
		paiVMI->scratch[i] = value[i]/count;
	}

	post_event(board);
	
}

static long read_ai(pai)
struct aiRecord	*pai;
{
	unsigned long value;
	struct vmeio *pvmeio;
	int status=0;

	pvmeio = (struct vmeio *)&(pai->inp.value);

	status=vmi3122_driver(pvmeio->card,pvmeio->signal,&value);

	if ( status == 0 )
		pai->rval = value;
	else
		recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);

	return(status);
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

static long special_linconv(pai,after)
    struct aiRecord     *pai;
    int after;
{
    if(!after) return(0);
    /* set linear conversion slope*/
    pai->eslo = (pai->eguf - pai->egul)/65535.0; /* 16-bit */
    return(0);
}
