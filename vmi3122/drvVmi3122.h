/* drvVmi3122.h */

struct  vmivme_3122_registers {
        unsigned short regs[128/2];
        unsigned short data[1024];       
	unsigned short scratch[960];
}; 

typedef struct vmivme_3122_registers vmic3122;


/* memory map register offsets  and sizes*/
#define bir     regs[0x00/2] /* board ID register */
#define csr     regs[0x02/2] /* control and status register */
#define ccr     regs[0x04/2] /* configuration control register */  
#define rcr	regs[0x06/2] /* rate control register */
#define icr	regs[0x08/2] /* interrupt control register */
#define ivr	regs[0x0A/2] /* interrupt vector register */
#define src	regs[0x0C/2] /* software reset command */
#define stc	regs[0x0E/2] /* software trigger command */
#define gain	regs[0x10/2] /* auto gain */
#define tr0	regs[0x20/2] /* interval timer 0 register */
#define tr1	regs[0x22/2] /* interval timer 1 register */
#define dcr	regs[0x24/2] /* data counter register */
#define tcr	regs[0x26/2] /* timer control register */

/* control and status register definitions */
#define LED_ON		0x0000 /* bit 15 */
#define LED_OFF		0x8000 /* bit 15 */
#define OFFSET_BIN	0x0000 /* bit 14 */
#define TWOS_COMP	0x4000 /* bit 14 */
#define SOFT_TRIG	0x0000 /* bits 13 and 12 */
#define EXT_TRIG	0x1000 /* bits 13 and 12 */
#define TIME_TRIG	0x2000 /* bits 13 and 12 */
#define END_BUFFER	0x0000 /* bit 10 */
#define MID_BUFFER	0x0400 /* bit 10 */

/* configuration and control register definitions */
#define GAIN_1X		0x0000 /* bits 9 and 8 */
#define GAIN_10X	0x0100 /* bits 9 and 8 */
#define GAIN_AUTO	0x0200 /* bits 9 and 8 */
#define AUTO_SCAN	0x0000 /* bits 7 and 6 */
#define SINGLE_SCAN	0x0040 /* bits 7 and 6 */

