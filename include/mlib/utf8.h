#ifndef UTF8_H
#define UTF8_H

#ifndef _STRING_
#include <string>
#endif

#ifndef _VECTOR_
#include <vector>
#endif

std::string narrow(const wchar_t *s);
std::wstring widen(const char *s);
std::string narrow(const std::wstring &s);
std::wstring widen(const std::string &s);

char **get_utf8argv (int *argc);
void free_utf8argv (int argc, char **argv);

std::vector<std::string> get_utf8argv ();

#endif