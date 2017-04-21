/* drvVmi4132.h */

/* header file for VMIVME-4132 32-Channel Analog Ouput Board */

struct vmivme_4132_registers {
	unsigned short bid;
	unsigned short csr;
	unsigned short adc;
	unsigned short unused[29];
	unsigned short chan[32];
};
typedef struct vmivme_4132_registers vmic4132;

long vmi4132_report(int level);
long vmi4132_init(void);
int vmi4132_write(unsigned short card, unsigned int chan, unsigned long *pval);


#define SHORT_SETTLE	0x8000
#define LED_OFF		0x4000
#define START_CONV	0x2000
#define BINARY_OFFSET	0x1000
#define ONLINE		0x0800
#define FAST_REFRESH	0x0100
#define START_SETTLE	0x0040
#define SCAN_HALT	0x0020

#define DATA_READY	0x8000
#define CONV_BUSY	0x2000
#define SETTLE_BUSY	0x0040

