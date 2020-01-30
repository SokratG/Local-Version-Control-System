/*
* Date: 2019/12/25
* utility.cpp
* This module have implement some helper function
*/
#include "stdafx.h"
#include "utility.h"





wstring s2ws(const string& str)
{
	using convert_typeX = codecvt_utf8<wchar_t>;
	wstring_convert<convert_typeX, wchar_t> converterX;

	return std::move(converterX.from_bytes(str));
}

string ws2s(const wstring& wstr)
{
	using convert_typeX = codecvt_utf8<wchar_t>;
	wstring_convert<convert_typeX, wchar_t> converterX;

	return std::move(converterX.to_bytes(wstr));
}




string S2HEX(const string& str)
{
	stringstream buff;
	for (auto& ch : str) {
		buff << std::hex << (unsigned)ch;
	}
	return std::move(buff.str());
}


string HEX2S(const string& hexstr)
{
	std::stringstream buff;
	for (size_t i = 0; i < hexstr.size(); i += 2) {
		string byte = hexstr.substr(i, 2);
		buff << static_cast<char>((unsigned)strtol(byte.c_str(), nullptr, 16));
	}
	return std::move(buff.str());
}

bool startwith(const string& from, const string str) {
	return  (from.rfind(str, 0) == 0); //check start position must be 0
}


list<wstring> SplitToWStringList(const wstring& str, const wchar_t ch) noexcept
{
	list<wstring> temp;
	wstringstream ss(str);
	wstring item;
	while (std::getline(ss, item, ch)) {
		temp.push_back(item);
	}
	return std::move(temp);
}

list<string> SplitToStringList(const string& str, const char ch) noexcept
{
	list<string> temp;
	stringstream ss(str);
	string item;
	while (std::getline(ss, item, ch)) {
		temp.push_back(item);
	}
	return std::move(temp);
}



//=========================================================================================
/*
	https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
*/
string replaceRegEx(const std::string& str, const std::string& from, const std::string& to) noexcept {
	if (from.empty())
		return string();

	return std::move(std::regex_replace(str, std::regex("\\" + from), to));
}

bool replace(std::string& str, const std::string& from, const std::string& to) noexcept {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) noexcept {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}
//=========================================================================================
