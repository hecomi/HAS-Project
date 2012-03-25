#include <iostream>
#include <boost/noncopyable.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>

#include <julius/juliuslib.h>
#include <mecab.h>

// MeCab::Taggerを吐き出す
class MeCabTagger : private boost::noncopyable
{
public:
	static MeCab::Tagger* getInstance() {
		static MeCab::Tagger *tagger = MeCab::createTagger("");
		return tagger;
	}
};

// 動詞か名詞かを判定する
bool chkVerbOrNoun(const std::string& str)
{
	auto tagger = MeCabTagger::getInstance();
	const MeCab::Node* node = tagger->parseToNode(str.c_str());
	for (node = node->next; node->next; node = node->next) {
		boost::tokenizer<> tok(std::string(node->feature));
		std::string pos = *(tok.begin());
		if (pos == "動詞" || pos == "名詞") {
			return true;
		}
	}
	return false;
}

void OnOutputResult(Recog *recog, void*)
{
	for (const RecogProcess *r = recog->process_list; r; r = r->next) {
		WORD_INFO *winfo = r->lm->winfo;

		for (int n = 0; n < r->result.sentnum; ++n) {
			Sentence s   = r->result.sent[n];
			WORD_ID *seq = s.word;
			int seqnum   = s.word_num;

			for (int i = 0; i < seqnum; ++i) {
				std::string output = winfo->woutput[seq[i]];
				std::cout << output;

				// 動詞か名詞だったらスコアを併せて出力
				if ( chkVerbOrNoun(output) ) {
					std::cout << boost::format("(%1%)") % s.confidence[i];
				}
			}
			std::cout << std::endl;
		}
	}
}

int main(int argc, char* argv[])
{
	// Arguments
	const char* j_argv[] = {
		"test",
		"-C",
		"hmm_mono.jconf",
		"-gram",
		"gram/kaden",
		"-input",
		"mic"
	};
	const int j_argc = sizeof(j_argv)/sizeof(j_argv[0]);

	// Jconf: configuration parameters
	// load configurations from command arguments
	Jconf *jconf = j_config_load_args_new(j_argc, const_cast<char**>(j_argv));
	if (jconf == nullptr) {
		std::cout << "Error @ j_config_load_args_new" << std::endl;
		return -1;
	}

	// Recog: Top level instance for the whole recognition process
	// create recognition instance according to the jconf
	Recog *recog = j_create_instance_from_jconf(jconf);
	if (recog == nullptr) {
		std::cout << "Error @ j_create_instance_from_jconf" << std::endl;
		return -1;
	}

	// Register callback
	callback_add(recog, CALLBACK_EVENT_SPEECH_READY, [](Recog *recog, void*) {
		std::cout << "<<< PLEASE SPEAK! >>>" << std::endl;
	}, nullptr);

	callback_add(recog, CALLBACK_EVENT_SPEECH_START, [](Recog *recog, void*) {
		std::cout << "...SPEECH START..." << std::endl;
	}, nullptr);

	callback_add(recog, CALLBACK_RESULT, OnOutputResult, nullptr);

	// Initialize audio input
	if (j_adin_init(recog) == FALSE) {
		return -1;
	}

	// Open input stream and recognize
	switch (j_open_stream(recog, nullptr)) {
		case  0: break; // success
		case -1: std::cout << "Error in input stream" << std::endl; return -1;
		case -2: std::cout << "Failed to begin input stream" << std::endl; return -1;
	}

	// Disable debug & verbose messages
	j_disable_debug_message();
	j_disable_verbose_message();

	// Recognition loop
	boost::thread thr([&]() {
		int ret = j_recognize_stream(recog);
		if (ret == -1) return -1;
	});

	// Exit if Enter key pressed
	std::string line;
	getline(std::cin, line);

	j_close_stream(recog);
	thr.join();
	j_recog_free(recog);

	return 0;
}
