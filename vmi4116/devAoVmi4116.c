/* devAoVmi4116.c */
/********************************************************************* 

EPICS Device Support Routines for VMIC 4116 16-bit, 8-channel
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
#include <stdioLib.h>
#include <string.h>

#include <dbDefs.h>
#include <recSup.h>
#include <dbAccess.h>
#include <alarm.h>
#include <recGbl.h>
#include <devSup.h>
#include <link.h>
#include <dbScan.h>
#include <aoRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvVmi4116.h"

static long init_record();
static long write_ao();
static long special_linconv();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_ao;
	DEVSUPFUN	special_linconv;
} devAoVmi4116 = {
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	write_ao,
	special_linconv };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAoVmi4116);
#endif

static long init_record(struct aoRecord *pao)
{
	switch (pao->out.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pao,"devAoVmi4116 (init_record) Illegal OUT field");
		return(S_db_badField);
	}

	pao->eslo = (pao->eguf -pao->egul)/65535.0;

	return(2);
}

static long write_ao(struct aoRecord *pao)
{
	struct vmeio *pvmeio;
	unsigned long value;

	pvmeio = (struct vmeio *)&(pao->out.value);
	value = pao->rval;
	if ( vmi4116_write(pvmeio->card,pvmeio->signal,&value) != OK ) 
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
	
	return(0);
}

static long special_linconv(struct aoRecord *pao, int after)
{
	if (!after) return(0);
	pao->eslo = (pao->eguf -pao->egul)/65535.0;
	return(0);
}


