LIBJULIUS=./julius/julius-4.2.1/libjulius
LIBSENT  =./julius/julius-4.2.1/libsent

IJULIUS  = -I$(LIBJULIUS)/include -I$(LIBSENT)/include  `$(LIBSENT)/libsent-config --cflags` `$(LIBJULIUS)/libjulius-config --cflags`
LJULIUS  = -L$(LIBJULIUS) `$(LIBJULIUS)/libjulius-config --libs` -L$(LIBSENT) `$(LIBSENT)/libsent-config --libs`

CXX      = g++-4.6
CXXFLAGS = -g -O2 -Wall -std=c++0x
LDFLAGS  = -lboost_thread -lboost_system -lboost_regex -lalut

SOURCES  = $(shell ls *.cpp) oll/oll.cpp
OBJECTS  = $(SOURCES:.cpp=.o)
TARGET   = has

CLEAN    = $(RM) *.o *.bak *~ core TAGS

############################################################

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LJULIUS)

julius.o home_automation_system.o:
	$(CXX) $(CXXFLAGS) -c $(@:.o=.cpp) $(IJULIUS)

clean:
	$(CLEAN)
	$(RM) $(OBJECTS)

distclean:
	$(CLEAN)
	$(RM) $(TARGET)
