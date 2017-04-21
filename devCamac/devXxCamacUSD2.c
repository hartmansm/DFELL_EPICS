/* devAoCamacUSD2.c */

/*
 * Device Support routines for the CAMAC USD-2 Stepper Motor Driver
 * from Budker INP.
*/

/*
 Copyright (C) 2007  Duke University
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


/* 
 * This device support ignores A and F in the record's INP field. 
 * cdreg() is called with A0; other A values are offsets from this ext 
 * (A1 = A0 + 0x20, A2 = A0 + 0x40, etc.)
*/

#include	<vxWorks.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<taskLib.h>
#include	<math.h>

#include	<dbDefs.h>
#include	<dbAccess.h>
#include	<recSup.h>
#include	<devSup.h>
#include	<link.h>
#include	<aoRecord.h>
#include	<aiRecord.h>

#include	"camacLib.h"


/* the following parameters are used to initialize the stepper motor
 * driver. These should be operator controlled parameters with their 
 * own device support. 
 * Possibility that the position readback offset (see comment in read_ai
 * below) is frequency dependent, so it is probably best to just leave 
 * these hardcodes as they are now.
 */
/* frequency = (0-255)*20 Hz */
#define FREQ 5 /* 100 Hz */
/* frequency changing rate = (0-255)*400 Hz/sec */
#define RATE 1 /* 400 Hz/sec */ 
/* accelerating time = (0-255)*2 msec */
#define ACCELTIME 250 /* 500 msec */

/* use for delay while initializing the card in ao init between writes */
#define USD2_DELAY 0.01 /* seconds */


/* We should also have device support to read the status register (A15 F1)
 * bit 1: busy = 0, free = 1
 * bit 2: K- limit switch when 1
 * bit 3: K+ limit switch when 1
 * bit 4: remote = 0, manual = 1
 * 
 * but this doesn't seem to work correctly with all cards with all crate 
 * controllers.
 */

struct dinfo{
	short		f;
	int		ext;
	long		mask;
};

/* Create the dset for devAoCamacUSD2 */
static long init_record_ao();
static long write_ao();
static long special_linconv_ao();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_ao;
	DEVSUPFUN	special_linconv;
}devAoCamacUSD2={
	6,
	NULL,
	NULL,
	init_record_ao,
	NULL,
	write_ao,
	special_linconv_ao};

/* Create the dset for devAiCamacUSD2 */
static long init_record_ai();
static long read_ai();
static long special_linconv_ai();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_ai;
	DEVSUPFUN	special_linconv;
}devAiCamacUSD2={
	6,
	NULL,
	NULL,
	init_record_ai,
	NULL,
	read_ai,
	special_linconv_ai};


static long init_record_ai(pai)
struct aiRecord *pai;
{
	struct camacio *pcamacio;
	struct dinfo *pcio;

	/* ai.inp must be CAMAC_IO */
	switch (pai->inp.type) {
	case (CAMAC_IO) :
		pcio = (struct dinfo *)malloc(sizeof(struct dinfo));
		if (pcio == NULL) {
			return 2;
		}
		pcio->ext = 0;
		pcamacio = (struct camacio *)&(pai->inp.value);

		cdreg(&(pcio->ext), pcamacio->b, pcamacio->c, pcamacio->n, 1);
		if(!(pcio->ext)) return 2;

		/* set linear conversion slope */
		pai->eslo=(pai->eguf - pai->egul)/65535.0;

		pai->dpvt = (long *)pcio;
		break;

	default :
		recGblRecordError(S_db_badField,(void *)pai, 
			"devAiCamacUSD2 (init_record) Illegal INP field");
			return(S_db_badField);
	}

	return 2;
}

static long init_record_ao(pao)
struct aoRecord	*pao;
{
	struct camacio *pcamacio;
	struct dinfo *pcio;
	int data = 0;
	int q = 0;
	int ext = 0;
	int ticks;

