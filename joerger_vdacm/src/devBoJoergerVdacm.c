/* devBoJoergerVdacm.c */

/* EPICS bo record for Joerger VDACM */
/* Use for optional software trigger of module */

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
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <recGbl.h>
#include <boRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvJoergerVdacm.h"

extern int vdacm_trigger(unsigned short card, unsigned char *mode, unsigned char direction);

static long init_record(struct boRecord *pbo);
static long write_bo(struct boRecord *pbo);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_bo;
} devBoVdacm = {
	5,
	NULL,
	NULL,
	init_record,
	NULL,
	write_bo};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devBoVdacm);
#endif

static long init_record(struct boRecord *pbo) 
{
	/* output must be VME_IO */
	switch (pbo->out.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pbo,
			"devboVdacm (init_record) Illegal OUT field");
		return(S_db_badField);
	}

	return 0;	
}

static long write_bo(struct boRecord *pbo)
{
	int status;
	struct vmeio *pvmeio;
	unsigned char *mode = 0;

	pvmeio = (struct vmeio *)&(pbo->out.value);

	*mode = pbo->rval;
	status = vdacm_trigger(pvmeio->card, mode, VDACM_WRITE);

	return status; 

}
