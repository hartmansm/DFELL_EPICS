/* devAoHy2670.c */

/********************************************************************* 

EPICS Device Support Routines for Hytec VDD 2670 18-bit, 2-channel
Digital to Analog Converter board

Copyright (c) 2003,2005 Duke University
Author:         Steven Hartman
                Duke Free-Electron Laser Laboratory
                <hartman@fel.duke.edu>

Version:        1.1

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

************************************************************************/

#include <vxWorks.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <alarm.h>
#include <dbDefs.h>
#include <dbAccess.h>
#include <alarm.h>
#include <recGbl.h>
#include <recSup.h>
#include <devSup.h>
#include <special.h>
#include <aoRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvHy2670.h"

static long init_record();
static long write_ao();
static long special_linconv();

struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN write_ao;
	DEVSUPFUN special_linconv;
}devAoHy2670={
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	write_ao,
	special_linconv};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAoHy2670);
#endif

static long init_record(pao)
struct aoRecord *pao;
{
	/* must be VME_IO */
	switch (pao->out.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pao,
			"devAoHy2670 (init_record) Illegal OUT field");
		return(S_db_badField);
	}

	/* set linear conversion slope -- 18-bit */
	pao->eslo = (pao->eguf - pao->egul) / 262143.0;

	return(2);
}

static long write_ao(pao)
struct aoRecord *pao;
{
	struct vmeio *pvmeio;
	long status;
	unsigned long value;

	pvmeio = (struct vmeio *)&(pao->out.value);
	value = pao->rval;
	status = hy2670_write(pvmeio->card,pvmeio->signal,&value);
	if (status != OK ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
	}

	return(status);
}

static long special_linconv(pao,after)
struct aoRecord *pao;
int after;
{
	if(!after) return(0);
	pao->eslo = (pao->eguf - pao->egul) / 262143.0;
	return(0);
}
