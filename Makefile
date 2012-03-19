LIBJULIUS=./julius/julius-4.2.1/libjulius
LIBSENT  =./julius/julius-4.2.1/libsent

IJULIUS  = -I$(LIBJULIUS)/include -I$(LIBSENT)/include  `$(LIBSENT)/libsent-config --cflags` `$(LIBJULIUS)/libjulius-config --cflags`
LJULIUS  = -L$(LIBJULIUS) `$(LIBJULIUS)/libjulius-config --libs` -L$(LIBSENT) `$(LIBSENT)/libsent-config --libs`

CXX      = g++-4.6
CXXFLAGS = -g -O2 -Wall -std=c++0x $(IJULIUS)
LDFLAGS  = -lboost_thread -lboost_system -lboost_regex -lalut $(LJULIUS)

SOURCES  = $(shell ls *.cpp)
OBJECTS  = $(SOURCES:.cpp=.o)
TARGET   = has

############################################################

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

clean:
	$(RM) *.o *.bak *~ core TAGS

distclean:
	$(RM) *.o *.bak *~ core TAGS
	$(RM) $(TARGET)
