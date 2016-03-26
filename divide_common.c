/*
    divide_common.c
    
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
#include <termios.h>

#include "divide_common.h"

int_fast8_t glb_iDigitsMax = 30 ; // max number of digits allowed


//........................................
// Common functions:
//
// - numberCheck : Check of given string only contain digits
// - getkey      : check keyboard (nowait)
//........................................


//
// check if string only contains numbers
//
int_fast8_t numberCheck( char *cNumber ) {
   int_fast8_t iCnt ;
   int_fast8_t iLen ;
   
   if ( cNumber == NULL ) {
      fprintf( stderr, "No number given\n" );
      return 1 ;
   }
   iCnt = 0 ;
   while ( *cNumber != '\0' ) {
      iCnt++ ;
      if ( *cNumber < '0' || *cNumber > '9' ) {
         fprintf( stderr, "%c on position %d is not a digit\n", *cNumber, iCnt );
         return 1 ;
      }
      
      cNumber++ ;
      if ( iCnt > glb_iDigitsMax )  {
         iLen = strlen( cNumber ) + iCnt ;
         fprintf( stderr, "Number too big, maximum is %d digits, given is %d\n", glb_iDigitsMax, iLen );
         return 1;
      }
   }
   return 0 ;
}

// http://stackoverflow.com/questions/2984307/c-key-pressed-in-linux-console
int getkey( void ) {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}
