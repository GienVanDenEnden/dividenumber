/*
    divide_threads.c
    
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
#include <unistd.h>

#include "divide_common.h"
#include "divide_calc.h"
#include "divide_alloc.h"
#include "divide_file.h"
#include "divide_threads.h"


// display
int              glb_iDispThreadState       = 1 ; // display thread status

// thread data
         int_fast8_t       glb_iThreadMax   = -1   ; // max number of threads, -1 = auto detect
volatile int_fast8_t       glb_iThreadFree  = 0    ; // number of threads free, indicator, not 100% correct by threads, start & end threads use mutex
         struct strThread *glb_arrThreads   = NULL ; // thread data
volatile int_fast8_t       glb_iThreadStop  = 0    ; // 1 too stop all the threads

pthread_mutex_t   glb_mutex_threadstart   ; // thread start/stop 
pthread_mutex_t   glb_mutex_resultfile    ; // locking too write in the result file



//...........................................
// Thread handling
// - threadInit            : Init threads
// - threadClean           : Clean all threads
// - threadStart           : Start a new thread
// - threadEnd             : Stop a new thread
// - threadWaitLoop        : Wait loop too see if a thread is running
//
// - threadSaveState       : Save all threads and stop them
// - threadContinue        : Continue threads (read files and start threads)
//
// - thread_signal_handler : Signal handler
//...........................................
void thread_signal_handler (int signum)
{
   // save state 
   if ( signum == SIGUSR2  )
   {
      struct strThread      *thread               ;
      struct strDivideState *strState             ;
      int                    iCnt                 ;  
      pthread_t              currentThread        ;
      char                   filename[ PATH_MAX ] ;
      
      strState      = NULL          ;
      currentThread = pthread_self();
      
      // search threadnr
      for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
         thread = glb_arrThreads + iCnt ;
         // find current thread
         if ( thread->strState != NULL ) {
            if ( pthread_equal( currentThread, thread->thread )) {
               strState = thread->strState ;
               break ;
            }
         }
      }
      if ( strState != NULL ) {
         // get filename
         fileStateName( iCnt, filename );
         
         // save state
         fileStateSave( filename, strState );
         
         if ( glb_iDispThreadState == 1 ) {
            printf( "Thread %d saved\n", iCnt );
         }
         
      } else {
         fprintf( stderr, "Cannot find thread\n" );
      }
   }
   
   pthread_exit( NULL );
}


void threadInit( void ) 
{
   int_fast8_t       iCnt   ;
   struct strThread *thread ;
   
   struct sigaction new_action ;

   // setup signal handling, use SIGUSR1 too reach the threads
                  new_action.sa_handler = thread_signal_handler;
   sigemptyset ( &new_action.sa_mask );
                  new_action.sa_flags   = 0;
   
   sigaction (SIGUSR1, &new_action, NULL);
   sigaction (SIGUSR2, &new_action, NULL);
   
 
   // -1 = auto detect max threads 
   if ( glb_iThreadMax < 0 ) {
      glb_iThreadMax = sysconf(_SC_NPROCESSORS_ONLN);
   }
   if ( glb_iDispThreadState == 1 ) {
      printf( "Max number of threads: %d\n", glb_iThreadMax );
   }
 
   glb_arrThreads = malloc( glb_iThreadMax * sizeof( struct strThread )) ;
   if ( glb_arrThreads == NULL ) {
      fprintf( stderr, "Cannot alloc memory for threads\n" );
      exit(1);
   }
   glb_iThreadFree = glb_iThreadMax ;
   for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
      thread = glb_arrThreads + iCnt ;
      thread->ithreadnr = iCnt   ;
      thread->strState  = NULL   ;
   }
   
   // init mutex
   if ( pthread_mutex_init( &glb_mutex_threadstart, NULL ) != 0 ) {
      fprintf( stderr, "Cannot create mutex threadstart\n" );
      exit(1);
   }
   if ( pthread_mutex_init( &glb_mutex_resultfile, NULL ) != 0 ) {
      fprintf( stderr, "Cannot create mutex glb_mutex_resultfile\n" );
      exit(1);
   }
   
}

void threadClean ( void )
{
   int_fast8_t       iCnt   ;
   struct strThread *thread ;
   int_fast8_t       iFound ;
   int               iSignal;

   if ( glb_iDispThreadState == 1 ) {
      printf( "Stop all running threads\n" );
   }
   
   pthread_mutex_lock( &glb_mutex_threadstart );

   glb_iThreadFree = -100 ; // no free thread anymore 
   
   // stop all the threads and clean up the memory
   iSignal = SIGUSR1 ; // send signal only 1 time
   do {
      iFound = 0 ;
      for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
         thread = glb_arrThreads + iCnt ;
         // kill running thread
         if ( thread->strState != NULL ) {
            if ( pthread_kill( thread->thread, iSignal ) != 0 ) {
               strDivideStateFree( thread->strState );
               thread->strState = NULL ;
            } else {
               iFound = 1 ;
            }
         }
      }
      iSignal = 0 ; // used to wait for thread too stop
   } while( iFound == 1 ) ;
   

   // unlock threadstart   
   pthread_mutex_unlock( &glb_mutex_threadstart );
   
   // clean mutex
   pthread_mutex_destroy( &glb_mutex_threadstart );
   pthread_mutex_destroy( &glb_mutex_resultfile  );
}


// search for the free thread and start it, -1 = not started, otherwise thread number
int_fast8_t threadStart( struct strDivideState *strState )
{
   int_fast8_t iCnt    ;
   int_fast8_t iResult ;
   
   struct strThread *thread ;
   
   if ( glb_iThreadFree <= 0 ) {
      return -1 ;
   }
   
   // lock threadstart 
   pthread_mutex_lock( &glb_mutex_threadstart );
   
   // start new thread
   iResult = -1; 
   for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
      thread = glb_arrThreads + iCnt ;
      // kill running thread
      if ( thread->strState == NULL ) {
         glb_iThreadFree-- ;
         thread->strState = strState ;
         
         pthread_create( &thread->thread, NULL, &numberThread, (void *)strState ); 
         
         iResult = thread->ithreadnr ; 
         break ;
      }
   }
   if ( glb_iDispThreadState == 1 && iResult >= 0 ) {
      printf( "Start thread: %d, free :%d, iCurNumber: %d, max: %d, min: %d\n", iResult, glb_iThreadFree, strState->iCurNumber, strState->iMaxCurNr, strState->iMinCurNr );
   }
   
   // unlock threadstart   
   pthread_mutex_unlock( &glb_mutex_threadstart );
   
   return iResult ;
}

void threadEnd( struct strDivideState *strState )
{
   // make thread free
   int_fast8_t       iCnt   ;
   struct strThread *thread ;
   int_fast8_t       iFound = 0 ;
   
   // signal that termination is busy, do nothing
   if ( glb_iThreadFree < 0 ) {
      return ;
   }
   
   // lock threadstart 
   pthread_mutex_lock( &glb_mutex_threadstart );

   for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
      thread = glb_arrThreads + iCnt ;
      if ( thread->strState == strState ) {
         
         if ( glb_iDispThreadState == 1 ) {
            printf( "End thread %d, free: %d, iMinCurNr: %d, iCurNumber: %d\n", iCnt, glb_iThreadFree + 1, strState->iMinCurNr, strState->iCurNumber );
         }
         strDivideStateFree( thread->strState );
         thread->strState = NULL ;
         glb_iThreadFree++ ;
         iFound = 1 ;
         break ;
      }
   }
   if ( iFound == 0 ) {
      fprintf( stderr, "End thread, not found, iCurNumber: %d\n", strState->iCurNumber );
   }

   // unlock threadstart   
   pthread_mutex_unlock( &glb_mutex_threadstart );
}

void threadSaveState( void )
{
   char              filename[ PATH_MAX ] ;
   int               iCnt                 ;
   int               iFound               ;
   struct strThread *thread               ;
   
   // lock threadstart 
   pthread_mutex_lock( &glb_mutex_threadstart );
   
   printf( "\n" );
   printf( "Start save state\n" );
   

   // send save signal
   for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
      thread = glb_arrThreads + iCnt ;
      if ( thread->strState != NULL ) {
         if ( pthread_kill( thread->thread, SIGUSR2 ) == 0 ) {
             // do nothing
         } else {
            // can theoretical not ...
            // remove existing file state for this thread
            fileStateName( iCnt, filename );
            remove( filename );
         }
      } else {
         // remove existing file state for this thread
         fileStateName( iCnt, filename );
         remove( filename );
      }
   }
   
   // wait for all the threads too stop
   do {
      iFound = 0 ;

      for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
         thread = glb_arrThreads + iCnt ;
         if ( thread->strState != NULL ) {
            if ( pthread_kill( thread->thread, 0 ) == 0 ) {
               iFound++ ;
            }
         }
      }
   } while( iFound != 0 ) ;
   
   
   // unlock 
   printf( "End save state\n" );

   // unlock threadstart   
   pthread_mutex_unlock( &glb_mutex_threadstart );
}



// check if a thread is running, if no thread is running then leave
void threadWaitLoop( void )
{
   int               iCnt       ;
   int               iFound     ;
   int               iDef       ;
   int               iKey       ;
   int               iSaveState ;
   struct strThread *thread     ;
   
   if ( glb_iDispThreadState == 1 ) {
      printf( "Start threadWaitLoop, S = save state and stop program\n" );
   }
   
   // check every second
   iSaveState = 0 ;
   while( 1 ) {
      sleep( 1 );
      iFound = 0 ;
      iDef   = 0 ;
      
      
      for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
         thread = glb_arrThreads + iCnt ;
         
         if ( thread->strState != NULL ) {
            iDef++ ;
            if ( pthread_kill( thread->thread, 0 ) == 0 ) {
//            if ( pthread_kill( thread->thread, SIGTERM ) == 0 ) {
               
//               printf( "Thread %d used\n", iCnt ) ;
               iFound++ ;
            }
         }
      }
 
      if ( iFound == 0 ) {
         break;
      }
      if ( glb_iDispThreadState == 1 ) {
         printf( "Running threads %d, defined %d, max %d\n", iFound, iDef, glb_iThreadMax );
      }
      if ( glb_iThreadStop == 1 ) {
         break ;
      }
      iKey = getkey() ;
      if ( iKey == 83 || iKey == 115 ) { // S of s 
         iSaveState = 1 ;
         break ;
      }
      
   }

   // save all threads
   if ( iSaveState == 1 ) {
      threadSaveState();
   }
   
   if ( glb_iDispThreadState == 1 ) {
      printf( "\n" );
      printf( "End threadWaitLoop\n" );
   }
}


void threadContinue( void ) 
{
   int               iFound = 0 ; // found thread files
   int               iCnt       ;
   char              filename[ PATH_MAX ] ;
   struct strThread *thread     ;

   printf( "Start reading state files\n" );
   
   // lock threadstart 
   pthread_mutex_lock( &glb_mutex_threadstart );

   // load state files   
   for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
      thread = glb_arrThreads + iCnt ;
      
      fileStateName( iCnt, filename );
      if( access( filename, F_OK ) != -1 ) {
         // file exists. load it
         thread->strState = fileStateLoad( filename );
      }      
   }
   
   // start threads
   for ( iCnt = 0; iCnt < glb_iThreadMax; iCnt++ ) {
      thread = glb_arrThreads + iCnt ;
      
      if ( thread->strState != NULL ) {
         printf( "Start thread %d\n", iCnt );
         iFound++ ;
         pthread_create( &thread->thread, NULL, &numberThread, (void *)thread->strState ); 
      }
   }

   // unlock threadstart   
   pthread_mutex_unlock( &glb_mutex_threadstart );

   printf( "End reading state files\n" );
   
   
   if ( iFound == 0 ) {
      fprintf( stderr, "Cannot find state files too continue\n" );
      exit(1);
   } 
}
