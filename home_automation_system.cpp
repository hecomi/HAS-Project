#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/make_shared.hpp>
#include <vector>
#include "home_automation_system.hpp"
#include "v8.hpp"

/* ------------------------------------------------------------------------- */
//  JavaScript
/* ------------------------------------------------------------------------- */
class JavaScriptInstance : private boost::noncopyable
{
public:
	static boost::shared_ptr<hecomi::V8::JavaScript> get()
	{
		static auto js = boost::make_shared<hecomi::V8::JavaScript>();
		return js;
	}
};

/* ------------------------------------------------------------------------- */
//  JavaScript へ Export する : iRemocon
/* ------------------------------------------------------------------------- */
class iRemoconJS : public hecomi::V8::ExportToJSIF
{
private:
	//! iRemocon を制御するクラス
	boost::shared_ptr<iRemocon> iremocon_;

	//! 初期化したかどうか
	bool if_initialized_;

	//! 初期化したかどうかを返す
	bool initialized() {
		if (!if_initialized_) {
			std::cerr << "Error! iRemocon has not been initialized yet." << std::endl;
			return false;
		}
		return true;
	}

public:
	//! コンストラクタ
	iRemoconJS() : if_initialized_(false) {}

	//! JavaScript へエクスポートする関数
	boost::any func(const std::string& func_name, const v8::Arguments& args)
	{
		if (func_name == "connect") {
			v8::String::Utf8Value ip(args[0]);
			int port = args[1]->Int32Value();
			iremocon_ = boost::make_shared<iRemocon>(*ip, port);
			if_initialized_ = true;
		}
		else if (func_name == "send") {
			if (!initialized()) return false;
			int ir_num = args[0]->Int32Value();
			return iremocon_->ir_send(ir_num);
		}
		else if (func_name == "receive") {
			if (!initialized()) return false;
			int ir_num = args[0]->Int32Value();
			return iremocon_->ir_recieve(ir_num);
		}
		else if (func_name == "exec_command") {
			if (!initialized()) return false;
			v8::String::Utf8Value command(args[0]);
			return iremocon_->exec_command(*command);
		}
		return 0;
	}
};

/* ------------------------------------------------------------------------- */
//  JavaScript へ Export する : TextToSpeech
/* ------------------------------------------------------------------------- */
class TextToSpeechJS : public hecomi::V8::ExportToJSIF
{
private:
	//! TextToSpeech で喋らせるクラス
	boost::shared_ptr<TextToSpeech> tts_;

	//! 初期化したかどうか
	bool if_initialized_;

	//! 初期化したかどうかを返す
	bool initialized() {
		if (!if_initialized_) {
			std::cerr << "Error! TextToSpeech has not been initialized yet." << std::endl;
			return false;
		}
		return true;
	}

public:
	//! コンストラクタ
	TextToSpeechJS() : if_initialized_(false) {}

	//! JavaScript へエクスポートする関数
	boost::any func(const std::string& func_name, const v8::Arguments& args)
	{
		if (func_name == "init") {
			v8::String::Utf8Value voice_dir(args[0]);
			v8::String::Utf8Value dic_dir(args[1]);
			tts_ = boost::make_shared<TextToSpeech>(*voice_dir, *dic_dir);
			if_initialized_ = true;
		}
		else if (func_name == "talk") {
			if (!initialized()) return false;
			v8::String::Utf8Value str(args[0]);
			int fperiod = 220; // default
			if (args[1]->IsInt32()) fperiod = args[1]->Int32Value();
			tts_->talk(*str, fperiod);
		}
		else if (func_name == "retalk") {
			if (!initialized()) return false;
			tts_->retalk();
		}
		else if (func_name == "stop") {
			if (!initialized()) return false;
			tts_->stop();
		}
		return 0;
	}
};

/* ------------------------------------------------------------------------- */
//  JavaScript へ Export する : Julius
/* ------------------------------------------------------------------------- */
class JuliusJS : public hecomi::V8::ExportToJSIF
{
private:
	// Julius を制御するクラス
	boost::shared_ptr<Julius> julius_;

	//! 初期化したかどうか
	bool if_initialized_;

	//! 初期化したかどうかを返す
	bool initialized() {
		if (!if_initialized_) {
			std::cerr << "Error! Julius has not been initialized yet." << std::endl;
			return false;
		}
		return true;
	}

public:
	//! コンストラクタ
	JuliusJS() : if_initialized_(false) {}

	//! JavaScript へエクスポートする関数
	boost::any func(const std::string& func_name, const v8::Arguments& args)
	{
		if (func_name == "init") {
			v8::String::Utf8Value jconf_path(args[0]);
			julius_ = boost::make_shared<Julius>(*jconf_path);

			// 待機時処理
			julius_->add_speech_ready_callback([](Recog* recog, void* ptr){
				JuliusJS* _this = static_cast<JuliusJS*>(ptr);
				_this->on_speech_ready(recog);
			}, this);

			// 発話開始時処理
			julius_->add_speech_start_callback([](Recog* recog, void* ptr){
				JuliusJS* _this = static_cast<JuliusJS*>(ptr);
				_this->on_speech_start(recog);
			}, this);

			// 結果が返された時の処理
			julius_->add_result_callback([](Recog* recog, void* ptr){
				JuliusJS* _this = static_cast<JuliusJS*>(ptr);
				_this->on_result(recog);
			}, this);

			if_initialized_ = true;
		}
		else if (func_name == "start") {
			julius_->start();
		}
		return 0;
	}

