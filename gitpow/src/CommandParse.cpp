/*
* Author: Gasparyan Sokrat
* Date: 2020/01/07
* ComamndParse.cpp
* This module have implement main functionality of local 
* system version control commands.
*/
#include "stdafx.h"
#include "Exception.h"
#include "CommandParse.h"
#include "GitRepository.h"
#include "GitObject.h"
#include "utility.h"
#define DIFFLIB_ENABLE_EXTERN_MACROS
#include "difflib/difflib.h"
#include <functional>

/*
char *p = getenv("USER");
*/



const static set<wstring> ValidMode{ L"commit", L"tree", L"blob", 
									 L"size", L"type", L"pretty" };


CommandParse::CommandParse() :
	CommandList({
				{L"add",		 &CommandParse::cmd_add},
				{L"cat-file",	 &CommandParse::cmd_cat_file},
				{L"checkout",	 &CommandParse::cmd_checkout},
				{L"commit",		 &CommandParse::cmd_commit},
				{L"hash-object", &CommandParse::cmd_hash_object},
				{L"init",		 &CommandParse::cmd_init},
				{L"status",		 &CommandParse::cmd_status},
				{L"ls-files",	 &CommandParse::cmd_ls_files},
				{L"diff",		 &CommandParse::cmd_diff},
				{L"ls",			 &CommandParse::cmd_ls},
				{L"cd",			 &CommandParse::cmd_cd},
				{L"pwd",		 &CommandParse::cmd_pwd},
				{L"help",		 &CommandParse::cmd_help},
				{L"exit",		 &CommandParse::cmd_exit}
		})
{
}


/*
---CommandParse
Method: HandleCommand
Description: Handle command with given arguments
*/
string CommandParse::HandleCommand(const wstring& strcmd, GitRepository& rep)
{
	if (strcmd.empty())
		return string();

	try
	{
		this->listAttr = SplitToWStringList(strcmd, L' ');

		if (CommandList.find(*listAttr.begin()) == CommandList.end())
			throw wstring(L"Not valid command: \"" + *listAttr.begin() + L"\"!");

		auto command = listAttr.begin();

		HistoryCommand.emplace_back(*command);
		MessageToUser.clear();
#ifdef TEST 
		auto fptr = CommandList.find(*listAttr.begin());
		auto func = fptr->second;
		(this->*func)();
#endif		
		std::invoke(CommandList[*command], this, rep);

	}
	catch (const wstring & e)
	{
		throw CommandExeption(e);
	}
	catch (FileSystemExeption & e)
	{
		throw CommandExeption(e.getMessage());
	}
	catch (RepositoryException & e)
	{
		throw CommandExeption(e.getMessage());
	}
	catch (CommandExeption & ce) 
	{
		throw ce;
	}	
	catch (...)
	{
		throw CommandExeption(L"Unknow type exception");
	}
	
	string RetVal(std::move(MessageToUser));

	return RetVal;
}


