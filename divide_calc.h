/*
    divide_calc.h
    
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

// the walk state for strDivideState for 1 digit
struct strWalkState {
   int_fast8_t  iDigit          ; // current digit too calculate

   int_fast8_t  iOverflowValue  ; // array of overflow value too start with
   int_fast8_t  iOverflowStart  ; // array for each digit position in overflow array where too start for iNumbers2
   int_fast8_t  iOverflowPos    ; // array for each digit position in overflow array
   int_fast8_t *OverflowDigits  ; // matrix, for each digit an array of overflow values
   
   int_fast8_t iLastWalk        ; // walk in last
//   int_fast8_t iDigOver         ; // last digit with overflow
   
   int_fast8_t  iWalk1          ; // 9..0
   int_fast8_t  iWalk2          ; // 9..0
   int_fast8_t  iStart1         ; // start Numbers 1
   int_fast8_t  iStart2         ; // start Numbers 2
   int_fast8_t  bCalc1          ; // calc start Numbers 1, 1=true, 0=false
   int_fast8_t  bCalc2          ; // calc start Numbers 1, 1=true, 0=false
   
   struct strLastMulAddFour *strLast;
} ;

// state for divide number
struct strDivideState {
   char                *cNumber    ;  // number too divide
   
   int_fast8_t          iLenNumber ;  // number of digits in cNumber
   int_fast8_t          iMaxCurNr  ;  // max current number = iLenNumber - 1 
   int_fast8_t          iMinCurNr  ;  // min current number (for threads)
   int_fast8_t          iMaxCurCPU ;  // max number too start a new thread
   
   int_fast8_t          iMaxLenNum1;  // max lengte number1
   int_fast8_t          iMaxLenNum2;  // max length number2
   int_fast8_t          iLenHalve  ;  // halve the length of the number, needed too dynamicaly determine max iNumber2

   int_fast8_t         *iNumbers1  ;  // the numbers 1 found
   int_fast8_t         *iNumbers2  ;  // the numbers 2 found

   int_fast8_t          iCurNumber ;  // current number

   struct strWalkState *arrWalk    ; // iCurNumber walk state
   
   int_fast8_t          iFound     ; // number found = 1
};


extern int_fast8_t glb_iNumberFindAll ; // if 1 then find all the numbers, otherwise stop by the first found
extern int_fast8_t glb_iDigitsCalcCPU ; // number of digits 1 (one) CPU can process in (estimate) 1 second, use to start threads


void *numberThread( void *data    );  // start divide within a thread
void  numberDivide( char *cNumber );  // Start divide








