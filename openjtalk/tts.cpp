/* ----------------------------------------------------------------- */
/*           The Japanese TTS System "Open JTalk"                    */
/*           developed by HTS Working Group                          */
/*           http://open-jtalk.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2008-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifdef __cplusplus
#define OPEN_JTALK_C_START extern "C" {
#define OPEN_JTALK_C_END   }
#else
#define OPEN_JTALK_C_START
#define OPEN_JTALK_C_END
#endif                          /* __CPLUSPLUS */

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cmath>

/* Main headers */
#include "mecab.h"
#include "njd.h"
#include "jpcommon.h"
#include "HTS_engine.h"

/* Sub headers */
#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"
#include "njd2jpcommon.h"

/* for outside use */
#include "tts.hpp"

#define MAXBUFLEN 1024

struct OpenJTalk_structs{
	Mecab mecab;
	NJD njd;
	JPCommon jpcommon;
	HTS_Engine engine;
};

void OpenJTalk_initialize(OpenJTalk_structs* open_jtalk, HTS_Boolean use_lpf, int sampling_rate,
		int fperiod, double alpha, int stage, double beta, int audio_buff_size,
		double uv_threshold, HTS_Boolean use_log_gain, double gv_weight_mgc,
		double gv_weight_lf0, double gv_weight_lpf)
{
	Mecab_initialize(&open_jtalk->mecab);
	NJD_initialize(&open_jtalk->njd);
	JPCommon_initialize(&open_jtalk->jpcommon);
	if (use_lpf)
		HTS_Engine_initialize(&open_jtalk->engine, 3);
	else
		HTS_Engine_initialize(&open_jtalk->engine, 2);
	HTS_Engine_set_sampling_rate(&open_jtalk->engine, sampling_rate);
	HTS_Engine_set_fperiod(&open_jtalk->engine, fperiod);
	HTS_Engine_set_alpha(&open_jtalk->engine, alpha);
	HTS_Engine_set_gamma(&open_jtalk->engine, stage);
	HTS_Engine_set_log_gain(&open_jtalk->engine, use_log_gain);
	HTS_Engine_set_beta(&open_jtalk->engine, beta);
	HTS_Engine_set_audio_buff_size(&open_jtalk->engine, audio_buff_size);
	HTS_Engine_set_msd_threshold(&open_jtalk->engine, 1, uv_threshold);
	HTS_Engine_set_gv_weight(&open_jtalk->engine, 0, gv_weight_mgc);
	HTS_Engine_set_gv_weight(&open_jtalk->engine, 1, gv_weight_lf0);
	if (use_lpf)
		HTS_Engine_set_gv_weight(&open_jtalk->engine, 2, gv_weight_lpf);
}

void OpenJTalk_load(OpenJTalk_structs* open_jtalk, char *dn_mecab, char *fn_ms_dur, char *fn_ts_dur,
		char *fn_ms_mgc, char *fn_ts_mgc, char **fn_ws_mgc, int num_ws_mgc,
		char *fn_ms_lf0, char *fn_ts_lf0, char **fn_ws_lf0, int num_ws_lf0,
		char *fn_ms_lpf, char *fn_ts_lpf, char **fn_ws_lpf, int num_ws_lpf,
		char *fn_ms_gvm, char *fn_ts_gvm, char *fn_ms_gvl, char *fn_ts_gvl,
		char *fn_ms_gvf, char *fn_ts_gvf, char *fn_gv_switch)
{
	Mecab_load(&open_jtalk->mecab, dn_mecab);
	HTS_Engine_load_duration_from_fn(&open_jtalk->engine, &fn_ms_dur, &fn_ts_dur, 1);
	HTS_Engine_load_parameter_from_fn(&open_jtalk->engine, &fn_ms_mgc, &fn_ts_mgc, fn_ws_mgc, 0,
			FALSE, num_ws_mgc, 1);
	HTS_Engine_load_parameter_from_fn(&open_jtalk->engine, &fn_ms_lf0, &fn_ts_lf0, fn_ws_lf0, 1,
			TRUE, num_ws_lf0, 1);
	if (HTS_Engine_get_nstream(&open_jtalk->engine) == 3)
		HTS_Engine_load_parameter_from_fn(&open_jtalk->engine, &fn_ms_lpf, &fn_ts_lpf, fn_ws_lpf, 2,
				FALSE, num_ws_lpf, 1);
	if (fn_ms_gvm != NULL) {
		if (fn_ts_gvm != NULL)
			HTS_Engine_load_gv_from_fn(&open_jtalk->engine, &fn_ms_gvm, &fn_ts_gvm, 0, 1);
		else
			HTS_Engine_load_gv_from_fn(&open_jtalk->engine, &fn_ms_gvm, NULL, 0, 1);
	}
	if (fn_ms_gvl != NULL) {
		if (fn_ts_gvl != NULL)
			HTS_Engine_load_gv_from_fn(&open_jtalk->engine, &fn_ms_gvl, &fn_ts_gvl, 1, 1);
		else
			HTS_Engine_load_gv_from_fn(&open_jtalk->engine, &fn_ms_gvl, NULL, 1, 1);
	}
	if (HTS_Engine_get_nstream(&open_jtalk->engine) == 3 && fn_ms_gvf != NULL) {
		if (fn_ts_gvf != NULL)
			HTS_Engine_load_gv_from_fn(&open_jtalk->engine, &fn_ms_gvf, &fn_ts_gvf, 2, 1);
		else
			HTS_Engine_load_gv_from_fn(&open_jtalk->engine, &fn_ms_gvf, NULL, 2, 1);
	}
	if (fn_gv_switch != NULL)
		HTS_Engine_load_gv_switch_from_fn(&open_jtalk->engine, fn_gv_switch);
}

