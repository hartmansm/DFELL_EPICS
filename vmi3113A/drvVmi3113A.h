/* drvVmi3113A.h */

/* header file for VMIVME-3113A scanning 12-bit ADC */

struct  vmivme_3113A_registers {
	unsigned short config_register[64];
	unsigned short chan[64];	
}; 
typedef struct vmivme_3113A_registers vmic3113A;


long vmi3113A_report(int level);
long vmi3113A_init(void);
int vmi3113A_driver(unsigned short card, unsigned int chan, unsigned long *pval);


#define bid	config_register[0x00]	/* board id register */
#define csr	config_register[0x01]	/* control & status register */
#define cfr	config_register[0x02]	/* configuration control register */
#define icr	config_register[0x05]	/* interrupt control register */
#define ivr    	config_register[0x09]	/* interrupt vector register */
#define tm0	config_register[0x10]	/* timer 0 register */
#define tm1	config_register[0x11]	/* timer 1 register */
#define tm2	config_register[0x12]	/* timer 2 register */
#define tmc	config_register[0x13]	/* timer control register */

#define VALID_3113A_ID	0x1100 /* board ID */

#define GAIN_DELAY		0x8000 
#define FAIL_LED_OFF		0x4000
#define START_CONVERT		0x2000
#define TWOS_COMPLEMENT		0x1000
#define EN_EXTERNAL_START	0x0800
#define TIMER_START		0x0400
#define SOFTWARE_RESET		0x0200
#define AUTOSCAN_BIT		0x01C0
#define SCANNING_INTERRUPT	0x0100
#define SCANNING_POLL		0x00C0
#define RANDOM_INTERRUPT	0x0080
#define RANDOM_POLL		0x0040
#define AUTOSCANNING		0x0000
#define BIT_4			0x0003
#define BIT_3			0x0002
#define BIT_2			0x0001
#define BIT_1			0x0000
#define MUX_CH0			0x0000
#define MUX_END			0x003E

#define NEW_DATA_READY		0x8000
#define END_OF_SCAN		0x2000

#define FLAG_BIT		0x80
#define FLAG_AUTO_CLEAR		0x40
#define EXTERNAL_VECTOR		0x20
#define INTERRUPT_ENABLE	0x10
#define INTR_AUTO_CLEAE		0x08
#define REQUEST_LEVEL_7		0x07
#define REQUEST_LEVEL_6		0x06
#define REQUEST_LEVEL_5		0x05
#define REQUEST_LEVEL_4		0x04
#define REQUEST_LEVEL_3		0x03
#define REQUEST_LEVEL_2		0x02
#define REQUEST_LEVEL_1		0x01
#define INTERRUPTS_OFF		0x00

#define READ_BACK_COMMAND	0xC0 /* tmc bit def's */
#define SELECT_COUNTER_2	0x80
#define SELECT_COUNTER_1	0x40
#define SELECT_COUNTER_0	0x00
#define RW_LSB_MSB		0x30
#define RW_MSB			0x20
#define RW_LSB			0x10
#define LATCH_COUNTER		0x00
#define HARD_TRIG_STROBE	0x0A
#define SOFT_TRIG_STROBE	0x08
#define SQUARE_WAVE		0x06
#define RATE_GENERATOR		0x04
#define HARD_TRIG_ONE_SHOT	0x02
#define INTR_ON_TERM_COUNT	0x00
#define BCD_COUNTER		0x01
#define BINARY_COUNTER		0x00

