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

all: $(TARGET)


$(TARGET) : $(OBJS)
	$(CC) $(LIBS) $(OBJS) -o $(TARGET)
	
%.o : %.c $(HEADS)
	$(CC) $(CFLAGS) -c $< -o $@

	
clean: 
	rm $(OBJS) $(TARGET)

	