void OpenJTalk_synthesis(OpenJTalk_structs* open_jtalk, char *txt, FILE * wavfp, FILE * logfp)
{
	char buff[MAXBUFLEN];

	text2mecab(buff, txt);
	Mecab_analysis(&open_jtalk->mecab, buff);
	mecab2njd(&open_jtalk->njd, Mecab_get_feature(&open_jtalk->mecab),
			Mecab_get_size(&open_jtalk->mecab));
	njd_set_pronunciation(&open_jtalk->njd);
	njd_set_digit(&open_jtalk->njd);
	njd_set_accent_phrase(&open_jtalk->njd);
	njd_set_accent_type(&open_jtalk->njd);
	njd_set_unvoiced_vowel(&open_jtalk->njd);
	njd_set_long_vowel(&open_jtalk->njd);
	njd2jpcommon(&open_jtalk->jpcommon, &open_jtalk->njd);
	JPCommon_make_label(&open_jtalk->jpcommon);
	if (JPCommon_get_label_size(&open_jtalk->jpcommon) > 2) {
		HTS_Engine_load_label_from_string_list(&open_jtalk->engine,
				JPCommon_get_label_feature(&open_jtalk->jpcommon),
				JPCommon_get_label_size(&open_jtalk->jpcommon));
		HTS_Engine_create_sstream(&open_jtalk->engine);
		HTS_Engine_create_pstream(&open_jtalk->engine);
		HTS_Engine_create_gstream(&open_jtalk->engine);
		if (wavfp != NULL)
			HTS_Engine_save_riff(&open_jtalk->engine, wavfp);
		if (logfp != NULL) {
			fprintf(logfp, "[Text analysis result]\n");
			NJD_fprint(&open_jtalk->njd, logfp);
			fprintf(logfp, "\n[Output label]\n");
			HTS_Engine_save_label(&open_jtalk->engine, logfp);
			fprintf(logfp, "\n");
			HTS_Engine_save_information(&open_jtalk->engine, logfp);
		}
		HTS_Engine_refresh(&open_jtalk->engine);
	}
	JPCommon_refresh(&open_jtalk->jpcommon);
	NJD_refresh(&open_jtalk->njd);
	Mecab_refresh(&open_jtalk->mecab);
}

void OpenJTalk_clear(OpenJTalk_structs* open_jtalk)
{
	Mecab_clear(&open_jtalk->mecab);
	NJD_clear(&open_jtalk->njd);
	JPCommon_clear(&open_jtalk->jpcommon);
	HTS_Engine_clear(&open_jtalk->engine);
}

/* Getfp: wrapper for fopen */
FILE *Getfp(const char *name, const char *opt)
{
   FILE *fp = fopen(name, opt);

   if (fp == NULL) {
      fprintf(stderr, "ERROR: Getfp() in open_jtalk.c: Cannot open %s.\n", name);
      exit(1);
   }

   return (fp);
}

