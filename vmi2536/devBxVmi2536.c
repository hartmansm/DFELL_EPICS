/* devBxVmi2536.c */

/* EPICS device support routines for VMIVME-2536 
 * 32-channel bi and 32-channel bo module
*/

/*
 Copyright (C) 2003,2005  Duke University
 Steven Hartman <hartman@fel.duke.edu>
 Duke Free Electron Laser Laboratory

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

#include "drvVmi2536.h"

static long init_bi();
static long init_bo();
static long read_bi();
static long write_bo();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_bi;
} devBiVmi2536 = {
	5,
	NULL,
	NULL,
	init_bi,
	NULL,
	read_bi };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devBiVmi2536);
#endif

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_bo;
} devBoVmi2536 = {
	5,
	NULL,
	NULL,
	init_bo,
	NULL,
	write_bo };

static long init_bi(pbi)
struct biRecord *pbi;
{
	switch (pbi->inp.type) {
	case (VME_IO) :
		pbi->udf = FALSE;
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pbi,
			"devBiVmi2536 (init_record) Illegal INP field");
		return(S_db_badField);
	}
	return(0);
}

static long init_bo(pbo)
struct boRecord *pbo;
{
	switch (pbo->out.type) {
	case (VME_IO) :
		pbo->udf = FALSE;
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pbo,
			"devBoVmi2536 (init_record) Illegal OUT field");
		return(S_db_badField);
	}
	return(0);
}

static long read_bi(pbi)
struct biRecord *pbi;
{
	unsigned long value;
	struct vmeio *pvmeio;
	long status;

	pvmeio = (struct vmeio *)&(pbi->inp.value);
	status = vmi2536_read(pvmeio->card,pvmeio->signal,&value);
	if(status == 0) 
		pbi->rval = value;
	else
		recGblSetSevr(pbi,READ_ALARM,INVALID_ALARM);

	return(status);
}
	
static long write_bo(pbo)
struct boRecord *pbo;
{
	unsigned long value;
	struct vmeio *pvmeio;
	long status;

	pvmeio = (struct vmeio *)&(pbo->out.value);
	value = pbo->rval;
	status = vmi2536_write(pvmeio->card,pvmeio->signal,&value);
	if(status != 0) 
		recGblSetSevr(pbo,WRITE_ALARM,INVALID_ALARM);

	return(status);
}

