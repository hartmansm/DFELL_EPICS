/* drvWfVmi3122.c */

/* 
 * EPICS driver for VMIVME-3122 high performance 16-bit Analog to Digital 
 * Converter Board used as a waveform digitizer.
 *
 *
 * Copyright (c) 2002,2005 Duke University
 * Author:	Steven Hartman
 *		Duke Free Electron Laser Laboratory
 *		<hartman@fel.duke.edu>
 *
 * Version:	1.1
 * Please check www.fel.duke.edu/epics for the most recent version.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 *
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

#include <dbDefs.h>
#include <dbAccess.h>
#include <drvSup.h>
#include <recSup.h>
#include <recGbl.h>
#include <devSup.h>
#include <link.h>
#include <dbScan.h>
#include <waveformRecord.h>
#include <boRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include <drvWfVmi3122.h>

#define GAIN	GAIN_1X
#define SCAN	SINGLE_SCAN

long vmi3122_report(int level);
int vmi3122_driver(unsigned short card, unsigned int data, unsigned long *pval);
int vmi3122_trig(unsigned short card);

int numVmi3122cards;

static long init_record();
static long init_borecord();
static long get_ioint_info();
static long read_wf();
static long write_bo();
static int vmi3122_addr;


struct {
	long	number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvVmi3122={
	2,
	vmi3122_report,
	NULL};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvVmi3122);
#endif

static int vmi3122_addr;
static unsigned short **pwf_vmi3122;

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN       get_ioint_info;
	DEVSUPFUN	read_wf;
} devWfVmi3122={
	5,
	NULL,
	NULL,
	init_record,
	get_ioint_info,
	read_wf};
	
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devWfVmi3122);
#endif

struct {
        long            number;
        DEVSUPFUN       report;
        DEVSUPFUN       init;
        DEVSUPFUN       init_record;
        DEVSUPFUN       get_ioint_info;
        DEVSUPFUN       write_bo;
}devBoVmi3122={
        5,
        NULL,
        NULL,
        init_borecord,
        NULL,
        write_bo};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devBoVmi3122);
#endif

static IOSCANPVT ioscanpvt;
static void int_service(IOSCANPVT ioscanpvt) {
	scanIoRequest(ioscanpvt);
}

/* This must be called one time from st.cmd before iocInit */
int Vmi3122Config(int numcards,unsigned long baseoffset, unsigned int buffer,unsigned long rate, unsigned int irq_level, unsigned char ivec_base)
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

	pwf_vmi3122 = (unsigned short **)calloc(numVmi3122cards,sizeof(*pwf_vmi3122));
	if(!pwf_vmi3122) {
		printf("Vmi3122Config: failed to allocate space\n");
		return(ERROR);
	}

	boards_present = pwf_vmi3122;

	if ( (status = sysBusToLocalAdrs(VME_AM_STD_SUP_DATA,(char *)baseoffset,(char **)&vmi3122_addr)) != OK){
		printf("Addressing error in Vmi3122Config\n");
		return(ERROR);
	}

	board = (( vmic3122 *)((int)vmi3122_addr));

	for ( i = 0; i < numVmi3122cards; i++, board++, boards_present++) {

		if (vxMemProbe((char *)board,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*boards_present = (unsigned short *)board;

			board->src = 1; /* arbitrary data to initiate sofware reset */

			/* set buffer size, one active channel (0) */
			switch (buffer) {
			case 16:
				board->ccr = 0x0000 | GAIN | SCAN;
				break;
			case 32:
				board->ccr = 0x1800 | GAIN | SCAN;
				break;
			case 64:
				board->ccr = 0x3800 | GAIN | SCAN;
				break;
			case 128:
				board->ccr = 0x6000 | GAIN | SCAN;
				break;
			case 256:
				board->ccr = 0x8800 | GAIN | SCAN;
				break;
			case 512:
				board->ccr = 0xB000 | GAIN | SCAN;
				break;
			case 1024:
				board->ccr = 0xD800 | GAIN | SCAN;
				break;
			default:
				printf("Vmi3122Config: Invalid buffer size (%d)\n",buffer);
				return(ERROR);
			}

			/* control and status register */
			board->csr = LED_OFF | OFFSET_BIN | SOFT_TRIG | END_BUFFER;

			/* scan rate */
			if ( rate > 100000 || rate < 1 ) {
				printf("Vmi3122Config: Rate out of range (%ld)\n",rate);
				return(ERROR);
			}
			board->rcr = (12.5e6 / rate ) - 3;

			/* interrupts */
			scanIoInit(&ioscanpvt);
			if (intConnect(INUM_TO_IVEC(ivec_base+i),(VOIDFUNCPTR)int_service,(int)ioscanpvt) != OK) {
				printf("Vmi3122Config: intConnect failed\n");
				return(ERROR);
			}

			if ( sysIntEnable(irq_level) != OK) {
				printf("Vmi3122Config: intLevel out of range (%d)\n",irq_level);
				return(ERROR);
			}

			board->icr = 0xFF00 | irq_level;
			board->ivr = 0xFF00 | (ivec_base + i);

		}
		else {
			printf("No Vmi3122 card present\n");
			*boards_present = 0;
		}
	}
	return(OK);
}

