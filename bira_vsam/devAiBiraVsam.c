/* devAiBiraVsam.c */

/*********************************************************************

EPICS Device Support Routines for BiRa 7305 VSAM (Smart Analog Monitor)
Author:	Steven Hartman
	Duke Free-Electron Laser Laboratory
	<hartman@fel.duke.edu>

Version:        1.1
Please check <www.fel.duke.edu/epics> for the most recent version.

Copyright (c) 2004,2005 Duke University

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
#include <types.h>
#include <stdioLib.h>
#include <string.h>

#include <dbDefs.h>
#include <dbAccess.h>
#include <alarm.h>
#include <recGbl.h>
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <dbScan.h>
#include <aiRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvBiraVsam.h"

#define FACTOR  33554431.0      /* 2^25 - 1 */
#define SCALE   10.24           /* +- SAM range */


static long init_record();
static long read_ai();
static long special_linconv();

struct {
        long            number;
        DEVSUPFUN       report;
        DEVSUPFUN       init;
        DEVSUPFUN       init_record;
        DEVSUPFUN       get_ioint_info;
        DEVSUPFUN       read_ai;
        DEVSUPFUN       special_linconv;
} devAiBiraVsam = {
        6,
        NULL,
        NULL,
        init_record,
        NULL,
        read_ai,
        special_linconv
};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAiBiraVsam);
#endif

/* init_record */
static long init_record(pai)
struct aiRecord *pai;
{
	/* ai.inp must be VME_IO */
	switch (pai->inp.type) {
	case (VME_IO) :
		pai->udf = FALSE;
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pai,
			"devAiBiraVsam (init_record) Illegal INP field");
		return(S_db_badField);
	}

	/*  set linear conversion slope*/
	/* 26-bit dynamic range */
	pai->eslo = (pai->eguf - pai->egul)/67108863.0;

	return(0);
}

/* read_ai */
static long read_ai(pai)
struct aiRecord *pai;
{
	float value;
	struct vmeio *pvmeio;
	int status;

	pvmeio = (struct vmeio *)&(pai->inp.value);
	status = vsam_driver(pvmeio->card,pvmeio->signal,&value);

	if ( value == 99.999 ) { /* failed calibration */
		recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
	}
	if ( value == 50.0 ) { /* input changing too fast */
		recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
	}

	/* convert floating point readback to a scaled rval so that 
	   linear conversion will work */
	pai->rval = (long) ( (value * FACTOR / SCALE ) + FACTOR );
	pai->udf = FALSE;
	return(0);
}

static long special_linconv(pai,after)
	struct aiRecord     *pai;
	int after;
{
	if(!after) return(0);
	pai->eslo = (pai->eguf - pai->egul)/67108863.0; /* 26 bits */
	return(0);
}

