/* drvJoergerVdacm.c */

/* EPICS driver for Joerger VDACM 8 channel, 16-bit waveform generator */

/*
 Copyright (C) 2005  Duke University
 Author: Steven Hartman <hartman@fel.duke.edu>, Duke FEL Laboratory
 Version: 1.2
 Please check <www.fel.duke.edu/epics> for the most recent version.

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
#include <vxLib.h>
#include <sysLib.h>
#include <vme.h>
#include <intLib.h>
#include <iv.h>
#include <logLib.h>

#include <dbDefs.h>
#include <dbScan.h>
#include <drvSup.h>
#include <epicsVersion.h>
#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	#include <epicsExport.h>
#endif

#include "drvJoergerVdacm.h"

#define NUMCHANS 8	/* channels per card */
#define DATAWORDS 0x8000 /* number of data points available per channel */

/* these can be changed in st.cmd before iocInit if necessary */
unsigned int vdacm_num_cards = 8;	/* number of cards in crate */
unsigned int vdacm_data_base = 0x110000;  /* base address of data in A24 */
unsigned int vdacm_register_base = 0x1100; /* base address of register in A16 */
unsigned int vdacm_ivec_base = 0xa0; /* assign an interrupt vector */

long vdacm_init(void);
long vdacm_report(int);
IOSCANPVT *vdacm_ioscanpvt = 0;

struct {
	long		number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} drvJoergerVdacm = {
	2,
	vdacm_report,
	vdacm_init };

#if (EPICS_VERSION==3) && (EPICS_REVISION>=14)
	epicsExportAddress(drvet,drvJoergerVdacm);
#endif

static unsigned short **pvdacm_register;
static unsigned short **pvdacm_data;
static int vdacm_register_addr;
static int vdacm_data_addr;

static long vdacm_isr(unsigned short card) 
{
	vdacm_register *pregister;

	if ((pregister = (vdacm_register *)pvdacm_register[card]) == 0 ) {
		logMsg("drvJoergerVdacm: in vdacm_isr() Cannot clear interrupt register on vdacm card #%d\n",card,0,0,0,0,0);
		return ERROR;
	}

	pregister->int_reset = RESET;
	scanIoRequest(vdacm_ioscanpvt[card]);
	return 0;
}