static long get_ioint_info(int cmd, struct waveformRecord *pr, IOSCANPVT *ppvt) {

	*ppvt = ioscanpvt;
	return(0);
}

static long init_record(pwf)
struct waveformRecord	*pwf;
{

	/* wf.inp must be an VME_IO */
	switch (pwf->inp.type) {
	case (VME_IO) :
		pwf->udf = FALSE;
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pwf,
		    "devWfVmi3122 (init_record) Illegal INP field");
		return(S_db_badField);
	}
	if (pwf->ftvl != DBF_ULONG) {
		recGblRecordError(S_db_badField, (void *)pwf,
		    "devWvVmi3122 (init_record) Illegal FTVL field");
		return(S_db_badField);
	}

	return(0);
}

static long init_borecord(pbo)
struct boRecord *pbo;
{
	/* bo.out must be an VME_IO */
	switch (pbo->out.type) {
	case (VME_IO) :
		pbo->udf = FALSE;
		break;
	default:
		recGblRecordError(S_db_badField,(void *)pbo,
		    "devBoVmi3122 (init_record) Illegal OUT field");
		return(S_db_badField);
	}
	return(0);
}

int vmi3122_driver(unsigned short card, unsigned int signal, unsigned long *pval) {
	register vmic3122 *pwfVMI;
	if (( pwfVMI = ( vmic3122 *)pwf_vmi3122[card]) == 0)
		return(ERROR);
	*pval = pwfVMI->data[signal];

	return(OK);
}

int vmi3122_trig(unsigned short card) {
	register vmic3122 *pboVMI;
	if (( pboVMI = ( vmic3122 *)pwf_vmi3122[card]) == 0)
		return(ERROR);
	pboVMI->stc = 1;

	return(OK);
}

static long read_wf(pwf)
struct waveformRecord	*pwf;
{
	unsigned long *pvalue=0;
	struct vmeio *pvmeio;
	int status=0;
	long i;
	unsigned long numread;
	unsigned long *pdest=pwf->bptr;

	pvmeio = (struct vmeio *)&(pwf->inp.value);
	numread = pwf->nelm;

	for (i=0; i < numread;i++) {
		status=vmi3122_driver(pvmeio->card,i,pvalue);
		*pdest++ = *pvalue;
	}

	pwf->nord = numread;

	return(status);
}

static long write_bo(pbo)
struct boRecord *pbo;
{
	struct vmeio *pvmeio;
	int status;

	pvmeio = (struct vmeio *)&(pbo->out.value);
	status=vmi3122_trig(pvmeio->card);

	return(status);
}

long vmi3122_report(int level)
{
	int i;
	vmic3122 *pwfVMI;

	for (i = 0; i < numVmi3122cards; i++) {
		if (pwf_vmi3122[i]) {
			printf("  waveform: VMIVME-3122 card number %d is present\n",i);
			if ( level > 0 ) {
				pwfVMI = (vmic3122 *)pwf_vmi3122[i];
				printf("  Address = %p, Board ID = 0x%x, CSR = 0x%x, CCR = 0x%x, RCR = 0x%x, ICR = 0x%x, IVR = 0x%x\n",pwfVMI,pwfVMI->bir,pwfVMI->csr,pwfVMI->ccr,pwfVMI->rcr,pwfVMI->icr,pwfVMI->ivr);
			}
		}
	}
	return(OK);
}

