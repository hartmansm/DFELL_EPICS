/* devAiCamac0612.c */

/* Device Support routines for the CAMAC module D0612 ADC20 which is a
 * single channel 20-bit ADC from BINP. 
 *
 * This device support ignores A and F in the record's INP field. 
 * cdreg() is called with A0; other A values are offsets from this ext 
 * (A1 = A0 + 0x20, A2 = A0 + 0x40, etc.)
 *
 * Do not set record SCAN field to < ADCDELAY.
 *
 * Notes on the ADC module are in comments at end of this file. 
 *
 * Requires the CamacLib from Jefferson National Laboratory (CEBAF).
 *
 * Version 1.0:	initial release
 * Version 1.1:	use callback to handle delay for ADC processing time
 *
*/

/*
 Copyright (C) 2002  Duke University
 Steven Hartman <hartman@fel.duke.edu>

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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <wdLib.h>

#include <dbDefs.h>
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <callback.h>
#include <aiRecord.h>

#include "camacLib.h"

#define F0	0	/* camac read */
#define F16	16	/* camac write */
#define F25	25	/* start ADC measurement */

/* see chart below for TIME and RANGE meanings */
#define TIME	7	/* integration time code for ADC at 20-bit resol. */
#define	RANGE	0	/* voltage range of ADC, corresponds to 8.192 V */
#define ADCDELAY 0.250	/* seconds required for measuring by ADC */

static long init_record();
static long special_linconv();
static long read_ai();

struct callback {
	CALLBACK callback;
	struct dbCommon *precord;
	WDOG_ID wd_id;
};

struct dinfo{
	short           f;
	int             ext;
	long            mask;
	struct callback	*pcallback;
};

struct {
	long	number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN read_ai;
	DEVSUPFUN special_linconv;
}devAiCamac0612={
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	read_ai,
	special_linconv
};

static void myCallback(pcallback)
	struct callback *pcallback;
{
	struct dbCommon *precord;
	struct rset *prset;

	callbackGetUser(precord,pcallback);
	prset = (struct rset *)precord->rset;
	dbScanLock(precord);
	(*prset->process)(precord);
	dbScanUnlock(precord);
}


static long init_record(pai)
struct aiRecord	*pai;
{
	struct	dinfo *pcio;
	struct	camacio *pcamacio;
	struct	callback	*pcallback;
	int    data;
	int	q; 

	/* ai.inp must be a CAMAC_IO */
	switch (pai->inp.type) {
	case (CAMAC_IO) :
		pcio = (struct dinfo *)malloc(sizeof(struct dinfo));
		if (pcio == NULL) {
		    return(2); /* failed */
		}
		pcallback = (struct callback *)(calloc(1,sizeof(struct callback)));
		pcio->pcallback = pcallback;
		callbackSetCallback(myCallback, &pcallback->callback);
		callbackSetUser(pai, &pcallback->callback);
		pcallback->precord = (struct dbCommon *)pai;
		pcallback->wd_id = wdCreate();
		pcio->ext = 0;
		pcamacio = (struct camacio *)&(pai->inp.value);
		cdreg(&(pcio->ext), pcamacio->b, pcamacio->c, pcamacio->n, 0);
		
		if(!(pcio->ext)) {
			pai->dpvt = (long *)pcio;
			return(2); /* cdreg failed */
		}

		pai->dpvt = (long *)pcio;

		break;

	default :
		recGblRecordError(S_db_badField,(void *)pai,
				"devAiCamac0612 (init_record) Illegal INP field");
			return(S_db_badField);
	}

	pai->eslo = (pai->eguf - pai->egul)/1048576.0; /* 20 bits */

	/* set up the ADC specifying range and accuracy */
	data = 8 * RANGE + TIME;
	cfsa(F16, (pcio->ext + 0x60), &data, &q);
	if ( q != 1 ) {
		recGblRecordError(S_db_badField,(void *)pai,"devAiCamac0612 (init_record) Failed to setup ADC");
		return(2);
	}

