#include <iostream>
#include "tts.h"

int main(int argc, char const* argv[])
{
	char
		*word = "これはテストです",
		*name = "test.wav",
		*dic  = "open_jtalk_dic_utf_8-1.05",
		*voice = "mei_normal";

	OpenJTalk_TTS(word, name, dic, voice);

	return 0;
}
