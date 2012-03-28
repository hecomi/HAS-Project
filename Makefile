LIBJULIUS=./julius/julius-4.2.1/libjulius
LIBSENT  =./julius/julius-4.2.1/libsent

IJULIUS  = -I$(LIBJULIUS)/include -I$(LIBSENT)/include  `$(LIBSENT)/libsent-config --cflags` `$(LIBJULIUS)/libjulius-config --cflags`
LJULIUS  = -L$(LIBJULIUS) `$(LIBJULIUS)/libjulius-config --libs` -L$(LIBSENT) `$(LIBSENT)/libsent-config --libs`

OPENJTALK_DIR = openjtalk
LOPENJTALK = ./openjtalk/open_jtalk-1.05/text2mecab/libtext2mecab.a ./openjtalk/open_jtalk-1.05/mecab/src/libmecab.a ./openjtalk/open_jtalk-1.05/mecab2njd/libmecab2njd.a ./openjtalk/open_jtalk-1.05/njd/libnjd.a ./openjtalk/open_jtalk-1.05/njd_set_pronunciation/libnjd_set_pronunciation.a ./openjtalk/open_jtalk-1.05/njd_set_digit/libnjd_set_digit.a ./openjtalk/open_jtalk-1.05/njd_set_accent_phrase/libnjd_set_accent_phrase.a ./openjtalk/open_jtalk-1.05/njd_set_accent_type/libnjd_set_accent_type.a ./openjtalk/open_jtalk-1.05/njd_set_unvoiced_vowel/libnjd_set_unvoiced_vowel.a ./openjtalk/open_jtalk-1.05/njd_set_long_vowel/libnjd_set_long_vowel.a ./openjtalk/open_jtalk-1.05/njd2jpcommon/libnjd2jpcommon.a ./openjtalk/open_jtalk-1.05/jpcommon/libjpcommon.a /home/hecomi/Program/cpp/HAS/openjtalk/hts_engine_API-1.06/lib/libHTSEngine.a
IOPENJTALK = -DHAVE_CONFIG_H -I./openjtalk/open_jtalk-1.05/ -I./openjtalk/open_jtalk-1.05/mecab -I./openjtalk/open_jtalk-1.05/text2mecab -I./openjtalk/open_jtalk-1.05/mecab/src -I./openjtalk/open_jtalk-1.05/mecab2njd -I./openjtalk/open_jtalk-1.05/njd -I./openjtalk/open_jtalk-1.05/njd_set_pronunciation -I./openjtalk/open_jtalk-1.05/njd_set_digit -I./openjtalk/open_jtalk-1.05/njd_set_accent_phrase -I./openjtalk/open_jtalk-1.05/njd_set_accent_type -I./openjtalk/open_jtalk-1.05/njd_set_unvoiced_vowel -I./openjtalk/open_jtalk-1.05/njd_set_long_vowel -I./openjtalk/open_jtalk-1.05/njd2jpcommon -I./openjtalk/open_jtalk-1.05/jpcommon -I./openjtalkhome/hecomi/Program/cpp/HAS/openjtalk/hts_engine_API-1.06/include -finput-charset=UTF-8 -fexec-charset=UTF-8 -MT open_jtalk.o -MD -MP -MF ./openjtalk/open_jtalk-1.05/bin/.deps/open_jtalk.Tpo

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
	cd $(OPENJTALK_DIR) && $(MAKE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LJULIUS) $(LOPENJTALK)

test.o home_automation_system.o:
	$(CXX) $(CXXFLAGS) -c $(@:.o=.cpp) $(IJULIUS) $(IOPENJTALK)

julius.o:
	$(CXX) $(CXXFLAGS) -c $(@:.o=.cpp) $(IJULIUS)

text_to_speech.o:
	$(CXX) $(CXXFLAGS) -c $(@:.o=.cpp) $(IOPENJTALK)

clean:
	$(CLEAN)

distclean:
	$(CLEAN)
	cd $(OPENJTALK_DIR) && $(MAKE) clean
	$(RM) $(TARGET)
