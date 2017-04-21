/* drvIsegVHS.c */

/*

EPICS driver for ISEG Precision High Voltage Power Supply
VHS Multi Channel Series (4 or 12 channel).

Author: Steven Hartman
	Duke Free-Electron Laser Laboratory
	<hartman@fel.duke.edu>

Version:        1.0
Please check <www.fel.duke.edu/epics> for the most recent version.

Copyright (c) 2007 Duke University

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

#include "drvIsegVHS.h"

/* these can be changed in st.cmd before iocInit if necessary */
unsigned int num_iseg_vhs_cards = 2; /* number of cards in crate */
unsigned long iseg_vhs_base = 0x4000; /* base address of card 0 (A16) */


struct {
	long            number;
	DRVSUPFUN       report;
	DRVSUPFUN       init;
} drvIsegVHS = {
	2,
	iseg_vhs_report,
	iseg_vhs_init
};

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvIsegVHS);
#endif

static unsigned short **pvhs;
static int vhs_addr;

/* initialization */
long iseg_vhs_init(void) {
	unsigned short **boards_present;
	long val;
	int	status;
	short i;
	iseg_vhs * board;

	pvhs = calloc(num_iseg_vhs_cards,sizeof(*pvhs));
	if (!pvhs) {
		return ERROR;
	}

	boards_present = pvhs;

	if ((status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)iseg_vhs_base,(char **)&vhs_addr)) !=OK ) {
		printf("Addressing error in iseg_vhs_init\n");
		return ERROR;
	}

	board = (iseg_vhs *)vhs_addr;

	for ( i = 0; i < num_iseg_vhs_cards; i++, board++, boards_present++) {
		if (vxMemProbe((char *)board,VX_READ,sizeof(long),(char *)&val) == OK ) {
			*boards_present = (unsigned short *)board;

			/* setup board with kill disabled, no fine adjustment,
			 * no interrupt (not supported by card), no special
			 * mode, and clear any kill signals and event signals
			*/
			board->module_data.control = CLEAR;
		}
		else {
			*boards_present = 0;
		}
	}
	return 0;
}

/* enable or disable high voltage for a channel */
long iseg_vhs_chan_on(unsigned int card, unsigned int chan, unsigned int *pval) {
	iseg_vhs * board;

	if ( (board = (iseg_vhs *)pvhs[card]) == 0)
		return ERROR;
	
	if (*pval)
		board->channels[chan].control = SET_ON; /* enable HV */
	else
		board->channels[chan].control = SET_OFF; /* disable HV */
	
	return 0;
}

/* read voltage for a channel */
long iseg_vhs_vread(unsigned int card, unsigned int chan, float *pvolts) {

	uint16_t * pvalue;
	iseg_vhs * board;

	if ( (board = (iseg_vhs *)pvhs[card]) == 0) 
		return ERROR;

	/* voltage read register is 32-bit floating point, but card only
	 * supports 16-bit data transfers.
	*/
	pvalue = (uint16_t *)pvolts;
	*pvalue = board->channels[chan].f_voltage_read[1];
	*pvalue++ = board->channels[chan].f_voltage_read[0];

	return 0;
}

/* read current for a channel */
long iseg_vhs_iread(unsigned int card, unsigned int chan, float *pamps) {

	uint16_t * pvalue;
	iseg_vhs * board;

	if ( (board = (iseg_vhs *)pvhs[card]) == 0) 
		return ERROR;

	/* current read register is 32-bit floating point, but card only
	 * supports 16-bit data transfers.
	*/
	pvalue = (uint16_t *)pamps;
	*pvalue = board->channels[chan].f_current_read[1];
	*pvalue++ = board->channels[chan].f_current_read[0];

	return 0;
}


/* set voltage for a channel */
long iseg_vhs_vwrite(unsigned int card, unsigned int chan, float *pvolts) {

	uint16_t * pvalue;
	iseg_vhs * board;

	if ( (board = (iseg_vhs *)pvhs[card]) == 0) 
		return ERROR;
	
	/* voltage set register is 32-bit floating point, but card only
	 * supports 16-bit data transfers.
	*/
	pvalue = (uint16_t *)pvolts;
	board->channels[chan].f_voltage_set[1] = *pvalue;
	board->channels[chan].f_voltage_set[0] = *pvalue++;

	return 0;
}

