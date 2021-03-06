CC = g++
CFLAGS = -std=c++11
LDFLAGS =
TARGET = client
sources = ../client.cpp ../server_client_common.cpp client_tester.cpp
objects = $(sources:.cpp=.o)
dependence:=$(sources:.cpp=.d)

all: $(objects)
	$(CC) $^ $(LDFLAGS) -o $(TARGET)

include $(dependence)

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY: clean

clean:
	rm -f $(TARGET) $(objects) $(dependence)

