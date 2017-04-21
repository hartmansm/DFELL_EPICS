/* drvHy2670.h */

#ifndef drvHy2670H
#define drvHy2670H

struct hytec_2670 {
	unsigned long unused1[4];
	unsigned long dac_b;
	unsigned long unused2[3];
	unsigned long dac_a;
	unsigned long unused3[7];
	unsigned long range;
	unsigned long unused4[47];
};

/* DAC range codes */
#define ZERO_TO_TEN	0x00
#define NEGTEN_TO_TEN	0x01
#define ZERO_TO_FIVE	0x02
#define NEGFIVE_TO_FIVE	0x03

long hy2670_write(unsigned short card, unsigned short chan, unsigned long *pval);


#endif /*drvHy2670H */
