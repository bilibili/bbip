GEN_SRC := bbip.cpp bbip_constants.cpp bbip_types.cpp ../../libbbip.c
GEN_OBJ := $(patsubst %.cpp,%.o, $(GEN_SRC))

THRIFT_DIR := /usr/local/include/thrift
BOOST_DIR := /usr/local/include
BBIP_DIR := ../../

INC := -I$(THRIFT_DIR) -I$(BOOST_DIR) -I$(BBIP_DIR)

.PHONY: all clean

all: bbip_server

%.o: %.cpp
	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@

#  -lthriftnb -levent
bbip_server: bbip_server.o $(GEN_OBJ)
	$(CXX) $^ -o $@ -L/usr/local/lib -lthrift -lpthread

clean:
	$(RM) *.o bbip_server

