// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "Util.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>

namespace orderentry
{
std::string nextToken(const std::vector<std::string> & tokens, size_t & pos)
{
    if(pos < tokens.size())
    {
        return tokens[pos++];
    }
    return "";
}

uint32_t toUint32(const std::string & input)
{
    char * end;
    uint32_t value = strtoul(input.c_str(), &end, 10);
    if(*end != '\0')
    {
        value = INVALID_UINT32;
    }
    return value;
}

uint32_t toInt32(const std::string & input)
{
    char * end;
    uint32_t value = strtol(input.c_str(), &end, 10);
    if(*end != '\0')
    {
        value = INVALID_INT32;
    }
    return value;
}


liquibook::book::Price stringToPrice(const std::string & str)
{
    if(str == "MARKET" || str == "MKT")
    {
        return 0;
    } else
    {
        return toUint32(str);
    }
}

std::string promptForString(const std::string & prompt, bool uppercase)
{
    std::cout << "\n" << prompt << ": " << std::flush;
    std::string input;
    std::getline(std::cin, input);
    if(uppercase)
    {
      std::transform(input.begin(), input.end(), input.begin(), toupper);
    }
    return input;
}

liquibook::book::Price promptForPrice(const std::string & prompt)
{
    std::string str = promptForString(prompt);
    return stringToPrice(str);
}

uint32_t promptForUint32(const std::string & prompt)
{
    std::cout << "\n" << prompt << ": " << std::flush;
    std::string input;
    std::getline(std::cin, input);
    return toUint32(input);
}

int32_t promptForInt32(const std::string & prompt)
{
    std::cout << "\n" << prompt << ": " << std::flush;
    std::string input;
    std::getline(std::cin, input);
    return toInt32(input);
}

bool promptForYesNo(const std::string & prompt)
{
    while(true)
    {
        std::string input = promptForString(prompt);
        if(input == "Y" || input == "YES" || input == "T" || input == "TRUE")
        {
            return true;
        }
        if(input == "N" || input == "NO" || input == "F" || input == "FALSE")
        {
            return false;
        }
    }
}

// trim from start (in place)
std::string &  ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), 
        std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end (in place)
std::string &  rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), 
        std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends (in place)
std::string &  trim(std::string &s) 
{
    return ltrim(rtrim(s));
}

// trim from start (copying)
std::string ltrimmed(std::string s) 
{
    ltrim(s);
    return s;
}

// trim from end (copying)
std::string rtrimmed(std::string s) 
{
    rtrim(s);
    return s;
}

// trim from both ends (copying)
std::string trimmed(std::string s) 
{
    trim(s);
    return s;
}

}

