/*
* Author: Gasparyan Sokrat
* Date: 2019/12/29
* GitObject.h - this header file is a application definition.
*/
#pragma once
#include "stdafx.h"
#include "GitRepository.h"
#include "sha1/sha1.hpp"


static std::hash<wstring> default_type_hashing;

using hash_sum_git_obj = struct {
	const size_t hash_blob{ std::move(default_type_hashing(L"blob")) };
	const size_t hash_tree{ std::move(default_type_hashing(L"tree")) };
	const size_t hash_commit{ std::move(default_type_hashing(L"commit")) };
};
const hash_sum_git_obj hash_type;


// like interface
class GitObject
{	
private:
	enum class objtype {
		BLOB,
		TREE,
		COMMIT
	};
	using typeobj = struct {
		const string blob{ "blob" };
		const string tree{ "tree" };
		const string commit{ "commit" };
	};
private:
	
	const typeobj fmt;
	SHA1 sha;
	GitRepository gitrep;

public:
	GitObject() = delete;
	GitObject& operator=(const GitObject&) = delete;
	GitObject(const GitObject&) = delete;
	static tuple<string, string> Object_read(GitRepository& gitrep, const string& sha_prefix);
	// fmt == object type
	static string Object_hash(GitRepository& gitrep, const string& fmt, string& data, bool active_write = true);
	static string Object_find(const GitRepository& gitrep, const string& sha_prefix);
	
};


#ifdef TEST
string Object_find(const GitRepository& gitrep, const wstring& sha_prefix);
#endif