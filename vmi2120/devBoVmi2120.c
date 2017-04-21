/* devboVmi2120.c */

/************************************************************************

EPICS Device Support Routines for VMI 2120 64-bit High Voltage Digital
Output Megamodule.

Author:		Steven Hartman
		Duke Free-Electron Laser Laboratory
		<hartman@fel.duke.edu>
	
Version:        1.1
Please check <www.fel.duke.edu/epics> for the most recent version.

Copyright (c) 2004,2005 Duke Univerisity

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
#include <biRecord.h>
#include <boRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvVmi2120.h"

static long init_bo();
static long write_bo();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_bo;
} devBoVmi2120 = {
	5,
	NULL,
	NULL,
	init_bo,
	NULL,
	write_bo };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devBoVmi2120);
#endif

static long init_bo(pbo)
struct boRecord *pbo;
{
	switch (pbo->out.type) {
	case (VME_IO) :
		pbo->udf = FALSE;
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pbo,
			"devBoVmi2120 (init_record) Illegal OUT field");
		return(S_db_badField);
	}
	return(0);
}

static long write_bo(pbo)
struct boRecord *pbo;
{
	unsigned char value;
	struct vmeio *pvmeio;
	long status;

	pvmeio = (struct vmeio *)&(pbo->out.value);
	value = pbo->rval;
	status = vmi2120_write(pvmeio->card,pvmeio->signal,&value);
	if(status != 0) 
		recGblSetSevr(pbo,WRITE_ALARM,INVALID_ALARM);

	return(status);
}

