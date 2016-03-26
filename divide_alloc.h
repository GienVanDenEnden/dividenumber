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

void                    strDivideStateFree(  struct strDivideState *strState ); // Alloc structure and initialize is
struct strDivideState * strDivideStateAlloc( char *cNumber                   ); // Free structure and all its subcomponents
struct strDivideState * strDivideStateClone( struct strDivideState *strOrg   ); // Clone structure

