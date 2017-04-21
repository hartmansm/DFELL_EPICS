/* devBoCamac3224.c */

/* Device Support routines for the CAMAC BiRa 3224 48-channel binary 
 * output module. This module has dual 24-bit output registers.
 *
 * Requires the CamacLib from Jefferson National Laboratory (CEBAF).
 *
 * Copyright (c) 2002 Duke University
 * Steven Hartman <hartman@fel.duke.edu>, Duke Free Electron Laser Laboratory
 */

#include	<vxWorks.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

#include	<dbDefs.h>
#include	<dbAccess.h>
#include        <recSup.h>
#include	<devSup.h>
#include	<module_types.h>
#include	<boRecord.h>
#include 	"camacLib.h"

#define F18	18	/* selectively set bit */
#define F21	21	/* selectively clear bit */

struct dinfo{
	short           f;
	int             ext;
	long            mask;
};

/* Create the dset for devBoCamac3224 */
static long init_record();
static long write_bo();


struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_bo;
}devBoCamac3224={
		5,
		NULL,
		NULL,
		init_record,
		NULL,
		write_bo};

static long init_record(pbo)
struct boRecord	*pbo;
{
	struct camacio *pcamacio;
	struct dinfo *pcio;
	unsigned short channel;
	unsigned short reg = 0; /* output register, 0-23 -> 0, 24-47 -> 1 */

	/* bo.out must be a CAMAC_IO */
	switch (pbo->out.type) {
	case (CAMAC_IO) :
		pcio = (struct dinfo *)malloc(sizeof(struct dinfo));
		if (pcio == NULL) {
			return(2);
		}
		pcio->ext = 0;
		pcamacio = (struct camacio *)&(pbo->out.value);
		channel = pcamacio->a;
		if ( channel > 23 ) {
			channel -= 24;
			reg = 1; /* high register */
		}

		cdreg(&(pcio->ext), pcamacio->b, pcamacio->c, pcamacio->n, reg);

		if(!(pcio->ext)) return(2);

		pbo->mask = 1;
		pbo->mask <<= channel;
		pbo->dpvt = (long *)pcio;

		break;

	default :
		recGblRecordError(S_db_badField,(void *)pbo,
		    "devBoCamac3224 (init_record) Illegal OUT field");
		return(S_db_badField);
	}
	return(0);
}

static long write_bo(pbo)
struct boRecord	*pbo;
{
	register struct dinfo *pcio;
	int   data;
	int   q = 0;

	pcio = (struct dinfo *)pbo->dpvt;
	if(!(pcio->ext)) return(2);

	data = pbo->mask;

	if ( pbo->rval ) {	/* set bit */
		cfsa(F18, pcio->ext, &data, &q);
	}
	else {	/* clear the bit */
		cfsa(F21, pcio->ext, &data, &q);
	}

	if ( q != 1 ) {
		recGblSetSevr(pbo,WRITE_ALARM,INVALID_ALARM);
		return(2);
	}

	pbo->udf = FALSE;

	return(0);
}
