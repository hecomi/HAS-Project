#ifndef INCLUDE_OPENJTALK_HPP
#define INCLUDE_OPENJTALK_HPP

#include <string>

int OpenJTalk_TTS(const std::string& sentence, const std::string& wav_filename, const std::string& dic_dir, const std::string& voice_dir);

#endif // INCLUDE_OPENJTALK_HPP
