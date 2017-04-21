# VMIVME-3122 ADC Module (waveform)

This module, as distributed, configures the VMIVME-3122 as a single
channel waveform recorder / transient digitizer with a software
trigger. The card is capable of recording up to 100 kilo-samples
per second with memory for up to 1024 16-bit samples.

Prior to iocInit, the function Vmi3122Config must be called:

```Vmi3122Config(int numcards, unsigned long baseoffset, unsigned int buffer, unsigned long rate, unsigned int irq_level, unsigned char ivec_base); ```

See the st.cmd file included in the distribution for an example.
The baseoffset must be on a 2048-word boundary and correspond to
the setting on jumbers E5, E6 and E8. (The board should also be
configured to use supervisory access (E13) and standard address
space (E11)). The buffer size must be 16, 32, 64, 128, 256, 512 or
1024 and corresponds to the number of samples to record. This should
match the NELM field on the waveform record. The rate is in
samples/second with 100,000 the maximum.

On software trigger (caused by processing the bo trigger record),
the ADC will begin sampling at the rate specified until the buffer
is filled. An interrupt will be generated when the buffer is filled
which will cause the waveform record to process. The distribution
includes a sample database with these two records.

The board is also capable of external triggering using the P2
connector. If you choose to do this, edit the driver to change
SOFT_TRIG to EXT_TRIG and recompile. The board can also be used in
autoscanning mode. Redefine SCAN and recompile.
