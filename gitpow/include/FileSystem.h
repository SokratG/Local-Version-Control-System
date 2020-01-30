/*
* Author: Gasparyan Sokrat
* Date: 2019/12/23
* FileSystem.h - this header file is a application definition.
*/
#pragma once
#include "stdafx.h"
#include <filesystem>
#include <list>

namespace fs = std::filesystem;
using std::list;
using std::filesystem::path;
using std::filesystem::directory_entry;
using std::filesystem::file_status;
using std::filesystem::current_path;
using std::filesystem::directory_iterator;
using std::filesystem::create_directory;

wstring getHomedir();

class FileSystem
{
private:
	
	wstring CurrentWorkDir;
	bool change_list;
#ifdef TEST
	void FileHandle(wfstream& file);
	static char type_char(file_status fs) noexcept;
#endif
	void construct_list_wstring_files(const path& dir, set<wstring>& files) noexcept;
	void construct_list_string_files(const path& dir, set<string>& files) noexcept;
public:
	FileSystem();
	wstring getDirectory() const;
	void setDirectory(const wstring& str);
	void setDirectory(const string& str);
	void setDirectory(const path& str);
	set<wstring> getListAllWstringFiles();
	set<string> getListAllStringFiles();
	list<wstring> getListWStringFiles(bool withoutpath = false);
	list<string> getListStringFiles(bool withoutpath = false);
	bool createFolder(const wstring& folder, bool hide = false);
	bool createFolder(const string& folder, bool hide = false);
	void createFile(const wstring& file, const wstring& folder);
	void createFile(const string& file, const string& folder);
	void createFileAndWrite(const wstring& file, const wstring& folder, const wstring& data);
	void createFileAndWrite(const string& file, const string& folder, const string& data);
	void createFileAndWrite(const path& fullfilepath, const string& data);
	void writeFile(const wstring& file, const wstring& folder, const wstring& data);
	void writeFile(const string& file, const string& folder, const string& data);
	void writeFile(const path& fullfilepath, const string& data);
	list<string> readFromFileToList(const wstring& folder, const wstring& file);
	string readFromFileToString(const wstring& folder, const wstring& file);
	string readFromFileToString(const string& folder, const string& file);
	string readFromFileToString(const path& fullfilepath);
	static string transformListToString(const list<string>& liststr) noexcept;
};

