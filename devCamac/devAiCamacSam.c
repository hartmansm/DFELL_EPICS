/* devAiCamacSam.c */

/*
 * Device Support routines for the CAMAC modules:
 * 	BiRa 5305 Smart Analog Monitor (SAM)
 * 	DSP 2032 Scanning DVM
 *
 * Requires the CamacLib from Jefferson National Laboratory (CEBAF). 
 *
 * This device support ignores the F field and parm in the record's INP field. 
*/

/* Note for the CAMAC SAM:
 * Each channel requires 20 ms for processing ( 16 ms for averaging and 4
 * ms for formatting). Total refresh time for all 32 channels is therefore 
 * 640 ms (typical). The worst case refresh rate (if all 32 channels see 
 * a large input step and polarity reversal) is 2 seconds. Set scan rate 
 * appropriately.
*/

/*
 Copyright (C) 2001 Duke University
 Steven Hartman <hartman@fel.duke.edu>, Duke Free Electron Laser Laboratory

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
#include <taskLib.h>
#include <math.h>

#include <dbDefs.h>
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <aiRecord.h>

#include "camacLib.h"

#define	FACTOR	33554431.0	/* 2^25 - 1 */
#define SCALE	10.24		/* +- SAM range */
#define SAM_MASK	0xff00	/* mask to remove range and ac info bits */ 
#define IEEE_FORMAT	0x4	/* IEEE format for floating point output */
#define RESET_DELAY	0.02	/* seconds for SAM to reset (estimated) */
#define F0	0   /* sequential read from channel set by F17 */
#define F9	9   /* processor reset */
#define F16	16  /* load control register */
#define F17	17  /* set starting channel */


static long init_record();
static long special_linconv();
static long read_ai();

struct dinfo{
	short           f;
	int             ext;
	long            mask;
};

struct {
	long	number;
	DEVSUPFUN report;
	DEVSUPFUN init;
	DEVSUPFUN init_record;
	DEVSUPFUN get_ioint_info;
	DEVSUPFUN read_ai;
	DEVSUPFUN special_linconv;
}devAiCamacSam={
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	read_ai,
	special_linconv
};

static long init_record(pai)
struct aiRecord	*pai;
{
	struct	camacio *pcamacio;
	struct	dinfo *pcio;
	int	data;
	int	q; 
	int	ticks;

	/* ai.inp must be a CAMAC_IO */
	switch (pai->inp.type) {
	case (CAMAC_IO) :
		pcio = (struct dinfo *)malloc(sizeof(struct dinfo));
		if (pcio == NULL) {
		    return(2); /* failed */
		}
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
				"devAiCamacSam (init_record) Illegal INP field");
			return(S_db_badField);
	}

	/* -10.24 V to +10.24 V as a 26-bit ADC */
	pai->eslo = (pai->eguf - pai->egul)/67108863.0; /* 26 bits */

	/* issue a F9 to reset SAM processor */
	cfsa(F9, pcio->ext, &data, &q);

	/* wait for reset to complete */
	ticks = ceil (sysClkRateGet() * RESET_DELAY );
	taskDelay(ticks);

	/* use IEEE format for floating point output */
	data = IEEE_FORMAT;
	cfsa(F16, pcio->ext, &data, &q);

	return(0);
}

static long read_ai(pai)
struct aiRecord	*pai;
{
	register struct	dinfo *pcio;
	struct	camacio *pcamacio;
	int		q = 0;
	int		lsvalue, msvalue;
	unsigned short	*pvalue;
	float		volts;
	int		channel;

	pcio = (struct dinfo *)pai->dpvt;
	if(!(pcio->ext)) return(2);

	/* set channel to read from */
	pcamacio = (struct camacio *)&(pai->inp.value);
	channel = pcamacio->a;
	cfsa(F17, pcio->ext, &(channel), &q);

	q = 0;
	/* each read is 16 bits of a 32 bit word */
	cfsa(F0, pcio->ext, &lsvalue, &q);
	if ( q != 1 ) { /* invalid read */
		recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
		return(2);
	}
	cfsa(F0, pcio->ext, &msvalue, &q);
	if ( q != 1 ) { /* invalid read */
		recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
		return(2);
	}

	/* Note: periodically the SAM computes AC noise. During this time,
	 * readings cannot occur ( q == 0 ) and the VAL for the record will
	 * be the previous reading. It will be flagged INVALID. The next
	 * read should return a valid value. 
	 */

	/* combine 2 16-bit reads to get 32-bit volt reading */
	pvalue = (unsigned short *) &volts;
	*pvalue++ = (unsigned short) msvalue; 
	lsvalue = lsvalue & SAM_MASK; /* strip out AC and range nibbles */
	*pvalue = (unsigned short) lsvalue;

	if ( volts >= 90.0 ) { /* unsuccessful calibration or digitization */
		recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
		return(2);
	}

	pai->rval = (long) ( (volts * FACTOR / SCALE ) + FACTOR );
	pai->udf = FALSE;
	return(0);
}

static long special_linconv(pai,after)
    struct aiRecord	*pai;
    int after;
{
	if(!after) return(0);
	/* treating SAM like a 26-bit ADC */
	pai->eslo = (pai->eguf - pai->egul)/67108863.0; /* 26 bits */
	return(0);
}
