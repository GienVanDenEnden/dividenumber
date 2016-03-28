/*
    divide_file.c
    
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
#include <linux/limits.h>
#include <time.h>

#include "divide_tables.h"
#include "divide_common.h"
#include "divide_calc.h"
#include "divide_alloc.h"

char glb_cFileState[  PATH_MAX ] = "out_state_%" SCNdFAST8 ".txt"  ; // file name too store state, for each thread 1 file
char glb_cFileResult[ PATH_MAX ] = "out_result.txt"                ; // all the found numbers will be stored here


//........................................
// File functions:
// - fileStateSave : Save state
// - fileStateLoad : Load state
// - fileStateName : Give filename for given threadnr
//........................................
void fileStateName( int_fast8_t iThreadNr, char *filename )
{
   sprintf( filename, glb_cFileState, iThreadNr );
}

int fileStateSave( char *filename, struct strDivideState *strState )
{
   int iCnt  ;
   int iCnt2 ;
   
   time_t     rawtime  ;
   struct tm *timeinfo ;

   // open file voor appending
   FILE *strFile = fopen( filename, "w");
   
   if ( strFile == NULL ) {
      return 1 ;
   }

   time ( &rawtime );
   timeinfo = localtime ( &rawtime );
   
   
   fprintf( strFile, "#\n" );
   fprintf( strFile, "# Dividenumber state\n" );
   
   fprintf( strFile, "# Date: %04d-%02d-%02d %02d:%02d:%02d\n"
                   , timeinfo->tm_year + 1900 
                   , timeinfo->tm_mon + 1
                   , timeinfo->tm_mday
                   , timeinfo->tm_hour
                   , timeinfo->tm_min
                   , timeinfo->tm_sec
                   );
   fprintf( strFile, "#\n" );
   fprintf( strFile, "\n" );
   
   fprintf( strFile, "[NumberState]\n" );
   fprintf( strFile, "number         : %s\n"            , strState->cNumber     );
   fprintf( strFile, "iLenNumber     : %" SCNdFAST8 "\n", strState->iLenNumber  ); 
   fprintf( strFile, "iMaxCurNr      : %" SCNdFAST8 "\n", strState->iMaxCurNr   ); 
   fprintf( strFile, "iMinCurNr      : %" SCNdFAST8 "\n", strState->iMinCurNr   ); 
   fprintf( strFile, "iMaxLenNum1    : %" SCNdFAST8 "\n", strState->iMaxLenNum1 ); 
   fprintf( strFile, "iMaxLenNum2    : %" SCNdFAST8 "\n", strState->iMaxLenNum2 ); 
   fprintf( strFile, "iLenHalve      : %" SCNdFAST8 "\n", strState->iLenHalve   ); 
   fprintf( strFile, "iCurNumber     : %" SCNdFAST8 "\n", strState->iCurNumber  ); 
   fprintf( strFile, "iFound         : %" SCNdFAST8 "\n", strState->iFound      ); 
   
   for( iCnt = 0; iCnt <= strState->iCurNumber; iCnt++ ) {
     fprintf( strFile, "[Current_%d]\n", iCnt );
     fprintf( strFile, "number1        : %" SCNdFAST8 "\n", strState->iNumbers1[ iCnt ]              );
     fprintf( strFile, "number2        : %" SCNdFAST8 "\n", strState->iNumbers2[ iCnt ]              );

     fprintf( strFile, "iDigit         : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iDigit         );
     fprintf( strFile, "iOverflowValue : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iOverflowValue );
     fprintf( strFile, "iOverflowStart : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iOverflowStart );
     fprintf( strFile, "iOverflowPos   : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iOverflowPos   );

     fprintf( strFile, "iLastWalk      : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iLastWalk   );
     
     fprintf( strFile, "iWalk1         : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iWalk1         );
     fprintf( strFile, "iWalk2         : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iWalk2         );
     fprintf( strFile, "iStart1        : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iStart1        );
     fprintf( strFile, "iStart2        : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].iStart2        );
     fprintf( strFile, "bCalc1         : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].bCalc1         );
     fprintf( strFile, "bCalc2         : %" SCNdFAST8 "\n", strState->arrWalk[ iCnt ].bCalc2         );
     
     for( iCnt2 = 0; iCnt2 < strState->arrWalk[ iCnt ].iOverflowPos; iCnt2++ ) {
        fprintf( strFile, "Overflow_%d     : %" SCNdFAST8 "\n", iCnt2, strState->arrWalk[ iCnt ].OverflowDigits[ iCnt2 ] );
     }
   }
   
   fprintf( strFile, "[End]\n" );

   fclose( strFile );
   
   return 0;
}

// read state file and give structure back or NULL
struct strDivideState * fileStateLoad( char *filename )
{
   // open file voor reading
   FILE *strFile        ;
   char  cLine[   4096 ];
   char  cDummy1[ 4096 ];
   char  cDummy2[ 4096 ];
   int   iDummy         ;
   int   iLen           ;
   char *linechar       ;
   int   iLineNr        ; // current line numer
   int   iEnd           ; // is end tag found 1=yes
   int   iState         ; // state of current 
   int   iWalkBlok      ; // current walk blok
   int   iOverflowblok  ; 
   int   iCnt           ;
   
   struct strDivideState *strState = NULL ; // the state too be returned
   struct strWalkState   *oWalk    = NULL ; // current walk blok
   
   
   strFile = fopen( filename, "r");
   if ( strFile == NULL ) {
      return NULL ;
   }
   
   iLineNr = 0 ;
   iEnd    = 0 ;
   iState  = 0 ; // 0 noting, 1=start found
   while ( fgets( cLine, sizeof(cLine), strFile )) {
      iLineNr++;
      iLen = strlen( cLine );
      
      // remove next lines
      for( linechar = cLine + iLen - 1 ; linechar > cLine;  linechar-- ) {
         if ( *linechar == '\r' || *linechar == '\n' ) {
            *linechar = '\0';
         } else {
            break;
         }
      }

      iLen = strlen( cLine );
      // min length with data is 2 char, used too skip empty lines
      if ( iLen <  2 ) {
         continue ;
      }
      // # = comment string
      if ( cLine[ 0 ] == '#' ) {
         continue;
      }
      
//      printf( "Line: %d; %s\n", iLineNr, cLine );
      
      // end of state file reach
      if ( strcmp( cLine, "[End]" ) == 0 ) {
         iEnd = 1 ;
         break;
      }
      
      // start of blok
      if ( strcmp( cLine, "[NumberState]" ) == 0 ) {
         if ( iState != 0 ) {
            fprintf( stderr, "Incorrecte [NumberState] at line %d, file %s\n", iLineNr, filename );
            break;
         }
         iState = 1 ;
         continue ;
      }
      
      
      if ( iState  == 1 ) {
         // state = 1 main block
         // state = 2 walk blok
         if ( strncmp( cLine, "number", 6 ) == 0 ) {
            sscanf( cLine, "%s : %s", cDummy1, cDummy2 );
            
            if ( strlen( cDummy2 ) < 1 ) {
               fprintf( stderr, "No valid number '%s' at linenr %d, file %s\n", cDummy2, iLineNr, filename );
               break;
            }
            if ( numberCheck( cDummy2 ) != 0 ) {
               fprintf( stderr, "No valid number '%s' at linenr %d, file %s\n", cDummy2, iLineNr, filename );
               break;
            }
            // create state 
            strState = strDivideStateAlloc( cDummy2 ) ;
            
            // printf( "Number found: %s\n", cDummy2 );
         } else {
            if ( strState == NULL ) {
               fprintf( stderr, "Number must be the first parameter '%s' at linenr %d, file %s\n", cLine, iLineNr, filename );
               break;
            }                          //12345678901
            if        ( strncmp( cLine, "iLenNumber" , 10 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &strState->iLenNumber  ); 
            } else if ( strncmp( cLine, "iMaxCurNr"  ,  9 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &strState->iMaxCurNr   ); 
            } else if ( strncmp( cLine, "iMinCurNr"  ,  9 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &strState->iMinCurNr   ); 
            } else if ( strncmp( cLine, "iMaxLenNum1", 11 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &strState->iMaxLenNum1 ); 
            } else if ( strncmp( cLine, "iMaxLenNum2", 11 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &strState->iMaxLenNum2 ); 
            } else if ( strncmp( cLine, "iLenHalve"  ,  9 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &strState->iLenHalve   ); 
            } else if ( strncmp( cLine, "iCurNumber" , 10 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &strState->iCurNumber  ); 
            } else if ( strncmp( cLine, "iFound"     ,  6 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &strState->iFound      ); 

            } else if ( strncmp( cLine, "[Current_"  ,  9 ) == 0 ) { 
               sscanf( cLine, "%9s%d"  , cDummy1, &iWalkBlok );
               // must always be 0 = first blok
               if( iWalkBlok != 0 ) {
                  fprintf( stderr, "Incorrect blok '%s' at linenr %d, file %s\n", cLine, iLineNr, filename );
                  break;
               }
               iState = 2 ; // read blok
               oWalk  = strState->arrWalk + iWalkBlok ; // current walk blok
            } else {
              fprintf( stderr, "Unknown field '%s' at linenr %d, file %s\n", cLine, iLineNr, filename );
              break ;
            }
         }
      } else if ( iState == 2 ) { //12345678901234
         if ( strncmp( cLine, "[Current_"  ,  9 ) == 0 ) { 
           sscanf( cLine, "%9s%d"  , cDummy1, &iWalkBlok );
           // check 1, 0 is bij de blobk above
           if( iWalkBlok < 1 || iWalkBlok > strState->iMaxCurNr ) {
              fprintf( stderr, "Blok nr: %d, %s\n", iWalkBlok, cDummy1 );
              fprintf( stderr, "Incorrect blok '%s' at linenr %d, file %s\n", cLine, iLineNr, filename );
              break;
           }
           oWalk  = strState->arrWalk + iWalkBlok ; // current walk blok         
         } else if ( strncmp( cLine, "iDigit"         ,  6 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iDigit         ); 
         } else if ( strncmp( cLine, "iOverflowValue" , 14 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iOverflowValue );   
         } else if ( strncmp( cLine, "iOverflowStart" , 14 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iOverflowStart );   
         } else if ( strncmp( cLine, "iOverflowPos"   , 12 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iOverflowPos   );   

         } else if ( strncmp( cLine, "iLastWalk"      ,  9 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iLastWalk      );   
            
         } else if ( strncmp( cLine, "iWalk1"         ,  6 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iWalk1         );   
         } else if ( strncmp( cLine, "iWalk2"         ,  6 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iWalk2         );   
         } else if ( strncmp( cLine, "iStart1"        ,  7 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iStart1        );   
         } else if ( strncmp( cLine, "iStart2"        ,  7 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->iStart2        );   
         } else if ( strncmp( cLine, "bCalc1"         ,  6 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->bCalc1         );   
         } else if ( strncmp( cLine, "bCalc2"         ,  6 ) == 0 ) { sscanf( cLine, "%s : %" SCNdFAST8 , cDummy1, &oWalk->bCalc2         );   
         } else if ( strncmp( cLine, "Overflow_"      ,  9 ) == 0 ) { 
            sscanf( cLine, "%9s%d : %d", cDummy1, &iOverflowblok, &iDummy );
            if ( iOverflowblok < 0 || iOverflowblok > 3 * strState->iLenNumber ) {
               fprintf( stderr, "Incorrect overflow '%s' at linenr %d, file %s\n", cLine, iLineNr, filename );
               break;
            } 
            oWalk->OverflowDigits[ iOverflowblok ] = iDummy ;
         } else if ( strncmp( cLine, "number1"      ,  7 ) == 0 ) {
            sscanf( cLine, "%s : %d", cDummy1, &iDummy );
            strState->iNumbers1[ iWalkBlok ] = iDummy ;
         } else if ( strncmp( cLine, "number2"      ,  7 ) == 0 ) { 
            sscanf( cLine, "%s : %d", cDummy1, &iDummy );
            strState->iNumbers2[ iWalkBlok ] = iDummy ;
         } else {
            fprintf( stderr, "Unknown field '%s' at linenr %d, file %s\n", cLine, iLineNr, filename );
            break ;
         }
      } else {
         fprintf( stderr, "Unknown line '%s' at linenr %d, file %s\n", cLine, iLineNr, filename );
         break;
      }
   }
   
   if ( iEnd != 1 ) {
      strDivideStateFree( strState );
      strState = NULL ; 
   } else {
      // restore last
      for ( iCnt = 1; iCnt <= strState->iCurNumber; iCnt++ ) {
         oWalk = strState->arrWalk + iCnt ;
         oWalk->strLast = &(glb_arrNoLastMulAdd[ oWalk->iDigit ][ oWalk->iOverflowValue ][ strState->iNumbers2[ 0 ] ][ strState->iNumbers1[ 0 ] ] );
      }
      
      // consistency check strState ????
   }
   
   fclose( strFile );
   
   return strState ;
}
