
#include "dallas.h"
#include "ADuC832.h"
#include "new_types.h"
 
struct Termo Dallas;

BYTE X=0;										//������� ���������� ���� �� 0 �� 7, ��� �����������
												//���� ���������� (����������)
void Timer0_int() interrupt 1					//**************************************************
{												//��������� ���������� �������� �������
 TEMPER=1;										//����� 1-wire ���. 1 (P2^5)
 TR0=0;											//���� ������� 0
}


void InitDallas()								//��������� ���������� ������ � 1-wire
{
 Dallas.term_state=Reset1;						//��������� �������� ������� � �����
 Dallas.temperature=0;							//������� ���������� ��� ����������� (�������)
 Dallas.port_state=Ready;						//������ ������ � 1-wire
 Dallas.Dallas_Num=0;							//����� ���������� 0..2, ��������� 0
}

void OneWire_One_transmit()						//��������� �������� ��������� 1-wire***************
{
 ET1=0;											//������ ���������� ��� ������� 1
 Dallas.port_state=Bit_Wait;					//������ 1-wire - ����� ����� ����� ������
 TEMPER=0;
 TEMPER=0;										//�������� ���=1
 TEMPER=0;										//0,178us (2,136us), ���� ���� ���� ��� >1us
 TEMPER=1;
 ET1=1;											//��������� ���������� ��� ������� 1 
}

void OneWire_Zero_transmit()					//��������� �������� ���� �� 1-wire*****************
{
 TH0=Timer_period_H(90);						//
 TL0=Timer_period_L(90);						//���� 90 ������ 5,36us (64,32us) ���, ���� �� 60us �� 120us
 Dallas.port_state=Bit_Wait;					//������ 1-wire - ����� ����� ����� ������
 TEMPER=0;										//�������� �� 1-wire=���.0
 TR0=1;                                         //��������� ������ 0
}

void OneWire_Reset_transmit()				    //�������� ������� reset � 1-wire
{
 TH0=Timer_period_H(600);
 TL0=Timer_period_L(600);						//���� 600 ������ 35,7us (429us), ���� �� 480us �� 960us
 TEMPER=0;										//�������� �� 1-wire=���.0
 TR0=1;                                         // Timer 0 start
}

void OneWire_Read_bit()							//������ ������ ����
{
 ET1=0;	                                        //��������� ���������� ��� ������� 1
 TEMPER=0;
 TEMPER=0;
 TEMPER=0;
 TEMPER=1;
 Dallas.read_port=P2;	                        //���������� ������� ����, ��� �� ��� �������, ��� ���������� ���
 ET1=1;	                                        //��������� ���������� ��� ������� 1
 Dallas.read_bit=(Dallas.read_port&0x20)?1:0;	//��������� ���, ��������������� TEMPER
}												//0�20=00100000 �.�. ����� P2^5

void OneWare_Command_Wait()						//�������� ������� (429us)
{
 TR0=0;											 //������������� ������ 0
 TH0=Timer_period_H(600);						 //������� ��� ������� ����� 600
 TL0=Timer_period_L(600);						 //������� ��� ������� ����� 600
 TR0=1;                                          //��������� ������ 0 
}

void OneWare_Bit_Wait()							 //�������� ����� ����� ������ (35,7us)
{
 TR0=0;											 //�� ������ ������ ������������� ������ 0
 TH0=Timer_period_H(50);						 //������� ��� ������� ����� 50
 TL0=Timer_period_L(50);						 //������� ��� ������� ����� 50
 TR0=1;                                          //��������� ������ 0
}


