/* drvVmi1182.h */

struct vmivme_1182_registers {
	unsigned short bid;
	unsigned short csr;
	unsigned short str;
	unsigned short ivr;
	unsigned short data[4];
	unsigned short cos[4];
	unsigned short pai[4];
	unsigned short dsr[64];
	unsigned short pac[64];
	unsigned short ttr;
	unsigned short maxsoe;
	unsigned short soecr;
	unsigned short soeir;
	unsigned short unused1;
	unsigned short ac;
	unsigned short unused2;
	unsigned short rlr;
	unsigned short unused3[872];
	unsigned short forceled;
	unsigned short soerr;
	unsigned short unused[254];
	unsigned short soebuf[6912];
};
typedef struct vmivme_1182_registers vmic1182;

/* control and status register definitions */
#define BID		0x3800	/* board ID */
#define SELFTEST_OK	0x0cba	/* indicates successful self test of board */
#define LED_OFF		0x8000	/* turn off fail led */
#define TESTMODE_OFF	0x4000	/* turn off test mode */

/* debounce select register */
#define COS_DISABLE	0x0000	/* disable change of state interrupt */
#define COS_FALLING	0x4000	/* COS interrupt on falling edge */
#define COS_RISING	0x8000	/* COS interrupt on rising edge */
#define COS_BOTH	0xc000	/* COS interrupt on both edges */

/* function declarations */
long vmi1182_init(void);
int vmi1182_read(unsigned short card, unsigned short word, unsigned short *pval);
long vmi1182_report(int level);
