/* drvBiraVsam.c */

/*********************************************************************

EPICS driver for BiRa 7305 VSAM (Smart Analog Monitor)

Author:	Steven Hartman
	Duke Free-Electron Laser Laboratory
	<hartman@fel.duke.edu>

Version:        1.1
Please check <www.fel.duke.edu/epics> for the most recent version.

Copyright (c) 2004,2005 Duke University

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

********************************************************************/

#include <vxWorks.h>
#include <stdlib.h>
#include <stdio.h>
#include <vxLib.h>
#include <sysLib.h>
#include <vme.h>

#include <dbDefs.h>
#include <drvSup.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvBiraVsam.h"

#define NUM_CHANNELS 32 /* channels per card */

/* these can be changed in st.cmd before iocInit if necessary */
unsigned int num_vsam_cards = 2;      /* number of cards in crate */
unsigned int vsam_base = 0x900000;    /* base address of card 0 (A24) */


struct {
	long            number;
	DRVSUPFUN       report;
	DRVSUPFUN       init;
} drvBiraVsam = {
	2,
	vsam_report,
	vsam_init
};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvBiraVsam);
#endif

static unsigned short **pai_vsam;
static int vsam_addr;

/* initialize the vsam */
long vsam_init(void)
{
	unsigned short **boards_present;
	long	val;
	int	status;
	short	i;
	bira_vsam * board;

	pai_vsam = (unsigned short **)calloc(num_vsam_cards,sizeof(*pai_vsam));
		if (!pai_vsam) {
			return ERROR;
		}

		boards_present = pai_vsam;

		if ((status = sysBusToLocalAdrs(VME_AM_STD_SUP_DATA,(char *)(long)vsam_base, (char **)&vsam_addr)) != OK){

			printf("Addressing error in vsam_init\n");
			return(ERROR);
		}

		board = ((bira_vsam * )((int)vsam_addr));

		for ( i = 0; i < num_vsam_cards; i++, board ++, boards_present ++ ) {
		if (vxMemProbe((char *)board,VX_READ,sizeof(long),(char *)&val) == OK) {

			*boards_present = (unsigned short *)board;
			 board->mode_control = NORMAL_MODE; /* normal scan, data available, big endian */

			}
			else {
				*boards_present = 0;
			}
		}
		return(0);
}

/* vsam driver */
int vsam_driver(unsigned short card, unsigned int signal, float *pval) 
{
	register bira_vsam *pvsam;
	if ((pvsam = (bira_vsam *)pai_vsam[card]) == 0)
		return(ERROR);
	*pval = pvsam->chan[signal];

	return(0);
}

/* dbior */
long vsam_report(int level)
{
	int i, j;
	bira_vsam *pvsam;
	float value;

	for ( i = 0; i < num_vsam_cards; i++ ) {
		if (pai_vsam[i]) {
			printf("  AI: Bira VSAM: card %d is present.\n",i);
			if ( level > 0 ) {
				pvsam = (bira_vsam *) pai_vsam[i];
				printf("    Address = %p, Status Register = 0x%04lx\n",pvsam,pvsam->status);
				if ( pvsam->status == CALIB_SUCCESS ) 
					printf("    (internal calibration successful)\n");
				else
					printf("    (internal calibration failed)\n");
			}
			if ( level > 1 ) {
				for ( j = 0; j < NUM_CHANNELS; j++ ) {
					vsam_driver(i,j,&value);
					printf("     #C%d S%.2d = %10.6f volts\n",i,j,value);
				}
			}
		}
	}
	return(0);
}