	//! 発話待機時のコールバック
	void on_speech_ready(Recog* recog)
	{
		std::cout << "[HAS] <<< PLEASE SPEECH! >>>" << std::endl;
		auto js = JavaScriptInstance::get();
		js->exec("Julius.onSpeechReady();");
	}

	//! 発話開始時のコールバック
	void on_speech_start(Recog* recog)
	{
		std::cout << "[HAS] SPEECHING..." << std::endl;
		auto js = JavaScriptInstance::get();
		js->exec("Julius.onSpeechStart();");
	}

	//! 認識結果が返された時のコールバック
	void on_result(Recog* recog)
	{
		std::cout << "[HAS] ANALYZED!" << std::endl;

		// 結果を走査
		for (const RecogProcess *r = recog->process_list; r; r = r->next) {
			WORD_INFO *winfo = r->lm->winfo;

			// 仮説の数に応じてループ
			for (int n = 0; n < r->result.sentnum; ++n) {
				// Windows だと起こらないらしいが Ubuntu だとたまに起こる…
				if (r->result.sent == nullptr) break;

				Sentence s   = r->result.sent[n];
				WORD_ID *seq = s.word;
				int seqnum   = s.word_num;

				// 認識結果の文章を取得
				std::string output;
				for (int i = 1; i < seqnum-1; ++i) {
					// result[n][i] = winfo->woutput[seq[i]];
					output += winfo->woutput[seq[i]];
				}

				auto js = JavaScriptInstance::get();
				js->exec("Julius.onResult('" + output + "')");
			}
		}
	}
};

/* ------------------------------------------------------------------------- */
//  Constructor
/* ------------------------------------------------------------------------- */
HomeAutomationSystem::HomeAutomationSystem()
{
	init_v8();
	learn_commands_from_xml("data/commands.xml");
}

/* ------------------------------------------------------------------------- */
//  Destructor
/* ------------------------------------------------------------------------- */
HomeAutomationSystem::~HomeAutomationSystem()
{
}

/* ------------------------------------------------------------------------- */
//  認識させる文章と対応して送信する IR の番号を書いた
//  XML を読み込んでマップに格納
/* ------------------------------------------------------------------------- */
bool HomeAutomationSystem::learn_commands_from_xml(const std::string& file_name)
{
	using namespace boost::property_tree;

	ptree root;
	read_xml(file_name, root);

	for (const auto& pt : root.get_child("iRemocon")) {
		auto word = pt.second.get_optional<std::string>("<xmlattr>.word");
		auto num  = pt.second.get_optional<int>("<xmlattr>.num");
		if (!word || !num) {
			std::cerr << boost::format("[HAS] Error: %1% is invalid\n") % file_name;
			return false;
		}
		str_ir_map_.insert( std::make_pair(word.get(), num.get()) );
		std::cout << boost::format("[HAS] LEARNED: [%1%]\t%2%\n") % num.get() % word.get();
	}

	return true;
}

/* ------------------------------------------------------------------------- */
//  文章を引数にとって予め登録した言葉に合致するか調べる
//  合致した場合は送出する IR 信号の番号を返す
/* ------------------------------------------------------------------------- */
boost::optional<int> HomeAutomationSystem::interpret(const std::string& sentence) const
{
	using namespace std;

	// 登録されたコマンドに対応するIR番号を返す
	for (const auto& str_ir : str_ir_map_) {
		std::string regex_str = str_ir.first;
		int ir_num            = str_ir.second;

		boost::regex r(regex_str);
		boost::smatch m;

		if ( boost::regex_search(sentence, m, r) ) {
			return ir_num;
		}
	}

	return boost::optional<int>();
}

/* ------------------------------------------------------------------------- */
//  v8 を初期化する
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::init_v8()
{
	// JavaScript のインスタンスを得る
	auto js = JavaScriptInstance::get();

	// print 関数
	js->set("print", [](const v8::Arguments& args)->v8::Handle<v8::Value> {
		v8::String::Utf8Value str(args[0]);
		std::cout << *str << std::endl;
		return v8::Undefined();
	});

	// iRemocon
	hecomi::V8::ExportToJS<iRemoconJS> iRemocon("iRemocon");
	iRemocon.add_func("connect");
	iRemocon.add_func<bool>("send");
	iRemocon.add_func<bool>("receive");
	iRemocon.add_func<bool>("exec_command");
	js->set(iRemocon);

	// OpenJTalk
	hecomi::V8::ExportToJS<TextToSpeechJS> openjtalk("OpenJTalk");
	openjtalk.add_func("init");
	openjtalk.add_func("talk");
	openjtalk.add_func("retalk");
	openjtalk.add_func("stop");
	js->set(openjtalk);

	// Julius
	hecomi::V8::ExportToJS<JuliusJS> julius("__Julius__");
	julius.add_func("init");
	julius.add_func("start");
	js->set(julius);

	// Julius instance
	js->exec("var Julius = new __Julius__();");
}

/* ------------------------------------------------------------------------- */
//  JavaScript を実行する
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::exec_script(const std::string& script_path)
{
	auto js = JavaScriptInstance::get();
	js->exec_script(script_path);
}
