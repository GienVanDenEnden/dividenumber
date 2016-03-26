/*
    divide_calc.c
    
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
#include <pthread.h>
#include <signal.h>
#include <linux/limits.h>

#include "divide_tables.h"
#include "divide_calc.h"
#include "divide_alloc.h"
#include "divide_file.h"
#include "divide_threads.h"

int_fast8_t glb_iNumberFindAll =  0 ; // if 1 then find all the numbers, otherwise stop by the first found
int_fast8_t glb_iDigitsCalcCPU = 12 ; // number of digits 1 (one) CPU can process in (estimate) 1 second, use to start threads




//........................................
// Process functions:
//
// - numberFoundPrint            : print the found numbers
// - numberCheckFound            : Check if valid numbers are found
// - numberCalcCurrentNumber     : Calculate the current digit, the main calculation routine
// - numberOverflowCurNumberStart: Calculate the start overflow for the current digit
// - numberStart2                : dertermine the start number for 2, number1 must be set
// - numberStart1                : dertermine the start number for 1 
// - numberWalkCalcStart         : calculate the start position for iCurNumber
//
// - numberWalkNumItterative     : walk all the numbers, main process loop
//
// - numberDivide                : Start divide
//........................................


//                          x3        x2       x1
//                          y3        y2       y1 
//                     --------------------------
//                     y1 * x3   y1 * x2  y1 * x1
//            y2 * x3  y2 * x2   y2 * x1        0
//   y3 * x3  y3 * x2  y3 * x1         0        0
//----------------------------------------------
//d6      d5       d4       d3        d2       d1
//
// d1 = y1 * x1
// d2 = y1 * x2 + y2 * x1             + overflow d1
// d3 = y1 * x3 + y2 * x2 + y3 * x1   + overflow d2
// d4 =           y2 * x3 + y3 * x2   + overflow d3
// d5 =                     y3 * x3   + overflow d4
// d6 =                                 overflow d5

void numberFoundPrint( struct strDivideState *strState )
{
   int_fast8_t iCnt     ; 
   int_fast8_t iFnd = 1 ; 
   int_fast8_t iNul     ;

   // lock result file
   pthread_mutex_lock( &glb_mutex_resultfile );

   printf( "\nFound number:\n" );
   for( iCnt = strState->iCurNumber; iCnt >= 0 ; iCnt-- ) {
      printf( "%c", strState->iNumbers1[ iCnt ] + '0' );
   }
   printf( "\n" );

   for( iCnt = strState->iCurNumber; iCnt >= 0 ; iCnt-- ) {
      printf( "%c", strState->iNumbers2[ iCnt ] + '0' );
   }
   printf( "\n" );
   
   if ( strState->iNumbers1[ 0 ] == 1  ) {
      iFnd = 0 ;
      for( iCnt = strState->iCurNumber; iCnt >= 1 ; iCnt-- ) {
         if ( strState->iNumbers1[ iCnt ] != 0 ) {
            iFnd = 1;
            break ;
         }
      }
   }

/* DEBUG   
   int iDCnt    ;
   int iOverPos ;
   int iOver    ;
   struct strMultiAddFour *stMulAdd ;
   
   for ( iDCnt = strState->iCurNumber; iDCnt >= 0; iDCnt-- ) {
      printf( "Curr : %d, Digit:%d, n1: %d, n2: %d, OverVal: %d, OverPos: %d, LastW: %d"
            , iDCnt
            , strState->arrWalk[ iDCnt ].iDigit
            , strState->iNumbers1[ iDCnt ]
            , strState->iNumbers2[ iDCnt ]
            , strState->arrWalk[ iDCnt ].iOverflowValue
            , strState->arrWalk[ iDCnt ].iOverflowPos
            , strState->arrWalk[ iDCnt ].iLastWalk
      );
      for ( iOverPos = 0; iOverPos < strState->arrWalk[ iDCnt ].iOverflowPos; iOverPos++ ) {
         if ( iOverPos == 0 ) {
            printf( " Overflow:" );
         }
         printf( " %d", strState->arrWalk[ iDCnt ].OverflowDigits[ iOverPos ] );
      }
      
      printf( "\n" );
      
      if ( strState->arrWalk[ iDCnt ].strLast != NULL ) {

         stMulAdd = &( strState->arrWalk[ iDCnt ].strLast->strLast[ strState->arrWalk[ iDCnt ].iLastWalk - 1 ] );
         
         printf( "%d * %d + %d * %d + %d = %d"
               , stMulAdd->n1 
               , stMulAdd->n2
               , stMulAdd->n3 
               , stMulAdd->n4 
               , stMulAdd->n5
               , stMulAdd->iRstLast
         );
         for( iOver = 0; iOver < stMulAdd->iNrOverflow; iOver++ ) {
            if ( iOver == 0 ) {
               printf( "  Over: " );
            } else {
               printf( " + " );
            }
            printf( "%d", stMulAdd->iOverflow[ iOver ] );
         }
         printf( "\n" );
         
      }
   }
END DEBUG */     

     
   // found stop all
   if ( iFnd == 1 && glb_iNumberFindAll == 0 ) {
      glb_iThreadStop = 1 ;

      // stop create new threads      
      glb_iThreadFree = -100 ; // no free thread anymore 
   }

   // open file voor appending
   FILE *strFile = fopen( glb_cFileResult, "a");
   
   if ( strFile == NULL ) {
      fprintf( stderr, "Cannot open result file: %s\n", glb_cFileResult ) ;
   } else {
      fprintf( strFile, "Number  : %s\n", strState->cNumber );
      
      fprintf( strFile, "Number 1: " );
      iNul = 1 ;
      for( iCnt = strState->iCurNumber; iCnt >= 0 ; iCnt-- ) {
         if ( iNul == 1 && strState->iNumbers1[ iCnt ] == 0 ) {
            fprintf( strFile, " " );
         } else {
            iNul = 0 ;
            fprintf( strFile, "%c", strState->iNumbers1[ iCnt ] + '0' );
         }
      }
      fprintf( strFile, "\n" );

      fprintf( strFile, "Number 2: " );
      iNul = 1 ;
      for( iCnt = strState->iCurNumber; iCnt >= 0 ; iCnt-- ) {
         if ( iNul == 1 && strState->iNumbers2[ iCnt ] == 0 ) {
            fprintf( strFile, " " );
         } else {
            iNul = 0 ;
            fprintf( strFile, "%c", strState->iNumbers2[ iCnt ] + '0' );
         }
      }
      fprintf( strFile, "\n\n" );
      
      fclose( strFile );
   }
   
   // unlock result file
   pthread_mutex_unlock( &glb_mutex_resultfile );
}

