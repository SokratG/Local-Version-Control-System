/*
* Author: Gasparyan Sokrat
* Date: 2020/01/07
* GitRepository.cpp
* Module implement GitRepository function member,
* which handle local system version control functionality.
*/
#include "stdafx.h"
#include "GitRepository.h"
#include "GitObject.h"
#include <functional>
#include "utility.h"

using std::wstringstream;




using header_index = struct {
	const string signature{ "DIRC" };
	const size_t version = 2;
	size_t num_entries;
};

//==============================================
static string&& entry_extract(const tuple<map<string, size_t>, string, ul, string>&);
static tuple<string, size_t, size_t> HeaderUnpack(const string&);
static string EntryPack(const EntryTypes& entry);
static EntryTypes EntriesUnpack(const string& entry_data, const size_t fields_end_index);
static string HeaderPack(const header_index& header);
//==============================================


GitRepository::GitRepository()
{
}

/*
---GitRepository
Method: writeConfig
Description: Write config file with configure in .git dir
*/
bool GitRepository::writeConfig(const wstring_view& dir, const wstring& file)
{
	path cfgdir = dir;
	cfgdir /= file;

	if (!exists(cfgdir))
		return false;

	wfstream fss{ cfgdir, std::ios::binary | std::ios::out };


	if (!fss.is_open()) {
		return false;
	}

	fss << L".core" << endl;
	fss << L"repositoryformatversion" << L" " << this->cfgrep.corerep.repformatversion << endl;
	fss << L"filemode" << L" " << this->cfgrep.corerep.filemode << endl;
	fss << L"bare" << L" " << this->cfgrep.corerep.bare << endl;

	fss.close();

	if (fss.is_open())
		return false;

	return true;
}

/*
---GitRepository
Method: readConfig
Description: Read config file with configure in .git dir
*/
bool GitRepository::readConfig(const wstring_view & dir, const wstring & file)
{
	path cfgdir = dir;
	cfgdir /= file;

	if (!exists(cfgdir))
		return false;

	wfstream fss{ cfgdir, std::ios::binary | std::ios::in };
	wstring line;

	if (!fss.is_open()) {
		return false;
	}
	wstring tempbuf;
	std::getline(fss, line); // core
	this->cfgrep.corename = line;

	std::getline(fss, line); // formatversion
	wstringstream ws{ line };
	ws >> tempbuf >> this->cfgrep.corerep.repformatversion;
	ws.clear();


	std::getline(fss, line); // filemode
	ws.str(line);
	ws >> tempbuf >> this->cfgrep.corerep.filemode;
	ws.clear();

	std::getline(fss, line); // bare
	ws.str(line);
	ws >> tempbuf >> this->cfgrep.corerep.bare;
	

	if (!ws.end || std::getline(fss, line))
		return false;

	fss.close();

	if (fss.is_open())
		return false;

	return true;
}


tuple<string, size_t, size_t> HeaderUnpack(const string& entry_data)
{
	// only 12 bytes for data
	constexpr size_t offset = sizeof(ul); // length of signature

	auto ItData = entry_data.begin();

	string signature{ ItData, ItData + offset };
	std::advance(ItData, offset);

	string version_str{ ItData, ItData + offset };
	size_t version = std::stoul(version_str);
	std::advance(ItData, offset);

	string num_entries_str{ ItData, ItData + offset };
	size_t num_entries = std::stoul(num_entries_str, 0, 16);

	return { signature, version, num_entries };
}