/*
---CommandParse
Method: cmd_add
Description: Add all file paths to git index.
*/
void CommandParse::cmd_add(GitRepository& rep)
{
	namespace fls = std::filesystem;
	
	if (listAttr.size() != 2)
		throw CommandExeption(L"Error command!\nTry change directory to your workdirectory and let's try command:\nadd -a");

	auto Args = listAttr.begin();
	std::advance(Args, 1); // path in argument
	
	list<EntryTypes> all_entries(std::move(rep.read_index()));
	
	FileSystem fs;
	
	if (*Args != L"-a")
		throw CommandExeption(L"Invalid argument");

	path curdir = rep.getCurrentGitDir();
	
	try
	{	
		fs.setDirectory(curdir);
	}
	catch (FileSystemExeption& e){
		throw e;
	}
	catch (...) {
		throw wstring(L"Unknow Error to add command");
	}

	string pathstring{ std::move(curdir.string()) }; //ws2s(*Args)

	EntryCompare cmp;

	set<EntryTypes, EntryCompare> entries(cmp);

	if (!all_entries.empty()) { //check first writing index
		std::copy_if<>(all_entries.begin(), all_entries.end(), std::inserter(entries, entries.begin()), [&](auto entry) {
			return (pathstring != std::get<3>(entry));
			});
	}
	
	
	set<string> files(std::move(fs.getListAllStringFiles()));
	
	for (const auto& file : files) {
		
		if (file.rfind(".git") != string::npos)
			continue;

		string data{ std::move(fs.readFromFileToString(file)) };
		string sha1{ std::move(GitObject::Object_hash(rep, "blob", data)) };

		fls::perms p = fls::status(file).permissions();

		auto ftime = fls::last_write_time(file);
		std::time_t cftime = to_time_t(ftime);

		struct tm times = *localtime(&cftime);
		
		size_t uid = static_cast<size_t>(p & fls::perms::set_uid); // not used in Windows OS
		size_t gid = static_cast<size_t>(p & fls::perms::set_gid); // not used in Windows OS

		size_t mode = static_cast<size_t>(p & fls::perms::owner_read & fls::perms::owner_write & fls::perms::owner_exec);

		us flags = static_cast<us>(file.size());
		
		EntryTypes IndexEntry {
		{ {"ctime_s", 0u}, {"ctime_n", 0u}, {"mtime_s", times.tm_sec},
		{"mtime_n", times.tm_min}, {"dev", 0u}, {"ino", 0u},
		{"mode", 0u}, {"uid", uid}, {"gid", gid}, {"size", fls::file_size(file)/(1024*1024)} }, // map	
		{sha1},	// sha1
		flags,	// flag
		{file}	// path
		};

		
		entries.insert(IndexEntry);
	}

	try
	{
		rep.write_index(entries);
	}
	catch (FileSystemExeption& fe)
	{
		throw fe;
	}
	catch (...)
	{
		throw wstring(L"Unknow Error to add command");
	}

	MessageToUser.assign("Add object complete!");
}



/*
---CommandParse
Method: cmd_cat_file
Description: Write the contents of (or info about) object with given SHA-1 prefix to
		Message object. If mode is "commit", "tree", or "blob", return message of
		object. If mode is "size", return the size of the object. If mode is
		"type", return the type of the object. If mode is "pretty", message a
		prettified version of the object.
*/
void CommandParse::cmd_cat_file(GitRepository& rep)
{
	if (listAttr.size() != 4)
		throw CommandExeption(L"Error command!\nTry change directory to your workdirectory and let's try command:\ncat-file -t [MODE] HASH-PREFIX");

	Message<wstring> message(L"");
	if (!valid_cmd_catfile(listAttr, message)) {
		throw CommandExeption(message.getMessage());
	}

	
	auto Itcmd = listAttr.begin();
	std::advance(Itcmd, 2);	// mode type after -t
	wstring mode = *Itcmd;

	std::advance(Itcmd, 1); // sha-prefix

	auto infoobj = GitObject::Object_read(rep, std::move(ws2s(*Itcmd)));

	auto objtype = std::get<0>(infoobj);
	string msg;
	if (mode == L"blob" || mode == L"tree" || mode == L"commit") {
		if (mode != s2ws(objtype)) {
			throw CommandExeption(L"expected object another type: " + mode);
		}
		msg.assign(std::get<1>(infoobj));
	}
	else if (mode == L"size")
		msg.assign(std::to_string(std::get<1>(infoobj).size()));
	else if (mode == L"type")
		msg.assign(objtype);
	else if (mode == L"pretty") {
	
		if (objtype == "commit" || objtype == "blob") {
			msg.assign( std::get<1>(infoobj) );
		}
		else if (objtype == "tree") { // Not support
			
#ifdef OFF
			list<tuple<size_t, string, string>> entries(std::move(rep.read_tree("", std::get<1>(infoobj))));
			for (const auto& entry : entries) {
			}
#endif
			msg.assign("Operation with tree not supported");
		}
		else
			throw RepositoryException(L"unhandling object type");
	}

	MessageToUser.assign("Result cat-file: " + msg);
}

/*
---CommandParse
Method: cmd_checkout
Description: not implemented
*/
void CommandParse::cmd_checkout(GitRepository& rep)
{
}