//
// calc the current number (iCurNumber)
// this must be done after iNumbers1 & iNumbers2 are set
// dq = sum( y[1..q] * x[q..1] ) + overload d[q-1]
//
int_fast8_t numberCalcCurrentNumber( struct strDivideState *strState, struct strWalkState *oWalk )
{
   
   int_fast8_t       iSum2     ;
   int_fast8_t       iPrev     ;
   struct strNumber  stNum     ;  // multiplier result
   struct strNumber  stAdd     ;  // add result
   
   // set start of overflow back too the end of iNumbers1   
   oWalk->iOverflowPos = oWalk->iOverflowStart ;
   
   // calc the sum for the current digit
   // dq = sum( y[1..q] * x[q..1] )
   iPrev = oWalk->iOverflowValue ;
   
   // iSum1 = 0 ;
   iSum2 = strState->iCurNumber ; 
   
   stNum = glb_arrNumber[ strState->iNumbers1[ 0 ] ][ strState->iNumbers2[ iSum2 ] ];
   
   // overflow opslaan
   if ( stNum.iRstMulB2 > 0 ) {
      oWalk->OverflowDigits[ oWalk->iOverflowPos ] = stNum.iRstMulB2 ;
      oWalk->iOverflowPos++ ;
   }
   // add with previous number
   stAdd = glb_arrNumber[ stNum.iRstMulB1 ][ iPrev ];
   
   if ( stAdd.iRstAddB2 > 0 ) {
      oWalk->OverflowDigits[ oWalk->iOverflowPos ] = stAdd.iRstAddB2 ;
      oWalk->iOverflowPos++ ;
   }
   iPrev = stAdd.iRstAddB1 ;
   
   if ( iSum2 != 0 ) {
      stNum = glb_arrNumber[ strState->iNumbers1[ iSum2 ] ][ strState->iNumbers2[ 0 ] ];

      // overflow opslaan
      if ( stNum.iRstMulB2 > 0 ) {
         oWalk->OverflowDigits[ oWalk->iOverflowPos ] = stNum.iRstMulB2  ;
         oWalk->iOverflowPos++ ;
      }
      // add with previous number
      stAdd = glb_arrNumber[ stNum.iRstMulB1 ][ iPrev ];
      
      if ( stAdd.iRstAddB2 > 0 ) {
         oWalk->OverflowDigits[ oWalk->iOverflowPos ] = stAdd.iRstAddB2  ;
         oWalk->iOverflowPos++ ;
      }
      iPrev = stAdd.iRstAddB1 ;
   }
   return iPrev ;
   
}


