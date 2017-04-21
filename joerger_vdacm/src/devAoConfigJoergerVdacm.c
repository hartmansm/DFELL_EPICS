/* devAoConfigJoergerVdacm.c */

/* EPICS ao record for Joerger VDACM */
/* Use to configure waveform length or to waveform start location */

/* Use parm field of OUT to specify length (l or L) or start (s or S). */

/*
 Copyright (C) 2005  Duke University
 Author: Steven Hartman <hartman@fel.duke.edu>, Duke FEL Laboratory
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


static long init_record(struct aoRecord *pao);
static long write_ao(struct aoRecord *pao);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_ao;
	DEVSUPFUN	special_linconv;
} devAoConfigVdacm = {
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	write_ao,
	NULL};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAoConfigVdacm);
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
			"devAoConfigVdacm (init_record) Illegal OUT field");
		return(S_db_badField);
	}
	
	parm = pao->out.value.vmeio.parm;

	/* check for valid parm */
	if ( *parm == 'L' || *parm == 'l' || *parm == 'S' || *parm == 's' ) {
		return 2; /* do not convert */
	}
	else {
		recGblRecordError(S_db_badField,(void *)pao,
			"devAoConfigVdacm (init_record) Illegal PARM field");
		return(S_db_badField);
	}

}

static long write_ao(struct aoRecord *pao)
{
	int status = -1;
	struct vmeio *pvmeio;
	unsigned short *value = 0;
	char *parm;


	pvmeio = (struct vmeio *)&(pao->out.value);
	*value = pao->rval;

	parm = pvmeio->parm;

	if ( *parm == 'L' || *parm == 'l' ) {
		status = vdacm_length(pvmeio->card, value, VDACM_WRITE);
	}

	if ( *parm == 'S' || *parm == 's' ) {
		status = vdacm_start(pvmeio->card, value, VDACM_WRITE);
	}

	if ( status ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
	}

	return status; 

}