	/* write 0 in final address register */
	data = 0;
	cfsa(F16, (pcio->ext + 0x40), &data, &q);
	if ( q != 1 ) {
		recGblRecordError(S_db_badField,(void *)pai,"devAiCamac0612 (init_record) Failed to setup ADC");
		return(2);
	}

	return(0);
}

static long read_ai(pai)
struct aiRecord	*pai;
{
	register struct dinfo *pcio;
	struct callback *pcallback;
	int data;
	int ticks;
	int q;

	pcio = (struct dinfo *)pai->dpvt;
	if(!(pcio->ext)) return(2);

	pcallback = pcio->pcallback;
	
	if (pai->pact ) { /* have already started async processing */

		/* write 0 in initial address register */
		data = 0;
		cfsa(F16, (pcio->ext + 0x20), &data, &q);
		if ( q != 1 ) {
			recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
			return(2);
		}

		/* read ADC */
		cfsa(F0, pcio->ext, &data, &q);
		if ( q != 1 ) {
			recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
			return(2);
		}

		if ( data > 0x80000 ) { /* voltage is negative */
			data -= 0x100000;
		}

		data += 0x80000;

		/* if (RANGE == 0) 0x0000 == -8.192 volts, 0xFFFFF == 8.192 volts */
		/* if (RANGE == 1) 0x0000 == -0.512 volts, 0xFFFFF == 0.512 volts */

		pai->rval = (long)data;

	}

	else { /* starting async processing */

		/* write 0 in initial address register */
		data = 0;
		cfsa(F16, (pcio->ext + 0x20), &data, &q);
		if ( q != 1 ) {
			recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
			return(2);
		}

		/* trigger ADC to start taking data */
		data = 0;
		cfsa(F25, pcio->ext, &data, &q);
		if ( q != 1 ) {
			recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
			return(2);
		}

		/* callback after measurement is finished */
		ticks = ceil( sysClkRateGet() * ADCDELAY );
		callbackSetPriority(pai->prio,&pcallback->callback);
		wdStart(pcallback->wd_id,ticks,(FUNCPTR)callbackRequest,(int)pcallback);
		pai->pact = TRUE;
		return(0);
	}

	return(0);
}

static long special_linconv(pai,after)
    struct aiRecord	*pai;
    int after;
{
	if(!after) return(0);
	pai->eslo = (pai->eguf - pai->egul)/1048576.0; /* 20 bits */
	return(0);
}


/***** 
 *
 *  D0612 ADC20:
 *  ------------
 *
 *  TIME codes define integration time and, therefore, sensitivity:
 *
 *  (time based on 60Hz)
 *
 *  TIME code  Integration Time (ms)  Number of bits
 *  ---------  ---------------------  --------------
 *     0             1.04                   13
 *     1             2.08                   14
 *     2             4.16                   15
 *     3             8.33                   16
 *     4             16.67                  17
 *     5             33.33                  18
 *     6             66.67                  19
 *     7             133.33                 20
 *
 *  RANGE codes define range of number of bits:
 *
 *  RANGE code       Range (mV)
 *  ----------       ----------
 *     0               8192.0
 *     1               512.0
 *
 *  Conversion calculation:
 *
 *  The data returned from the unit is a 20 bit number representing
 *  an integer from 0x00000 to 0xFFFFF and uses two's complement for
 *  the sign of the number.  Let C be the code returned by the module.
 *  Let K be the code we actually convert.  Then:
 *
 *  If C <= 0x80000 -> K = C and the voltage is positive
 *
 *  If C > 0x80000 -> K = C - 0x100000 and the voltage is negative
 *
 *  The conversion of K to get the magnitude of the voltage is
 *
 *  V(millivolts) = ( 2 * K ) / (2^(TIME + 4 * RANGE))      - or -
 *  V(millivolts) = ( 2 * K ) / [ ( 2^TIME ) * ( 1 + 15 * RANGE ) ]
 *
 *  where TIME and RANGE are the codes given in the beginning of the program.
 *
 ****/