// only calc the digit no overflow, just for speed
int_fast8_t numberCalcCurrentNumberFast( struct strDivideState *strState, struct strWalkState *oWalk )
{
   int_fast8_t       iSum2     ;
   int_fast8_t       iPrev     ;

   struct strNumber stNum     ;  // multiplier result
   struct strNumber stAdd     ;  // add result
   
   // calc the sum for the current digit
   // dq = sum( y[1..q] * x[q..1] )
   iPrev = oWalk->iOverflowValue ;
   
   iSum2 = strState->iCurNumber ; 
   
   stNum = glb_arrNumber[ strState->iNumbers1[ 0 ] ][ strState->iNumbers2[ iSum2 ] ];
   
   // add with previous number
   stAdd = glb_arrNumber[ stNum.iRstMulB1 ][ iPrev ];
   
   iPrev = stAdd.iRstAddB1 ;
   
   if ( iSum2 != 0 ) {
      stNum = glb_arrNumber[ strState->iNumbers1[ iSum2 ] ][ strState->iNumbers2[ 0 ] ];

      stAdd = glb_arrNumber[ stNum.iRstMulB1 ][ iPrev ];
      
      iPrev = stAdd.iRstAddB1 ;
   }
   return iPrev ;
}


//
// check if valid numbers are found
//
int_fast8_t numberCheckFound( struct strDivideState *strState )
{
   int_fast8_t iFound    = 0 ; // return value
   int_fast8_t iNum2Null     ;
   int_fast8_t iSum1         ;
   int_fast8_t iSum2         ;

   if ( strState->iCurNumber == strState->iMaxCurNr ) {
   
      // bij overflow niets gevonden...
      if ( strState->arrWalk[ strState->iCurNumber ].iOverflowPos == 0  ) {
         // niet alleen overflow maar ook toekomstige nummers checken
         
         // voorbeeld nummer 12312343
         // voorbeeld nummer 193

         // 1e start altijd met nullen
         // 2e kan max aantal cijfers hebben
         // check omgekeerd evenredig
         // tel aantal nullen waar nummer2 mee begint
         // zover cijfers + 1 mag nummer 1 bevatten
         iNum2Null = 0 ;
         for( iSum1 = strState->iCurNumber; iSum1 >= 0; iSum1-- ) {
            if ( strState->iNumbers2[ iSum1 ] == 0 ) {
               iNum2Null++ ;
            } else {
               break;
            }
         }
         // 0 digits
         // allow is 1 digit
         iNum2Null++ ; // the numbers of digits in iNumbers2
         iSum2 = 0 ;
         for( iSum1 = strState->iCurNumber; iSum1 >= iNum2Null; iSum1-- ) {
            if ( strState->iNumbers1[ iSum1 ] != 0 ) {
               iSum2 = 1 ;
               break ;
            }
         }
         
         if ( iSum2 > 0 ) {
            // er is nog een volgende cijfers
            // m.a.w. no match
         } else  {
            iFound = 1 ;
            numberFoundPrint( strState );
            
// DEBUG            
//            // save found structure,
//            fileStateSave( cSaveName, strState );
            
            strState->iFound = 1 ;
         }
      } 
// DEBUG      
//      else {
//         printf( "Overflow found, skip this\n" );
//      }
   } 
   
   return iFound ;
}


