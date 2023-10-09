/***********************************************************************************

Inspried by PHPPLD.
2021 Rue Mohr.
This generic ROM-generator framework was customized for:

scanless keyboard encoder LUT, serial output via self-clocked FSM

This is set up for a 4x4 grid

INPUTS
 A0 : oscillator feedback
 
 A1 A2 A3 A4 : state counter feedback
 
 A5 A6 A7 A8 A9 A10 A11 A12: keyboard rows and columns

OUTPUTS

D0 : inverted oscillator output
D1 : non-inverted oscillator output for feedback latch

D2 D3 D4 D5: state counter output

D6 : serial data out

D7 : clock line for keyboard holding latch
 
FEEDBACK
 D2 D3 D4 D5 -> A1 A2 A3 A4 



Address bits      8 bit rom size

       -- no parallel roms available --
     8                  2 k
     9                  4 k
     10                 8 k
     
       -- eeproms available from here --
     11                 16 k  (28C16)
     12                 32 k  (28C32)
     
       -- eprom practical sizes from here --
     13                 64 k  (2764)
     14                 128 k (27128)
     15                 256 k 
     16                 512 k
     17                 1 M  (27010)
     18                 2 M
     19                 4 M
     20                 8 M

       -- flash from here up --



**************************************************************************************/


#include <stdio.h>
#include <stdint.h>
#include "ROMLib.h"
#include "7seg.h"


// the number of address lines you need !!!???!!!
#define InputBits 13

// the output data size, 8 or 16
#define OutputBits 8

// default output value
#define DFOutput  0x00



// Tuck this one away!. Bit reverser!  Please dont use this in real fft code,
//   YOU KNOW how many bits your working with, and you can use a 
//   specific case generator for it.
uint8_t uniReverse(uint8_t i, uint8_t bits) {

  uint8_t r, m, b;
  r = 0;             // result
  m = 1 << (bits-1); // mask will travel right
  b = 1;             // bit will travel left
  
  while(m) {
    if (i&b) r |=m;
    b <<= 1;
    m >>= 1;  
  }
  
  return r;  

}


// count set bits, unrolled edition.
// if using assember shift into the carry and use addc, 0
uint8_t bitCount(uint16_t n) {  
   uint8_t rv;
   rv = 0;
   if (n & 0x0001) rv++;
   if (n & 0x0002) rv++;
   if (n & 0x0004) rv++;
   if (n & 0x0008) rv++;
   if (n & 0x0010) rv++;
   if (n & 0x0020) rv++;
   if (n & 0x0040) rv++;
   if (n & 0x0080) rv++;   
   if (n & 0x0100) rv++;
   if (n & 0x0200) rv++;
   if (n & 0x0400) rv++;
   if (n & 0x0800) rv++;
   if (n & 0x1000) rv++;
   if (n & 0x2000) rv++;
   if (n & 0x4000) rv++;
   if (n & 0x8000) rv++; 
   
   return rv;
}

// convert a character and bit position to a serial level, 10 bits per character. 
uint8_t SerialChar(char c, uint16_t bit) {
  
  switch (bit) {
    case 0: return 0; // start bit
    case 1: return (c & 0x01)!=0;
    case 2: return (c & 0x02)!=0;
    case 3: return (c & 0x04)!=0;
    case 4: return (c & 0x08)!=0;
    case 5: return (c & 0x10)!=0;
    case 6: return (c & 0x20)!=0;
    case 7: return (c & 0x40)!=0;
    case 8: return (c & 0x80)!=0;
    case 9: return 1; // stop bit          
  }

}



int main(void) {

  uint16_t counterI;
  uint16_t charI;
  uint16_t msgNumI;

  
  uint8_t  TxD0O, TxD1O; 
  
  
  uint8_t message[] = {8, 6, 7, 5, 3, 0, 9, 16};
  uint8_t message2[] = {'J', 'E', 'N', ' ', 'N', 'Y', ' ', ' '};
  
  uint32_t AP;

  uint32_t out;  // leave it alone!

  setup();       // open output file.
  
  
  // loop thru each address
  for( A=0; A<(1<<InputBits); A++) { // A is a bitfield (a mash-up of the vars we want)
     
     AP = (A+4)&0x3F; // offset everything 4 bits to invert A2
       
     // reset vars  
     TxD0O   = 1;
     TxD1O   = 1;
     
     // build input values
    spliceValueFromField( &counterI,           AP,  3,   0, 1, 2);  
    spliceValueFromField( &charI,              AP,  3,   3, 4, 5);  
    spliceValueFromField( &msgNumI,            A,   1,   6);
   
    printf("\n>bit: %d, char %d, msgNum %d", counterI, charI, msgNumI);
   
    if (msgNumI == 0) {    
      TxD0O = ((~CG[message[charI]] & (0x80>>counterI))==0)?0:1;    
      printf(" %d", message[charI]);
    } else {
      TxD0O = ((~CG2[message2[charI]-'0'] & (0x80>>counterI))==0)?0:1;   
      printf(" %c", message2[charI]);
    }
   
     // reconstitute the output
     // assign default values for outputs     
     out = DFOutput;
          
     spliceFieldFromValue( &out, TxD0O,         1,  0); 
     spliceFieldFromValue( &out, TxD1O,         1,  1); 
         
     // submit entry to file
     write(fd, &out, OutputBits>>3);  // >>3 converts to bytes, leave it!
  }
  
  cleanup(); // close file
  
  return 0;
}