long vdacm_init(void)
{
	unsigned short **board_register_present;
	unsigned short **board_data_present;
	short shval;
	int status;
	int interrupt_level;
	int interrupt_vector;
	short i;
	vdacm_register * board_register;
	vdacm_data * board_data;

	pvdacm_register = calloc(vdacm_num_cards,sizeof(*pvdacm_register));
	if ( !pvdacm_register ) {
		printf("drvJoergerVdacm: failed to allocate space for register in vdacm_init\n");
		return ERROR;
	}
	pvdacm_data = calloc(vdacm_num_cards,sizeof(*pvdacm_data));
	if ( !pvdacm_data ) {
		printf("drvJoergerVdacm: failed to allocate space for data in vdacm_init\n");
		return ERROR;
	}
	vdacm_ioscanpvt = calloc(vdacm_num_cards,sizeof(*vdacm_ioscanpvt));
	if ( !vdacm_ioscanpvt ) {
		printf("drvJoergerVdacm: failed to allocate space for ioscanpvt in vdacm_init\n");
		return ERROR;
	}
	
	board_register_present = pvdacm_register;
	board_data_present = pvdacm_data;

	if ((status = sysBusToLocalAdrs(VME_AM_SUP_SHORT_IO,(char *)(long)vdacm_register_base, (char **)&vdacm_register_addr)) != OK ) {
		printf("drvJoergerVdacm: Addressing Error in vdacm_init for control register.\n");
		return ERROR;
	}

	if ((status = sysBusToLocalAdrs(VME_AM_STD_SUP_DATA,(char *)(long)vdacm_data_base, (char **)&vdacm_data_addr)) != OK ) {
		printf("drvJoergerVdacm: Addressing Error in vdacm_init for data register.\n");
		return ERROR;
	}

	board_register = (vdacm_register *)((int)vdacm_register_addr);
	board_data = (vdacm_data *)((int)vdacm_data_addr);

	for ( i = 0; i < vdacm_num_cards; i++, board_register++, board_data++, board_register_present++, board_data_present++ ) {
		if(vxMemProbe((char *)board_register,VX_READ,sizeof(short),(char *)&shval) == OK) {
			*board_register_present = (unsigned short *)board_register;
			*board_data_present = (unsigned short *)board_data;
			
			/* initialize board */

			board_register->dac_reset = RESET; /* arbitrary data to reset all DAC outputs */
			board_register->update_enable = ENABLECHANALL;
			board_register->csr = CSR_DEFAULT | SYNC_INIT ;

			/* setup interrupts to let us know when the waveform has been run */
			interrupt_vector = vdacm_ivec_base + i;
			scanIoInit(&vdacm_ioscanpvt[i]);
			if ( intConnect(INUM_TO_IVEC(interrupt_vector),(VOIDFUNCPTR)vdacm_isr,i) != OK ) {
				printf("drvJoergerVdacm: intConnect failed\n");
				return ERROR;
			}
			/* interrupt level is coded on board using jumbers (in 2 places!)
			 * (See jumbers near P1 and at I1,I2,I4 which must match)
			 * This value is readable from IRQ register so use that to
			 * set interrupt level here */
			interrupt_level =  ( board_register->irq & 0x07 ); /* last 3 bits */
			if ( sysIntEnable(interrupt_level) != OK ) {
				printf("drvJoergerVdacm: sysIntEnable failed\n");
				return ERROR;
			}
			/* the register named "interrupt status/ID register" is
			 * used to encode the chosen interrupt vector */
			board_register->int_status = interrupt_vector;
			/* enable IRQ response */
			board_register->irq = ENABLE_IRQ;

		}
		else {
			*board_register_present = 0;
			*board_data_present = 0;
		}
	}

    return 0;
}

int vdacm_trigger(unsigned short card, unsigned char *mode, unsigned char direction)
{
	vdacm_register *pregister;

	if ((pregister = (vdacm_register *)pvdacm_register[card]) == 0 ) {
		return ERROR;
	}
	if ( direction == VDACM_READ ) {
		*mode = ( pregister->csr & ACTIVE ); /* bit 0 is active status */
		return 0;
	}

	pregister->trigger = *mode;

	return 0;
}

int vdacm_update_enable(unsigned short card, unsigned short *mode, unsigned char direction)
{
	vdacm_register *pregister;

	if ((pregister = (vdacm_register *)pvdacm_register[card]) == 0 ) {
		return ERROR;
	}
	if ( direction == VDACM_READ ) {
		*mode = pregister->update_enable;
		return 0;
	}

	pregister->update_enable = *mode;

	return 0;

}

int vdacm_length(unsigned short card, unsigned short *length, unsigned char direction)
{
	vdacm_register *pregister;

	if ((pregister = (vdacm_register *)pvdacm_register[card]) == 0 ) {
		return ERROR;
	}

	if ( pregister->csr & ACTIVE ) {
		printf("drvJoergerVdacm (vdacm_length): cannot set length while card is active\n");
		return ERROR;
	}

	if ( direction == VDACM_READ ) {
		*length = pregister->length_hi << 8; /* hight byte */
		*length += pregister->length_lo & 0x00ff; /* low byte */
		return 0;
	}

	pregister->length_hi = ( *length >> 8); /* high byte */
	pregister->length_lo = ( *length & 0x00ff); /* low byte */

	return 0;
}

int vdacm_start(unsigned short card, unsigned short *location, unsigned char direction)
{
	vdacm_register *pregister;

	if ( *location > DATAWORDS ) {
		printf("drvJoergerVdacm (vdacm_start): location out of range\n");
		return(ERROR);
	}

	if ((pregister = (vdacm_register *)pvdacm_register[card]) == 0 ) {
		return ERROR;
	}

	if ( pregister->csr & ACTIVE ) {
		printf("drvJoergerVdacm (vdacm_start): cannot set start location while card is active\n");
		return ERROR;
	}

	if ( direction == VDACM_READ ) {
		*location = pregister->start_hi << 8; /* high byte */
		*location += pregister->start_lo & 0x00ff; /* low byte */
		return 0;
	}

	pregister->start_hi = ( *location >> 8 );  /* high byte */
	pregister->start_lo = ( *location & 0x00ff ); /* low byte */

	return 0;
}

