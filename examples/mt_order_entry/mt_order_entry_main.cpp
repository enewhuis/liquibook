// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "Market.h"
#include "Util.h"
#include <fstream>
#include <iomanip>
#include <string>
#include <locale>
#include <cstring>
#include <algorithm> 
#include <vector>
#include <iterator>

using namespace orderentry;

int main(int argc, const char * argv[])
{
    bool done = false;
    bool prompt = true;
    bool interactive = true;
    bool fileActive = false;
    std::ostream * log = &std::cout;
    std::ifstream commandFile;
    std::ofstream logFile;

    if(argc > 1)
    {
        std::string filename = argv[1];
        if(filename != "-")
        {
            commandFile.open(filename);
            if(!commandFile.good())
            {
                std::cerr << "Can't open command file " << filename << ". Exiting." << std::endl;
                return -1;
            }
            interactive = false;
            fileActive = true;
        }
    }
    if(argc > 2)
    {
        const char * filename = argv[2];
        logFile.open(filename);
        if(!logFile.good())
        {
            std::cerr << "Can't open log file " << filename << ". Exiting." << std::endl;
            return -1;
        }
        log = & logFile;
    }

    Market market(log);
    while( !done)
    {
        std::string input;
        if(fileActive) 
        {
            std::getline(commandFile, input);
            if(!commandFile.good())
            {
                if(interactive)
                {
                    input = "# Switching to console input.";               
                    fileActive = false;
                }
                else
                {
                    input = "# end of command file.";
                    done = true;
                }
            }
            // if it came from a file, echo it to the log
            if(input.substr(0,2) != "##") // don't log ## comments.
            {
                *log << input << std::endl;
            }
        }
        else
        {
            if(prompt)
            {
                std::cout << "Action[" << Market::prompt() 
                << "\t(?)    help for more options and detail.\n"
                << "\t(Quit) ]\n";
                prompt = false;
            }
            std::cout << "> " << std::flush;
            std::getline(std::cin, input);
        }
        std::transform(input.begin(), input.end(), input.begin(), toupper);
        if(log != &std::cout && !fileActive)
        {
            if(input.substr(0,2) != "##") // don't log ## comments.
            {
                *log << input << std::endl;
            }
        }

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
            else if(command == "F" || command == "FILE")
            {
                if(fileActive)
                {
                    std::cout << "Only one input file at a time can be open." << std::endl;
                }
                else
                {
                    std::cout << "Command file name: " << std::flush;
                    std::string filename;
                    std::getline(std::cin, filename);
                    commandFile.open(filename);
                    if(commandFile.good())
                    {
                        fileActive = true;
                    }
                    else
                    {
                        std::cout << "Cannot open " << filename << std::endl;
                    }
                }
            }
            else if(command == "?" || command == "HELP")
            {
                market.help();
                *log << "(F)ile  Open or Close a command input file\n"
                    << "\tArguments\n"
                    << "\t\t<FileName>  Required if no file is open. Must not appear if file is open.\n";
                *log << "QUIT  Exit from this program.\n";
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
