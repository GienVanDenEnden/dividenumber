/*
    divide_alloc.c
    
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "divide_calc.h"
#include "divide_alloc.h"




//........................................
// struct strDivideState functions:
//
// - strDivideStateAlloc  : Alloc structure and initialize is
// - strDivideStateFree   : Free structure and all its subcomponents
// - strDivideStateClone  : Clone structure
//........................................
void strDivideStateFree( struct strDivideState *strState ) 
{
   int_fast8_t iCnt ;
   
   if ( strState == NULL ) {
      return ; // nothing too do
   }
   
   // free data
   free( strState->iNumbers1 );
   free( strState->iNumbers2 );
   
   if ( strState->arrWalk != NULL ) {
      for( iCnt = 0; iCnt < strState->iLenNumber; iCnt++ ) {
         free( strState->arrWalk[ iCnt ].OverflowDigits  );
      }
   }
   free( strState->arrWalk        );
   free( strState->cNumber        );
   free( strState );
}


struct strDivideState * strDivideStateAlloc( char *cNumber ) 
{
   int_fast8_t            iAllocOk = 1 ;
   int_fast8_t            iCnt         ;
   struct strDivideState *strState     ;
   
   // alloc data
   strState              = malloc( sizeof( struct strDivideState ) );
   if ( strState == NULL ) {
      return NULL ;
   }
 
   // alloc sub items
   strState->cNumber     = strdup( cNumber ) ;  // number too divide
   if ( strState->cNumber == NULL ) {
      free( strState );
      return NULL ;
   }
   
   strState->iLenNumber  = strlen( strState->cNumber );  // number of digits in cNumber
   strState->iMaxCurNr   = strState->iLenNumber - 1 ;
   
   strState->iMaxCurCPU  = ( strState->iMaxCurNr - glb_iDigitsCalcCPU ) / 2 ;
   
   strState->iMaxLenNum1 = strState->iLenNumber / 2 + strState->iLenNumber % 2;  // max lengte number1
   strState->iMaxLenNum2 = strState->iLenNumber  ;  // max length number2
   strState->iLenHalve   = strState->iMaxLenNum1 ; // initieel same length as iMaxLenNum1, but iMaxLenNum1 can change....
   
   strState->iNumbers1   = malloc( strState->iLenNumber * sizeof( int_fast8_t ) );  // the numbers 1 found
   strState->iNumbers2   = malloc( strState->iLenNumber * sizeof( int_fast8_t ) );  // the numbers 2 found
   
   strState->iCurNumber  = 0 ;  // current number in iNumbers1 & iNumbers2
   strState->iFound      = 0 ;  // 1 = found, 0 = not found
   strState->iMinCurNr   = 0 ;  // min current number, for main loop

   strState->arrWalk     = malloc( strState->iLenNumber * sizeof( struct strWalkState ) ); // iCurNumber walk state
   
   // init walk state
   if ( strState->arrWalk != NULL ) {
      for( iCnt = 0; iCnt < strState->iLenNumber; iCnt++ ) {
         
         strState->arrWalk[ iCnt ].iLastWalk = 0 ;
//         strState->arrWalk[ iCnt ].iDigOver  = 0 ;
         
         strState->arrWalk[ iCnt ].iWalk1  = 9 ;
         strState->arrWalk[ iCnt ].iWalk2  = 9 ;
         strState->arrWalk[ iCnt ].iStart1 = 9 ; // start Numbers 1
         strState->arrWalk[ iCnt ].iStart2 = 9 ; // start Numbers 2
         strState->arrWalk[ iCnt ].bCalc1  = 1 ; // calc start Numbers 1, 1=true, 0=false
         strState->arrWalk[ iCnt ].bCalc2  = 1 ; // calc start Numbers 1, 1=true, 0=false

         strState->arrWalk[ iCnt ].iOverflowValue = 0 ; // array of overflow value too start with
         strState->arrWalk[ iCnt ].iOverflowStart = 0 ; // array for each digit position in overflow array where too start for iNumbers2
         strState->arrWalk[ iCnt ].iOverflowPos   = 0 ; // array for each digit position in overflow array
         strState->arrWalk[ iCnt ].OverflowDigits = malloc( 3 * strState->iLenNumber * sizeof( int_fast8_t ) ); // matrix, for each digit an array of overflow values
         
         strState->arrWalk[ iCnt ].strLast        = NULL ;
         
         if ( iCnt >= strState->iMaxLenNum1 ) {
            strState->arrWalk[ iCnt ].iStart1 = 0 ;
         }
         strState->arrWalk[ iCnt ].iDigit  = strState->cNumber[ strState->iLenNumber - 1 - iCnt ]  - '0' ;
      }
   }
   
   // check of all alloc is succes
   if ( strState->iNumbers1      == NULL ||
        strState->iNumbers2      == NULL ||
        strState->arrWalk        == NULL 
      ) {
      iAllocOk = 0 ;
   } else {
      for( iCnt = 0; iCnt < strState->iLenNumber; iCnt++ ) {
         if ( strState->arrWalk[ iCnt ].OverflowDigits  == NULL ) {
            iAllocOk = 0 ;
         }
      }
   }
        
   if ( iAllocOk == 0 ) {
      // not all memory allocated
      strDivideStateFree( strState );
      strState = NULL ;
   } else {
      memset( strState->iNumbers1, 0 ,strState->iLenNumber * sizeof( int_fast8_t ) );  // the numbers 1 found
      memset( strState->iNumbers2, 0, strState->iLenNumber * sizeof( int_fast8_t ) );  // the numbers 2 found
   }
   return strState ;
}

// clone given structure
struct strDivideState * strDivideStateClone( struct strDivideState *strOrg ) 
{
   struct strDivideState *strClone ;
   int_fast8_t            iCnt     ;
   int_fast8_t            iCnt2    ;
   
   if ( strOrg == NULL ) {
      return NULL ;
   }
   strClone = strDivideStateAlloc( strOrg->cNumber );
   if ( strClone == NULL ) {
      return NULL ;
   }

   strClone->iLenNumber  = strOrg->iLenNumber  ;  // number of digits in cNumber
   strClone->iMaxCurNr   = strOrg->iMaxCurNr   ;  // max current number = iLenNumber - 1 
   strClone->iMinCurNr   = strOrg->iMinCurNr   ;  // min current number (for threads)
   strClone->iMaxLenNum1 = strOrg->iMaxLenNum1 ;  // max lengte number1
   strClone->iMaxLenNum2 = strOrg->iMaxLenNum2 ;  // max length number2
   strClone->iLenHalve   = strOrg->iLenHalve   ;  // halve the length of the number, needed too dynamicaly determine max iNumber2
   strClone->iCurNumber  = strOrg->iCurNumber  ;  // current number
   strClone->iFound      = strOrg->iFound      ; // number found = 1

   for( iCnt = 0; iCnt <= strClone->iCurNumber; iCnt++ ) {
      strClone->iNumbers1[ iCnt ] = strOrg->iNumbers1[ iCnt ] ;
      strClone->iNumbers2[ iCnt ] = strOrg->iNumbers2[ iCnt ] ;

      strClone->arrWalk[ iCnt ].iLastWalk      = strOrg->arrWalk[ iCnt ].iLastWalk ;
//      strClone->arrWalk[ iCnt ].iDigOver       = strOrg->arrWalk[ iCnt ].iDigOver  ;
      
      strClone->arrWalk[ iCnt ].iDigit         = strOrg->arrWalk[ iCnt ].iDigit         ; // current digit too calculate
      strClone->arrWalk[ iCnt ].iOverflowValue = strOrg->arrWalk[ iCnt ].iOverflowValue ; // array of overflow value too start with
      strClone->arrWalk[ iCnt ].iOverflowStart = strOrg->arrWalk[ iCnt ].iOverflowStart ; // array for each digit position in overflow array where too start for iNumbers2
      strClone->arrWalk[ iCnt ].iOverflowPos   = strOrg->arrWalk[ iCnt ].iOverflowPos   ; // array for each digit position in overflow array
      strClone->arrWalk[ iCnt ].iWalk1         = strOrg->arrWalk[ iCnt ].iWalk1         ; // 9..0
      strClone->arrWalk[ iCnt ].iWalk2         = strOrg->arrWalk[ iCnt ].iWalk2         ; // 9..0
      strClone->arrWalk[ iCnt ].iStart1        = strOrg->arrWalk[ iCnt ].iStart1        ; // start Numbers 1
      strClone->arrWalk[ iCnt ].iStart2        = strOrg->arrWalk[ iCnt ].iStart2        ; // start Numbers 2
      strClone->arrWalk[ iCnt ].bCalc1         = strOrg->arrWalk[ iCnt ].bCalc1         ; // calc start Numbers 1, 1=true, 0=false
      strClone->arrWalk[ iCnt ].bCalc2         = strOrg->arrWalk[ iCnt ].bCalc2         ; // calc start Numbers 1, 1=true, 0=false
      
      strClone->arrWalk[ iCnt ].strLast        = strOrg->arrWalk[ iCnt ].strLast ;

      for( iCnt2 = 0; iCnt2 <= strClone->arrWalk[ iCnt ].iOverflowPos; iCnt2++ ) {
         strClone->arrWalk[ iCnt ].OverflowDigits[ iCnt2 ] = strOrg->arrWalk[ iCnt ].OverflowDigits[ iCnt2 ] ;
      }
   }
   
   return strClone ;
}
