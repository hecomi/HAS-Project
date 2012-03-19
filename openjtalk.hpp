#ifndef INCLUDE_OPENJTALK_HPP
#define INCLUDE_OPENJTALK_HPP

#include <string>

/**
 * Open JTalkでTTSするクラス
 */
class OpenJTalk
{
public:
	/**
	 * OpenJTalkに渡すパラメータを生成
	 * @param [in] voice_dir	音素などが入ったディレクトリ
	 */
	explicit OpenJTalk(const std::string& voice_dir);

	/**
	 * 引数の言葉を喋らせる
	 * @param [in] str	喋らせる文章
	 */
	void talk(const std::string& str);

private:
	//! 音素などが入ったディレクトリ
	std::string voice_dir_;

	//! OpenJTalkに渡すパラメータ
	std::string param2openjtalk_;
};

#endif // INCLUDE_OPENJTALK_HPP

