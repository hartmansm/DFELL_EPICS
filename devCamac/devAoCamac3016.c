/* devAoCamac3016.c */

/*
 * Device Support routines for the CAMAC 3016 16-bit DAC.
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

/* 
 * This device module ignores the F field specified in the OUT field of 
 * the record. The appropriate value for F is coded below. 
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
#define	F16 16	/* camac write */

struct dinfo{
	short		f;
	int		ext;
	long		mask;
};

/* Create the dset for devAoCamac3016 */
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
}devAoCamac3016={
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

	/* ao.out must be a CAMAC_IO */
	switch (pao->out.type) {
	case (CAMAC_IO) :
		pcio = (struct dinfo *)malloc(sizeof(struct dinfo));
		if (pcio == NULL) {
			return(2);
		}
		pcio->ext = 0;
		pcamacio = (struct camacio *)&(pao->out.value);

		cdreg(&(pcio->ext), pcamacio->b, pcamacio->c, pcamacio->n, pcamacio->a);
		if(!(pcio->ext)) return(2);

		/* set linear conversion slope */
		pao->eslo=(pao->eguf - pao->egul)/65535.0;

		pao->dpvt = (long *)pcio;
		break;

	default :
		recGblRecordError(S_db_badField,(void *)pao, 
			"devAoCamac3016 (init_record) Illegal OUT field");
			return(S_db_badField);
	}

	return(2);
}

static long write_ao(pao)
struct aoRecord	*pao;
{
register struct dinfo *pcio;
	int	q = 0;
	int	data;

	pcio = (struct dinfo *)pao->dpvt;
	if(!(pcio->ext)) return(2);
	
	data = (int)pao->rval;
	cfsa(F16, pcio->ext, &(data), &q);
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
	pao->eslo = (pao->eguf -pao->egul)/65535.0;
	return(0);
}
