/* drvJoergerVdacm.h */

/* header file for Joerger Model VDACM 8-Channel, 16-Bit Waveform Generator */

struct vdacm_register {
	unsigned short board_reset;
	volatile unsigned short csr;
	volatile unsigned short clock;
	volatile unsigned short update_enable;
	volatile unsigned short read_channel;
	volatile unsigned short write_channel;
	volatile unsigned short length_hi;
	volatile unsigned short length_lo;
	volatile unsigned short irq;
	volatile unsigned short int_status;
	volatile unsigned short int_reset;
	volatile unsigned short trigger;
	volatile unsigned short dac_reset;
	volatile unsigned short lockout;
	volatile unsigned short start_hi;
	volatile unsigned short start_lo;
	unsigned long unused[56];
}; 
typedef struct vdacm_register vdacm_register;

struct vdacm_data {
	signed short data[0x8000];
}; 
typedef struct vdacm_data vdacm_data;

#define RESET	0x01
#define TRIGGER 0x01
#define STOP	0x00

/* csr */
#define ENABLE_TRIGLOCK 0x80
#define ENABLE_RETRIG	0x40
#define DISABLE_SWSTOP	0x20
#define DISABLE_SWTRIG	0x10	
#define DISABLE_EXTTRIG	0x08
#define SYNC_INIT	0x04
#define REPEAT	0x02
#define ACTIVE	0x01
#define CSR_DEFAULT 0x00

/* clock */
#define CLOCK1	0x00 /* 100 kHz, divide 1 */
#define CLOCK2	0x01 /* 50 kHz, divide 2*/
#define CLOCK4	0x02 /* 25 kHz, divide 4 */
#define CLOCK8	0x03 /* 12.5 kHz, divide 8 */
#define CLOCK16	0x04 /* 6.25 kHz, divide 16 */
#define CLOCK32	0x05 /* 3.125 kHz, divide 32 */
#define CLOCK64	0x06 /* 1.5625 kHz, divide 64 */
#define CLOCK128	0x07 /* 0.78125, divide 128 kHz */
#define INTCLOCK	0x00 /* internal clock */
#define EXTCLOCK	0x08 /* external clock */

/* channel enable */
#define ENABLECHAN0 0x01	
#define ENABLECHAN1 0x02
#define ENABLECHAN2 0x04
#define ENABLECHAN3 0x08
#define ENABLECHAN4 0x10
#define ENABLECHAN5 0x20
#define ENABLECHAN6 0x40
#define ENABLECHAN7 0x80
#define DISABLECHANALL 0x00
#define ENABLECHANALL ( ENABLECHAN0 | ENABLECHAN1 | ENABLECHAN2 | ENABLECHAN3 | ENABLECHAN4 | ENABLECHAN5 | ENABLECHAN6 | ENABLECHAN7 )

/* IRQ */
#define ENABLE_IRQ	0x80

/* used for read or write on configuration routines */
#define VDACM_WRITE 0
#define VDACM_READ 1
