#ifndef INCLUDE_HOME_AUTOMATION_SYSTEM_HPP
#define INCLUDE_HOME_AUTOMATION_SYSTEM_HPP

#include <string>
#include <map>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include "v8.h"
#include "iremocon.hpp"
#include "julius.hpp"
#include "text_to_speech.hpp"
// #include "oll.hpp"

/**
 *  Home Automation System 本体
 */
class HomeAutomationSystem
{
private:
	//! コマンドを格納する変数
	std::map<std::string, int> str_ir_map_;

	//! キャンセルかどうか待機中に保持しておくための前回喋ったコマンド
	std::string pre_str_;

	/**
	 * 認識させる文章と対応して送信する IR の番号を書いた
	 * XML を読み込んでマップに格納。
	 * @param[in] cmd	コマンド（e.g. テレビを消して）
	 * @return			ファイルの読み込みの成否
	 */
	bool learn_commands_from_xml(const std::string& file_name);

	/**
	 * 文章を引数にとって予め登録した言葉に合致するか調べる。
	 * 合致した場合は送出する IR 信号の番号を返す
	 * @param[in] num	解釈させる文章
	 * @return			発信する学習した IR 信号番号
	 */
	boost::optional<int> interpret(const std::string& sentence) const;

	/**
	 * v8 を初期化する
	 */
	void init_v8();

public:
	/**
	 * Constructor
	 */
	HomeAutomationSystem();

	/**
	 * Destructor
	 */
	~HomeAutomationSystem();

	/**
	 * JavaScript を実行する
	 */
	void exec_script(const std::string& script_path);
};

#endif // INCLUDE_HOME_AUTOMATION_SYSTEM_HPP
