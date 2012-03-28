CXX      = g++
CXXFLAGS = -g -c -O2 -Wall -DHAVE_CONFIG_H -I./open_jtalk-1.05/ -I./open_jtalk-1.05/mecab -I ./open_jtalk-1.05/text2mecab -I ./open_jtalk-1.05/mecab/src -I ./open_jtalk-1.05/mecab2njd -I ./open_jtalk-1.05/njd -I ./open_jtalk-1.05/njd_set_pronunciation -I ./open_jtalk-1.05/njd_set_digit -I ./open_jtalk-1.05/njd_set_accent_phrase -I ./open_jtalk-1.05/njd_set_accent_type -I ./open_jtalk-1.05/njd_set_unvoiced_vowel -I ./open_jtalk-1.05/njd_set_long_vowel -I ./open_jtalk-1.05/njd2jpcommon -I ./open_jtalk-1.05/jpcommon -I /home/hecomi/Program/cpp/HAS/openjtalk/hts_engine_API-1.06/include -finput-charset=UTF-8 -fexec-charset=UTF-8 -MT open_jtalk.o -MD -MP -MF ./open_jtalk-1.05/bin/.deps/open_jtalk.Tpo
CLEAN    = $(RM) *.o *.bak *~ core TAGS

all: tts.o
	g++ tts.o -o test ./open_jtalk-1.05/text2mecab/libtext2mecab.a ./open_jtalk-1.05/mecab/src/libmecab.a ./open_jtalk-1.05/mecab2njd/libmecab2njd.a ./open_jtalk-1.05/njd/libnjd.a ./open_jtalk-1.05/njd_set_pronunciation/libnjd_set_pronunciation.a ./open_jtalk-1.05/njd_set_digit/libnjd_set_digit.a ./open_jtalk-1.05/njd_set_accent_phrase/libnjd_set_accent_phrase.a ./open_jtalk-1.05/njd_set_accent_type/libnjd_set_accent_type.a ./open_jtalk-1.05/njd_set_unvoiced_vowel/libnjd_set_unvoiced_vowel.a ./open_jtalk-1.05/njd_set_long_vowel/libnjd_set_long_vowel.a ./open_jtalk-1.05/njd2jpcommon/libnjd2jpcommon.a ./open_jtalk-1.05/jpcommon/libjpcommon.a ./hts_engine_API-1.06/lib/libHTSEngine.a
clean:
	$(CLEAN)
