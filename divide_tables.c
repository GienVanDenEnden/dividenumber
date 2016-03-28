/*
    divide_tables.c
    
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
#include <stdint.h>
#include <inttypes.h>

#include "divide_tables.h"

struct strNumber         glb_arrNumber[10][10]                ; // dimension 10 x 10 for all single digits, multiple & adding
struct strMultiAddFour   glb_arrMulAdd[10][10][10][10]        ; // n1, n2, n3, n4 = n1 * n2 + n3 * n4 = result
struct strLastMulAddFour glb_arrNoLastMulAdd[10][10][10][10]  ; // number last digits waar 1=Digit, 2=overflow(n5) 3=N1, 4=N3

// Number array functions:
//
// - numberArrayFill : Fill the arrays
// - numberArrayPrint: Print the arrays too default output
// ..........................................

//
// Fill the arrays
//
void numberArrayFill( void )
{
   int_fast8_t       iLeft  ;
   int_fast8_t       iRight ;
   struct strNumber *stNum  ;
   
   for( iLeft = 0; iLeft < 10; iLeft++ ) {
      for( iRight = 0 ; iRight < 10; iRight++ ) {
         stNum = &(glb_arrNumber[ iLeft ][ iRight ]);
         stNum->iNum1 = iLeft ;
         stNum->iNum2 = iRight;
         
         stNum->cNum1 = iLeft  + '0' ;
         stNum->cNum2 = iRight + '0' ;
         
         // iNum1 * iNum2
         stNum->iRstMul = iLeft * iRight ;
         sprintf( stNum->cRstMul, "%02d", stNum->iRstMul );
         
         stNum->iRstMulB1 = stNum->cRstMul[ 1 ] - '0' ;
         stNum->iRstMulB2 = stNum->cRstMul[ 0 ] - '0' ;

         stNum->cRstMulB1 = stNum->cRstMul[ 1 ] ;
         stNum->cRstMulB2 = stNum->cRstMul[ 0 ] ;
         
         // iNum1 + iNum2 
         stNum->iRstAdd = iLeft + iRight ;
         sprintf( stNum->cRstAdd, "%02d", stNum->iRstAdd );
         
         stNum->iRstAddB1 = stNum->cRstAdd[ 1 ] - '0' ;
         stNum->iRstAddB2 = stNum->cRstAdd[ 0 ] - '0' ;

         stNum->cRstAddB1 = stNum->cRstAdd[ 1 ] ;
         stNum->cRstAddB2 = stNum->cRstAdd[ 0 ] ;
      }
   }
   
   // fill the multiadd array
   int_fast8_t       iN1  ;
   int_fast8_t       iN2  ;
   int_fast8_t       iN3  ;
   int_fast8_t       iN4  ;
   int_fast8_t       iN5  ;

   struct strNumber stNum1  ;
   struct strNumber stNum2  ;
   struct strNumber stAdd   ;
   
   struct strMultiAddFour *stMulAdd     ;
   struct strMultiAddFour  stMulAddOVer ;
   
   // counbt number of end digits
   for( iN1 = 0; iN1 < 10; iN1++ ) {
      for( iN2 = 0; iN2 < 10; iN2++ ) {
         for( iN3 = 0; iN3 < 10; iN3++ ) {
            for( iN5 = 0; iN5 < 10; iN5++ ) {
               glb_arrNoLastMulAdd[ iN1 ][iN5][ iN2 ][ iN3 ].iCount = 0 ;
            }
         }
      }
   }

   // fill multi add structure and count end digits
   for( iN1 = 0; iN1 < 10; iN1++ ) {
      for( iN2 = 0; iN2 < 10; iN2++ ) {
         for( iN3 = 0; iN3 < 10; iN3++ ) {
            for( iN4 = 0; iN4 < 10; iN4++ ) {
               stMulAdd = &( glb_arrMulAdd[ iN1 ][ iN2 ][ iN3 ][iN4 ] ) ;
               
               stMulAdd->n1          = iN1 ;
               stMulAdd->n2          = iN2 ;
               stMulAdd->n3          = iN3 ;
               stMulAdd->n4          = iN4 ;
               stMulAdd->n5          = 0   ;
               stMulAdd->iNrOverflow = 0   ; 
   
               stNum1 = glb_arrNumber[ iN1 ][ iN2 ] ;
               stNum2 = glb_arrNumber[ iN3 ][ iN4 ] ;
               
               stAdd  = glb_arrNumber[ stNum1.iRstMulB1 ][ stNum2.iRstMulB1 ];
               
               stMulAdd->iRstLast  = stAdd.iRstAddB1 ;

               if ( stNum1.iRstMulB2 > 0 ) {
                  stMulAdd->iOverflow[ stMulAdd->iNrOverflow ] = stNum1.iRstMulB2 ;
                  stMulAdd->iNrOverflow++ ;
               }
               if ( stNum2.iRstMulB2 > 0 ) {
                  stMulAdd->iOverflow[ stMulAdd->iNrOverflow ] = stNum2.iRstMulB2 ;
                  stMulAdd->iNrOverflow++ ;
               }
               if ( stAdd.iRstAddB2 > 0 ) {
                  if ( stMulAdd->iNrOverflow == 0 ) {
                     stMulAdd->iOverflow[ stMulAdd->iNrOverflow ] = stAdd.iRstAddB2 ;
                     stMulAdd->iNrOverflow++ ;
                  } else {
                     stMulAdd->iOverflow[ stMulAdd->iNrOverflow - 1 ]++ ;
                  }
               }
               if ( stMulAdd->iNrOverflow == 2 ) {
                  if ( stMulAdd->iOverflow[ 0 ] + stMulAdd->iOverflow[ 1 ] < 10 ) {
                     stMulAdd->iOverflow[ 0 ] += stMulAdd->iOverflow[ 1 ] ;
                     stMulAdd->iNrOverflow = 1 ;
                  }
               }
               
               // collect end digits
              for( iN5 = 0; iN5 < 10; iN5++ ) {

                 stMulAddOVer = *stMulAdd ;
                 stMulAddOVer.n5 = iN5 ;
               
                 stAdd  = glb_arrNumber[ stMulAdd->iRstLast ][ stMulAddOVer.n5 ];

                 stMulAddOVer.iRstLast = stAdd.iRstAddB1 ;
                 if ( stAdd.iRstAddB2 > 0 ) {
                    if ( stMulAddOVer.iNrOverflow == 0 ) {
                       stMulAddOVer.iOverflow[ stMulAddOVer.iNrOverflow ] = stAdd.iRstAddB2 ;
                       stMulAddOVer.iNrOverflow++ ;
                    } else {
                       if ( stMulAddOVer.iOverflow[ stMulAddOVer.iNrOverflow - 1 ] < 9 ) {
                          stMulAddOVer.iOverflow[ stMulAddOVer.iNrOverflow - 1 ]++ ;
                       } else {
                          stMulAddOVer.iOverflow[ stMulAddOVer.iNrOverflow ] = stAdd.iRstAddB2 ;
                          stMulAddOVer.iNrOverflow++ ;
                       }
                    }
                 }
                  
                 glb_arrNoLastMulAdd[ stMulAddOVer.iRstLast ][ iN5 ][ iN1 ][ iN3 ].strLast[ glb_arrNoLastMulAdd[ stMulAddOVer.iRstLast ][ iN5 ][ iN1 ][ iN3 ].iCount ] = stMulAddOVer ;
                 glb_arrNoLastMulAdd[ stMulAddOVer.iRstLast ][ iN5 ][ iN1 ][ iN3 ].iCount++ ;
              }
            }
         }
      }
   }
}

//
// print number arrays too default output
//
void numberArrayPrint( void )
{
   int_fast8_t       iLeft  ;
   int_fast8_t       iRight ;
   struct strNumber *stNum  ;

   int_fast8_t       iN1  ;
   int_fast8_t       iN2  ;
   int_fast8_t       iN3  ;
   int_fast8_t       iN4  ;
   int_fast8_t       iN5  ;
   int_fast8_t       iOver;

   struct strMultiAddFour    *stMulAdd ;
   struct strLastMulAddFour  *stFour   ;
   
   printf( "glb_arrNumber\n" );
   for( iLeft = 0; iLeft < 10; iLeft++ ) {
      for( iRight = 0 ; iRight < 10; iRight++ ) {
         stNum = &(glb_arrNumber[ iLeft ][ iRight ]);
         
         printf( "%d (%c) * %d (%c) = %2d (%2s) [%d (%c), %d (%c)],  %d + %d = %2d (%2s) [%d (%c), %d (%c)]\n"
               , stNum->iNum1
               , stNum->cNum1
               , stNum->iNum2
               , stNum->cNum2
               // multi
               , stNum->iRstMul
               , stNum->cRstMul
               , stNum->iRstMulB2
               , stNum->cRstMulB2
               , stNum->iRstMulB1
               , stNum->cRstMulB1
               
               , stNum->iNum1
               , stNum->iNum2
               // Add
               , stNum->iRstAdd
               , stNum->cRstAdd
               , stNum->iRstAddB2
               , stNum->cRstAddB2
               , stNum->iRstAddB1
               , stNum->cRstAddB1
               
               );
      }
      // printf( "\n" );
   }
   printf( "\n" );
   
   
   printf( "\nstrMultiAddFour:\n" );
   for( iN1 = 0; iN1 < 10; iN1++ ) {
      for( iN2 = 0; iN2 < 10; iN2++ ) {
         for( iN3 = 0; iN3 < 10; iN3++ ) {
            for( iN4 = 0; iN4 < 10; iN4++ ) {
               stMulAdd = &( glb_arrMulAdd[ iN1 ][ iN2 ][ iN3 ][iN4 ] ) ;

               printf( "%d * %d + %d * %d = %d"
                     , stMulAdd->n1 
                     , stMulAdd->n2
                     , stMulAdd->n3 
                     , stMulAdd->n4 
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
      }
   }

   printf( "\nstrLastMulAddFour:\n" );
   for( iN1 = 0; iN1 < 10; iN1++ ) {
      for( iN2 = 0; iN2 < 10; iN2++ ) {
         for( iN3 = 0; iN3 < 10; iN3++ ) {
            for( iN4 = 0; iN4 < 10; iN4++ ) {
               stFour = &( glb_arrNoLastMulAdd[ iN1 ][ iN2 ][ iN3 ][iN4 ] ) ;
               printf( "Digit: %d, Overflow: %d, n1: %d, n3: %d, count: %d\n", iN1, iN2, iN3, iN4, stFour->iCount );
               for( iN5 = 0; iN5 < stFour->iCount; iN5++ ) {

                  stMulAdd = &( stFour->strLast[ iN5 ] ) ;
                  

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
         }
      }
   }
}
