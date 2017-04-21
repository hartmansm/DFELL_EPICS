/* devAoCamacD0643.c */

/*
 * Device Support routines for the CAMAC D0643 20-bit DAC.
 *
 * Requires the CamacLib from Jefferson National Laboratory (CEBAF). 
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

/* This device module ignores the F field specified in the OUT field of 
 * the record. The appropriate value for F is coded below. The A field is 
 * also ignored as the DAC has only one channel. A0 is used except for 
 * reading or writing the lower 4 bits which is addressed with A1.
 *
 * By default, this device routine is set up to cause the DAC to be 
 * initialized to 0 volts. The DAC can be read but this routine does
 * not use that feature. 
 *
 * Hardware Device Notes at end of this file.
 *
*/

#include	<vxWorks.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

#include	<dbDefs.h>
#include	<dbAccess.h>
#include	<recSup.h>
#include	<devSup.h>
#include	<link.h>
#include	<aoRecord.h>

#include	"camacLib.h"

#define	F0  0	/* camac read */
#define	F16 16	/* camac write and set */
#define	F18 18	/* camac write without set */
#define F26 26	/* set after write */

struct dinfo{
	short		f;
	int		ext;
	long		mask;
};

/* Create the dset for devAoCamacD0643 */
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
}devAoCamacD0643={
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	write_ao,
	special_linconv};

static long init_record(pao)
struct aoRecord	*pao;
{
	struct camacio *pcamacio;
	struct dinfo *pcio;
	int q;
	int data = 0;

	/* ao.out must be a CAMAC_IO */
	switch (pao->out.type) {
	case (CAMAC_IO) :
		pcio = (struct dinfo *)malloc(sizeof(struct dinfo));
		if (pcio == NULL) {
			return(2);
		}
		pcio->ext = 0;
		pcamacio = (struct camacio *)&(pao->out.value);

		cdreg(&(pcio->ext), pcamacio->b, pcamacio->c, pcamacio->n, 0);
		if(!(pcio->ext)) return(2);

		/* set linear conversion slope */
		pao->eslo=(pao->eguf - pao->egul)/1048575.0;

		pao->dpvt = (long *)pcio;
		break;

	default :
		recGblRecordError(S_db_badField,(void *)pao, 
			"devAoCamacD0643 (init_record) Illegal OUT field");
			return(S_db_badField);
	}

	cfsa(F16, pcio->ext, &data, &q);
	pao->rval = 0x80000;
	return(0);
}

static long write_ao(pao)
struct aoRecord	*pao;
{
register struct dinfo *pcio;
	int	q = 0;
	long	data;
	int	nulldata = 0;
	int	msvalue;
	int	lsvalue;

	pcio = (struct dinfo *)pao->dpvt;
	if(!(pcio->ext)) return(2);
	
	data = pao->rval;
	data -= 0x80000;
	if ( data < 0 ) { /* negative */
		data *= -1;
		data +=	0x80000; 
	}

	msvalue = (int)( data >> 4);
	lsvalue = (int)( data & 0xF );

	/* write upper 16 bits (F18 A0) */
	cfsa(F18, pcio->ext, &(msvalue), &q);
	if ( q != 1 ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
		return(2);
	}
	/* write lower 4 bits (F18 A1) */
	cfsa(F18, pcio->ext + 0x20, &(lsvalue), &q);
	if ( q != 1 ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
		return(2);
	}

	/* start DAC conversion */
	cfsa(F26, pcio->ext, &nulldata, &q);
	if ( q != 1 ) {
		recGblSetSevr(pao,WRITE_ALARM,INVALID_ALARM);
		return(2);
	}

	pao->udf = FALSE;
	return(0);
}

static long special_linconv(pao,after)
struct aoRecord *pao;
int after;
{
	if(!after) return(0);
	/* set linear conversion slope*/
	pao->eslo = (pao->eguf -pao->egul)/1048575.0;
	return(0);
}

/* Device Notes for D0643 DAC 20:
 *
 * Output Range:	+/- 8.192 Volts (see note below)
 * Binary bits:		19 plus sign
 * LSB sensitivity:	15.625 uV
 * Nonlinearity:	0.0001%
 * Long Term Drift:	0.001% per 3 months
 * TC of output volt:	0.0003%/deg C + 2 uV/deg C
 * Time w/ 0.001% acc:	100 ms
 * Current input:	1 A at +6 V
 *
 * Commands:
 * F0  A0	read upper 16 bits
 * F0  A1	read lower 4 bits
 * F16 A0	write 16 upper bits, zero 4 lower and start conversion
 * F16 A1	write 4 lower bits, start converstion
 * F18 A0	write 16 upper bits
 * F18 A1	write 4 lower bits
 * F26 A0	start conversion
 *
 * Voltage Range Problem:
 * Two different D0643 DACs did not work correctly when set to less then
 * ~8.120 Volts. When attempting to set DAC to this level, the sign bit 
 * was reversed. Also, when setting to -8.192 Volts, the DAC read 0 Volts.
 * At all other settings, the DAC appears to work as expected.
 *
 */
