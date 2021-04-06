#pragma once
#include <string>
#include <windows.h>//TEXT macro
#include <tchar.h>

#ifdef _UNICODE

#define tstring wstring

#define tostream wostream

#define tifstream wifstream

#define tofstream wofstream

#define tstringstream wstringstream

#define tistringstream wistringstream

#define tostringstream wostringstream

#define tcout wcout

#define tto_string to_wstring

#else

#define tstring string

#define tostream ostream

#define tifstream ifstream

#define tofstream ofstream

#define tstringstream stringstream

#define tistringstream istringstream

#define tostringstream ostringstream

#define tcout cout

#define tto_string to_string

#endif

std::string to_string(const std::wstring& ws);
std::string to_string(const std::string& s);

std::wstring to_wstring(const std::string& s);

void tolower(std::string& s);
void toupper(std::string& s);