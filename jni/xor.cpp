#include "xor.hpp"
#include "proton/rtparam.hpp"
#include "proton/variant.hpp"
#include "Includes/Logger.h"
#include <string>
#include <bits/stdc++.h>

std::string Xor::encDec(const char* enc, int ln, bool mode)
{
    char key = 0x41; //Any char will work
	std::string output;
    output.resize(ln);

	for (int i = 0; i < ln; i++)
		output[i] = enc[i] ^ key;

	return output;
}