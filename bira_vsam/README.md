# BiRa 7305 VSAM Support Module

The VSAM uses the unusual A24/D32 access only. It will not fall
back to a smaller data access such as D16. The VxWorks BSP for the
mv167 board (and likely others) does not support this mode, offering
a maximum of D16 in A24. You will need to patch sysLib.c to allow
A24/D32 available from for the mv167. In my experience, other A24
cards continued to work fine after this patch, with the one exception
of the Creative Electronic System's (CES) CBD 8210 (VME to CAMAC
Branch Driver).

The diff for $WIND_BASE/target/config/mv167/sysLib.c for VxWorks 5.4 is:
```
	> 3feb04,slacpatch   changed A24/D16 to A24/D32 for the BiRa VSAM
	387c388
	<      *   STD (A24/D16)  0xf0000000 - 0xf0ffffff
	---
	>      *   STD (A24/D32)  0xf0000000 - 0xf0ffffff
	406,407c407,408
	<     *VMECHIP2_LBSAR           |= LBSAR2_AM_STD_USR_DATA |
	<                                  LBSAR2_D16;
	---
	>     *VMECHIP2_LBSAR           |= LBSAR2_AM_STD_USR_DATA | 
	>                                  LBSAR2_D32;
	
```

Other BSPs have not been tested but will likely also need to be
modified.  

BiRa also makes a CAMAC version of the SAM (BiRa 5305) with EPICS
software available in this repository in the devCamac directory.

