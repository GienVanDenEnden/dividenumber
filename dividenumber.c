/*
    dividenumber - Divide 1 number into 2 numbers
    
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
 
    8 CPU's (--findall)
    14 digits =     1s (real   0m01)
    15 digits =     9s (real   0m09)
    16 digits =     3s (real   0m09)
    17 digits =    25s (real   1m36)
    18 digits =    20s (real   2m09)
    19 digits =  3m23  (real  22m50)
    20 digits =  3m08  (real  23m50)
    21 digits = 33m35  (real 257m28)
    
    This program use brute force methode with optimisation.
    
*/  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <linux/limits.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "divide_tables.h"
#include "divide_common.h"
#include "divide_calc.h"
#include "divide_alloc.h"
#include "divide_file.h"
#include "divide_threads.h"


// version info
static char glb_cVersion[] = "0.0.1"     ; // current version
// static char cVerDate[] = "2016-03-07"; // yyyy-mm-dd



// init all global default in 1 place
void initDefaults( void )
{
   // divide_common.c 
   glb_iDigitsMax         = 30   ; // max number of digits allowed

   // divide_calc.c
   glb_iNumberFindAll     =  0   ; // if 1 then find all the numbers, otherwise stop by the first found
   glb_iDigitsCalcCPU     = 12   ; // number of digits 1 (one) CPU can process in (estimate) 1 second, use to start threads

   // divicde_file.c
   strcpy( glb_cFileState  , "out_state_%" SCNdFAST8 ".txt"  ); // file name too store state, for each thread 1 file
   strcpy( glb_cFileResult , "out_result.txt"                ); // all the found numbers will be stored here

   // divide_threads.c
   glb_iDispThreadState   =  1   ; // display thread status
   glb_iThreadMax         = -1   ; // max number of threads, -1 = auto detect
   glb_iThreadFree        =  0   ; // number of threads free, indicator, not 100% correct by threads, start & end threads use mutex
   glb_arrThreads         = NULL ; // thread data
   glb_iThreadStop        =  0   ; // 1 too stop all the threads
   
   // divide_tables.c 
   numberArrayFill(); // fill the number arrays
}

//...........................................
// Commandline opties
// - printHelp
// - printVersion
//...........................................

// print help information
void printHelp()
{
   printf ( "\nUsage:\n dividenumber [options] [number]\n\n" );

   printf ( "Options:\n" );
   printf ( " --help                Show help and exit\n" );
   printf ( " --version             Show version and exit\n" );
   printf ( " --continue            Continue with the last saved state\n" );
   printf ( " --nothreaddisplay     No thread state display\n" );
   printf ( " --findall             Find all the numbers, otherwise the first valid numbers\n" );
   printf ( " --nothreads           Do not use threads\n" );
   printf ( " --maxthreads <number> Set the maximum number of threads\n" );
   printf ( "\n" );
}


// print version information and exit
void printVersion()
{
   printf( "\n" );
   printf( "dividenumber\nversion: %s\n", glb_cVersion );
   printf( "\n" );
  
   printf( "Copyright (C) 2016 Gien van den Enden\n");
   printf( "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
   printf( "This is free software: you are free to change and redistribute it.\n");
   printf( "There is NO WARRANTY, to the extent permitted by law.\n");
   printf( "\n" );

   exit(0);
}


//........................................
// Main : Program start point
//........................................

int main( int argc, char *argv[] ) {
   
   char *cNumber   = NULL ;
   int   iCntArgv         ;
   int   iContinue = 0    ;
   
   // init all global defaults
   initDefaults();
   
   // process the commandline parameters
   for( iCntArgv = 1; iCntArgv < argc; iCntArgv++ ) {
     if (        strcmp( argv[ iCntArgv ], "--version"  ) == 0 ) {
         printVersion(); // this function exit program
         
     } else if ( strcmp( argv[ iCntArgv ], "--help" ) == 0 ) {
         printHelp(); // display help 
         exit(0);

     } else if ( strcmp( argv[ iCntArgv ], "--nothreaddisplay" ) == 0 ) {
         glb_iDispThreadState = 0 ;

     } else if ( strcmp( argv[ iCntArgv ], "--findall" ) == 0 ) {
         glb_iNumberFindAll  = 1 ;
 
     } else if ( strcmp( argv[ iCntArgv ], "--nothreads" ) == 0 ) {
         glb_iThreadMax = 0 ;

     } else if ( strcmp( argv[ iCntArgv ], "--maxthreads" ) == 0 ) {
         if ( iCntArgv < argc ) {
            iCntArgv++ ;
            glb_iThreadMax = atoi( argv[ iCntArgv ] ) ;
         } else {
            printf( "Number of threads is missing bij --maxthreads\n" );
            exit( 1 ) ;
         }
         
         
     } else if ( strcmp( argv[ iCntArgv ], "--printnumberarray" ) == 0 ) {
         // print number array, 
         numberArrayPrint();
         exit(0);

         
     } else if ( strcmp( argv[ iCntArgv ], "--continue" ) == 0 ) {
        iContinue = 1; 
        
     } else {
        if ( strncmp( argv[ iCntArgv ], "--", 2 ) == 0 ) {
           fprintf( stderr, "Unknown option: %s\n", argv[ iCntArgv ] );
           exit(1);
        }
        
        if ( cNumber != NULL ) {
           fprintf( stderr, "Error, more then 1 number given: %s\n", argv[ iCntArgv ] ); 
           exit(1);
        }
        cNumber = argv[ iCntArgv ] ;
     }
   }
   
   // no argument given, then print help and exit
   if ( cNumber == NULL && iContinue == 0 ) {
      printHelp();
      exit(0);
   }

   // init thread handling
   threadInit();

   
   // process data
   if ( cNumber != NULL ) {
      // check of it is a number
      if ( numberCheck( cNumber ) != 0 ) {
         threadClean(); 
         return 2 ;
      }
      
      // divide numer
      numberDivide( cNumber );
   }
   if ( iContinue == 1 ) {
      // read files and continue threads
      threadContinue();
   }

   // main wait loop for the threads
   if ( glb_iThreadMax > 0 ) {
      threadWaitLoop();
   }
 
   // clean thread data
   threadClean(); 
   
   if ( glb_iDispThreadState == 1 ) {
      printf( "The result(s) are set in file: %s\n", glb_cFileResult );
      printf( "Program end\n" );
   }
   
   // the end
   return 0;
}