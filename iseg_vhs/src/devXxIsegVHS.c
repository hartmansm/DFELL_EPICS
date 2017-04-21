/* devXxIsegVHS.c */

/* EPICS device support routines for ISEG VHS
 * VME High Voltage Power Supply
 * (bo, ai, ao)
 *
*/

/*
 Copyright (C) 2007  Duke University
 Steven Hartman <hartman@fel.duke.edu>
 Duke Free Electron Laser Laboratory

 Version: 1.0
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
#include <aiRecord.h>
#include <aoRecord.h>
#include <boRecord.h>
#include <epicsVersion.h>

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvIsegVHS.h"

static long init_ai();
static long init_ao();
static long init_bo();
static long read_ai();
static long write_ao();
static long write_bo();

/* bo record used to turn channel on or off */
struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_bo;
} devBoIsegVHS = {
	5,
	NULL,
	NULL,
	init_bo,
	NULL,
	write_bo };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devBoIsegVHS);
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
			"devBoIsegVHS (init_bo) Illegal OUT field");
		return(S_db_badField);
	}
	return 0;
}

static long write_bo(pbo)
struct boRecord *pbo;
{
	unsigned int value;
	struct vmeio *pvmeio;
	long status;

	pvmeio = (struct vmeio *)&(pbo->out.value);
	value = pbo->rval;
	status = iseg_vhs_chan_on(pvmeio->card,pvmeio->signal,&value);
	if(status != 0) 
		recGblSetSevr(pbo,WRITE_ALARM,INVALID_ALARM);

	return status;
}


/* ao record used to set channel voltage, current or rampspeed */
/* (use parm field to specify: V, I, or S) */

struct {
	long number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN write_ao;
	DEVSUPFUN special_linconv;
}devAoIsegVHS={
	6,
	NULL,
	NULL,
	init_ao,
	NULL,
	write_ao,
	NULL};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAoIsegVHS);
#endif

static long init_ao(pao)
struct aoRecord *pao;
{
	char *parm;

	/* must be VME_IO */
	switch (pao->out.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pao,
			"devAoIsegVHS (init_ao) Illegal OUT field");
		return(S_db_badField);
	}

	parm = pao->out.value.vmeio.parm;

	/* check for valid parm */
	if ( *parm=='V' || *parm=='v' || *parm=='I' || *parm=='i' || *parm=='S' || *parm=='s' ) {

		return 2; /* do not convert */
	}
	else {
		recGblRecordError(S_db_badField,(void *)pao,
			"devAoIsegVHS (init_record) Illegal PARM field");
		return(S_db_badField);
	}
}

static long write_ao(pao)
struct aoRecord *pao;
{
	struct vmeio *pvmeio;
	long status;
	float value;
	char *parm;

	pvmeio = (struct vmeio *)&(pao->out.value);
	value = pao->val;

	parm = pvmeio->parm;

	/* write voltage for channel */
	if ( *parm == 'V' || *parm == 'v' ) {
		status = iseg_vhs_vwrite(pvmeio->card,pvmeio->signal,&value);
	}

	/* write current for channel */
	if ( *parm == 'I' || *parm == 'i' ) {
		status = iseg_vhs_iwrite(pvmeio->card,pvmeio->signal,&value);
	}

	/* write voltage ramping speed for card */
	if ( *parm == 'S' || *parm == 's' ) {
		status = iseg_vhs_vrampspeed(pvmeio->card,&value);
	}

	if (status != OK ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
	}

	return status;
}


/* ai record used to read channel voltage or current */
/* (use parm field to specify: V or I) */

struct {
	long            number;
	DEVSUPFUN       report;
	DEVSUPFUN       init;
	DEVSUPFUN       init_record;
	DEVSUPFUN       get_ioint_info;
	DEVSUPFUN       read_ai;
	DEVSUPFUN       special_linconv;
} devAiIsegVHS = {
	6,
	NULL,
	NULL,
	init_ai,
	NULL,
	read_ai,
	NULL };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAiIsegVHS);
#endif

static long init_ai(pai)
struct aiRecord *pai;
{
	char *parm;

	/* ai.inp must be VME_IO */
	switch (pai->inp.type) {
	case (VME_IO) :
		pai->udf = FALSE;
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pai,
			"devAiIsegVHS (init_ai) Illegal INP field");
		return(S_db_badField);
	}

	parm = pai->inp.value.vmeio.parm;
	/* check for valid parm */
	if ( *parm=='V' || *parm=='v' || *parm=='I' || *parm=='i' ) {
		return 0;
	}
	else {
		recGblRecordError(S_db_badField,(void *)pai,
			"devAiIsegVHS (init_record) Illegal PARM field");
		return(S_db_badField);
	}
}

static long read_ai(pai)
struct aiRecord *pai;
{
	float value;
	struct vmeio *pvmeio;
	int status = -1;
	char *parm;

	pvmeio = (struct vmeio *)&(pai->inp.value);

	parm = pvmeio->parm;

	/* read voltage */
	if ( *parm == 'V' || *parm == 'v' ) {
		status = iseg_vhs_vread(pvmeio->card,pvmeio->signal,&value);
	}

	/* read current */
	if ( *parm == 'I' || *parm == 'i' ) {
		status = iseg_vhs_iread(pvmeio->card,pvmeio->signal,&value);
	}

	if (status != OK ) {
		recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
		return status;
	}

	pai->val = value;
	return 2; /* do not convert */

}