/*
---CommandParse
Method: cmd_commit
Description: Commit the current state of the index to master with given message.
*/
void CommandParse::cmd_commit(GitRepository& rep)
{
	
	Message<wstring> message(L"");
	string MsgCommit;

	if (!valid_cmd_commit(listAttr, message, MsgCommit)) {
		throw CommandExeption(message.getMessage());
	}
	
	string tree(std::move(rep.write_tree()));

	string parent(std::move(rep.get_local_master_hash()));



#ifdef __WIN32__
	const size_t buff_Size = 128;
	char username[buff_Size];
	DWORD Len = buff_Size;
	BOOL res = GetUserNameA(username, &Len);
	if (res == FALSE) {
		return; // error
	}
#elif defined __UNIX__
	const size_t buff_Size = 128;
	char username[buff_Size];
	int res = getlogin_r(username, buff_Size);
	if (res == 0)
		return; //Error
#else
#error("Error OS")
#endif
	string author(username);

	time_t rawtime; struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	string author_time(std::to_string(timeinfo->tm_hour) + ":" + std::to_string(timeinfo->tm_min) + ":" + std::to_string(timeinfo->tm_sec));

	
	string lines{ "tree " + tree };
	if (!parent.empty())
		lines.append("\nparent " + parent);

	lines.append("\nauthor " + author + " " + author_time);
	lines.append("\ncommiter " + author + " " + author_time);
	lines.append("\n" + MsgCommit);
	lines.append("\n");
	string sha(std::move(GitObject::Object_hash(rep, "commit", lines)));
	path master_path(rep.getCurrentGitDir() + L"/" + rep.gitdir.folder + L"/" + rep.gitdir.refs + L"/" + rep.gitdir.heads + L"/" + L"master");
	try
	{
		rep.fs.createFileAndWrite(master_path, { sha + "\n" });
	}
	catch (FileSystemExeption& fe){
		throw fe;
	}
	catch (...) {
		throw wstring(L"Unknow Error to commit command");
	}


	MessageToUser.assign("Commit given work directory is complete");
}

/*
---CommandParse
Method: cmd_hash_object
Description: Compute hash of object data of given type and write to object store if
				"write" is True. Return SHA-1 object hash as hex string.
*/
void CommandParse::cmd_hash_object(GitRepository& rep)
{
	if (listAttr.size() != 4)
		throw CommandExeption(L"Error command!\nTry change directory to your workdirectory and let's try command:\nhash-object -[w|r] [TYPE] [FILE]");

	Message<wstring> message(L"");
	if (!valid_cmd_hash(listAttr, message, rep)) {
		throw CommandExeption(message.getMessage());
	}

	auto Args = listAttr.begin();
	bool writeFlag = false;
	std::advance(Args, 1); // arguments after command

	// flag
	if (*Args == L"-w")
		writeFlag = true;

	// type
	std::advance(Args, 1);
	wstring type{ *Args };

	// file
	std::advance(Args, 1);

	FileSystem fs;
	fs.setDirectory(rep.getCurrentGitDir());

	string data{ fs.readFromFileToString(L"", *Args) };

	string sha_1{ GitObject::Object_hash(rep, ws2s(type), data, writeFlag) };


	MessageToUser.assign("Hash object: " + sha_1);
}

/*
---CommandParse
Method: cmd_init
Description: Create directory for repo and initialize .git directory.
*/
void CommandParse::cmd_init(GitRepository& rep)
{
	if (listAttr.size() != 1)
		throw CommandExeption(L"Error command!\nTry change directory to your workdirectory and let's try command - \"init\"");

	wstring dir{ rep.getCurrentGitDir() }; 
	
	// init git
	try
	{
		if (!rep.fs.createFolder(rep.gitdir.folder, true))					// ".git"
			return;
		
		rep.fs.setDirectory(dir + L"/" + rep.gitdir.folder);

		rep.fs.createFile(rep.gitdir.config, L"");							// "config.ini"
		rep.fs.createFolder(rep.gitdir.branches);							// "branches"
		rep.fs.createFolder(rep.gitdir.objects);							// "objects"
		rep.fs.createFolder(rep.gitdir.refs);								// "refs"
		rep.fs.createFolder(rep.gitdir.refs + L"/" + rep.gitdir.tags);      // "refs/tags"
		rep.fs.createFolder(rep.gitdir.refs + L"/" + rep.gitdir.heads);     // "refs/heads"


		rep.fs.createFileAndWrite(rep.gitdir.description, L"", L"Unnamed repository; edit this file 'description' to name the repository.\n");  //  "description"
		rep.fs.createFileAndWrite(rep.gitdir.HEAD, L"", L"ref: refs/heads/master\n");


		rep.cfgrep.corerep.bare = false;
		rep.cfgrep.corerep.filemode = false;
		rep.cfgrep.corerep.repformatversion = 0;

		rep.writeConfig(rep.fs.getDirectory(), rep.gitdir.config);

		rep.setCurrentGitDir(dir);
	}
	catch (FileSystemExeption & e)
	{
		throw e;
	}
	catch (...) {
		throw wstring(L"Unknow Error to init command");
	}

	

	MessageToUser.assign("Initialize a new empty repository complete.");
}