EntryTypes EntriesUnpack(const string& entry_data, const size_t fields_end_index)
{

	constexpr size_t hex = 16;	//hex
	constexpr size_t block_size = sizeof(ul); // 4 bytes
	//constexpr size_t sha1_size = 20; //sha1 size bytes
	constexpr size_t sha1_size = 40; //sha1 size bytes
	auto ItData = entry_data.begin();
	map<string, size_t> meta_data;

	// try optimize!!
	meta_data["ctime_s"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	meta_data["ctime_n"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);
	
	meta_data["mtime_s"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	meta_data["mtime_n"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	meta_data["dev"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	meta_data["ino"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	meta_data["mode"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	meta_data["uid"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	meta_data["gid"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	meta_data["size"] = stoul(string{ ItData, ItData + block_size }, 0, hex);
	std::advance(ItData, block_size);

	string sha1{ ItData , ItData + sha1_size };
	std::advance(ItData, sha1_size);

	us flags = static_cast<us>(stoul(string{ ItData, ItData + 2 }, 0, hex)); // unsigned short

	//size_t path_end = entry_data.find(_SYM_, fields_end_index);

	//assert(path_end != string::npos);

	string path{ entry_data.begin() + fields_end_index, entry_data.end()};


	return { meta_data, sha1, flags, path };
}


/*
---GitRepository
Method: read_index
Description: Read git index file and return list of IndexEntry objects.
*/
list<EntryTypes> GitRepository::read_index()
{

	constexpr size_t offset = 40; //offset bytes
	constexpr size_t header_offset = 12;
	string data;
	SHA1 sha;
	list<EntryTypes> entries;
	try {
		path _path(std::move(fs.getDirectory()));
		_path /= gitdir.folder + L"/index";

		if (!exists(_path))
			return list<EntryTypes>(); //first to index

		data.assign(std::move(fs.readFromFileToString(L"", _path)));

		sha.update({ data.begin(), data.end() - offset });	// last 40 bytes it's SHA-1 code

		string hexdigest{ std::move(sha.final()) };


		tuple<string, size_t, size_t> header{ std::move(HeaderUnpack({ data.begin(), data.begin() + header_offset})) };


		if (std::get<0>(header) != "DIRC")
			throw RepositoryException(L"invalid index signature");

		if (std::get<1>(header) != 2)
			throw RepositoryException(L"invalid index version");

		string entry_data{ data.begin() + header_offset, data.end() - offset }; // [12 : -40]

		// 62 bytes for entry
		size_t it = 0; const size_t block = 82, size_entry = entry_data.size();
		
		

		while ((it + block) < size_entry) {

			size_t field_end = it + block;
		
			size_t endOneEntry = entry_data.find("\xff", it);

			string entry(entry_data.begin() + it, entry_data.begin() + endOneEntry); 


			EntryTypes fields{ std::move(EntriesUnpack({ entry }, block)) };
			
			entries.push_back(fields);

			size_t s = std::get<3>(fields).size();

			size_t entry_len = (static_cast<size_t>((block + std::get<3>(fields).size() + 8)) / 8) * 8;

			it += entry_len;
		}

		if (entries.size() != std::get<2>(header)) // must equals length of entries in header meta info
			throw RepositoryException(L"corruption length data");
	}
	catch (FileSystemExeption & e) {
		throw e;
	}
	catch (...) {
		wcout << L"FATAL ERROR: Read index!" << endl;
		exit(1);
	}

	return std::move(entries);
}


string EntryPack(const EntryTypes& entry) 
{
	string data;
	stringstream stream;
	auto mapEntry = std::get<0>(entry);
	for (auto It : mapEntry) {
		data.append(I2HEX<size_t, sizeof(size_t)>(It.second));
	}

	data.append(std::get<1>(entry)); // add sha-1, 40 symbols

	data.append(I2HEX<us, 2>(static_cast<us>(std::get<2>(entry)))); // add flags unsigned short

	data.append(std::get<3>(entry)); // add path
	
	size_t length = static_cast<size_t>((82 + std::get<3>(entry).size() + 8) / 8) * 8; 

	string fill(length - 82 - std::get<3>(entry).size(), '\xff');

	data.append(fill);

	return std::move(data);
}


string HeaderPack(const header_index& header)
{
	// only 12 bytes for data
	constexpr size_t offset = sizeof(ul); // length of signature

	string data{header.signature};

	stringstream temp;

	temp << std::setfill('0') << std::setw(4) << header.version;
	
	data.append(temp.str());

	data.append(I2HEX<size_t, 4>(header.num_entries));

	return std::move(data);
}

/*
---GitRepository
Method: write_index
Description: Write list of EntryTypes objects to git index file.
*/
void GitRepository::write_index(set<EntryTypes, EntryCompare>& entries)
{
	if (entries.empty())
		return;

	header_index head;
	list<string> tempList;

	for (const auto& entry : entries) {
		tempList.emplace_back(std::move(EntryPack(entry)));
	}

	head.num_entries = tempList.size();

	string alldata(std::move(HeaderPack(head)));

	for (const auto& val : tempList) {
		alldata.append(val);
	}

	SHA1 sha;

	sha.update(alldata);

	alldata.append(sha.final()); 

	try
	{
		path _fs = fs.getDirectory();
		_fs /= L".git/index";

		if (!exists(_fs))
			fs.createFileAndWrite("", ".git/index", alldata);
		else
			fs.writeFile("", ".git/index", alldata);
	}
	catch (FileSystemExeption & e)
	{
		throw e;	
	}
}


/*
---GitRepository
Method: write_tree
Description: Write a tree object from the current index entries.
*/
string GitRepository::write_tree()
{

	list<EntryTypes> entries{ std::move(read_index()) };

	list<string> tree_entries;

	for (auto& entry : entries) {
		
		string path(std::move(std::get<3>(entry)));

		if (path.find("/") != string::npos)
			continue;
		
		size_t m = std::get<0>(entry).at("mode");
		
		string mode_path(std::move(I2OCT(m)) + " ");

		mode_path.append(path);

		string tree_entry(std::move(mode_path + _SYM_ + std::get<1>(entry)));

		tree_entries.emplace_back(tree_entry);
	}

	string data(std::move(fs.transformListToString(tree_entries)));

	return GitObject::Object_hash(*this, "tree", data);
	
}


/*
---GitRepository
Method: read_tree
Description: Read tree object with given SHA-1 (hex string) or data, and return list
			of (mode, path, sha1) tuples.
*/
list<tuple<size_t, string, string>> GitRepository::read_tree(const string& sha, const string& data)
{

	if (!sha.empty()) {
		tuple<string, string> ObjData = GitObject::Object_read(*this, sha);
		if (std::get<0>(ObjData) != "tree")
			throw RepositoryException(L"Invalid object type in read tree");
	}
	if (data.empty()) {
		throw RepositoryException(L"must specify \"sha1\" or \"data\"");
	}

	
	list<tuple<size_t, string, string>> entries;
	const size_t range = 1000;
	for (size_t i = 0, j = 0; j < range; ++j) {

		size_t end = data.find(_SYM_, i);
		if (end == string::npos)
			break;

		stringstream ModePath(std::move(data.substr(i, end)));

		size_t mode;

		ModePath >> std::oct >> mode;

		stringstream digest(std::move(data.substr(end+1, end+21)));

		string sha;

		digest >> std::hex >> sha; // ?check

		entries.emplace_back(std::make_tuple(mode, ModePath.str(), sha));

		i = end + 20 + 1;
	}

	return entries;
}


/*
---GitRepository
Method: get_local_master_hash
Description: Get current commit hash (SHA-1 string) of local master branch.
*/
string GitRepository::get_local_master_hash()
{
	path master_path(gitdir.folder);
	master_path /= gitdir.refs;
	master_path /= gitdir.heads;
	master_path /= "master";
	
	string hash;
	try
	{
		hash = std::move(fs.readFromFileToString(master_path));
	}
	catch (FileSystemExeption& fe)
	{
		throw fe;
	}

	return hash;
}

static string&& entry_extract(const tuple<map<string, size_t>, string, ul, string>& data)
{
	auto maping_data = std::get<0>(data); // map with meta data
	string extract_data;

	for (auto it : maping_data) {
		extract_data.append(it.first + "=" + std::to_string(it.second));
		extract_data.append(", ");
	}

	extract_data.append("sha1=" + std::get<1>(data)); // sha1 value
	extract_data.append(", ");
	extract_data.append("flags=" + std::to_string(std::get<2>(data))); // flags value
	extract_data.append(", ");
	extract_data.append("path=" + std::get<3>(data)); // path value

	return std::move(extract_data);
};


/*
---GitRepository
Method: GetStatus
Description: Get status of working copy, return tuple of (changed_paths, new_paths, deleted_paths).
*/
StatusTypes GitRepository::GetStatus() 
{
	path _checkGit = fs.getDirectory() + L"/" + gitdir.folder;
	if (!exists(_checkGit))
		throw RepositoryException(L"Current folder not version control");

	set<string> _paths{ fs.getListAllStringFiles() };

	set<string> paths;
	std::copy_if(_paths.begin(), _paths.end(), std::inserter(paths, paths.begin()), [](const string& str) {
				return str.rfind(".git") == string::npos;
		});

	list<EntryTypes> entries;
	try
	{
		entries = std::move(read_index());
	}
	catch (RepositoryException& e)
	{
		throw std::wstring(e.getMessage());
	}

	map<string, EntryTypes> entries_by_path;

	for (auto& ItEntry : entries) {
		entries_by_path[std::get<3>(ItEntry)] = std::move(ItEntry);
	}

	set<string> entry_paths;

	for (auto& ItEntry : entries_by_path) {
		entry_paths.insert(ItEntry.first);
	}

	set<string> catPaths;
	//std::set_union(paths.begin(), paths.end(), entry_paths.begin(), entry_paths.end(), std::inserter(catPaths, catPaths.begin()));
	std::merge(paths.begin(), paths.end(), entry_paths.begin(), entry_paths.end(), std::inserter(catPaths, catPaths.begin()));

	set<string> changed;
	std::copy_if(catPaths.begin(), catPaths.end(), std::inserter(changed, changed.begin()), [&](auto path) {
					string data{ std::move(fs.readFromFileToString(path)) };
					return (GitObject::Object_hash(*this, "blob", data, false) != std::get<1>(entries_by_path[path])); // sha value
					});

	set<string> newed;
	std::set_difference(paths.begin(), paths.end(), entry_paths.begin(), entry_paths.end(), std::inserter(newed, newed.begin()));

	set<string> deletes;
	std::set_difference(entry_paths.begin(), entry_paths.end(), paths.begin(), paths.end(), std::inserter(newed, newed.begin()));


	return StatusTypes{ changed, newed, deletes };
}

bool EntryCompare::operator()(const EntryTypes& _k, const EntryTypes& _j) const
{
	return std::get<3>(_k) < std::get<3>(_j);
}
