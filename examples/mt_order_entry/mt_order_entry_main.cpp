// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "Market.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <locale>
#include <cstring>
#include <algorithm> 
#include <vector>
#include <iterator>

namespace
{

template<typename INPUT_STRING,typename DELIMITER_STRING,typename STRING_CONTAINER>
void split(const INPUT_STRING & input, const DELIMITER_STRING & delimiter, STRING_CONTAINER & tokens)
{
    size_t pos = 0;
    size_t end = input.length();
    while(pos < end)
    {
        auto last = input.find_first_of(delimiter, pos);
        if(last == std::string::npos)
        {
            last = end;
        }
        tokens.push_back(input.substr(pos, last - pos));
        pos = ++last;
    }
}

}

using namespace orderentry;

int main(int argc, const char * argv[])
{
    bool done = false;
    bool prompt = true;
    Market market;
    while( !done)
    {
        if(prompt)
        {
            std::cout << "Action[" << Market::prompt() 
            << "\t(#)    comment\n"
            << "\t(?)    help\n"
            << "\t(Quit) ]\n";
            prompt = false;
        }
        std::cout << "> " << std::flush;
        std::string input;
        std::getline(std::cin, input);
        std::transform(input.begin(), input.end(), input.begin(), toupper);
        // if input ends in a ';' be sure there's a space before it to simplify parsing.
        if(input.length() > 1)
        {
            if(input.back() == ';')
            {
                input.pop_back();
                if(input.back() == ' ')
                {
                    input.pop_back();
                }
                input.append(" ;");
            }
        }
        std::vector< std::string> words;
        split(input," \t\v\n\r", words);
        if(!words.empty())
        {        
            const std::string command = words[0];
            if(command == "QUIT")
            {
                done = true;
            }
            else if(command[0] == '#')
            {
                // nothing
            }
            else if(command == "?" || command == "HELP")
            {
                market.help();
                bool prompt = true;
            }
            else if(!market.apply(words))
            {
                std::cerr << "Cannot process command";
                for(auto word = words.begin(); word != words.end(); ++ word)
                {
                    std::cerr << ' ' << *word;
                }
                std::cerr << std::endl;
                bool prompt = true;
            }
        }
    }
    return 0;
}
