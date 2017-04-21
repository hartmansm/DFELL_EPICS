# CAMAC Device Support Routines

This software requires CamacLib

The SAM is a 32 channel module. CamacLib defines MAX_SUBADDR as 15
(corresponding to the 16 subaddresses in the CAMAC standard). In
order to use all 32 channels on a SAM module, redefine MAX_SUBADDR
as 31. No other changes are needed since reads from the SAM do not
use the A field to address the SAM channel.