//
// calc the overflow for the current number (icurNumber)
// This must be done before starting the current number
//
void numberOverflowCurNumberStart( struct strDivideState *strState, struct strWalkState *oWalk )
{
   int_fast8_t          iCnt   ;
   int_fast8_t          iNum1  ;
   int_fast8_t          iNum2  ;
   struct strNumber     stAdd  ;
   struct strWalkState *oWprev ;
   int_fast8_t         *OverflowDigits  ;
   int_fast8_t         *OverflowWalk    ;
   
   // copy overflow from strState->iCurNumber - 1 to current
   oWalk->iOverflowPos   = 0                     ; // first free number
   oWalk->iOverflowValue = 0                     ; // extra value too start with
   OverflowWalk          = oWalk->OverflowDigits ;
   iNum2                 = 0                     ;
   oWalk->iLastWalk      = 0                     ;

   if ( strState->iCurNumber > 0 ) {
      oWprev         = strState->arrWalk + strState->iCurNumber - 1 ;
      OverflowDigits = oWprev->OverflowDigits ;
      for( iCnt = 0; 
           iCnt < oWprev->iOverflowPos ; 
           iCnt++ 
         ) {
         iNum1 = OverflowDigits[ iCnt ] ;

         stAdd = glb_arrNumber[ iNum1 ][ iNum2 ];
         iNum2 = stAdd.iRstAddB1 ; 
         if ( stAdd.iRstAddB2 > 0 ) {
            if (  oWalk->iOverflowPos > 0 
                  && *( OverflowWalk - 1 ) < 9
               ) {
               ( *( OverflowWalk - 1) ) ++ ;
            } else {
               *OverflowWalk = stAdd.iRstAddB2 ;
               OverflowWalk++ ;
               oWalk->iOverflowPos++ ;
            }
         }
      }
   }

   //   
   // dq = sum( y[1..q] * x[q..1] ) + overload d[q-1]
   // 2 .. q -1 is constant for iCurNumber
   int_fast8_t       iSum1   ;
   struct strNumber  stNum   ;
   int_fast8_t       iEnd    ;
   int_fast8_t      *Numbers1;
   int_fast8_t      *Numbers2;

   iEnd     = strState->iCurNumber ;
   Numbers1 = strState->iNumbers1 + 1 ;
   Numbers2 = strState->iNumbers2 + strState->iCurNumber - 1 ;

   for( iSum1 = 1 
      ; iSum1 < iEnd
      ; iSum1++,  Numbers1++, Numbers2--
      ) {
      
      if ( *Numbers1 == 0 || *Numbers2 == 0 ) {
         continue ;
      }
      
      stNum = glb_arrNumber[ *Numbers1 ][ *Numbers2 ];
      if ( stNum.iRstMulB2 > 0 ) {
         *OverflowWalk = stNum.iRstMulB2 ;
         OverflowWalk++ ;
         oWalk->iOverflowPos++ ;
      }
      
      // add with previous number
      stAdd = glb_arrNumber[ stNum.iRstMulB1 ][ iNum2 ];
      if ( stAdd.iRstAddB2 > 0 ) {
         if (  oWalk->iOverflowPos > 0 
            && *( OverflowWalk - 1 ) < 9
            ) {
            ( *( OverflowWalk - 1 ) ) ++ ;
         } else {
            *OverflowWalk = stAdd.iRstAddB2 ;
             OverflowWalk++ ;
             oWalk->iOverflowPos++ ;
         }
      }
      iNum2 = stAdd.iRstAddB1 ;
   }
   oWalk->iOverflowValue = iNum2 ;
   oWalk->iOverflowStart = oWalk->iOverflowPos ;

   // what is the last digit too search for
   if ( strState->iCurNumber > 0 ) {
      oWalk->strLast = &(glb_arrNoLastMulAdd[ oWalk->iDigit ][ oWalk->iOverflowValue ][ strState->iNumbers2[ 0 ] ][ strState->iNumbers1[ 0 ] ] );
   }
}


// dertermine the start number for 2, number1 must be set
int_fast8_t numberStart2( struct strDivideState *strState )
{
   int_fast8_t iStart = 9 ;

   if ( strState->iCurNumber >= strState->iMaxLenNum1 ) {
      // 0010 -> iMaxlen1 = 2, 
      // 0100 -> max len 3
      // ----
      // 1000
      //
      // better if strState->iCurNumber == strState->iMaxLenNum1
      // then each 0 is one if max len2 lesser

      // iCurNumber = 0-based, iMaxLen1 = 1-based
      if ( strState->iCurNumber >= strState->iLenHalve ) {
         int_fast8_t iMaxLen2 = strState->iLenHalve + 1  ;
         int_fast8_t iWalk    ;
         
         for ( iWalk = strState->iLenHalve - 1; iWalk > 0; iWalk-- ) {
            if ( strState->iNumbers1[ iWalk ] == 0 ) {
               iMaxLen2++;
            } else {
               break;
            }
         }
         if ( iMaxLen2 <= strState->iCurNumber ) {
            iStart = 0 ;
         }
      }
   }
   return iStart ;
}

