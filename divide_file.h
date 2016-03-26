/*
    divide_file.h
    
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


extern char glb_cFileState[  PATH_MAX ] ; // file name too store state, for each thread 1 file
extern char glb_cFileResult[ PATH_MAX ] ; // all the found numbers will be stored here


int                     fileStateSave( char       *filename, struct strDivideState *strState ); // Save state 
struct strDivideState * fileStateLoad( char       *filename )                                 ; // Load state
void                    fileStateName( int_fast8_t iThreadNr, char *filename )                ; // Give filename for given threadnr           
