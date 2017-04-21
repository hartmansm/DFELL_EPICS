/* devAiVmi3113A.c */

/********************************************************************* 

EPICS Device Support Routines for VMI 3113A scanning 12-bit, 64-channel
Analog to Digital Converter board

Copyright (c) 2001,2003,2005 Duke University
Author:         Steven Hartman
		Duke Free-Electron Laser Laboratory
		<hartman@fel.duke.edu>

Version:	1.2

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

#include "drvVmi3113A.h"

static long init_record();
static long read_ai();
static long special_linconv();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
        DEVSUPFUN       get_ioint_info;
	DEVSUPFUN	read_ai;
	DEVSUPFUN	special_linconv;
} devAiVmi3113A={
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	read_ai,
	special_linconv};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAiVmi3113A);
#endif

/* init_record */
static long init_record(pai)
struct aiRecord	*pai;
{
    /* ai.inp must be an VME_IO */
    switch (pai->inp.type) {
    case (VME_IO) :
        pai->udf = FALSE;
	break;
    default :
	recGblRecordError(S_db_badField,(void *)pai,
		"devAiVmi3113A (init_record) Illegal INP field");
	return(S_db_badField);
    }

    /* set linear conversion slope*/
    pai->eslo = (pai->eguf - pai->egul)/4095.0;	/* 12-bit */

    return(0);
}

/* read_ai */
static long read_ai(pai)
struct aiRecord	*pai;
{
   unsigned long value;
   struct vmeio *pvmeio;
   int status;

   pvmeio = (struct vmeio *)&(pai->inp.value);
   status=vmi3113A_driver(pvmeio->card,pvmeio->signal,&value);
   if(status==0) 
      pai->rval = value;
   else
      recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);

   return(status);
}

/* special_linconv */
static long special_linconv(pai,after)
    struct aiRecord     *pai;
    int after;
{
    if(!after) return(0);
    /* set linear conversion slope*/
    pai->eslo = (pai->eguf - pai->egul)/4095.0;	/* 12-bit */
    return(0);
}

