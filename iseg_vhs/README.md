# iseg VHS Module

## VME addressing

The base address used by this driver for card 0 is 0x4000 in A16
(address for card as shipped). To change this, follow the iseg VHS
manual instructions for reprogramming the EEPROM and define
iseg_vhs_base in st.cmd to match. Card 1 should be set to an address
of 0x4400 (base + 1024 bytes) 

## Functions Supported

The following functions of the VHS are supported:
 * enable/disable HV for an individual channel (bo)
 * set channel voltage (in volts) (ao)
 * set channel maximum current (in Amps) (ao)
 * set card voltage ramping speed (in percent, max 20%) (ao)
 * read channel voltage (in volts) (ai)
 * read channel current (in Amps) (ai)

In addition, an iseg_vhs_report(int) routine is provided to check some hardware parameters:

 * VME address
 * module status
 * serial number
 * firmware version
 * device class (4 or 12 channel version)
 * nominal voltage and current for each channel
 * front panel pot settings for maximum voltage and maximum current
 * board temperature
 * readbacks for power supplies P5, P12 and N12

This routine can be called directly from the shell or from the EPICS dbior function.

## Specifying INP and OUT parameters

For the ai and ao records, use the parm field of the VME address
(after the @) to specify the function: v or V for voltage readback
or set, i or I for current readback or set and s or S for voltage
ramping speed set. See the included .db file for examples.

