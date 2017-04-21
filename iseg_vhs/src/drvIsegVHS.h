/* drvIsegVHS.h */

/* header file for ISEG VME High Voltage Power Supply */

/* NOTE: board interface only supports 16 bit data transfers, but several 
   registers are 32-bit ints and many are 32-bit floating point. The 
   structures below only use 16-bit sizes. 32-bit ints use an array
   of two 16-bit ints. 32-bit floats also use an array of two 16-bit  
   ints and their names are preceeded by f_ to identify which ones are 
   floating point. The device driver software is responsible for 
   re-assembling the 32-bits types after the 16-bit data transfer.
*/

struct module_data {
	uint16_t	status;
	uint16_t	control;
	uint16_t	event_status;
	uint16_t	event_mask;
	uint16_t	event_chan_stat;
	uint16_t	event_chan_mask;
	uint16_t	event_group_stat[2];
	uint16_t	event_group_mask[2];
	uint16_t	f_voltage_ramp_speed[2];
	uint16_t	f_current_ramp_speed[2];
	uint16_t	f_voltage_max[2];
	uint16_t	f_current_max[2];
	uint16_t	f_supply_p5[2];
	uint16_t	f_supply_p12[2];
	uint16_t	f_supply_n12[2];
	uint16_t	f_temperature[2];
	uint16_t	serial_number[2];
	uint16_t	firmware_release[2];
	uint16_t	placed_chans;
	uint16_t	device_class;
	uint16_t	unused[16];
};
typedef struct module_data module_data_t;

struct	channels {
	uint16_t	status;
	uint16_t	control;
	uint16_t	event_status;
	uint16_t	event_mask;
	uint16_t	f_voltage_set[2];
	uint16_t	f_current_set[2];
	uint16_t	f_voltage_read[2];
	uint16_t	f_current_read[2];
	uint16_t	f_voltage_bounds[2];
	uint16_t	f_current_bounds[2];
	uint16_t	f_voltage_nominal[2];
	uint16_t	f_current_nominal[2];
	uint16_t unsed[4];
};
typedef struct channels channels_t;

struct	fixed_groups {
	uint16_t	f_voltage_allchans_set[2];
	uint16_t	f_current_allchans_set[2];
	uint16_t	f_voltage_allchans_bounds[2];
	uint16_t	f_current_allchans_bounds[2];
	uint16_t	emergency_allchans[2];
	uint16_t	on_allchans[2];
	uint16_t	unused[4];
};
typedef struct fixed_groups fixed_groups_t;

struct group_addr {
	uint16_t	lists;
	uint16_t	types;
}; 
typedef struct group_addr group_addr_t;

struct	variable_groups {
	group_addr_t group_addr[32];
	uint16_t	unused[48];
};
typedef struct variable_groups variable_groups_t;

struct	special_registers {
	uint16_t	new_base_addr;
	uint16_t	new_base_addr_xor;
	uint16_t	old_base_addr;
	uint16_t	new_base_addr_accepted;
	uint16_t	special_control_status;
	uint16_t	special_control_command;
	uint16_t	unused[42];
};
typedef struct special_registers special_registers_t;


/* This structure describes the whole board. It is 1024 bytes in size
   to match the memory space used by the board.
*/
struct iseg_vhs_registers {
	module_data_t	module_data;
	channels_t	channels[12];
	fixed_groups_t	fixed_groups;
	variable_groups_t	variable_groups;
	special_registers_t	special_registers;
};
typedef struct iseg_vhs_registers iseg_vhs;

/* for module control */
#define KILL_ENABLE	0x4000
#define FINE_ADJ	0x1000
#define CLEAR		0x0040
#define SPECIAL_MODE	0x0001

/* for channel control*/
#define SET_ON	0x0008	/* enable channel HV */
#define SET_OFF	0x0020	/* disable channel HV */
 

/* function declarations */
long iseg_vhs_report(int level);
long iseg_vhs_init(void);
long iseg_vhs_chan_on(unsigned int card, unsigned int chan, unsigned int *pval);
long iseg_vhs_vread(unsigned int card, unsigned int chan, float *pvolts);
long iseg_vhs_iread(unsigned int card, unsigned int chan, float *pamps);
long iseg_vhs_vwrite(unsigned int card, unsigned int chan, float *pvolts);
long iseg_vhs_iwrite(unsigned int card, unsigned int chan, float *pamps);
long iseg_vhs_vrampspeed(unsigned int card, float *pspeed);


