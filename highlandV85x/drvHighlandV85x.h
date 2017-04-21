/* drvHighlandV85x.h */

/* For Highland Technology V850/V851 or Berkeley Nucleonics B950/B951
 * Digital Delay Generator 
 */


struct ddg85x_registers{
	uint16_t vximfr; /* 0x00 VXI manufacturer ID, always 0xfeee */
	uint16_t vxitype; /* 0x02 module type, always 0x5943 */
	uint16_t vxists; /* 0x04 VXI status register */
	uint16_t unused06; 
	uint16_t voutlo; /* 0x08 low voltage level */
	uint16_t vouthi; /* 0x0A high voltage level */
	uint16_t actions; /* 0x0C module action command bits */
	uint16_t trglvl; /* 0x0E external trigger level set */
	uint16_t control; /* 0x10 module control bits */
	uint16_t intrate; /* 0x12 internal trigger reprate divisor */
	uint16_t unused14;
	uint16_t unused16;
	uint16_t unused18;
	uint16_t unused1A;
	uint16_t unused1C;
	uint16_t unused1E;
	uint16_t dly1hi; /* 0x20 delay 1 MS 16-bits */
	uint16_t dly1lo; /* 0x22 delay 1 LS 16-bits */
	uint16_t dly2hi; /* 0x24 delay 2 MS 16-bits */
	uint16_t dly2lo; /* 0x26 delay 2 LS 16-bits */
	uint16_t dly3hi; /* 0x28 delay 3 MS 16-bits */
	uint16_t dly3lo; /* 0x2A delay 3 LS 16-bits */
	uint16_t dly4hi; /* 0x2C delay 4 MS 16-bits */
	uint16_t dly4lo; /* 0x2E delay 4 LS 16-bits */
	uint16_t dly5hi; /* 0x30 delay 5 MS 16-bits */
	uint16_t dly5lo; /* 0x32 delay 5 LS 16-bits */
	uint16_t dly6hi; /* 0x34 delay 6 MS 16-bits */
	uint16_t dly6lo; /* 0x36 delay 6 LS 16-bits */
	uint16_t wave12; /* 0x38 waveform programming, channels 1/2 */
	uint16_t wave34; /* 0x3A waveform programming, channels 3/4 */
	uint16_t wave56; /* 0x3C waveform programming, channels 5/6 */
	uint16_t unused3E;
};
typedef struct ddg85x_registers ddg85x;


#define MFR_ID	0xfeee /* manufacturer ID */
#define TYPE_ID	0x5943 /* module type */
#define VLOW_FACTOR	-26214.0 /* multiplier for VOUT LO REGISTER */
#define VHIGH_FACTOR	13107.0 /* multiplier for VOUT HI REGISTER */
#define TRGLVL_FACTOR	13107.0 /* multiplier for ext trigger level */
#define BIT_RES		39.0625 /* bit resolution in picoseconds (32-bit) */
#define NSECFACTOR	(1000.0 / BIT_RES) /* scaling factor for nanoseconds */
#define WAVESHIFT	0x4 /* two nibbles of wave register for 2 chans */
#define INSERTDELAY	25.0 /* module insertion delay, in nsec  */

/* Action register */
#define FEOD	0x0001 /* forces end of delay */
#define CAL	0x0002 /* initiate calibration */
#define XFR	0x0080 /* queue new timing values */
#define FIRE	0x8000 /* trigger DDG */

/* Control register */
#define NEG_SLOPE	0x0100 /* negative slope */
#define TRIG_BY_FIRE	0x0200 /* trigger by fire action bit */ 
#define INT_TRIG	0x0400 /* internal trigger */
#define PRSCL100	0x0800 /* prescale the int rate generator by 100 */
#define LDNOW	0x1000 /* writes to INTRATE will load immediately */
#define CCAL	0x2000 /* force module into CAL mode, maintenance */
#define CLOCK_IN	0x4000 /* front panel clock is input */
#define DISARM	0x8000 /* DDG will not trigger */

/* Output Waveform Logic */
#define MODE0 	0x00	/* delay, positive */
#define MODE1	0x01	/* delay, negative */
#define MODE2	0x02	/* width, positive */
#define MODE3	0x03	/* width, negative */
#define MODE4	0x04	/* T0 delay, positive */
#define MODE5	0x05	/* T0 delay, negative */
#define MODE6	0x06	/* OR-ALL */
#define MODE7	0x07	/* OR-widths */
#define MODE8	0x08	/* one shot mode */