/*
---CommandParse
Method: cmd_status
Description: Show status of working copy.
*/
void CommandParse::cmd_status(GitRepository& rep)
{
	StatusTypes stat;
	try
	{
		stat = std::move(rep.GetStatus());
	}
	catch (const wstring& msg)
	{
		throw msg;
	}
	catch (FileSystemExeption & fe) {
		throw fe;
	}
	catch (RepositoryException & re) {
		throw re;
	}
	catch (...) {
		throw wstring(L"Unknow Error to status command");
	}
	
	
	MessageToUser.assign("Changed:\n");
	for (auto& changed : std::get<0>(stat)) {
		MessageToUser.append(changed + "\n");
	}		
	MessageToUser.append("New:\n");
	for (auto& changed : std::get<1>(stat)) {
		MessageToUser.append(changed + "\n");
	}	
	MessageToUser.append("Deleted:\n");
	for (auto& changed : std::get<2>(stat)) {
		MessageToUser.append(changed + "\n");
	}		
}


/*
---CommandParse
Method: cmd_ls_files
Description: Print list of files in index (including mode, SHA-1, and stage number
			if "details" is True).
*/
void CommandParse::cmd_ls_files(GitRepository& rep)
{
	Message<wstring> message(L"");

	if (!valid_cmd_lsfiles(listAttr, message)) {
		throw CommandExeption(message.getMessage());
	}

	
	auto Args = listAttr.begin();
	bool DetailsFlag = false;
	if (listAttr.size() > 1) {
		if (*Args == L"-s") {
			DetailsFlag = true;
			std::advance(Args, 1);
		}
	}
			
	list<EntryTypes> DataEntreis;
	try
	{
		DataEntreis = (std::move(rep.read_index()));
	}
	catch (RepositoryException & re){
		throw re;
	}
	catch (FileSystemExeption & fe) {
		throw fe;
	}
	catch (...) {
		throw wstring(L"Unknow Error to ls-files command");
	}

	stringstream strbuff;
	for (auto data : DataEntreis) {
		if (DetailsFlag) {
			ul stage = (std::get<2>(data) >> 12) & 3; // stage number
			strbuff << std::left << std::get<0>(data)["mode"] << "  " <<
				std::get<1>(data) << "  " << stage << "\t" << std::get<3>(data) << endl; // output << mode << sha-1 << flags << path
		}
		else {
			strbuff << std::get<3>(data) << endl; // path
		}
	}

	MessageToUser.assign(strbuff.str());
}

/*
---CommandParse
Method: cmd_diff
Description: Show diff of files changed (between index and working copy).
*/
void CommandParse::cmd_diff(GitRepository& rep)
{
	
	FileSystem fs;
	StatusTypes stat;
	try
	{
		stat = std::move(rep.GetStatus());
	}
	catch (const wstring & msg)
	{
		throw msg;
	}

	list<EntryTypes> entries;
	try
	{
		entries = std::move(rep.read_index());
	}
	catch (RepositoryException & re)
	{
		throw re;
	}

	map<string, EntryTypes> entries_by_path;

	for (auto& ItEntry : entries) {
		entries_by_path[std::get<3>(ItEntry)] = std::move(ItEntry);
	}

	list<string> difflines;
	size_t numerate = 0;
	for (auto& path : std::get<0>(stat)) {	// changed
		string sha1 = std::move(std::get<1>(entries_by_path[path]));	//get SHA-1 value

		tuple<string, string> ObjTData = std::move(GitObject::Object_read(rep, sha1));
		
		if (std::get<0>(ObjTData) != "blob") {
			throw wstring(L"Invalid type object given path in repository");
		}

		string working_lines = std::move(fs.readFromFileToString(path));

		size_t _after = working_lines.size();
		size_t _before = std::get<1>(ObjTData).size();

		

		string differ(std::move(diff_control(std::get<1>(ObjTData), working_lines)));
		
		if (differ.empty())
			continue;

		difflines.emplace_back(std::move(differ));
	}


	for (const auto& ItDiff : difflines) {
		MessageToUser.append(ItDiff);
	}
}


