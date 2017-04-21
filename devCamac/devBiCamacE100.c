/* devBiCamacE100.c */

/* devBiCamacE100.c - Device Support Routines for CAMAC E100 32-bit
 * binary input module. 
 * Requires the CamacLib from Jefferson National Laboratory (CEBAF).
 *
 * Copyright (c) 2002 Duke University
 * Steven Hartman <hartman@fel.duke.edu>, Duke Free Electron Laser Laboratory
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
#include	<module_types.h>
#include	<biRecord.h>
#include	"camacLib.h"

#define F0	0	/* camac read */

struct dinfo{
	short	f;
	int	ext;
	long	mask;
};

/* Create the dset for devBiE100 */
static long init_record();
static long read_bi();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_bi;
}devBiCamacE100={
		5,
		NULL,
		NULL,
		init_record,
		NULL,
		read_bi};

static long init_record(pbi)
struct biRecord	*pbi;
{
	struct camacio *pcamacio;
	struct dinfo *pcio;
	unsigned short channel;
	unsigned short reg = 0; /* input register, 0-15 -> 0, 16-31 -> 1 */

	/* bi.inp must be a CAMAC_IO */
	switch (pbi->inp.type) {
	case (CAMAC_IO) :
		pcio = (struct dinfo *)malloc(sizeof(struct dinfo));
		if (pcio == NULL) {
			return(2);
		}
		pcio->ext = 0;
		pcamacio = (struct camacio *) &(pbi->inp.value);
		channel = pcamacio->a;
		if ( channel > 15 ) {
			channel -= 16;
			reg = 1; /* high register */
		}

		cdreg(&(pcio->ext), pcamacio->b, pcamacio->c, pcamacio->n, reg);

		if(!(pcio->ext)) return(2); /* cdreg failed if ext is zero */


		pbi->mask=1;
		pbi->mask <<= channel;
		pbi->dpvt = (long *)pcio;

		break;

	default :
		recGblRecordError(S_db_badField,(void *)pbi,
			"devBiCamacE100 (init_record) Illegal INP field");
		return(S_db_badField);
	}

	return(0);
}

static long read_bi(pbi)
struct biRecord	*pbi;
{
	register struct dinfo *pcio;
	int	data;
	int	q = 0;

	pcio = (struct dinfo *)pbi->dpvt;
	if(!(pcio->ext)) return(2);

	cfsa(F0, pcio->ext, &data, &q);

	if ( q != 1 ) { /* invalid read */
		recGblSetSevr(pbi,READ_ALARM,INVALID_ALARM);
		return(2);
	}

	pbi->rval = data & pbi->mask;

	return(0);
}
