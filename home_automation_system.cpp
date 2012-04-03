#include <functional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/make_shared.hpp>
#include "home_automation_system.hpp"

/* ------------------------------------------------------------------------- */
//  Constructor
/* ------------------------------------------------------------------------- */
HomeAutomationSystem::HomeAutomationSystem()
: state_(HAS_STATE::INIT)
{
	iremocon_  = boost::make_shared<iRemocon>("192.168.0.7", 51013);
	julius_    = boost::make_shared<Julius>("data/setting.jconf");
	tts_       = boost::make_shared<TextToSpeech>("data/mei_normal", "openjtalk/open_jtalk_dic_utf_8-1.05");

	learn_commands_from_xml("data/commands.xml");
	add_julius_callback();
	julius_->start();
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
//  Julius へコールバック関数を登録
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::add_julius_callback()
{
	// 待機時処理
	julius_->add_speech_ready_callback([](Recog* recog, void* has){
		// callback の引数から this を復元
		HomeAutomationSystem* _this = static_cast<HomeAutomationSystem*>(has);
		_this->on_speech_ready(recog);
	}, this);

	// 発話開始時処理
	julius_->add_speech_start_callback([](Recog* recog, void* has){
		// callback の引数から this を復元
		HomeAutomationSystem* _this = static_cast<HomeAutomationSystem*>(has);
		_this->on_speech_start(recog);
	}, this);

	// 結果が返された時の処理
	julius_->add_result_callback([](Recog* recog, void* has){
		// callback の引数から this を復元
		HomeAutomationSystem* _this = static_cast<HomeAutomationSystem*>(has);
		_this->on_result(recog);
	}, this);
}

/* ------------------------------------------------------------------------- */
//  発話待機時のコールバック
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::on_speech_ready(Recog* recog)
{
	// キャンセル待機中
	if (state_ == HAS_STATE::WAITING) return;

	std::cout << "[HAS] PLEASE SPEACH!" << std::endl;
	state_ = HAS_STATE::READY;
}

/* ------------------------------------------------------------------------- */
//  発話開始時のコールバック
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::on_speech_start(Recog* recog)
{
	// キャンセル待機中
	if (state_ == HAS_STATE::WAITING) return;

	// HASが返答しているときに喋ったら喋りを中断してキャンセルするか聴く
	if (state_ == HAS_STATE::HAS_TALKING) {
		std::cout << "[HAS] CANCEL...?" << std::endl;
		state_ = HAS_STATE::WAITING;
		tts_->stop();
		return;
	}

	std::cout << "[HAS] YOU'RE SPEECHING NOW..." << std::endl;
	state_ = HAS_STATE::USER_SPEAKING;
}

/* ------------------------------------------------------------------------- */
//  認識結果が返された時のコールバック
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::on_result(Recog* recog)
{
	if (state_ != HAS_STATE::WAITING) {
		std::cout << "[HAS] YOUR COMMAND:" << std::endl;
		state_ = HAS_STATE::ANALYZING;
	}

	// 結果を走査
	for (const RecogProcess *r = recog->process_list; r; r = r->next) {
		WORD_INFO *winfo = r->lm->winfo;

		for (int n = 0; n < r->result.sentnum; ++n) {
			// Windows だと起こらないらしい
			if (r->result.sent == nullptr) break;

			Sentence s   = r->result.sent[n];
			WORD_ID *seq = s.word;
			int seqnum   = s.word_num;

			// 認識結果の文章を取得
			std::string output;
			for (int i = 1; i < seqnum-1; ++i) {
				output += winfo->woutput[seq[i]];
			}
			std::cout << output << std::endl;

			// キャンセル待機中
			if (state_ == HAS_STATE::WAITING) {
				// キャンセルワードに引っかかる場合
				for (const auto& str : {"キャンセル", "うん", "します", "はい"}) {
					std::cout << str << " : " << output << std::endl;
					if (output.find(str) != std::string::npos) {
						talk("キャンセルしました");
						state_ = HAS_STATE::DONE;
						return;
					}
				}

				// キャンセルワードに引っかからなかった場合
				// （= 物音等で中断してしまった場合）
				talk("失礼しました。" + pre_str_ + "を実行します。");
				auto ir_num = interpret(pre_str_);
				if (ir_num) {
					// IR信号を発信する
					state_ = HAS_STATE::EXECUTING;
					iremocon_->ir_send(ir_num.get());
					state_ = HAS_STATE::DONE;
				}
				return;
			}

			// コマンドに対応するIR番号を発信
			auto ir_num = interpret(output);
			if (ir_num) {
				state_ = HAS_STATE::HAS_TALKING;
				talk(output + "ですね。わかりました。");

				// 途中で喋りを中断された場合は返事をしてループを抜ける
				if (state_ == HAS_STATE::WAITING) {
					pre_str_ = output;
					talk("キャンセルしますか？");
					return;
				}

				// IR信号を発信する
				state_ = HAS_STATE::EXECUTING;
				iremocon_->ir_send(ir_num.get());
				state_ = HAS_STATE::DONE;
			}
		}
	}
}

/* ------------------------------------------------------------------------- */
//  認識開始
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::start()
{
	julius_->start();
}

/* ------------------------------------------------------------------------- */
//  喋らせる
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::talk(const std::string& str)
{
	tts_->talk(str, 200);
}

/* ------------------------------------------------------------------------- */
//  機械学習をするための素性リストを書きだす
/* ------------------------------------------------------------------------- */
/* under constructing... */
