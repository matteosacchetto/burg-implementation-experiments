#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <iostream>
#include <string>
#include <sstream>
#include "tty.hpp"

class logger
{
private:
    logger();
    ~logger();

    static std::string format(std::string s, std::size_t n_spaces)
    {
        std::stringstream si(s);
        std::stringstream so;
        std::string to;
        bool first = true;

        while (std::getline(si, to, '\n'))
        {
            if (!first)
            {
                so << std::string(n_spaces, ' ');
            }
            so << to << "\n";
            first = false;
        }

        return so.str();
    }

public:
    static void info(std::string s, std::size_t indent = 0)
    {
        std::cout << std::string(isTTY(std::cout) ? "\u001B[44m\u001B[37m\u001B[1m  INFO  \u001B[22m\u001B[39m\u001B[49m " : "|  INFO  | ") << logger::format(s, 9 + indent);
    };

    static void error(std::string s, std::size_t indent = 0)
    {
        std::cout << std::string(isTTY(std::cout) ? "\u001B[41m\u001B[37m\u001B[1m  ERROR  \u001B[22m\u001B[39m\u001B[49m " : "|  ERROR  | ") << logger::format(s, 10 + indent);
    };

    static void warning(std::string s, std::size_t indent = 0)
    {
        std::cout << std::string(isTTY(std::cout) ? "\u001B[43m\u001B[30m\u001B[1m  WARN  \u001B[22m\u001B[39m\u001B[49m " : "|  WARN  | ") << logger::format(s, 9 + indent);
    };

    static void success(std::string s, std::size_t indent = 0)
    {
        std::cout << std::string(isTTY(std::cout) ? "\u001B[42m\u001B[37m\u001B[1m  SUCCESS  \u001B[22m\u001B[39m\u001B[49m " : "|  SUCCESS  | ") << logger::format(s, 12 + indent);
    };
};

#endif