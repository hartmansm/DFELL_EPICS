/* devBiVmi1182.c */

/* EPICS device support routines for VMIVME-1182 64-channel bi module */

/*
 Copyright (C) 2004,2005  Duke University
 Author: Steven Hartman <hartman@fel.duke.edu>
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
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvVmi1182.h"

static long init_bi();
static long read_bi();
static long get_ioint_info();

extern IOSCANPVT vmi1182_ioscanpvt[];

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_bi;
} devBiVmi1182 = {
	5,
	NULL,
	NULL,
	init_bi,
	get_ioint_info,
	read_bi };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devBiVmi1182);
#endif

static long init_bi(pbi)
struct biRecord *pbi;
{
	switch (pbi->inp.type) {
	case (VME_IO) :
		pbi->udf = FALSE;
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pbi,
			"devBiVmi1182 (init_record) Illegal INP field");
		return(S_db_badField);
	}
	return(0);
}

static long get_ioint_info(int cmd, struct biRecord *pbi,IOSCANPVT *ppvt)
{
	struct vmeio *pvmeio;
	IOSCANPVT ioscanpvt;

	pvmeio = (struct vmeio *)&(pbi->inp.value);
	ioscanpvt = vmi1182_ioscanpvt[pvmeio->card];
	*ppvt = ioscanpvt;
	return(0);
}

static long read_bi(pbi)
struct biRecord *pbi;
{
	unsigned short value;
	unsigned short dataword;
	unsigned int bit_num;
	struct vmeio *pvmeio;
	long status;

	pvmeio = (struct vmeio *)&(pbi->inp.value);

	if ( pvmeio->signal < 16 )
		dataword = 3;
	else if ( pvmeio->signal < 32 )
		dataword = 2;
	else if ( pvmeio->signal < 48 )
		dataword = 1;
	else if ( pvmeio->signal < 64 )
		dataword = 0;
	else {
		recGblSetSevr(pbi,READ_ALARM,INVALID_ALARM);
		return ERROR;
	}
	
	status = vmi1182_read(pvmeio->card,dataword,&value);
	if (status == 0) {
		bit_num = pvmeio->signal - 16 * (pvmeio->signal / 16);

		if ( value & (1 << bit_num) )
			pbi->rval = 1;
		else
			pbi->rval = 0;
	}
	else
		recGblSetSevr(pbi,READ_ALARM,INVALID_ALARM);

	return(status);
}
	

