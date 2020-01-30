#include "stdafx.h"
#include "FileSystem.h"
#include "Exception.h"
#include "utility.h"


#ifdef OFF
	static const wchar_t* HomeDirectory = L"C:\\Users\\Public";
	static const char* Home = getenv("HOME");
#endif



#ifdef __WIN32__
wstring getHomedir() {

		WCHAR homedir[256];

		if (SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, homedir) != S_OK) {
			return wstring();
		}
#ifdef NOT
		snprintf(homedir, 128, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
		if (homedir == NULL)
			return string();
#endif
		return wstring(homedir);
	}
#elif defined __UNIX__
string getHomedir() {
		char* homedir = getenv("HOME");

		if (homedir == NULL)
			return string();
		/*
		uid_t uid = getuid();
		uid_t uid = getuid();
		struct passwd* pw = getpwuid(uid);

		if (pw == NULL) {
			return string();
		}
		string hmdir = pw->pw_dir;
		*/
		return string(homedir);
	}
#endif




FileSystem::FileSystem() : CurrentWorkDir(current_path().wstring()), change_list(false)
{

}


void FileSystem::construct_list_wstring_files(const path & dir, set<wstring>& files) noexcept
{	

	// iterate all object in directory
	for (const auto& entry : directory_iterator(dir, std::filesystem::directory_options::skip_permission_denied)) {

		fs::file_status ft(status(entry));
		auto type = ft.type();

		if (type == fs::file_type::directory)
			construct_list_wstring_files(entry, files);
		else if (type == fs::file_type::fifo || type == fs::file_type::socket || type == fs::file_type::unknown)
			continue;
		else {
			files.insert(canonical(entry.path()).wstring());
		}	
	}
}


void FileSystem::construct_list_string_files(const path& dir, set<string>& files) noexcept
{

	
	for (const auto& entry : directory_iterator(dir, std::filesystem::directory_options::skip_permission_denied)) {

		fs::file_status ft(status(entry));
		auto type = ft.type();

		if (type == fs::file_type::directory)
			construct_list_string_files(entry, files);
		else if (type == fs::file_type::fifo || type == fs::file_type::socket || type == fs::file_type::unknown)
			continue;
		else {
			files.insert(canonical(entry.path()).string());
		}
	}
}

#ifdef TEST
char FileSystem::type_char(file_status fs) noexcept
{
	if		(is_directory(fs))		{ return 'd'; }
	else if (is_symlink(fs))		{ return 'l'; }
	else if (is_character_file(fs)) { return 'c'; }
	else if (is_block_file(fs))		{ return 'b'; }
	else if (is_fifo(fs))			{ return 'p'; }
	else if (is_socket(fs))			{ return 's'; }
	else if (is_other(fs))			{ return 'o'; }
	else if (is_regular_file(fs))	{ return 'f'; }
	return '?';
}
#endif


wstring FileSystem::getDirectory() const
{

	if (CurrentWorkDir.empty())
		throw FileSystemExeption(L"can't get file directory");

	return std::move(wstring{ CurrentWorkDir });

}

set<wstring> FileSystem::getListAllWstringFiles()
{

	if (CurrentWorkDir.empty())
		throw FileSystemExeption(L"Error no valid name directory");

	static set<wstring> files;

	if (!change_list) {

		path Directory = CurrentWorkDir;

		if (!exists(Directory))
			throw FileSystemExeption(L"Error directory no exists");

	
		this->construct_list_wstring_files(Directory, files);
		
		change_list = true;
	}
	
	return std::move(files);
}

set<string> FileSystem::getListAllStringFiles()
{

	if (CurrentWorkDir.empty())
		throw FileSystemExeption(L"Error no valid name directory");

	static set<string> files;

	if (!change_list) {

		path Directory = CurrentWorkDir;

		if (!exists(Directory))
			throw FileSystemExeption(L"Error directory no exists");


		this->construct_list_string_files(Directory, files);

		change_list = true;
	}

	return std::move(files);
}


