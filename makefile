#
#    makefile
#    
#    Copyright (C) 2016 Gien van den Enden - gien.van.den.enden@gmail.com
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
# 

TARGET = dividenumber

SRCS   = dividenumber.c   \
         divide_tables.c  \
         divide_common.c  \
         divide_calc.c    \
         divide_alloc.c   \
         divide_file.c    \
         divide_threads.c
         
HEADS  = divide_tables.h  \
         divide_common.h  \
         divide_calc.h    \
         divide_alloc.h   \
         divide_file.h    \
         divide_threads.h 
         
CC     = gcc
CFLAGS = -Ofast -Wall -Wextra -pthread
LIBS   = -lpthread

OBJS   = $(SRCS:.c=.o)

all    : $(TARGET)


$(TARGET) : $(OBJS)
	$(CC) $(LIBS) $(OBJS) -o $(TARGET)

	
%.o : %.c $(HEADS)
	$(CC) $(CFLAGS) -c $< -o $@

	
clean: 
	rm $(OBJS) $(TARGET)

	