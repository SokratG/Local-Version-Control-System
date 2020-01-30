/*
* Author: Gasparyan Sokrat
* Date: 2019/12/26
* CommandParse.h - this header file is a application definition.
*/

#pragma once
#include "stdafx.h"
#include <unordered_map>


using std::unordered_map;
using StatusTypes = tuple<set<string>, set<string>, set<string>>; // changed, new, deleted

class GitRepository;



template<typename MSG = string>
class Message {
private:
	MSG msg;
	const bool w_type = false;
public:
	explicit Message(MSG message) : msg(message) {}
	Message& operator=(const Message&) = delete;
	Message(const Message&) = delete;
	void setMessage(MSG message) { msg = std::move(message); }
	MSG getMessage() const { return msg; }
};

class CommandParse {
	using cmdfunc = void(CommandParse::*)(GitRepository& );

private:
	unordered_map<wstring, cmdfunc> CommandList;
	list<wstring> listAttr;
	string MessageToUser;
	list<wstring> HistoryCommand;
private:
	bool valid_cmd_hash(const list<wstring>& listAttr, Message<wstring>& message, const GitRepository& rep);
	bool valid_cmd_catfile(const list<wstring>& listAttr, Message<wstring>& message);
	bool valid_cmd_lsfiles(const list<wstring>& listAttr, Message<wstring>& message);
	bool valid_cmd_commit(const list<wstring>& listAttr, Message<wstring>& message, string& data);
	string diff_control(const string& str1, const string& str2);
protected:

	void cmd_add(GitRepository& rep);
	void cmd_cat_file(GitRepository& rep);
	void cmd_checkout(GitRepository& rep);
	void cmd_commit(GitRepository& rep);
	void cmd_hash_object(GitRepository& rep);
	void cmd_init(GitRepository& rep);
	void cmd_status(GitRepository& rep);
	void cmd_ls_files(GitRepository& rep);
	void cmd_diff(GitRepository& rep);
	void cmd_cd(GitRepository& rep);
	void cmd_ls(GitRepository& rep);
	void cmd_pwd(GitRepository& rep);
	void cmd_help(GitRepository& rep);
	void cmd_exit(GitRepository& rep);

public:
	string HandleCommand(const wstring& str, GitRepository& rep);
	explicit CommandParse();
#ifdef TEST
	wstring getdir(GitRepository& rep) const;
#endif
};