list<wstring> FileSystem::getListWStringFiles(bool withoutpath)
{

	if (CurrentWorkDir.empty())
		throw FileSystemExeption(L"Error no valid name directory");

	list<wstring> lists_obj;

	path Directory = CurrentWorkDir;
	
	
	for (const auto& entry : directory_iterator(Directory, std::filesystem::directory_options::skip_permission_denied)) {
		if (withoutpath)
			lists_obj.emplace_back(entry.path().filename().wstring());
		else
			lists_obj.emplace_back(canonical(entry.path()).wstring());
	}

	return std::move(lists_obj);

}

list<string> FileSystem::getListStringFiles(bool withoutpath)
{
	if (CurrentWorkDir.empty())
		throw FileSystemExeption(L"Error no valid name directory");

	list<string> lists_obj;

	path Directory = CurrentWorkDir;

	
	for (const auto& entry : directory_iterator(Directory, std::filesystem::directory_options::skip_permission_denied)) {
		if (withoutpath)
			lists_obj.emplace_back(entry.path().filename().string());
		else
			lists_obj.emplace_back(canonical(entry.path()).string());
	}

	return lists_obj;
}

bool FileSystem::createFolder(const wstring& folder, bool hide)
{

	path Directory = CurrentWorkDir;

	if (!exists(Directory))			// check directory is exists
		throw FileSystemExeption(L"Error directory no exists");

	Directory /= folder;
	if (!exists(Directory)) { // create folder is not exists
		
		if (!create_directory(Directory)) //  std::filesystem
			return false;
		if (hide) {	
#ifdef __WIN32__
		SetFileAttributes(Directory.c_str(), FILE_ATTRIBUTE_HIDDEN);		
#endif
		}
		return true;
	}
	return false;

}