/*
---CommandParse
Method: cmd_cd
Description: Change currenrt workdirectory
*/
void CommandParse::cmd_cd(GitRepository& rep)
{
	if (listAttr.size() < 2 || listAttr.size() > 2)
		throw CommandExeption(L"Error command!\nTry for change directory try command - \"cd [PATH]\"");

	auto Args = listAttr.begin(); 
	std::advance(Args, 1); 
	

	path curdir = rep.getCurrentGitDir();

	if (*Args == L"..") {
		size_t pos = curdir.wstring().rfind(L"\\");
		if (pos == string::npos || pos == 2) // root dir
			return;
		curdir = curdir.wstring().substr(0, curdir.wstring().rfind(L"\\", pos - 1));
	}
	else if (Args->rfind(L"\\") == string::npos) {
		curdir /= *Args;
		fs::file_status ft(status(curdir));
		auto type = ft.type();
		if (type != fs::file_type::directory)
			throw wstring(L"Given path is not directory.");;
	}	
	else {
		size_t root = Args->find(L"\\");
		if (root == 2 && Args->find(L":")) // root dir
			curdir = *Args;
		else 
			curdir /= *Args;		
	}
	
	if (!exists(curdir))
		throw wstring(L"Given path is not exists.");
	rep.setCurrentGitDir(curdir);


	MessageToUser.assign("Now current path: " + curdir.string());
}

/*
---CommandParse
Method: cmd_ls
Description: Return list of file and dir in current workdir
*/
void CommandParse::cmd_ls(GitRepository& rep)
{
	list<string> files(std::move(rep.fs.getListStringFiles(true)));

	if (files.empty()) {
		MessageToUser.assign("");
		return;
	}
		

	for (const auto& file : files) {
		MessageToUser.append("-" + file);
		MessageToUser.append("\n");
	}
}

/*
---CommandParse
Method: cmd_pwd
Description: Return current workdir
*/
void CommandParse::cmd_pwd(GitRepository& rep)
{
	if (rep.getCurrentGitDir().empty()) {
		MessageToUser.assign("");
		return;
	}

	MessageToUser.assign("Current workdir: " + ws2s(rep.getCurrentGitDir()));
}

/*
---CommandParse
Method: cmd_help
Description: Return list of support command
*/
void CommandParse::cmd_help(GitRepository& rep)
{
	MessageToUser.assign("--Help--\n");

	MessageToUser.append("List of command and arguments:\n");

	MessageToUser.append("1. add -a\n");

	MessageToUser.append("2. cat-file -t [MODE] HASH-PREFIX\n");

	MessageToUser.append("3. checkout not supported!\n");

	MessageToUser.append("4. commit -m [MESSAGE]\n");

	MessageToUser.append("5. hash-object -[w|r] [TYPE] [FILE]\n");

	MessageToUser.append("6. init\n");

	MessageToUser.append("7. status\n");

	MessageToUser.append("8. ls-files -s\n");

	MessageToUser.append("9. diff\n");

	MessageToUser.append("10. cd [PATH]\n");

	MessageToUser.append("11. pwd\n");

	MessageToUser.append("12. exit\n");
}

/*
---CommandParse
Method: cmd_exit
Description: Exit programm
*/
void CommandParse::cmd_exit(GitRepository& rep)
{
	if (listAttr.size() > 1) {
		throw CommandExeption(L"Invalid number of agruments");
	}
	
	constexpr size_t one_SEC = 1000;
	constexpr size_t two_SEC = one_SEC * 2; // 2 second
	
	wcout << "\t\t\tGoodbye..." << endl;
 
	Sleep(one_SEC);

	exit(EXIT_SUCCESS);
}



