/*
* Author: Gasparyan Sokrat
* Date: 2019/12/21
* Exception.h - this header file is a application definition.
*/
#pragma once
#include <exception>
#include <string_view>


using std::wstring_view;
using std::exception;
using std::wstring;

class WindowException : public exception
{
private:
	wstring Message;
public:
	WindowException(const wchar_t* msg) : Message(msg) {}
	wstring&& getMessage() noexcept {
		return std::move(Message);
	}
};

class FileSystemExeption : public exception
{
private:
	wstring Message;
public:
	FileSystemExeption(const wchar_t* msg) : Message(msg) {}
	wstring&& getMessage() noexcept {
		return std::move(Message);
	}
};


class CommandExeption : public exception
{
private:
	wstring Message;
public:
	CommandExeption(const wstring& msg) : Message(msg) {}
	CommandExeption(const wstring&& msg) : Message(msg) {}
	wstring&& getMessage() noexcept {
		return std::move(Message);
	}
};


class RepositoryException : public exception
{
private:
	wstring Message;
public:
	RepositoryException(const wstring& msg) : Message(msg) {}
	RepositoryException(const wstring&& msg) : Message(msg) {}
	wstring&& getMessage() noexcept {
		return std::move(Message);
	}
};
