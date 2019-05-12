CXX = g++

CXXFLAGS = $(shell wx-config --cxxflags)  
LDFLAGS = --static -s
LDLIBS = $(shell wx-config --libs)
REZFLAGS = $(shell wx-config --rez-flags)

SRC = $(wildcard *.cc)
OBJ = $(SRC:.cc=.o)
PROGNAME = test.exe

all: $(PROGNAME)

$(PROGNAME): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ)  $(LDLIBS)

$.o: %.c
	$(CXX) -o $@ -mc $^ $(CXXFLAGS)

clean:
	rm -f $(OBJ)
	rm -f $(PROGNAME)
