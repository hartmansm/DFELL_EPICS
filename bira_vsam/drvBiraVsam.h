/* drvBiraVsam.h */

/* header file for BiRa 7305 VSAM */


struct bira_vsam_registers {
	float		chan[32];
	unsigned char	range[32];
	unsigned short	ac_component[32];
	unsigned long	system_reset;
	unsigned long	mode_control;
	unsigned long	status;
	unsigned long	unused0;
	unsigned long	diag_test;
	unsigned long	unused1[3];
};

typedef struct bira_vsam_registers bira_vsam;

long vsam_report(int level);
long vsam_init(void);
int vsam_driver(unsigned short card, unsigned int chan, float *pval);


#define NORMAL_MODE	0x00000000
#define FAST_SCAN	0x00000001
#define FIRMWARE	0x00000002
#define LITTLE_ENDIAN	0x00000004

#define	CALIB_SUCCESS	0x00000008

