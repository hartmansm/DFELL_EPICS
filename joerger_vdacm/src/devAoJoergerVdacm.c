/* devAoJoergerVdacm.c */

/* EPICS ao record for Joerger VDACM */
/* Use to set channel as a standard constant output DAC */

/* Use parm field of OUT to specify a DAC output memory location (0 - 32767) */

/*
 Copyright (C) 2005  Duke University
 Author: Steven Hartman <hartman@fel.duke.edu>, Duke FEL Laboratory
 Version: 1.2
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <alarm.h>
#include <dbDefs.h>
#include <dbAccess.h>
#include <recGbl.h>
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <aoRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvJoergerVdacm.h"

extern int vdacm_length(unsigned short card, unsigned short *length, unsigned char direction);
extern int vdacm_start(unsigned short card, unsigned short *location, unsigned char direction);
extern int vdacm_write(unsigned short card, unsigned int signal, unsigned int offset, unsigned int length, signed short *valuebuf);
extern int vdacm_trigger(unsigned short card, unsigned char *mode, unsigned char direction);

static long init_record(struct aoRecord *pao);
static long write_ao(struct aoRecord *pao);
static long special_linconv(struct aoRecord *pao, int after);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_ao;
	DEVSUPFUN	special_linconv;
} devAoVdacm = {
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	write_ao,
	special_linconv};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAoVdacm);
#endif

static long init_record(struct aoRecord *pao) 
{
	char *parm;

	/* ao.out must be VME_IO */
	switch (pao->out.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pao,
			"devAoVdacm (init_record) Illegal OUT field");
		return(S_db_badField);
	}
	
	parm = pao->out.value.vmeio.parm;

	if ( atoi(parm) >= 0  && atoi(parm) < 0x8000 ) {
		pao->eslo = (pao->eguf - pao->egul)/65535.0;
		return 2; /* do not convert */
	}
	else {
		recGblRecordError(S_db_badField,(void *)pao,
			"devAoVdacm (init_record) Illegal PARM field");
		return(S_db_badField);
	}

}

static long write_ao(struct aoRecord *pao)
{
	int status = 0;
	struct vmeio *pvmeio;
	signed short value;
	char *parm;
	unsigned short length;
	unsigned short start;
	unsigned char trigger = 1;


	pvmeio = (struct vmeio *)&(pao->out.value);

	value = pao->rval - 0x8000;
	parm = pvmeio->parm;
	start = atoi(parm);
	length = 0;
	
	/* setup of DAC mode and write to DAC channel */
	status = vdacm_start(pvmeio->card, &start, VDACM_WRITE);
	if ( status ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
		return status;
	}
	status = vdacm_length(pvmeio->card, &length, VDACM_WRITE);
	if ( status ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
		return status;
	}
	status = vdacm_write(pvmeio->card, pvmeio->signal, start, length + 1, &value);
	if ( status ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
		return status;
	}
	status += vdacm_trigger(pvmeio->card, &trigger, VDACM_WRITE);
	if ( status ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
		return status;
	}

	return 0; 

}

static long special_linconv(struct aoRecord *pao, int after)
{
	if (!after) return 0;
	pao->eslo = (pao->eguf - pao->egul)/65535.0;
	return 0;
}
