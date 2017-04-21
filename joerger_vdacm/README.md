# Joerger VDACM Waveform Generator Module

The base address used by this driver for card 0 data is 0x110000 (set using switches A16-19 and A20-23) in standard address space. The base address for card 0 csr is 0x1100 (set using switches A08-11 and A12-15) in short address space. The base address for the interrupt vector is 0xa0. (Increment the least addressable bit for each of these for each additional card.) All of these base addresses can be modified at run time before iocInit in the st.cmd file. The IRQ level from 1 to 7 should be set on the card in two places. The jumper near P1 sets the IRQ level used by the board. The 3 jumpers I1, I2, I4 are used to allow this value to be read from the IRQ level register. This driver uses the value read from the IRQ register to configure the interrupt so it is important to set the level in both places to match.

The following functions of the VDACM are supported:

 * readback individual channel waveform
 * write individual channel waveform
 * operate the VDACM as a constant output DAC
 * interrupt at end of waveform
 * trigger by software or allow hardware trigger
 * set waveform start location
 * set waveform length
 * set clock frequency and source

The following device support routines are provided:

## device(waveform, VME_IO, devWoVdacm, "VDACM write waveform")

This waveform record is used to write a waveform to an individual
channel on the VDACM. (Note: the EPICS waveform record is an input
record, but is being used as an output record in this case.) In the
record's INP field, use the parameter to specify an offset into the
waveform for writing. Since the module provides 32K points per
channel, the offset makes it possible to use multiple PVs to access
different regions of each channel's memory and store multiple
waveforms for each channel which can be accessed using the start
and length functions below. The NELM field specifies the size of
the waveform associated with that PV. The FTVL field must be "SHORT".
As is typical for EPICS device support, the channels on the module
are counted from 0 to 7, even though the front panel of the card
labels the channels 1 to 8. The waveform data register cannot be
written to when the card is active. Device support will set the
record to INVALID on such an attempt.  The manual for the VDACM
describes the card as +/- 10 Volt ouput, or 0 to +10 Volt output.
This is selectable for each individual channel using two jumpers.
Note that the 0 to 10 Volt range looks like 5 Volts +/- 5 volts
when you read or write the waveforms.

## device(waveform, VME_IO, devWfVdacm, "VDACM read waveform")

This waveform record is used to readback an individual channel on
the VDACM. This record may be more useful for setup rather then for
actual operation, but may be useful for reading back a different
section of the channel's memory then specifies by the write waveform
records. As with the waveform write record, use the parameter field
to specify an offset, the NELM to specify length of the waveform
and the FTVL must be "SHORT". Channels are number 0 to 7 (not 1 to
8). The waveform data register cannot be read when the card is
active. Device support will set the record to INVALID in this case.

## device(ao, VME_IO, devAoVdacm, "VDACM DAC")

It is possible to use the VDACM as a standard 16-bit constant output
DAC. Use this record to set up for this mode. The parameter field
specifies a memory offset to use for the channel (such as for using
a different segment of memory for ramping waveforms and for DAC
output). It is the application designer's responsibility to ensure
proper behaviour if this mode is used in conjunction with the ramping
mode in the same application.

## device(ao, VME_IO, devAoConfigVdacm, "VDACM configure")

This ao record can be used to set the waveform start address or the
waveform length address to choose the addresses in memory to be
scanned for DAC outputs. In the output field, the S field is ignored
as this setting effects all channels on the module. The parameter
specifies which function: 's' or 'S' for start address, 'l' or 'L'
for memory length. The start and length settings are independent
from the waveform write record to allow the selection of different
waveforms which can be stored into different areas of memory. The
start or length registers cannot be written to when the card is
active. Device support will set the record to INVALID if they are
written to when the card is active.

## device(mbbo, VME_IO, devMbboVdacm, "VDACM clock")

The 16 choices for this mbbo record are used to specify clock source
(internal or external), and frequency (1/1, 1/2, 1/4, 1/8, 1/16,
1/32, 1/64 and 1/128 of interal 100 kHz clock or of external clock
frequency). The channel field of the OUT is ignored as this setting
controls all channels on the card.

## device(bo, VME_IO, devBoVdacm, "VDACM trigger")

This bo record can be used as a software trigger to cause the card
to process the loaded waveforms. The channel field of the OUT is
ignored as the trigger effects all channels on the card. Note:
external triggers are also allowed and can be set by a TTL signal
to the VDACM's trigger input.

## device(bi, VME_IO, devBiVdacm, "VDACM active")

This bi record can be used to check the activity of the card, i.e.
whether the card is active. When the card is active, you cannot
read or write waveforms, or set start address or waveform length.

## device(event, VME_IO, devEventVdacm, "VDACM sync")

The VDACM can generate in interrupt when it has finished running a
waveform. This event record can be processed on I/O Intr to allow
EPICS processing of additional records at the end of the waveform.

## driver drvJoergerVdacm

The driver support drvJoergerVdacm provides the general functions
used by device support. These functions could also be used independently
of EPICS.
