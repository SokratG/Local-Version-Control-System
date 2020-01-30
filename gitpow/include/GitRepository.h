/*
* Author: Gasparyan Sokrat
* Date: 2020/01/02
* GitRepository.h - this header file is a application definition.
*/
#pragma once
#include "stdafx.h"
#include "CommandParse.h"
#include "FileSystem.h"
#include "Exception.h"

using us = unsigned short;
using ul = unsigned long;
wstring getHomedir();

using GitDir = struct {
	const wstring folder{ L".git" };
	const wstring config{ L"config.ini" };
	const wstring branches{ L"branches" };
	const wstring objects{ L"objects" };
	const wstring refs{ L"refs" };
	const wstring heads{ L"heads" };
	const wstring tags{ L"tags" };
	const wstring HEAD{ L"HEAD" };
	const wstring description{ L"description" };
	const wstring index{ L"index" };
};

static const array<string, 6> valid_mode{ {"blob", "commit", "tree", "size", "type", "pretty"} };

using EntryTypes = tuple<map<string, size_t>, string, us, string>;
using StatusTypes = tuple<set<string>, set<string>, set<string>>; // changed, new, deleted

//auto EntryCompare = [](const EntryTypes& _k, const EntryTypes& _j) const { return std::get<3>(_k) < std::get<3>(_j); };

struct EntryCompare {
	bool operator()(const EntryTypes& _k, const EntryTypes& _j) const;
};


/*
const set<string> IndexEntry{ {"ctime_s", "ctime_n", "mtime_s", "mtime_n", "dev", "ino", "mode",
								"uid", "gid", "size", "sha1", "flags", "path"} };

*/


using core = struct {
		int repformatversion;
		bool filemode;
		bool bare;
};

using ConfigRepo = struct {
	core corerep;
	// field name:
	wstring corename{ L".core" };
};




class GitRepository
{	
private:
	const size_t MaxEntries = 9999u;
	FileSystem fs;
	const GitDir gitdir;
	ConfigRepo cfgrep;

public:
	explicit GitRepository();
	wstring getCurrentGitDir() const {
		return std::move(fs.getDirectory());
	}
	void setCurrentGitDir(const wstring& strdir) {
		try
		{
			fs.setDirectory(strdir + L"\\");
		}
		catch (FileSystemExeption& e)
		{
			wcout << e.getMessage() << endl; //log! change to throw or logfile
		}
		
	}
	bool writeConfig(const wstring_view& dir, const wstring& file);
	bool readConfig(const wstring_view& dir, const wstring& file);
	list<EntryTypes> read_index();
	void write_index(set<EntryTypes, EntryCompare>& entries);
	string write_tree();
	list<tuple<size_t, string, string>> read_tree(const string& sha, const string& data);
	string get_local_master_hash();
	StatusTypes GetStatus();
	friend class CommandParse;
};


