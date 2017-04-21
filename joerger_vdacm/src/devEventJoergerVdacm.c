/* devEventJoergerVdacm.c */

/* EPICS event record for Joerger VDACM */
/* Use to monitor interrupt triggered at end of waveform scan. */

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
#include <dbScan.h>
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <recGbl.h>
#include <eventRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvJoergerVdacm.h"

extern IOSCANPVT *vdacm_ioscanpvt;

static long init_record(struct eventRecord *pevent);
static long get_ioint_info(int cmd, struct eventRecord *pevent, IOSCANPVT *ppvt);
static long read_event(struct eventRecord *pevent);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_event;
} devEventVdacm = {
	5,
	NULL,
	NULL,
	init_record,
	get_ioint_info,
	read_event};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devEventVdacm);
#endif

static long init_record(struct eventRecord *pevent) 
{
	/* input must be VME_IO */
	switch (pevent->inp.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pevent,
			"deveventVdacm (init_record) Illegal INP field");
		return(S_db_badField);
	}

	return 0;	
}

static long get_ioint_info(int cmd, struct eventRecord *pevent, IOSCANPVT *ppvt)
{
	struct vmeio *pvmeio;
	IOSCANPVT ioscanpvt;

	pvmeio = (struct vmeio *)&(pevent->inp.value);
	ioscanpvt = vdacm_ioscanpvt[pvmeio->card];
	*ppvt = ioscanpvt;

	return 0;
}

static long read_event(struct eventRecord *pevent)
{

	/* Don't need to do anything. This record is just used to signal
	 * the end-of-waveform I/O interrupt. Use the val field to specify
	 * which event to process on interrupt. */

	return 0; 

}
