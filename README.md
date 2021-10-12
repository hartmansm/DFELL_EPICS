# DFELL_EPICS

This repository containts
EPICS device support originally developed for Duke Free Electron
Laser Laboratory. The software is not being actively developed but
is still being used by multiple facilities. These software modules
were originally hosted at http://www.fel.duke.edu/epics/ and were 
moved here as-is when that web server was taken offline. 

Most of these modules were originally written for EPICS 3.13. They
have been updated to also work with 3.14. However, they are still
using VxWorks specific code. They do not take advantage of the EPICS
OSI layer available in 3.14. They can currently be used with EPICS
3.13 or 3.14 (VxWorks only) but should be updated to work with newer
releases and on different operating systems.

## Hardware Supported (VME)

### BiRa 7305 VSAM Support Module (ai)

* BiRa Systems -- 7305 VSAM 32 Channel, Smart Analog Monitor (ai)

### Highland Technology V850/V851 Digital Delay Generator Module

 * Highland Technology -- Highland V850/V851 4 or 6 channel Digital Delay Generator with 40 picosecond resolution
 * Berkeley Nucleonics -- BNC B950/B951 4 or 6 channel Digital Delay Generator with 40 picosecond resolution

### Hytec VDD 2670 DAC Module (ao)

* Hytec -- VDD 2670 2 Channel, 18-Bit Analog Output Board (ao)

### iseg VHS Module (bo, ai, ao)

 * iseg Spezialelektronik GmbH -- VHS 4/C 0xxx Precision VME High Voltage Power Supply, VHS Multi Channel Series (4 or 12 channels)

### Joerger VDACM Waveform Generator Module

 * Joerger -- VDACM 8 Channel, 16-Bit Waveform Generator Board

### VMIVME-1182 BI Module (bi)

 * VMIC -- VMIVME-1182 64-channel, Isolated Digital Input Board (bi)

### VMIVME-2120 BO Module (bo)

 * VMIC -- VMIVME-2120 64-channel, Digital Output Board (bo)

### VMIVME-2536 Digital I/O Module (bi, bo)

 * VMIC -- VMIVME-2536 Digital I/O (bi, bo) Board

### VMIVME-3113A ADC Module (ai)

 * VMIC -- VMIVME-3113A Scanning 12-bit Analog-to-Digital Converter Board (ai)

### VMIVME-3122 ADC Module (ai, waveform)

 * VMIC -- VMIVME-3122 VMIVME-3122 high performance 16-bit , 64 channel ADC, 100kHz, 1024 samples
 Three implementations are included here: 
   * vmi3122 (preferred) as a 64 channel ADC
   * vmic3122aiavg as a 64 channel ADC with built-in sample averaging
   * vmic3122wf as a transient digitizer

### VMIVME-4116 DAC Module (ao)

 * VMIC -- VMIVME-4116 8 Channel, 16-Bit Analog Output Board (ao)

### VMIVME-4132 DAC Module (ao)

 * VMIC -- VMIVME-4132 32 Channel, 12-Bit Analog Output Board (ao)
 
## Hardware Supported (CAMAC)

 ### CAMAC Device Support Routines 

(Requires camacLib)

 * DSP -- 3016 DAC
16 channel, 16-bit CAMAC digital to analog converter module.
 * DSP -- 2032 ADC
32 channel Scanning Digital Voltmeter (equivalent to 5305 SAM).
 * BiRa -- 5305 SAM
CAMAC module with 32 differential channels of slow analog to digital conversion with automatic ranging, polarity and calibration.
 * DSP --  E100 Binary Input
CAMAC module with 32 channels.
 * BiRa -- 2324 Binary Input
CAMAC module with dual 24-bit parallel input register.
 * BiRa -- 3224 Binary Output
CAMAC module with dual 24-bit digital output register.
 * BINP -- D0609 ADC
20-bit CAMAC integrating digital voltmeter.
 * BINP -- D0612 ADC
20-bit CAMAC integrating digital voltmeter.
 * BINP -- D0612 ADC with A0604 MUX
20-bit CAMAC integrating digital voltmeter, with A0604 16-channel thermocompensated analog multiplexer.
 * BINP -- D0643 DAC
1 channel, 20-bit CAMAC digital to analog converter.
 * BINP -- USD-2
CAMAC Stepper Motor Driver