BYTE TemperatureMeasure(void)					 //��������� ������ ����������� �� ������
{
 if(TR0)										 //���� ������ 0 �������
  return Dallas.temperature;					 //	  ������ ���� reset, ���� ��� ��� �� ��������, ����

 if(Dallas.port_state==Bit_Wait)				 //���� ��� ������� ���
 {
  OneWare_Bit_Wait();							 //���� ����� ����� ������
  Dallas.port_state=Ready;						 //������ 1-wire - �����
  return Dallas.temperature;					 //
 }

 if(Dallas.port_state==Command_Wait)			 //
 {
  OneWare_Command_Wait();						 //
  Dallas.port_state=Ready;						 //
  return Dallas.temperature;					 //
 }
 
 switch(Dallas.term_state)
 {
  case Reset1:

   OneWire_Reset_transmit();
   if (Dallas.Dallas_Num>2)
    {
	 Dallas.Dallas_Num=0;
	}
   Dallas.bit_cntr=0;
   Dallas.command=0x55;				        //Match Rom
   Dallas.term_state=Match_ROM_cmd;
   Dallas.port_state=Command_Wait;
   break;

  case Match_ROM_cmd:					   //10 8B 6E 4C 01 08 00 75

   if((Dallas.command>>Dallas.bit_cntr)&0x01)  // ...........................
    OneWire_One_transmit();					   // ��������� ������� ����������
   else										   // ������ ���� ��������� 64-� ������ ���
    OneWire_Zero_transmit();				   // ...........................

   if(Dallas.bit_cntr>=7)
   {
    Dallas.bit_cntr=0;
    Dallas.port_state=Command_Wait;
    Dallas.term_state=Write_64_Code;
	if (Dallas.Dallas_Num==0)
	 Dallas.command=0x10;                         //*****      //10 8B 6E 4C 01 08 00 75
    if (Dallas.Dallas_Num) 									   //28 16 B5 A3 01 00 00 E0
	 Dallas.command=0x28;									   //28 AA BB A3 01 00 00 38
	break;													  
   }
   Dallas.bit_cntr++;
   break;

  case Write_64_Code:

	 switch (X)
	 {
	 case 0:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x8B;
		if (Dallas.Dallas_Num==1)                 
		 Dallas.command=0x16;
		if (Dallas.Dallas_Num==2)
		 Dallas.command=0xAA;                   //*****				//10 8B 6E 4C 01 08 00 75
		X++;														//28 16 B5 A3 01 00 00 E0
		break;														//28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 1:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x6E;
		if (Dallas.Dallas_Num==1)
		 Dallas.command=0xB5;
		if (Dallas.Dallas_Num==2)
		 Dallas.command=0xBB;                   //*****	               //10 8B 6E 4C 01 08 00 75
		X++;														   //28 16 B5 A3 01 00 00 E0
		break;														   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 2:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x4C;
		if (Dallas.Dallas_Num)
		 Dallas.command=0xA3;                  //*****					//10 8B 6E 4C 01 08 00 75
		X++;															//28 16 B5 A3 01 00 00 E0
		break;															//28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 3:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		Dallas.command=0x01;				 //*****					 //10 8B 6E 4C 01 08 00 75
		X++;															 //28 16 B5 A3 01 00 00 E0
		break;															 //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 4:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x08;
		if (Dallas.Dallas_Num)
		 Dallas.command=0x00;					 //*****				 //10 8B 6E 4C 01 08 00 75
		X++;															 //28 16 B5 A3 01 00 00 E0
		break;															 //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 5:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		Dallas.command=0x00;					  //*****						//10 8B 6E 4C 01 08 00 75
		X++;																	//28 16 B5 A3 01 00 00 E0
		break;																	//28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 6:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x75;
		if (Dallas.Dallas_Num==1)
		 Dallas.command=0xE0;
		if (Dallas.Dallas_Num==2)
		 Dallas.command=0x38;					   //*****					   //10 8B 6E 4C 01 08 00 75
		X++;																   //28 16 B5 A3 01 00 00 E0
		break;																   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 7:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		X++;
		break;
	   }
	 Dallas.bit_cntr++;
	 break;
	 }
//    }
  if (X==8)
  {
  Dallas.bit_cntr=0;
 // Dallas.command=0x44;				        //Convert T
  Dallas.term_state=Delay1;
 // Dallas.delay_cntr=300;
 // Dallas.port_state=Command_Wait;
  X=0;
  }
  break;

  case Delay1:

   OneWare_Command_Wait();
//   if(!Dallas.delay_cntr)
//   {
    Dallas.term_state=Convert_cmd;
	Dallas.command=0x44;
//    break;
//   }
//   Dallas.delay_cntr--;
   break;
  
  case Convert_cmd:

   if((Dallas.command>>Dallas.bit_cntr)&0x01)
    OneWire_One_transmit();
   else
    OneWire_Zero_transmit();
   if(Dallas.bit_cntr>=7)
   {
    Dallas.bit_cntr=0;
    Dallas.delay_cntr=1300;		//1500
    Dallas.term_state=Delay2;
    break;
   }
   Dallas.bit_cntr++;
   break;

  case Delay2:

   OneWare_Command_Wait();	               //429us   429*1000us=429ms  (200ms typ.)
   if(!Dallas.delay_cntr)
   {
    Dallas.term_state=Reset2;
    break;
   }
   Dallas.delay_cntr--;
   break;

  case Reset2:

   OneWire_Reset_transmit();
   Dallas.bit_cntr=0;
   Dallas.command=0x55;				        //Match Rom
   Dallas.term_state=Match_ROM_cmd2;
   Dallas.port_state=Command_Wait;
   break;

 case Match_ROM_cmd2: 					   //10 8B 6E 4C 01 08 00 75	  Dallas, ����� DS1820
										   //28 16 B5 A3 01 00 00 E0      Dallas, ������. DS18B20
                    					   //28 AA BB A3 01 00 00 38      Dallas, ������. DS18B20
   if((Dallas.command>>Dallas.bit_cntr)&0x01)  // ...........................
    OneWire_One_transmit();					   // ��������� ������� ����������
   else										   // ������ ���� ��������� 64-� ������ ���
    OneWire_Zero_transmit();				   // ...........................

   if(Dallas.bit_cntr>=7)
   {
    Dallas.bit_cntr=0;
    Dallas.port_state=Command_Wait;
    Dallas.term_state=Write_64_Code2;
	if (Dallas.Dallas_Num==0)
	 Dallas.command=0x10;
	if (Dallas.Dallas_Num)
	 Dallas.command=0x28;		          //*****					   //10 8B 6E 4C 01 08 00 75
    break;						   									   //28 16 B5 A3 01 00 00 E0
   }																   //28 AA BB A3 01 00 00 38
   Dallas.bit_cntr++;
   break;

  case Write_64_Code2:

 //  while (X<8)
 //   {
	 switch (X)
	 {
	 case 0:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x8B;
		if (Dallas.Dallas_Num==1)
		 Dallas.command=0x16;
		if (Dallas.Dallas_Num==2)
		 Dallas.command=0xAA;				 //*****					   //10 8B 6E 4C 01 08 00 75
		X++;															   //28 16 B5 A3 01 00 00 E0
		break;															   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 1:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x6E;	
		if (Dallas.Dallas_Num==1)
		 Dallas.command=0xB5;
		if (Dallas.Dallas_Num==2)
		 Dallas.command=0xBB;   		//*****					   //10 8B 6E 4C 01 08 00 75
		X++;													   //28 16 B5 A3 01 00 00 E0
		break;													   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 2:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x4C;
		if (Dallas.Dallas_Num)
		 Dallas.command=0xA3;			//*****					   //10 8B 6E 4C 01 08 00 75
		X++;													   //28 16 B5 A3 01 00 00 E0
		break;													   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 3:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		Dallas.command=0x01;			  //*****					   //10 8B 6E 4C 01 08 00 75
		X++;														   //28 16 B5 A3 01 00 00 E0
		break;														   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 4:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x08;
		if (Dallas.Dallas_Num)
		 Dallas.command=0x00;			 //*****					   //10 8B 6E 4C 01 08 00 75
		X++;														   //28 16 B5 A3 01 00 00 E0
		break;														   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 5:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		Dallas.command=0x00;			 //*****					   //10 8B 6E 4C 01 08 00 75
		X++;														   //28 16 B5 A3 01 00 00 E0
		break;														   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 6:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		if (Dallas.Dallas_Num==0)
		 Dallas.command=0x75;
		if (Dallas.Dallas_Num==1)
		 Dallas.command=0xE0;
		if (Dallas.Dallas_Num==2)
		 Dallas.command=0x38;			//*****					   //10 8B 6E 4C 01 08 00 75
		X++;													   //28 16 B5 A3 01 00 00 E0
		break;													   //28 AA BB A3 01 00 00 38
	   }
	 Dallas.bit_cntr++;
	 break;

	 case 7:
	  
      if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
		X++;
		break;
	   }
	 Dallas.bit_cntr++;
	 break;
	 }
 //   }
  if (X==8)
  {
  Dallas.bit_cntr=0;
  //Dallas.command=0x44;				        //Convert T
  Dallas.term_state=Delay3;
//  Dallas.delay_cntr=300;
 // Dallas.port_state=Command_Wait;
  X=0;
 // Dallas.Counter=0x1B;
 }
  break;

  case Delay3:

   OneWare_Command_Wait();
