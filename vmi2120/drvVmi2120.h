/* drvVmi2120.h */

struct vmivme_2120_registers {
	unsigned char dr0;
	unsigned char dr1;
	unsigned char dr2;
	unsigned char dr3;
	unsigned char dr4;
	unsigned char dr5;
	unsigned char dr6;
	unsigned char dr7;
	unsigned short csr;
};
typedef struct vmivme_2120_registers vmic2120;

/* control and status register definitions */
#define TEST_MODE	0x0080	/* disable outputs for built in test */
#define LED_ON		0x0040	/* turn on fail led */
#define	ACTIVE		0x0000	/* write to CSR to activate board */

long vmi2120_init(void);
int vmi2120_write(unsigned short card, unsigned short word, unsigned char *pval);
long vmi2120_report(int level);
