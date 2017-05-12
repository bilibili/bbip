#IDIR=/usr/include/mysql
#LDIR=/usr/local/lib/mysql
#LDFLAGS=-O3 -D_FILE_OFFSET_BITS=64 -lpthread -lmysqlclient
IDIR=/usr/include/mysql
LDIR=/usr/local/lib
OBJS=main.cpp
CC=g++
CFLAGS=-c -g -Wall -march=i686
LDFLAGS=-O3 -D_FILE_OFFSET_BITS=64 -lpthread -lmysqlclient
TARGET=bbip_export

all: $(TARGET)
	@echo done.
	
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -L$(LDIR) -I$(IDIR) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) -o $@ $<
	
.cpp.o:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm *.o -f
	rm $(TARGET) -f
	