int vdacm_clock(unsigned short card, unsigned char *mode, unsigned char direction)
{
	vdacm_register *pregister;

	if ((pregister = (vdacm_register *)pvdacm_register[card]) == 0 ) {
		return ERROR;
	}

	if ( direction == VDACM_READ ) {
		*mode = pregister->clock;
		return 0;
	}

	pregister->clock = *mode;

	return 0;
}

int vdacm_write(unsigned short card, unsigned int signal, unsigned int offset, unsigned int length, signed short *valuebuf)
{
	vdacm_register *pregister;
	vdacm_data *pdata;
	int i;

	if ( (length + offset) > DATAWORDS ) {
		printf("drvJoergerVdacm (vdacm_write): length (%d) plus offset (%d) out of range (%d)\n", length, offset, DATAWORDS);
		return ERROR;
	}

	if ((pregister = (vdacm_register *)pvdacm_register[card]) == 0 ) {
		return ERROR;
	}

	if ((pdata = (vdacm_data *)pvdacm_data[card]) == 0 ) {
		return ERROR;
	}

	if ( pregister->csr & ACTIVE ) {
		printf("drvJoergerVdacm (vdacm_write): cannot write data while card is active\n");
		return ERROR;
	}

	pregister->write_channel = ( 1 << signal );

	for ( i = 0; i < length; i++ ) { 
		pdata->data[i + offset] = valuebuf[i];
	}

	return 0;
}

int vdacm_read(unsigned short card, unsigned int signal, unsigned int offset, unsigned int length, signed short *valuebuf)
{
	vdacm_register *pregister;
	vdacm_data *pdata;
	int i;

	if ( (length + offset) > DATAWORDS ) {
		printf("drvJoergerVdacm (vdacm_write): length (%d) plus offset (%d) out of range (%d)\n", length, offset, DATAWORDS);
		return ERROR;
	}

	if ((pregister = (vdacm_register *)pvdacm_register[card]) == 0 ) {
		return ERROR;
	}
	if ((pdata = (vdacm_data *)pvdacm_data[card]) == 0 ) {
		return ERROR;
	}

	if ( pregister->csr & ACTIVE ) {
		printf("drvJoergerVdacm (vdacm_read): cannot read data while card is active\n");
		return ERROR;
	}

	pregister->read_channel = signal;

	for ( i = 0; i < length; i++ ) { 
		valuebuf[i] = pdata->data[i + offset];
	}

	return 0;
}	

long vdacm_report(int level)
{

	vdacm_register *pregister;
	vdacm_data *pdata;
	int i;

	for ( i = 0; i < vdacm_num_cards; i++, pregister++ ) {
		if ( pvdacm_register[i] ) {
			printf("VDACM Waveform Genenerator: card %d is present.\n",i);
			if (level > 0) {
				pregister = (vdacm_register *)pvdacm_register[i];
				pdata = (vdacm_data *)pvdacm_data[i];
				printf("    Register Address = %p, Data Address = %p\
					\n    CSR = 0x%x, Clock = 0x%x, Update Enable = 0x%x, \
					\n    IRQ Register = 0x%x, Interupt Status = 0x%x, \
					\n    Read Channel = 0x%x, Write Channel = 0x%x, \
					\n    Length (high byte) = 0x%x, Length (low byte) = 0x%x, \
					\n    Start Location (high byte) = 0x%x, Start Location (low byte) = 0x%x\n", \
					pregister, pdata, \
					pregister->csr, pregister->clock, pregister->update_enable,\
					pregister->irq, pregister->int_status, \
					pregister->read_channel, pregister->write_channel, \
					pregister->length_hi, pregister->length_lo, \
					pregister->start_hi, pregister->start_lo);
			}
		}
	}

	return 0;
}