	/* ao.out must be a CAMAC_IO */
	switch (pao->out.type) {
	case (CAMAC_IO) :
		pcio = (struct dinfo *)malloc(sizeof(struct dinfo));
		if (pcio == NULL) {
			return 2;
		}
		pcio->ext = 0;
		pcamacio = (struct camacio *)&(pao->out.value);

		cdreg(&(pcio->ext), pcamacio->b, pcamacio->c, pcamacio->n, 1);
		if(!(pcio->ext)) return 2;

		/* initialize device */

		ticks = ceil( sysClkRateGet() * USD2_DELAY );

		/* A15 F10 is reset of L */
		cdreg(&ext,pcamacio->b, pcamacio->c, pcamacio->n, 15);
		cfsa(10, ext, &data, &q); 
		if ( q != 1 )
			printf("devAoCamacUSD2 (init_record): failed to reset\n");
		
		taskDelay(ticks);

		/* A3 F17 to write initial frequency */
		cdreg(&ext,pcamacio->b, pcamacio->c, pcamacio->n, 3);
		data = FREQ;
		cfsa(17, ext, &data, &q);
		if ( q != 1 )
			printf("devAoCamacUSD2 (init_record): failed to write frequency\n");
		taskDelay(ticks);

		/* A5 F17 to write frequency changing rate */
		cdreg(&ext,pcamacio->b, pcamacio->c, pcamacio->n, 5);
		data = RATE;
		cfsa(17, ext, &data, &q);
		if ( q != 1 )
			printf("devAoCamacUSD2 (init_record): failed to write rate\n");
		taskDelay(ticks);

		/* A7 F17 to write acceleration time */
		cdreg(&ext,pcamacio->b, pcamacio->c, pcamacio->n, 7);
		data = ACCELTIME;
		cfsa(17, ext, &data, &q);
		if ( q != 1 )
			printf("devAoCamacUSD2 (init_record): failed to write accel time\n");


		/* set linear conversion slope */
		pao->eslo=(pao->eguf - pao->egul)/65535.0;

		pao->dpvt = (long *)pcio;
		break;

	default :
		recGblRecordError(S_db_badField,(void *)pao, 
			"devAoCamacUSD2 (init_record) Illegal OUT field");
			return(S_db_badField);
	}

	return 2;
}

static long write_ao(pao)
struct aoRecord	*pao;
{
register struct dinfo *pcio;
	int	q = 0;
	int data;

	pcio = (struct dinfo *)pao->dpvt;
	if(!(pcio->ext)) return(2);
	
	/* rval is offset binary, stepper motor needs sign plus magnitude */
	data = pao->rval;
	if ( data & 0x8000 ) {
		data = data - 0x8000;
	}
	else {
		data = ~data;
	}

	/* write number of steps with A1 F16 */
	cfsa(16, pcio->ext, &data, &q);
	if ( q != 1 ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
		return 2;
	}

	pao->udf = FALSE;
	return 0;
}

static long read_ai(pai)
struct aiRecord *pai;
{
	register struct dinfo *pcio;
	int data = 0;
	int q = 0;

	pcio = (struct dinfo *)pai->dpvt;
	if(!(pcio->ext)) return(2);

	/* read number of steps with A1 F0 */
	cfsa(0, pcio->ext, &data, &q);
	if ( q != 1 ) {
		recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
		return 2;
	}

	/* the register never counts down all the way to zero. It always
	 * stops at 0xa or -0xa (0x800a). Subtract 0xa to make it
	 * work (ugly hack).
	 */
	data = data - 0xa;


	/* stepper motor is sign plus magnitude, rval should be offset binary */
	if ( data & 0x8000 ) 
		data = ~data & 0xffff;
	else 
		data = data + 0x8000;

	pai->rval = data;

	pai->udf = FALSE;
	return 0;
}

static long special_linconv_ao(pao,after)
struct aoRecord *pao;
int after;
{
	if(!after) return(0);
	/* set linear conversion slope*/
	pao->eslo = (pao->eguf -pao->egul)/65535.0;
	return 0;
}

static long special_linconv_ai(pai,after)
struct aiRecord *pai;
int after;
{
	if(!after) return(0);
	/* set linear conversion slope*/
	pai->eslo = (pai->eguf -pai->egul)/65535.0;
	return 0;
}
