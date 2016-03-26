/*
    divide_threads.h
    
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

// the hold the thread data
struct strThread {
   pthread_t              thread    ; // thread data
   int_fast8_t            ithreadnr ; // thread number in array
   struct strDivideState *strState  ; // state data
};


// display
         int               glb_iDispThreadState  ; // display thread status

// thread data
         int_fast8_t       glb_iThreadMax        ; // max number of threads, -1 = auto detect
volatile int_fast8_t       glb_iThreadFree       ; // number of threads free, indicator, not 100% correct by threads, start & end threads use mutex
         struct strThread *glb_arrThreads        ; // thread data
volatile int_fast8_t       glb_iThreadStop       ; // 1 too stop all the threads

         pthread_mutex_t   glb_mutex_threadstart ; // thread start/stop 
         pthread_mutex_t   glb_mutex_resultfile  ; // locking too write in the result file


// prototypes
void        threadInit(   void ) ;                           // Init threads
void        threadClean ( void );                            // Clean all threads
int_fast8_t threadStart( struct strDivideState *strState );  // Start a new thread
void        threadEnd(   struct strDivideState *strState );  // Stop a new thread

void        threadSaveState( void );                         // Wait loop too see if a thread is running

void        threadWaitLoop(  void );                         // Save all threads and stop them
void        threadContinue(  void );                         // Continue threads (read files and start threads)

