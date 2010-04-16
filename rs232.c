#include "ADuC832.h"
#include "rs232.h"
#include "new_types.h"

struct RS_Port_str RS_Port;

void Timer1_int() interrupt 3
{
 RS_Port.RS_tick++;
 RS_Port.RS_tick&=0x3;

 if(!RS_Port.RS_tick)      // RS_Transmit
 {
  switch(RS_Port.TX_Bit_cnt)
  {
   case 0:
    //��������� ������� ������
    if(!RS_Port.TX_Ready)	//Data in buffer
    {
    //���� ���� � ������ �� ��������
     RS_Port.TX_Byte_shift=RS_Port.TX_Byte;
     RS_Port.TX_Ready=1;		//����� �������� ��� ������ ���������� �����
     RS_Port.TX_Bit_cnt=9;

     RS_TX=0;			    	//����� ��������� ���
    }
    break;

   case 1:
    RS_Port.TX_Bit_cnt--;
    RS_TX=1;		               //����� ����-���
    break;

   default:
    //��������� ������� �� ������
    RS_TX=RS_Port.TX_Byte_shift&0x01;
    RS_Port.TX_Byte_shift>>=1;
    RS_Port.TX_Bit_cnt--;
  }
 }

 switch(RS_Port.RX_Bit_cnt)			   // RS_Receive
 {
  case 0:
   //���� ��������� ���
   if(!RS_RX)
    RS_Port.RX_Bit_cnt++;
   break;
  case 1:
   //��� ��� ��������� ������� ���������� ����
   if(!RS_RX)
   {
    RS_Port.RX_latch_tick=RS_Port.RS_tick;
    RS_Port.RX_Bit_cnt++;
   }
   else
    RS_Port.RX_Bit_cnt=0;
   break;

  case 10:
   if(RS_Port.RS_tick==RS_Port.RX_latch_tick)	      //��������� ������� ��������� ����
   {
    if(RS_RX)
    {
     RS_Port.RX_Byte=RS_Port.RX_Byte_shift;		      //���� ����-���, ���������� ���� � �����
	 RS_Port.RX_Ready=1;
    }
    RS_Port.RX_Bit_cnt=0;
   }
   break;

  default:
   if(RS_Port.RS_tick==RS_Port.RX_latch_tick)
   {
    RS_Port.RX_Byte_shift>>=1;
    if(RS_RX)
	 RS_Port.RX_Byte_shift|=0x80;
	RS_Port.RX_Bit_cnt++;
   }
 } 
}  

void InitRS232(void)
{
 RS_Port.RS_tick=0;	// from 0 to 3
 RS_Port.TX_Ready=1;
 RS_Port.TX_Bit_cnt=0;

 RS_Port.RX_Ready=0;
 RS_Port.RX_Byte=0;
 RS_Port.RX_Bit_cnt=0;
}  