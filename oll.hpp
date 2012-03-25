#ifndef INCLUDE_OLL_HPP
#define INCLUDE_OLL_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <boost/optional.hpp>
#include "oll/oll.hpp"

template<int TrainMethodNum> struct train_method { typedef void type; };
template<> struct train_method<0> { typedef oll_tool::P_s   type; }; // Perceptron
template<> struct train_method<1> { typedef oll_tool::AP_s  type; }; // Averaged Perceptron
template<> struct train_method<2> { typedef oll_tool::PA_s  type; }; // Passive Agressive
template<> struct train_method<3> { typedef oll_tool::PA1_s type; }; // Passive Agressive L1
template<> struct train_method<4> { typedef oll_tool::PA2_s type; }; // Passive Agressive L2
template<> struct train_method<5> { typedef oll_tool::PAK_s type; }; // Kernelized Passive Agressive
template<> struct train_method<6> { typedef oll_tool::CW_s  type; }; // Confidence Weighted
template<> struct train_method<7> { typedef oll_tool::AL_s  type; }; // ALMA HD

/**
 *  オンライン学習ライブラリの機能をまとめたクラス
 *  @template TrainMethodNum oll_tool::学習手法（P, AP, PA, PA1, PA2, PAK, CW, AL）
 */
template <int TrainMethodNum = oll_tool::PA1>
class OnlineLearningLibrary
{
public:
	typedef typename train_method<TrainMethodNum>::type TrainMethod;

	/**
	 *  コンストラクタ
	 *  @param[in] C    Regularization Parameter
	 *  @param[in] bias Bias
	 */
	OnlineLearningLibrary(float C = 1.f, float bias = 0.f)
	: tm_( static_cast<oll_tool::trainMethod>(TrainMethodNum) )
	{
		ol_.setC(C);
		ol_.setBias(bias);
	}

	/**
	 *  学習結果をファイルに保存
	 *  @param[in] file_name 保存先ファイル名
	 */
	bool save(const std::string& file_name)
	{
		if ( ol_.save(file_name.c_str()) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return false;
		}
		return true;
	}

	/**
	 *  学習結果をファイルから復元
	 *  @param[in] file_name 復元元ファイル名
	 */
	bool load(const std::string& file_name)
	{
		if ( ol_.load(file_name.c_str()) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return false;
		}
		return true;
	}

	/**
	 *  データを渡して学習させる
	 *  @param[in] flag true: +のデータ、false: -のデータ
	 *  @param[in] data 学習データ (format: id:val id:val ...)
	 */
	bool add(int flag, const std::string& data)
	{
		std::string format = ( (flag > 0) ? "1 " : "-1 " ) + data;
		oll_tool::fv_t fv;
		int y = 0;

		if (ol_.parseLine(format, fv, y) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return false;
		}

		TrainMethod a;
		ol_.trainExample(a, fv, y);

		return true;
	}

	/**
	 *  データをテストする
	 *  @param[in] data テストデータ : id:val id:val ...
	 */
	boost::optional<float> test(const std::string& data)
	{
		std::string format = "0 " + data;
		oll_tool::fv_t fv;
		int y = 0;

		if (ol_.parseLine(format, fv, y) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return boost::optional<float>();
		}

		return boost::optional<float>(ol_.classify(fv));
	}

private:
	//! OLL
	oll_tool::oll ol_;

	//! 学習手法
	oll_tool::trainMethod tm_;
};

#endif // INCLUDE_OLL_HPP
