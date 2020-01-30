// gitpow.cpp : 
//

#include "stdafx.h"
#include "GitRepository.h"
#include "utility.h"

#define DIFFLIB_ENABLE_EXTERN_MACROS
#include "difflib/difflib.h"

#define COMPLETE_CODE 0
#define ERROR_CODE -1

//D:\project\OpenSource\Cpp\boost_1_72_0
void HandleMessageLoop(CommandParse&, GitRepository&, HANDLE&);
void warning(const wstring& message, const HANDLE& hConsole);
void error(const wstring& message, const HANDLE& hConsole);


int main(int argc, wchar_t* argv[])
{
	static CommandParse cmd;
	static GitRepository git;

#ifdef __WIN32__
	static HANDLE hConsole;
#endif
	if (argc > 3) {
		wcerr << L"Error number of argument programm. Try Run - \"programmname.exe [PATH]\" " << endl;
		exit(EXIT_FAILURE);
	}
		

	try {
		if (argc > 2)
			git.setCurrentGitDir(wstring{ argv[1] });
		else
			git.setCurrentGitDir(getHomedir());
	}
	catch (FileSystemExeption & fe) {
		wcerr << L"Runtime memory error " << fe.getMessage() << endl;
		return ERROR_CODE;
	}

#ifdef __WIN32__
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	{ // Hello block
#ifdef __WIN32__
		SetConsoleTextAttribute(hConsole, CLBLUE);
#endif
		wcout << L"\t\t\tHELLO!" << endl;
		wcout << std::setw(35) << std::setfill(L' ') << L"Welcome Git-Pow" << endl;
		wcout << "Your current workdir: " << git.getCurrentGitDir() << endl;
		
	}

#ifdef __WIN32__
	SetConsoleTextAttribute(hConsole, CDEFUALT);
#endif
	HandleMessageLoop(cmd, git, hConsole);


	return COMPLETE_CODE;
}


void HandleMessageLoop(CommandParse& cmd, GitRepository& git, HANDLE& hConsole)
{
	
	wstring Command;

	wcout << "Please enter your command..." << endl;

	while (true) {
		
		string result;

		wcout << "Git>> ";

		std::getline(std::wcin, Command);

		try
		{
			result = cmd.HandleCommand(Command, git);
		}
		catch (CommandExeption & ce)
		{
			warning(ce.getMessage(), hConsole);
		}
		catch (const std::exception& e)
		{
			warning(s2ws(e.what()), hConsole);
		}
		catch (const std::bad_alloc & err_alloc)
		{
			error(s2ws(err_alloc.what()), hConsole);
		}
		catch (...) 
		{
			error(L"Unknow error", hConsole);
		}

		
		if (!result.empty()) {
			cout << result << endl;
		}

		wstring().swap(Command);

	}
}

void warning(const wstring& message, const HANDLE& hConsole)
{
#ifdef __WIN32__
	SetConsoleTextAttribute(hConsole, CYELLOW);
#endif
	wcerr << L"Runtime command error: " << message << endl;
#ifdef __WIN32__
	SetConsoleTextAttribute(hConsole, CDEFUALT);
#endif
}


void error(const wstring& message, const HANDLE& hConsole)
{
#ifdef __WIN32__
	SetConsoleTextAttribute(hConsole, CRED);
#endif

	wcerr << "Error: " << message << endl;

#ifdef __WIN32__
	SetConsoleTextAttribute(hConsole, CDEFUALT);
#endif
	exit(EXIT_FAILURE);
}