// Helper method:
bool CommandParse::valid_cmd_hash(const list<wstring>& listAttr, Message<wstring>& message, const GitRepository& rep)
{
	auto Args = listAttr.begin();

	std::advance(Args, 1); // arguments after command

	// valid flag
	if (*Args != L"-w" && *Args != L"-r") {
		message.setMessage(L"Invalid given flag: \"" + *Args + L"\"");
		return false;
	}

	// valid type
	std::advance(Args, 1);
	if (*Args != L"blob" && *Args != L"commit" && *Args != L"tree") {
		message.setMessage(L"Invalid given type of object: \"" + *Args + L"\"");
		return false;
	}

	// valid file
	std::advance(Args, 1);
	path file(rep.getCurrentGitDir());
	file /= *Args;
	if (!exists(file)) {
		message.setMessage(L"Given file doesn't exist: \"" + *Args + L"\"");
		return false;
	}

	return true;
}

bool CommandParse::valid_cmd_catfile(const list<wstring>& listAttr, Message<wstring>& message)
{
	auto Args = listAttr.begin();

	std::advance(Args, 1); // arguments after command

	// valid mode
	if (*Args != L"-t") {
		message.setMessage(L"Invalid given type of flag: \"" + *Args + L"\"");
		return false;
	}
	std::advance(Args, 1);

	auto isValid = ValidMode.find(*Args);
	if (isValid == ValidMode.end()) {
		message.setMessage(L"Invalid given type of mode: \"" + *Args + L"\"");
		return false;
	}

	std::advance(Args, 1);
	if ((*Args).size() < 2) {
		message.setMessage(L"Size given sha-prefix: \"" + *Args + L"\" should be more than 2");
		return false;
	}

	return true;
}

bool CommandParse::valid_cmd_lsfiles(const list<wstring>& listAttr, Message<wstring>& message)
{
	auto Args = listAttr.begin();

	if (listAttr.size() > 2) {
		message.setMessage(L"Invalid number arguments. ");
		return false;
	}

	std::advance(Args, 1); // arguments after command
	if (listAttr.size() > 1) {
		if (*Args != L"-s") {
			message.setMessage(L"Invalid given flag mode: \"" + *Args + L"\"");
			return false;
		}	
	}

	return true;
}

bool CommandParse::valid_cmd_commit(const list<wstring>& listAttr, Message<wstring>& message, string& data)
{
	auto Args = listAttr.begin();

	if (listAttr.size() < 2) {
		message.setMessage(L"Invalid given number arguments");
		return false;
	}

	if (listAttr.size() > 2) {
		std::advance(Args, 1); // arguments after command
		// valid mode
		if (*Args == L"-m") {
			{
				std::advance(Args, 1);
				for (size_t it = 3; it <= listAttr.size(); ++it) {
					data.append(ws2s(*Args)); // assign message
					data.append(" ");
					std::advance(Args, 1);
				}
			}

		}
		else {
			message.setMessage(L"Invalid given flag in arguments: \"" + *Args + L"\"");
			return false;
		}
	}
	
	return true;
}

string CommandParse::diff_control(const string& str1, const string& str2)
{

	size_t _after = str2.size();

	size_t _before = str1.size();

	auto matches = difflib::MakeSequenceMatcher<>(str2, str1); // before , after

	auto block = matches.get_matching_blocks();

	string differ{ "Difference with:\n" };

	if (abs(matches.ratio() - 1.) < 0.0005) {		// ration "equal" 1. 
		return string();
	}
	

	string _diff1{ str1 };
	string _diff2{ str2 };

	size_t count = 0;
	for (const auto& tdata : block) {
		++count;
		if (count == block.size()) // last unnecessary operation(specific on difflib)
			break;
		size_t start1 = 0, start2 = 0, length = 0;
		std::tie(start1, start2, length) = tdata;
		_diff2.insert(start1, "( ");
		_diff2.insert(length + 1, " )");
		_diff1.insert(start2, "( ");
		_diff1.insert(length + 1, " )");

	}

	if (_after > _before) {
		differ.append("- ");
		differ.append(_diff1);
		differ.append("\n");
		differ.append("+ ");
		differ.append(_diff2);
	}
	else {
		differ.append("+ ");
		differ.append(_diff1);
		differ.append("\n");
		differ.append("- ");
		differ.append(_diff2);
	}

	replaceAll(differ, "\r", "\n"); // fix bug 

	differ.append("========================");

	return differ;
}
