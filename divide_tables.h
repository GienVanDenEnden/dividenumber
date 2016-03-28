/*
    divide_tables.h
    
    Copyright (C) 2016 Gien van den Enden - gien.van.den.enden@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 
*/  

// result of multiple 2 numbers & add 2 numbers (single digit)
struct strNumber {
   
   int_fast8_t   iNum1      ; 
   int_fast8_t   iNum2      ;
   
   char          cNum1      ; // character format of iNum1
   char          cNum2      ; // character format of iNum2
   
   // iNum1 * iNum2 
   int_fast8_t   iRstMul    ; // Result of iNum1 * iNum2
   char          cRstMul[3] ; // character format of iRstMul, always 2 bytes with 0 terminator
   
   int_fast8_t   iRstMulB1  ; // lower result if iNum1 * iNum2, exampel  6 * 7 = 42, iRstMulB1 = 2
   int_fast8_t   iRstMulB2  ; // higer result if iNum1 * iNum2, exampel  6 * 7 = 42, iRstMulB2 = 4
   
   char          cRstMulB1  ; // character format of iRstMulB1
   char          cRstMulB2  ; // character format of iRstMulB2
   
   // iNum1 + iNum2 
   int_fast8_t   iRstAdd    ; // iNum1 + iNum2
   char          cRstAdd[3] ; // character format of iRstAdd, always 2 bytes with 0 terminator
   
   int_fast8_t   iRstAddB1  ;
   int_fast8_t   iRstAddB2  ;
   
   char          cRstAddB1  ;
   char          cRstAddB2  ;
   
};


// result from n1 * n2 + n3 * n4 + n5
struct strMultiAddFour 
{
   int_fast8_t n1 ;  
   int_fast8_t n2 ;
   
   int_fast8_t n3 ;
   int_fast8_t n4 ;

   int_fast8_t n5 ;
   
   int_fast8_t iRstLast    ; // last digit
   int_fast8_t iNrOverflow ; // number of iOverflowStart
   int_fast8_t iOverflow[3]; // overflows
   
   // Example
   // n1 = 9, n2 = 7, n3 = 8, n4 = 9, n5 = 0
   // 9 * 7 + 8 * 9 + 0 = 63 + 72 = 135
   // iRstLast     = 5
   // iNrOverflow  = 2
   // iOverflow[0] = 6
   // iOverflow[1] = 7
};

struct strLastMulAddFour 
{
   int                    iCount         ; // number of digits
   struct strMultiAddFour strLast[ 100 ] ; // max is 100
};


extern struct strNumber         glb_arrNumber[10][10]                ; // dimension 10 x 10 for all single digits
extern struct strMultiAddFour   glb_arrMulAdd[10][10][10][10]        ; // n1, n2, n3, n4 = n1 * n2 + n3 * n4 = result
extern struct strLastMulAddFour glb_arrNoLastMulAdd[10][10][10][10]  ; // number last digits waar 1=Digit, 2=overflow(n5) 3=N1, 4=N3


void numberArrayFill(  void );  // Fill the arrays
void numberArrayPrint( void );  // Print the arrrays too default output

