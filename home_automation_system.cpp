#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include "home_automation_system.hpp"

/* ------------------------------------------------------------------------- */
//  Constructor
/* ------------------------------------------------------------------------- */
HomeAutomationSystem::HomeAutomationSystem(
	const iRemocon& iremocon,
	const Julius& julius,
	const OpenJTalk& open_jtalk
) : iremocon_(iremocon), julius_(julius), open_jtalk_(open_jtalk)
{
	add_julius_callback();
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
			std::cerr << boost::format("Error: %1% is invalid\n") % file_name;
			return false;
		}
		str_ir_map_.insert( std::make_pair(word.get(), num.get()) );
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
	julius_.add_speech_ready_callback([](Recog*, void*){
		std::cout << "<<< PLEASE SPEECH! >>>" << std::endl;
	});

	// 発話開始時処理
	julius_.add_speech_start_callback([](Recog*, void*){
		std::cout << "<<< SPEAKING... >>>" << std::endl;
	});

	// 結果が返された時の処理
	julius_.add_result_callback([](Recog* recog, void* this_){
		// callback の引数から this を復元
		HomeAutomationSystem* has = static_cast<HomeAutomationSystem*>(this_);

		// 結果を走査
		for (const RecogProcess *r = recog->process_list; r; r = r->next) {
			WORD_INFO *winfo = r->lm->winfo;

			for (int n = 0; n < r->result.sentnum; ++n) {
				Sentence s   = r->result.sent[n];
				WORD_ID *seq = s.word;
				int seqnum   = s.word_num;

				std::string output = "";
				for (int i = 1; i < seqnum-1; ++i) {
					// 認識結果の文章を取得
					output += winfo->woutput[seq[i]];
				}
				std::cout << output << std::endl;

				// コマンドに対応するIR番号を発信
				auto ir_num = has->interpret(output);
				if (ir_num) {
					has->talk(output + "、を実行します");
					std::cout << ir_num << std::endl;
					// has->iremocon_.ir_send(ir_num.get());
				}
			}
		}
	}, this);
}

/* ------------------------------------------------------------------------- */
//  認識開始
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::start()
{
	julius_.start();
}

/* ------------------------------------------------------------------------- */
//  喋らせる
/* ------------------------------------------------------------------------- */
void HomeAutomationSystem::talk(const std::string& str)
{
	open_jtalk_.talk(str);
}