// calculate the start position for the iCurNumber
void numberWalkCalcStart( struct strDivideState *strState, struct strWalkState *oWalk )
{
   // count overflow numbers from the iCurNumber - 1 
   numberOverflowCurNumberStart( strState, oWalk );

   if ( strState->iCurNumber >= strState->iMaxLenNum1 ) {
      oWalk->iStart2 = numberStart2( strState );
   } else {
      oWalk->iStart2 = 9 ;
   }
   oWalk->iWalk1 = oWalk->iStart1 ;
   oWalk->iWalk2 = oWalk->iStart2 ;
   oWalk->bCalc1 = 0 ;
   oWalk->bCalc2 = 1 ;
}

//
// walk all the numbers, main process loop
// 
void numberWalkNumItterative( struct strDivideState *strState )
{
   int_fast8_t            iDigit   ;
   struct strWalkState   *oWalk    ;
   struct strDivideState *strClone ;
   int_fast8_t           *number1  ;
   int_fast8_t           *number2  ;
   int_fast8_t            iCnt     ;
   
   int_fast8_t           *OverflowWalk    ;

   struct strMultiAddFour   *strFour;
   
   MAINBLOCK:
   while( strState->iCurNumber >= strState->iMinCurNr ) {
      oWalk   = strState->arrWalk   + strState->iCurNumber ;
      
      // recalc values
      if ( oWalk->bCalc1 == 1 ) {   
         numberWalkCalcStart( strState, oWalk );
      }

      if ( strState->iCurNumber > 0 /* && strState->iCurNumber < 0 */ ) {
      
         while( oWalk->iLastWalk < oWalk->strLast->iCount /* strLast.iCount */ ) 
         {
            strFour = &( oWalk->strLast->strLast[ oWalk->iLastWalk ] );
            oWalk->iLastWalk++;
            
            if ( oWalk->iStart1 == 0 && strFour->n2 != 0 ) {
               continue ;
            }
            if ( oWalk->iStart2 == 0 && strFour->n4 != 0 ) {
               continue ;
            }
            
            strState->iNumbers1[ strState->iCurNumber ] = strFour->n2 ;
            strState->iNumbers2[ strState->iCurNumber ] = strFour->n4 ;
            
            oWalk->iOverflowPos = oWalk->iOverflowStart ;
            OverflowWalk = oWalk->OverflowDigits + oWalk->iOverflowPos ;

            for ( iCnt = 0; iCnt < strFour->iNrOverflow; iCnt++ ) {
               *OverflowWalk = strFour->iOverflow[ iCnt ];
               OverflowWalk++ ;
               oWalk->iOverflowPos++;
            }
            
            if ( strState->iCurNumber == strState->iMaxCurNr ) {
               if ( numberCheckFound( strState ) == 1 ) {
                  // return ; // <<<<<<<<<<<<<<<<
               }
            } else {
               // next digit
               strState->iCurNumber++;
               strState->arrWalk[ strState->iCurNumber ].bCalc1 = 1 ; // calc start again

               // start thread....
               if ( strState->iCurNumber <= strState->iMaxCurCPU ) {
                  if ( glb_iThreadFree > 0 ) {
                     // start new thread
                     strClone = strDivideStateClone( strState );
                     if ( strClone != NULL ) {
                        strClone->iMinCurNr = strState->iCurNumber ;
                        if ( threadStart( strClone ) < 0 ) {
                           // cannot start thread
                           strDivideStateFree( strClone );
                        } else {
                           // we are going back too the previous number
                           strState->iCurNumber--;
                        }
                     }
                  }
               }
               goto MAINBLOCK ; // <<<<<<<<<<<<<<<<<
            }
         }
      }
      else {
         number1 = strState->iNumbers1 + strState->iCurNumber ;
         number2 = strState->iNumbers2 + strState->iCurNumber ;
         
         // walk number1
         while( oWalk->iWalk1 >= 0 ) {
            
            // strState->iNumbers1[ strState->iCurNumber ] = oWalk->iWalk1 ;
            *number1 = oWalk->iWalk1 ;
            
            if ( oWalk->bCalc2 == 1 ) {
               oWalk->iWalk2 = oWalk->iStart2 ;
               oWalk->bCalc2 = 0 ;
            }
            
            // walk number2
            while( oWalk->iWalk2 >= 0 ) {

               // strState->iNumbers2[ strState->iCurNumber ] = oWalk->iWalk2 ;
               *number2 = oWalk->iWalk2 ;
               
               // calculate the current digit 
               iDigit = numberCalcCurrentNumberFast( strState, oWalk );

               if  ( iDigit == oWalk->iDigit ) {
                  numberCalcCurrentNumber( strState, oWalk );
                  if ( strState->iCurNumber == strState->iMaxCurNr ) {
                     if ( numberCheckFound( strState ) == 1 ) {
                        // return ; // <<<<<<<<<<<<<<<<
                     }
                  } else {
                     // current number processed, iNumber2 bevat current number, iWalk2 is next number
                     oWalk->iWalk2--; 
                     
                     // next digit
                     strState->iCurNumber++;
                     strState->arrWalk[ strState->iCurNumber ].bCalc1 = 1 ; // calc start again
                     
                     // start thread....
                     if ( strState->iCurNumber <= strState->iMaxCurCPU ) {
                        if ( glb_iThreadFree > 0 ) {
                           // start new thread
                           strClone = strDivideStateClone( strState );
                           if ( strClone != NULL ) {
                              strClone->iMinCurNr = strState->iCurNumber ;
                              if ( threadStart( strClone ) < 0 ) {
                                 // cannot start thread
                                 strDivideStateFree( strClone );
                              } else {
                                 // we are going back too the previous number
                                 strState->iCurNumber--;
                              }
                           }
                        }
                     }
                     
                     goto MAINBLOCK ; // <<<<<<<<<<<<<<<<<
                  }
               }
               oWalk->iWalk2--;
            }
            oWalk->bCalc2 = 1 ;
            oWalk->iWalk1-- ;
         }
      }      
      // end of loop, go to previous number
      strState->iCurNumber--;
   }
}


