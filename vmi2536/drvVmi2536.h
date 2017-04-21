/* drvVmi2536.h */

struct vmivme_2536_registers {
	unsigned short bid;
	unsigned short csr;
	unsigned long input;
	unsigned long output;
	unsigned long reserved;
};
typedef struct vmivme_2536_registers vmic2536;

/* control and status register definitions */
#define LED_OFF		0x8000	/* turn off fail led */
#define OUTPUTS_ON	0x4000	/* activate outputs */
#define INPUTS_ON	0x2000	/* activate inputs */

long vmi2536_init(void);
int vmi2536_read(unsigned short card, unsigned int data, unsigned long *pval);
int vmi2536_write(unsigned short card, unsigned int data, unsigned long *pval);
long vmi2536_report(int level);
