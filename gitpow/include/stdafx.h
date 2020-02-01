/*
* Author: Gasparyan Sokrat
* Date: 2019/12/19
* stdadf.h - this precompile header file is a application definition.
*/

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define NOMINMAX
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             

#include <windows.h>
#include <shlobj.h> 

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <sstream>
#include <regex>
#include <set>
#include <list>
#include <tuple>
#include <array>
#include <iomanip>
#include <map>
#include <cassert>
#include <chrono>
#include <stack>

//#define DEBUG
#define __WIN32__

#define _SYM_ "\xff" //special symbol for parsing data in hashing object

#define CRED 12
#define CBLUE 9
#define CGREEN 10
#define CLBLUE 11
#define CYELLOW 14
#define CDEFUALT 7


#ifdef __UNIX__
#include <unistd.h>
#endif

using std::string;
using std::wstring;
using std::wstring_view;
using std::string_view;
using std::wfstream;
using std::wofstream;
using std::wifstream;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::stringstream;
using std::wstringstream;
using std::cout;
using std::cerr;
using std::wcerr;
using std::endl;
using std::wcout;
using std::shared_ptr;
using std::make_shared;
using std::set;
using std::list;
using std::tuple;
using std::array;
using std::map;
using std::stack;
using namespace std::chrono_literals;
