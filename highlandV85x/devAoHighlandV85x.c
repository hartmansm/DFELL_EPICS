/* devAoHighlandV85x.c */

/*
 EPICS Device Support Routines for Highland V850 / V851 or Berkeley
 Nucleonics B950 / B951 Digital Delay Generator.

 Copyright (c) 2006 Duke University
 Author:    Steven Hartman
            Duke Free Electron Laser Laboratory
            <hartman@fel.duke.edu>
 
 Version:   1.0
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
#include <aoRecord.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvHighlandV85x.h"

static long init_record(struct aoRecord *pao);
static long write_ao(struct aoRecord *pao);

struct {
	long	number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_ao;
	DEVSUPFUN	special_linconv;
} devAoHighlandV85x = {
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	write_ao,
	NULL};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(dset,devAoHighlandV85x);
#endif

extern long ddg85x_delay(unsigned short, unsigned short, double);
extern long ddg85x_wave(unsigned short, unsigned short, unsigned char, unsigned char);
extern long ddg85x_disable(unsigned short, unsigned char);
extern long ddg85x_vout(unsigned short, double, double);
extern long ddg85x_triglev(unsigned short, double);

#define NUMCHANS 6

/* defaults for configuring the card to our needs */
#define VOUT_LOW 0.0
#define VOUT_HIGH 5.0
#define TRIG_LEVEL 2.0
#define WAVEMODE MODE0

static long init_record(struct aoRecord *pao) 
{
	int i;
	struct vmeio *pvmeio;

	switch (pao->out.type) {
	case (VME_IO) :
		break;
	default :
		recGblRecordError(S_db_badField,(void *)pao,"devAoHighlandV85x (init_record) Illegal OUT field");
		return(S_db_badField);
	}

	pvmeio = (struct vmeio *)&(pao->out.value);

	/* set up default parameters */
	ddg85x_vout(pvmeio->card, VOUT_LOW, VOUT_HIGH);
	ddg85x_triglev(pvmeio->card, TRIG_LEVEL);
	for ( i = 0 ; i < NUMCHANS; i=i+2 ) {
		ddg85x_wave(pvmeio->card,i,MODE0, MODE0);
	}

	return 2; /* do not convert */

}

static long write_ao(struct aoRecord *pao)
{
	struct vmeio *pvmeio;
	double *value = 0;

	pvmeio = (struct vmeio *)&(pao->out.value);
	
	*value = pao->val;

	ddg85x_delay(pvmeio->card,pvmeio->signal,*value);

	return 0;
}

