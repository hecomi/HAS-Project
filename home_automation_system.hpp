#ifndef INCLUDE_HOME_AUTOMATION_SYSTEM_HPP
#define INCLUDE_HOME_AUTOMATION_SYSTEM_HPP

#include <string>
#include <map>
#include <boost/optional.hpp>
#include "iremocon.hpp"
#include "julius.hpp"
#include "openjtalk.hpp"

class HomeAutomationSystem
{
private:
	//! コマンドを格納する変数
	std::map<std::string, int> str_ir_map_;

	//! iRemocon へ接続するクライアント
	iRemocon iremocon_;

	//! Julius インスタンス
	Julius julius_;

	//! 音声会話を制御するクラス
	OpenJTalk open_jtalk_;

	/**
	 * 文章を引数にとって予め登録した言葉に合致するか調べる。
	 * 合致した場合は送出する IR 信号の番号を返す
	 * @param [in] num	解釈させる文章
	 * @return			発信する学習した IR 信号番号
	 */
	boost::optional<int> interpret(const std::string& sentence) const;

	/**
	 * Julius のコールバック関数を追加する
	 */
	void add_julius_callback();

public:
	/**
	 * Constructor
	 * @param [in] iremocon	iRemocon インスタンス
	 * @param [in] julius	Julius インスタンス
	 * @param [in] open_jtalk	発話エンジン（OpenJTalk or GoogleTTS）
	 */
	HomeAutomationSystem(const iRemocon& iremocon, const Julius& julius, const OpenJTalk& open_jtalk);

	/**
	 * 認識させる文章と対応して送信する IR の番号を書いた
	 * XML を読み込んでマップに格納。
	 * @param [in] cmd	コマンド（e.g. テレビを消して）
	 * @return			ファイルの読み込みの成否
	 */
	bool learn_commands_from_xml(const std::string& file_name);

	/**
	 * プロセス開始
	 */
	void start();

	/**
	 * 発話エンジンを用いて発話する
	 */
	void talk(const std::string& str);
};

#endif // INCLUDE_HOME_AUTOMATION_SYSTEM_HPP
