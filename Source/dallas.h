#ifndef _DALLAS_H
#define _DALLAS_H

#include "new_types.h"

#define Sys_clk 16777  // kHz;
#define Timer_period_L(period) (BYTE)((0xffff+7-(WORD)((unsigned long)((unsigned long)period*Sys_clk)/(12*1000)))&0xff) //us
#define Timer_period_H(period) (BYTE)((0xffff+7-(WORD)((unsigned long)((unsigned long)period*Sys_clk)/(12*1000)))>>8) //us

void InitDallas(void);
BYTE TemperatureMeasure(void);
void OneWire_Reset_transmit(void);
void OneWire_One_transmit(void);
void OneWire_Zero_transmit(void);
void OneWire_Read_bit(void);

enum Term_State_en
{
  Reset1,
  Read_ROM_cmd,
  Match_ROM_cmd,
  Match_ROM_cmd2,
  Write_64_Code,
  Write_64_Code2,
  Read_family,
  Reset2,
  Skip_ROM_cmd1,
  Convert_cmd,
  Delay,
  Delay1,
  Delay2,
  Delay3,
  Reset3,
  Skip_ROM_cmd2,
  Read_scratchpad,
  Read_temper
};

enum Port_State_en
{
 Bit_Wait,
 Command_Wait,
 Ready
};

struct Termo
{
 enum Term_State_en term_state;
 BYTE bit_cntr;
 WORD delay_cntr;
 BYTE command;
 BYTE temperature;
 BYTE id;
 BYTE byte_read;
 enum Port_State_en port_state;
 BYTE read_bit;
 BYTE read_port;
 BYTE Dallas_Num;
};

extern struct Termo Dallas;

#endif