bool FileSystem::createFolder(const string& folder, bool hide)
{
	path Directory = CurrentWorkDir;

	if (!exists(Directory))			// check directory is exists
		throw FileSystemExeption(L"Error directory no exists");

	Directory /= folder;
	if (!exists(Directory)) { // create folder is not exists

		if (!create_directory(Directory)) //  std::filesystem
			return false;
		if (hide) {
#ifdef __WIN32__
			SetFileAttributes(Directory.c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
		}
		return true;
	}
	return false;
}

void FileSystem::createFile(const wstring& file, const wstring& folder)
{
	path Directory = this->CurrentWorkDir;
	if (!folder.empty())
		Directory /= folder;
	if (!file.empty())
		Directory /= file;

	
	std::wofstream fs{ Directory, std::ios::binary };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't create file");

	fs.close();

}

void FileSystem::createFile(const string& file, const string& folder)
{
	path Directory = this->CurrentWorkDir;
	if (!folder.empty())
		Directory /= folder;
	if (!file.empty())
		Directory /= file;

	
	std::ofstream fs{ Directory, std::ios::binary };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't create file");

	fs.close();
}

void FileSystem::createFileAndWrite(const wstring& file, const wstring& folder, const wstring& data)
{
	path Directory = this->CurrentWorkDir;
	if (!folder.empty())
		Directory /= folder;
	if (!file.empty())
		Directory /= file;

	
	wfstream fs{ Directory, std::ios::binary |  std::ios::out };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't create file");
	
	fs << data;

	fs.close();
}

void FileSystem::createFileAndWrite(const string& file, const string& folder, const string& data)
{
	path Directory = this->CurrentWorkDir;
	if (!folder.empty())
		Directory /= folder;
	if (!file.empty())
		Directory /= file;

	
	fstream fs{ Directory, std::ios::binary | std::ios::out };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't create file");

	fs << data;

	fs.close();
}

void FileSystem::createFileAndWrite(const path& fullfilepath, const string& data)
{
	path Directory = fullfilepath;


	fstream fs{ Directory, std::ios::binary | std::ios::out };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't create file");

	fs << data;

	fs.close();
}





void FileSystem::writeFile(const wstring & file, const wstring & folder, const wstring & data)
{
	path Directory = this->CurrentWorkDir;
	if (!folder.empty())
		Directory /= folder;
	if (!file.empty())
		Directory /= file;

	if (!exists(Directory))
		throw FileSystemExeption(L"Error given not exists!");

	wfstream fs{ Directory, std::ios::binary | std::ios::out };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't create file");

	fs << data;

	fs.close();
}

void FileSystem::writeFile(const string& file, const string& folder, const string& data)
{
	path Directory = this->CurrentWorkDir;
	if (!folder.empty())
		Directory /= folder;
	if (!file.empty())
		Directory /= file;

	if (!exists(Directory))
		throw FileSystemExeption(L"Error given not exists!");

	fstream fs{ Directory, std::ios::binary | std::ios::out };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't create file");

	fs << data;

	fs.close();
}

void FileSystem::writeFile(const path& fullfilepath, const string& data)
{
	path Directory = fullfilepath;

	if (!exists(Directory))
		throw FileSystemExeption(L"Error given not exists!");

	fstream fs{ Directory, std::ios::binary | std::ios::out };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't create file");

	fs << data;

	fs.close();
}

list<string> FileSystem::readFromFileToList(const wstring& folder, const wstring& file)
{
	path dir = CurrentWorkDir;
	if (!folder.empty())
		dir /= folder;
	if (!file.empty())
		dir /= file;


	if (!exists(dir))
		throw FileSystemExeption(L"Error given not exists!");

	ifstream fs{ dir, std::ios::binary };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't open file");

	string buff;
	list<string> liststr;

	while (std::getline(fs, buff)) {
		liststr.push_back(buff);
	}


	return std::move(liststr);
}

string FileSystem::readFromFileToString(const wstring& folder, const wstring& file)
{
	path dir = CurrentWorkDir;
	if (!folder.empty())
		dir /= folder;
	if (!file.empty())
		dir /= file;


	if (!exists(dir))
		throw FileSystemExeption(L"Error given not exists!");

	ifstream fs{ dir, std::ios::binary };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't open file");

	string buff;
	string longstring;	
	while (std::getline(fs, buff)) {
		longstring.append(buff);
	}

	return std::move(longstring);
}

string FileSystem::readFromFileToString(const string& folder, const string& file)
{
	path dir = CurrentWorkDir;
	if (!folder.empty())
		dir /= folder;
	if (!file.empty())
		dir /= file;

	if (!exists(dir))
		throw FileSystemExeption(L"Error given not exists!");

	ifstream fs{ dir, std::ios::binary };


	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't open file");

	string buff;
	string longstring;

	while (std::getline(fs, buff)) {
		longstring.append(buff);
	}


	return std::move(longstring);
}

string FileSystem::readFromFileToString(const path& fullfilepath)
{
	if (fullfilepath.empty())
		return string();

	path dir = fullfilepath;
	

	if (!exists(dir))
		throw FileSystemExeption(L"Error given not exists!");

	ifstream fs{ dir, std::ios::binary };

	if (!fs.is_open())
		throw FileSystemExeption(L"Error can't open file");

	string buff;
	string longstring;

	while (std::getline(fs, buff)) {
		longstring.append(buff);
	}


	return std::move(longstring);
}



string FileSystem::transformListToString(const list<string>& liststr) noexcept
{
	size_t stringsize = 0;
	for (auto str : liststr) {
		stringsize += str.size();
	}
	if (stringsize == 0) {
		return "";
	}

	stringsize += 1; // size + 1

	string buffer;
	buffer.resize(stringsize); // allocate full string
		

	size_t idx = 0;
	for (auto str : liststr) {
		size_t size = str.size();
		std::copy(str.begin(), str.end(), buffer.begin() + idx);
		idx += size;
	}


	return std::move(buffer);
}

void FileSystem::setDirectory(const wstring& str)
{
	if (str.empty())
		throw FileSystemExeption(L"Error empty directory name");

	this->CurrentWorkDir = str;

	this->change_list = false;
}

void FileSystem::setDirectory(const string& str)
{
	if (str.empty())
		throw FileSystemExeption(L"Error empty directory name");

	this->CurrentWorkDir = std::move(s2ws(str));

	this->change_list = false;
}

void FileSystem::setDirectory(const path& str)
{
	if (str.empty())
		throw FileSystemExeption(L"Error empty directory name");

	this->CurrentWorkDir = str;

	this->change_list = false;
}
