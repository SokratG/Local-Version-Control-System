/*
* Author: Gasparyan Sokrat
* Date: 2020/01/03
* utility.h - this header file is a application definition.
*/
#pragma once
#include "stdafx.h"
#include <locale>
#include <codecvt>

using std::wstring;
using std::string;
using std::wstring_convert;
using std::codecvt_utf8;
using std::stringstream;

wstring s2ws(const string& str);

string ws2s(const wstring& wstr);

string S2HEX(const string& str);

string HEX2S(const string& hexstr);


bool startwith(const string& from, const string str);

list<wstring> SplitToWStringList(const wstring& str, const wchar_t ch = L' ') noexcept;

list<string> SplitToStringList(const string& str, const char ch = ' ') noexcept;

template<typename Type = size_t, size_t ByteFill = 0>
string I2HEX(Type val) {
	stringstream stream;
	// << "0x" << 
	stream << std::setfill('0') << std::setw(ByteFill)
		<< std::hex << val;
	return std::move(stream.str());
}

template<typename Type = size_t, size_t ByteFill = 0>
string I2OCT(Type val) {
	stringstream stream;
	stream << std::setfill('0') << std::setw(ByteFill)
		<< std::oct << val;
	return std::move(stream.str());
}



/*
	fix to  decltype(file_time)::clock::to_time_t(file_time):
	https://stackoverflow.com/questions/56788745/how-to-convert-stdfilesystemfile-time-type-to-a-string-using-gcc-9 
*/
template <typename TP>
std::time_t to_time_t(TP tp)
{
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
		+ system_clock::now());
	return system_clock::to_time_t(sctp);
}

template<typename T>
void write_pod(std::ofstream& out, T& t)
{
	out.write(reinterpret_cast<char*>(&t), sizeof(T));
}

template<typename T>
void read_pod(std::ifstream& in, T& t)
{
	in.read(reinterpret_cast<char*>(&t), sizeof(T));
}

//=========================================================================================
string replaceRegEx(const std::string& str, const std::string& from, const std::string& to) noexcept;

bool replace(std::string& str, const std::string& from, const std::string& to) noexcept;

void replaceAll(std::string& str, const std::string& from, const std::string& to) noexcept;
//=========================================================================================
