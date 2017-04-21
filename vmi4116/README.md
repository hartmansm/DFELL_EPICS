# VMIVME-4116 DAC

Notice: My testing of this board has revealed that VMIVME-4116
modules prior to Revision P have a problem with a missing trace on
the printed circuit board to U6, pin 28. This prevents writing the
LSB of the data word to the channel 7 DAC. VMIC has generated an
ECO to correct this issue. The repair involves placing a jumper
from U6-p28 to U7-p28. Contact your VMIC sales rep for information
or for arranging warranty repair.
