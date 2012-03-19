#include <sstream>
#include <cstdlib>
#include <boost/format.hpp>
#include <AL/alut.h>

#include "openjtalk.hpp"

OpenJTalk::OpenJTalk(const std::string& voice_dir)
: voice_dir_(voice_dir)
{
	param2openjtalk_ =
		"echo %1% | open_jtalk "
		"-td %2%/tree-dur.inf "
		"-tf %2%/tree-lf0.inf "
		"-tm %2%/tree-mgc.inf "
		"-md %2%/dur.pdf "
		"-mf %2%/lf0.pdf "
		"-mm %2%/mgc.pdf "
		"-df %2%/lf0.win1 "
		"-df %2%/lf0.win2 "
		"-df %2%/lf0.win3 "
		"-dm %2%/mgc.win1 "
		"-dm %2%/mgc.win2 "
		"-dm %2%/mgc.win3 "
		"-ef %2%/tree-gv-lf0.inf "
		"-em %2%/tree-gv-mgc.inf "
		"-cf %2%/gv-lf0.pdf "
		"-cm %2%/gv-mgc.pdf "
		"-k  %2%/gv-switch.inf "
		"-x /usr/lib/open_jtalk/dic/utf-8 "
		"-s 48000 "
		"-p 240 "
		"-a 0.5 "
		"-u 0.5 "
		"-jm 1.0 "
		"-jf 1.0 "
		"-jl 1.0 "
		"-z 48000 "
		"-s 48000 "
		"-b 0.8 "
		"-ow %3%";
}

void OpenJTalk::talk(const std::string& str)
{
	// 出力 wav ファイル名
	const std::string output_wavfile_name = "__tmp.wav";

	// wav を生成
	std::stringstream ss;
	ss << boost::format(param2openjtalk_)
		% str
		% voice_dir_
		% output_wavfile_name;
	;
	std::cout << ss.str() << std::endl;
	system(ss.str().c_str());

	// alutの初期化
	int alut_argc = 0;
	char* alut_argv[] = {};
	alutInit(&alut_argc, alut_argv);

	// ソースの用意
	ALuint buf, src;
	ALenum state;
	buf = alutCreateBufferFromFile(output_wavfile_name.c_str());
	alGenSources(1, &src);
	alSourcei(src, AL_BUFFER, buf);

	// 再生
	alSourcePlay(src);
	alGetSourcei(src, AL_SOURCE_STATE, &state);
	while (state == AL_PLAYING) {
		alGetSourcei(src, AL_SOURCE_STATE, &state);
	}

	// 後片付け
	alDeleteSources(1, &src);
	alDeleteBuffers(1, &buf);
	alutExit();
	system(("rm " + output_wavfile_name).c_str());
}

// int main()
// {
// 	OpenJTalk ojt("mei_normal");
// 	ojt.talk("「電気をつけて」を実行します");
// 	return 0;
// }
