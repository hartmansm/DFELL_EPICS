/* devAiCamac0609.c */

/* Device Support routines for the CAMAC module D0609 ADC20 which is a
 * single channel 20-bit ADC from BINP. 
 *
 * This device support ignores A and F in the record's INP field. 
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

/* see chart below for TIME and RANGE meanings */
#define TIME	6	/* integration time code for ADC at 20-bit resol. */
#define	RANGE	0	/* voltage range of ADC, corresponds to 8.192 V */
#define ADCDELAY 0.480	/* seconds required for measuring by ADC */

#define F0	0	/* camac read */
#define F2	2	/* read control word, R4 is D; R3,R2,R1 is T */
#define F8	8	/* test LAM, Q=LAM */
#define F10	10	/* reset LAM, must be issued after power on */
#define F16	16	/* camac write */
#define F24	24	/* stop measurement, inhibit LAM (follow w/F10 */
#define F26	26	/* start measurement */

/* Subaddress A is arbitrary for all functions.
 * For all valid functions, X=1 and Q=1 (except F8).
 */

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
}devAiCamac0609={
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
				"devAiCamac0609 (init_record) Illegal INP field");
			return(S_db_badField);
	}

	pai->eslo = (pai->eguf - pai->egul)/1048576.0; /* 20 bits */

	/* reset LAM */
	data = 0;
	cfsa(F10, (pcio->ext), &data, &q);
	if ( q != 1 ) {
		recGblRecordError(S_db_badField,(void *)pai,"devAiCamac0609 (init_record) Failed to reset ADC");
		return(2);
	}

	/* set up the ADC specifying range and accuracy */
	data = 8 * RANGE + TIME;
	cfsa(F16, (pcio->ext), &data, &q);
	if ( q != 1 ) {
		recGblRecordError(S_db_badField,(void *)pai,"devAiCamac0609 (init_record) Failed to setup ADC");
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
	int q;
	int ticks;

	pcio = (struct dinfo *)pai->dpvt;
	if(!(pcio->ext)) return(2);

	pcallback = pcio->pcallback;

	data = 0;

	if (pai->pact ) { /* have already started async processing */

		/* read ADC */
		cfsa(F0, pcio->ext, &data, &q);
		if ( q != 1 ) {
			recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
			return(2);
		}

		if ( RANGE == 1 ) {
			pai->rval = (long)( 1024.0 * ( 512.0 + ( 9216.0 - (double)data/pow(2.0,(double)TIME) ) / ( 1.0 + 15.0 * (double)RANGE ) ) );
		}
		else {
			pai->rval = (long)( 64.0 * ( 8192.0 + ( 9216.0 - (double)data/pow(2.0,(double)TIME) ) / ( 1.0 + 15.0 * (double)RANGE ) ) );
		}

		/* finished async processing */
		return(0);
	}

	else { /* starting async processing */

		/* trigger ADC to start measurement */
		cfsa(F26, pcio->ext, &data, &q);
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
 *  D0609 ADC20:
 *  ------------
 *
 *  TIME codes define integration time and, therefore, sensitivity:
 *
 *  TIME code  Integration Time (ms)  Number of bits
 *  ---------  ---------------------  --------------
 *     0             5                      14
 *     1             10                     15
 *     2             20                     16
 *     3             40                     17
 *     4             80	                    18
 *     5             160                    19
 *     6             320                    20
 *
 *  Time of single measurement equals T + T/2 (from start of reading).
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
 *  U (mv) = (9216 - R/(2^T)) / (1+15*D)
 *  where R is code read from ADC, D is RANGE code, and T is TIME code 
 *
 *
 ****/
