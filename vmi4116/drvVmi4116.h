/* drvVmi4116.h */

/* header file for VMIVME-4116 8-Channel, 16-bit Analog Ouput Board */

struct vmivme_4116_registers {
	unsigned short chan[8];
	unsigned short csr;
	unsigned short unused[7];
};
typedef struct vmivme_4116_registers vmic4116;

long vmi4116_report(int level);
long vmi4116_init(void);
int vmi4116_write(unsigned short card, unsigned int chan, unsigned long *pval);

#define AO_P3		0x0100	/* enable ao to P3 connector */
#define START_CONVERT	0x0200	/* for delayed update mode only */
#define	TESTBUS		0x0800	/* enable ao to test bus */
#define AINTESTBS	0x1000	/* enable test bus 1 */
#define AOTESTBS	0x2000	/* enable test bus 2 */
#define LED_OFF		0x4000	/* turn off fail LED */

