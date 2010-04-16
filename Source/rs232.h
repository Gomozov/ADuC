#ifndef _RS232_H
#define _RS232_H

#include "new_types.h"

struct RS_Port_str
{
 BYTE RS_tick;	            // from 0 to 3
 BYTE TX_Ready;
 BYTE TX_Byte;				//Buffer
 BYTE TX_Byte_shift;
 BYTE TX_Bit_cnt;

 BYTE RX_Ready;
 BYTE RX_Byte;
 BYTE RX_Byte_shift;
 BYTE RX_Bit_cnt;
 BYTE RX_latch_tick;
};

extern struct RS_Port_str RS_Port;

void InitRS232(void);

#endif