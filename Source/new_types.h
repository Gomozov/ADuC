#ifndef New_Types
#define New_Types

#include "ADuC832.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned char BOOL;

sbit TEMPER = P2^5;// Dallas
sbit RS_TX = P3^1;  //P3^6; // выход RS
sbit RS_RX = P3^0;  //P3^7; // вход RS
sbit DIOD = P3^4;

#endif