int OpenJTalk_TTS(const std::string& sentence, const std::string& wav_filename, const std::string& dic_dir, const std::string& voice_dir)
{
	const size_t CHAR_BUF_SIZE = 128;

	FILE *wavfp = Getfp(wav_filename.c_str(), "wb");
	FILE *logfp = NULL;

	/* sentence */
	char talk_str[CHAR_BUF_SIZE]; strcpy(talk_str, (sentence).c_str());

	/* engine */
	OpenJTalk_structs open_jtalk;

	/* directory name of dictionary */
	char dn_mecab[CHAR_BUF_SIZE]; strcpy(dn_mecab, dic_dir.c_str());

	/* file names of models */
	char fn_ms_dur[CHAR_BUF_SIZE]; strcpy(fn_ms_dur, (voice_dir + "/dur.pdf").c_str());
	char fn_ms_mgc[CHAR_BUF_SIZE]; strcpy(fn_ms_mgc, (voice_dir + "/mgc.pdf").c_str());
	char fn_ms_lf0[CHAR_BUF_SIZE]; strcpy(fn_ms_lf0, (voice_dir + "/lf0.pdf").c_str());
	char *fn_ms_lpf = NULL;

	/* file names of trees */
	char fn_ts_dur[CHAR_BUF_SIZE]; strcpy(fn_ts_dur, (voice_dir + "/tree-dur.inf").c_str());
	char fn_ts_mgc[CHAR_BUF_SIZE]; strcpy(fn_ts_mgc, (voice_dir + "/tree-mgc.inf").c_str());
	char fn_ts_lf0[CHAR_BUF_SIZE]; strcpy(fn_ts_lf0, (voice_dir + "/tree-lf0.inf").c_str());
	char *fn_ts_lpf = NULL;

	/* file names of windows */
	char **fn_ws_mgc = static_cast<char**>(malloc(sizeof(char)*3));
	char **fn_ws_lf0 = static_cast<char**>(malloc(sizeof(char)*3));
	for (int i = 0; i < 3; ++i) {
		fn_ws_mgc[i] = static_cast<char*>(malloc(sizeof(char)*CHAR_BUF_SIZE));
		fn_ws_lf0[i] = static_cast<char*>(malloc(sizeof(char)*CHAR_BUF_SIZE));
		sprintf(fn_ws_mgc[i], (voice_dir + "/mgc.win%d").c_str(), i+1);
		sprintf(fn_ws_lf0[i], (voice_dir + "/lf0.win%d").c_str(), i+1);
	}
	char **fn_ws_lpf = NULL;
	int num_ws_mgc = 3, num_ws_lf0 = 3, num_ws_lpf = 0;

	/* file names of global variance */
	char fn_ms_gvm[CHAR_BUF_SIZE]; strcpy(fn_ms_gvm, (voice_dir + "/gv-mgc.pdf").c_str());
	char fn_ms_gvf[CHAR_BUF_SIZE]; strcpy(fn_ms_gvf, (voice_dir + "/gv-lf0.pdf").c_str());
	char *fn_ms_gvl = NULL;

	/* file names of global variance trees */
	char fn_ts_gvm[CHAR_BUF_SIZE]; strcpy(fn_ts_gvm, (voice_dir + "/tree-gv-mgc.inf").c_str());
	char fn_ts_gvf[CHAR_BUF_SIZE]; strcpy(fn_ts_gvf, (voice_dir + "/tree-gv-lf0.inf").c_str());
	char *fn_ts_gvl = NULL;

	/* file names of global variance switch */
	char fn_gv_switch[CHAR_BUF_SIZE]; strcpy(fn_gv_switch, (voice_dir + "/gv-switch.inf").c_str());

	/* global parameter */
	int sampling_rate = 48000;
	int fperiod = 240;
	double alpha = 0.5;
	int stage = 0;               /* gamma = -1.0/stage */
	double beta = 0.8;
	int audio_buff_size = 48000;
	double uv_threshold = 0.5;
	double gv_weight_mgc = 1.0;
	double gv_weight_lf0 = 1.0;
	double gv_weight_lpf = 1.0;
	HTS_Boolean use_log_gain = FALSE;
	HTS_Boolean use_lpf = FALSE;

	/* initialize and load */
	OpenJTalk_initialize(&open_jtalk, use_lpf, sampling_rate, fperiod, alpha, stage, beta,
			audio_buff_size, uv_threshold, use_log_gain, gv_weight_mgc,
			gv_weight_lf0, gv_weight_lpf);
	OpenJTalk_load(&open_jtalk, dn_mecab, fn_ms_dur, fn_ts_dur, fn_ms_mgc, fn_ts_mgc,
			fn_ws_mgc, num_ws_mgc, fn_ms_lf0, fn_ts_lf0, fn_ws_lf0, num_ws_lf0,
			fn_ms_lpf, fn_ts_lpf, fn_ws_lpf, num_ws_lpf, fn_ms_gvm, fn_ts_gvm,
			fn_ms_gvl, fn_ts_gvl, fn_ms_gvf, fn_ts_gvf, fn_gv_switch);

	/* synthesis */
	OpenJTalk_synthesis(&open_jtalk, talk_str, wavfp, logfp);

	/* free */
	OpenJTalk_clear(&open_jtalk);
	free(fn_ws_mgc);
	free(fn_ws_lf0);
	free(fn_ws_lpf);
	if (wavfp != NULL)
		fclose(wavfp);
	if (logfp != NULL)
		fclose(logfp);

	return 0;
}

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

