/* devWoJoergerVdacm.c */

/* EPICS waveform (output) record for Joerger VDACM */
/* Use to write waveform to module (using waveform as waveout record). */
/* INP parm specifies offset into module's waveform */

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

#include <alarm.h>
#include <dbDefs.h>
#include <dbAccess.h>
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <recGbl.h>
#include <waveformRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

extern int vdacm_write(unsigned short card, unsigned int signal, unsigned int offset, unsigned int length, signed short *valuebuf);
extern int vdacm_read(unsigned short card, unsigned int signal, unsigned int offset, unsigned int length, signed short *valuebuf);

static long init_record(struct waveformRecord *pwf);
static long write_wf(struct waveformRecord *pwf);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_wf;
} devWoVdacm = {
	5,
	NULL,
	NULL,
	init_record,
	NULL,
	write_wf};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devWoVdacm);
#endif

static long init_record(struct waveformRecord *pwf) 
{
	/* wf.inp must be VME_IO */
	switch (pwf->inp.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pwf,
			"devWoVdacm (init_record) Illegal INP field");
		return(S_db_badField);
	}

	if (pwf->ftvl != DBF_SHORT) {
		recGblRecordError(S_db_badField, (void *)pwf,
			"devWoVdacm (init_record) Illegal FTVL field");
		return(S_db_badField);
	}

	return 0;
}

static long write_wf(struct waveformRecord *pwf)
{
	int status;
	struct vmeio *pvmeio;
	unsigned int length, offset;

	pvmeio = (struct vmeio *)&(pwf->inp.value);

	length = pwf->nelm;
	offset = atoi(pwf->inp.value.vmeio.parm);

	status = vdacm_write(pvmeio->card, pvmeio->signal, offset, length, pwf->bptr);

	if ( status == 0 ) pwf->nord = length;
	else recGblSetSevr(pwf,WRITE_ALARM,INVALID_ALARM);

	return status;

}