// start number from thread
void *numberThread( void *data )
{
   struct strDivideState *strState ;
   
   strState = (struct strDivideState *)data;
   
   numberWalkNumItterative( strState );
   
   // make thread free
   threadEnd( strState );
   
   return NULL ;
}


// divide number 
void numberDivide( char *cNumber )
{
   int                    iThread  ;
   int_fast8_t            iCnt     ;
   struct strDivideState *strState ;
  
   strState = strDivideStateAlloc( cNumber ) ;
   
   if ( strState == NULL ) {
      fprintf( stderr, "Cannot allocate memory\n" );
      return ;
   }

   //
   // if last number is even then iNumbers1 is fixed on 2
   // if last number is 5 then iNumbers is fixed on 5
   // if iCurPos == 0 and iMaxlen1 > 1 then only valid digits are 1,3,7 and 9
   //
   
   // even numbers of numbers ending with a 5
   // only 1 number seaking
   switch( strState->arrWalk[ 0 ].iDigit ) {
      case 0 :
      case 2 :
      case 4 :
      case 5 :
      case 6 :
      case 8 :
         strState->iMaxLenNum1 =  1 ;
         strState->iMaxCurCPU  = -1 ;
         for( iCnt = 0; iCnt < strState->iLenNumber; iCnt++ ) {
            if ( iCnt >= strState->iMaxLenNum1 ) {
               strState->arrWalk[ iCnt ].iStart1 = 0 ;
            }
         }
         break ; 
   }
   
   // delete result file
   remove( glb_cFileResult );
   
   printf( "Length number      : %d\n", strState->iLenNumber  );
   printf( "Max length number 1: %d\n", strState->iMaxLenNum1 );
   printf( "Max length number 2: %d\n", strState->iMaxLenNum2 );
   printf( "Halve length nr    : %d\n", strState->iLenHalve   );
   printf( "Number             : %s\n", strState->cNumber     );

   if ( glb_iThreadMax  > 0 ) {
      iThread = threadStart( strState );
      if ( iThread < 0 ) {
         fprintf( stderr, "Cannot start thread\n" );
         exit(1);
      }
   } else {
      // no threads
      numberWalkNumItterative( strState );
   }
}