//   if(!Dallas.delay_cntr)
 //  {
    Dallas.term_state=Read_scratchpad;
	Dallas.command=0xbe;
    break;
 //  }
 //  Dallas.delay_cntr--;
   break;

  case Read_scratchpad:
   
   if((Dallas.command>>Dallas.bit_cntr)&0x01)  
       OneWire_One_transmit();					   
      else										   
       OneWire_Zero_transmit();				  
      if(Dallas.bit_cntr>=7)
       {
        Dallas.bit_cntr=0;
        Dallas.port_state=Command_Wait;
		Dallas.byte_read=0;
		Dallas.term_state=Read_temper;
		break;
	   }
	 Dallas.bit_cntr++;
	 break;

  case Read_temper:
   
   if (Dallas.Dallas_Num==0)
    {
     OneWire_Read_bit();
     Dallas.byte_read>>=1;
     if(Dallas.read_bit)
      Dallas.byte_read|=0x80;
     if(Dallas.bit_cntr>=7)
      {
       Dallas.temperature=Dallas.byte_read>>1;
       Dallas.term_state=Reset1;
	   Dallas.Dallas_Num++;
    break;
      }
     Dallas.bit_cntr++;
    break;
    }
	else
	{
	 OneWire_Read_bit();
     Dallas.byte_read>>=1;
     if(Dallas.read_bit)
      Dallas.byte_read|=0x80;
     if(Dallas.bit_cntr>=11)
      {
       Dallas.temperature=Dallas.byte_read;//>>1;
       Dallas.term_state=Reset1;
	   Dallas.Dallas_Num++;
    break;
      }
     Dallas.bit_cntr++;
    break;
	}
 }
 return Dallas.temperature;
}
