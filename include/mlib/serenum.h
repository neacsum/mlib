#pragma once
#include <vector>
#include <windows.h>

bool SerEnum_UsingCreateFile (std::vector<int>& ports);
bool SerEnum_UsingSetupAPI (std::vector<int>& ports, std::vector<std::string>& names);
bool SerEnum_UsingRegistry (std::vector<int>& ports);