/* set current for a channel */
long iseg_vhs_iwrite(unsigned int card, unsigned int chan, float *pamps) {

	uint16_t * pvalue;
	iseg_vhs * board;

	if ( (board = (iseg_vhs *)pvhs[card]) == 0) 
		return ERROR;
	
	/* current set register is 32-bit floating point, but card only
	 * supports 16-bit data transfers.
	*/
	pvalue = (uint16_t *)pamps;
	board->channels[chan].f_current_set[1] = *pvalue;
	board->channels[chan].f_current_set[0] = *pvalue++;

	return 0;
}

/* set voltage ramp speed for a card */
long iseg_vhs_vrampspeed(unsigned int card, float *pspeed) {

	uint16_t * pvalue;
	iseg_vhs * board;

	if ( (board = (iseg_vhs *)pvhs[card]) == 0) 
		return ERROR;
	
	/* ramp speed set register is 32-bit floating point, but card only
	 * supports 16-bit data transfers.
	*/
	pvalue = (uint16_t *)pspeed;
	board->module_data.f_voltage_ramp_speed[1] = *pvalue;
	board->module_data.f_voltage_ramp_speed[0] = *pvalue++;

	return 0;
}



/* dbior, hardware report */
long iseg_vhs_report(int level) {
	int i, j;
	iseg_vhs * board;
	uint16_t * pvalue;
	int numchans = 0;
	long serialnumber;
	float nomv;
	float nomi;
	float maxv;
	float maxi;
	float temperature;
	float supply_p5;
	float supply_p12;
	float supply_n12;

	for ( i = 0; i < num_iseg_vhs_cards; i++ ) {
		if (pvhs[i]) {
			printf("  ISEG VHS card %d is present.\n",i);
			if ( level > 0 ) {
				board = (iseg_vhs *) pvhs[i];
				serialnumber = ( board->module_data.serial_number[0] << 16 ) + board->module_data.serial_number[1];
				printf("    Address = %p, Status = 0x%x\n",board,board->module_data.status);
				printf("    Serial Number = %ld, Firmware = 0x%04x%04x, Device Class = 0x%x\n", 
				serialnumber,
				board->module_data.firmware_release[0],  \
				board->module_data.firmware_release[1], \
				board->module_data.device_class);
				if ( board->module_data.placed_chans == 0xf ) {
					printf("    4 Channel Version (VHS 40x)\n");
					numchans = 4;
				}
				else if ( board->module_data.placed_chans == 0xfff ) {
					printf("    12 Channel Version (VHS C0x)\n");
					numchans = 12;
				}
				else
					printf("Unknown Channel Version!\n");	
				printf("    Chan\tNom. Voltage\tNom. Current\n");
				for (j = 0; j < numchans; j++ ) {
					printf("      %d",j);
					pvalue = (uint16_t *)&nomv;
					*pvalue = board->channels[j].f_voltage_nominal[1];
					*pvalue++ = board->channels[j].f_voltage_nominal[0];
					printf("    \t  %.2f V",nomv);

					pvalue = (uint16_t *)&nomi;
					*pvalue = board->channels[j].f_current_nominal[1];
					*pvalue++ = board->channels[j].f_current_nominal[0];
					printf("    \t  %.2e A\n",nomi);
				}
					
					
				if (level > 1 ) {
					pvalue = (uint16_t *)&maxv;
					*pvalue = board->module_data.f_voltage_max[1];
					*pvalue++ = board->module_data.f_voltage_max[0];
					printf("    Front panel pot setting: Max Voltage = %.2f%%, ",maxv);

					pvalue = (uint16_t *)&maxi;
					*pvalue = board->module_data.f_current_max[1];
					*pvalue++ = board->module_data.f_current_max[0];
					printf("Max Current = %.2f%%\n",maxi);

					if ( level > 2 ) {
						pvalue = (uint16_t *)&temperature;
						*pvalue = board->module_data.f_temperature[1];
						*pvalue++ = board->module_data.f_temperature[0];
						printf("    Temperature = %.2f C, ",temperature);

						pvalue = (uint16_t *)&supply_p5;
						*pvalue = board->module_data.f_supply_p5[1];
						*pvalue++ = board->module_data.f_supply_p5[0];
						printf("PSU P5 = %.2f V, ",supply_p5);

						pvalue = (uint16_t *)&supply_p12;
						*pvalue = board->module_data.f_supply_p12[1];
						*pvalue++ = board->module_data.f_supply_p12[0];
						printf("P12 = %.2f V, ",supply_p12);

						pvalue = (uint16_t *)&supply_n12;
						*pvalue = board->module_data.f_supply_n12[1];
						*pvalue++ = board->module_data.f_supply_n12[0];
						printf("N12 = %.2f V\n",supply_n12);
					}

				}
			}
		}
	}
	return 0;
}
