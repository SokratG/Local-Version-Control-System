/*
* Author: Gasparyan Sokrat
* Date: 2020/01/04
* GitRepository.cpp
* Module implement GitObject static function member,
* which helping handling repository object.
*/
#include "stdafx.h"
#include "Exception.h"
#include "GitObject.h"
#include "Zlib.h"
#include "utility.h"


static const GitDir dir;

/*
---GitObject
Method: Object_read
Description: Read object with given SHA-1 prefix and return tuple data of
			data_bytes and object_type or throw exception if not found.
*/
tuple<string, string> GitObject::Object_read(GitRepository& gitrep, const string& sha_prefix)
{
	string shapath;
	try
	{
		shapath = std::move(GitObject::Object_find(gitrep, sha_prefix));
	}
	catch (RepositoryException & e)
	{
		throw e;
	}

	FileSystem fs;
	fs.setDirectory(shapath);
	string temp = std::move(fs.readFromFileToString(L"", L""));
	string full_data{Zlib::decompress(temp)}; //all path in shapath

	auto null_index = full_data.find(_SYM_); //end symbol

	if (null_index == string::npos)
		throw RepositoryException(L"Corruption data in object");

	auto startFullData = full_data.begin();
	string header{ startFullData, startFullData + null_index };

	list<string> objects_data = SplitToStringList(header);

	if (objects_data.size() != 2)	// must be object type and size of data
		throw RepositoryException(L"Corruption data in object");

	auto obj_meta_data = objects_data.begin();
	string object_type = *obj_meta_data;

	std::advance(obj_meta_data, 1);

	size_t size_data = 0;
	string data{ startFullData + null_index + 2, full_data.end() }; // +1

	try
	{
		size_data = stoi(*obj_meta_data);
	}
	catch (...)
	{
		throw RepositoryException(L"Corruption meta data in object");
	}
	
	//check correct data:
	if (size_data != data.size())
		throw RepositoryException(L"Corruption size of data in object");




	return { object_type, data };
}

/*
---GitObject
Method: Object_hash
Description: Compute hash of object data of given type and write to object store if
			"write" is True. Return SHA-1 object hash as hex string.
*/
string GitObject::Object_hash(GitRepository& gitrep, const string& fmt, string& data, bool active_write)
{
	string header{ fmt };
	header.append(" " + std::to_string(data.size()));

	string full_data = header + " " + _SYM_ + " " + data;
#ifdef TEST
	string full_data = header + "\0" + data;
#endif
	SHA1 sha1;
	sha1.update(full_data);
	string sha = std::move(sha1.final());


	// create and write obj file

	auto ItSha = sha.begin();
	std::advance(ItSha, 2);
	string namefold{ sha.begin(), ItSha }; //sha[0:2] - name folder for object

	if (active_write) {
		FileSystem fs;
		try
		{
			fs.setDirectory(gitrep.getCurrentGitDir() + L"/" + dir.folder + L"/" + dir.objects);
			fs.createFolder(namefold);
			fs.createFileAndWrite(string(ItSha, sha.end()), namefold, Zlib::compress(full_data));
		}
		catch (FileSystemExeption & e)
		{
			throw e;
		}
	}

	return std::move(sha);
};

/*
---GitObject
Method: Object_find
Description: Find object with given SHA-1 prefix and return path to object in object
			 store, or throw exception if there are no objects or multiple objects
			 with this prefix.
*/
string GitObject::Object_find(const GitRepository& gitrep, const string& sha_prefix)
{

	if (sha_prefix.size() < 2)
		return "";
	auto ItSha = sha_prefix.begin();
	std::advance(ItSha, 2);

	FileSystem fs;
	string object;
	wstring workdir{ gitrep.getCurrentGitDir() + dir.folder + L"/" + dir.objects + L"/" + wstring(sha_prefix.begin(), ItSha) };
	try
	{
		fs.setDirectory(workdir);
		string rest{ ItSha, sha_prefix.end() };

		bool obj_multiple_check = false;

		list<string> listobject( std::move(fs.getListStringFiles(true)) );
		for (auto str : listobject) {
			if (startwith(str, rest)) { //chect start position must be 0
				if (obj_multiple_check)
					throw RepositoryException(L"multiple objects with sha prefix");
				object.assign(str);
				obj_multiple_check = true;
			}
		}
	}
	catch (FileSystemExeption & e)
	{
		wcout << e.getMessage() << endl;
	}

	return (ws2s(workdir) + "/" + object);
};
