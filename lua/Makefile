INCLUDE = -I/usr/local/include -I/usr/include/lua5.1 -I/usr/include/luajit-2.0/ -I../
LIBOPTS = -shared
FLAGS = -fpic
CXX = g++
CC = gcc
CFLAGS=-c -g -Wall -O0 -gstabs+

%.a: %.c $(DEPS)
	$(CC) -static -c -fPIC -o $@ $< $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -fPIC -o $@ $< $(CFLAGS)

bbip.so: lua_bbip.cpp ../libbbip.c
	$(CXX) $^ $(INCLUDE) $(FLAGS) $(LIBOPTS) -o $@

install: bbip.so
	cp bbip.so /usr/local/lib/lua/5.1/
