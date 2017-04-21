/* devMbboJoergerVdacm.c */

/* EPICS mbbo record for VDACM */
/* Use to set clock mode of module */

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
#include <mbboRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvJoergerVdacm.h"

extern int vdacm_clock(unsigned short card, unsigned char *mode, unsigned char direction);

static long init_record(struct mbboRecord *pmbbo);
static long write_mbbo(struct mbboRecord *pmbbo);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_mbbo;
} devMbboVdacm = {
	5,
	NULL,
	NULL,
	init_record,
	NULL,
	write_mbbo};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devMbboVdacm);
#endif

static long init_record(struct mbboRecord *pmbbo) 
{
	/* mbbo.out must be VME_IO */
	switch (pmbbo->out.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pmbbo,
			"devMbboVdacm (init_record) Illegal OUT field");
		return(S_db_badField);
	}

	pmbbo->shft = 0x0;
	pmbbo->mask = 0x0;

	return 0;	
}

static long write_mbbo(struct mbboRecord *pmbbo)
{
	int status;
	struct vmeio *pvmeio;
	unsigned char *mode = 0;

	pvmeio = (struct vmeio *)&(pmbbo->out.value);
	*mode = pmbbo->rval;

	status = vdacm_clock(pvmeio->card, mode, VDACM_WRITE);

	return status; 

}
