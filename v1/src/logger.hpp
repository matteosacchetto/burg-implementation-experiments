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
    static inline void info(const std::string& s, std::size_t indent = 0)
    {
        std::cerr << std::string(isTTY(std::cerr) ? "\u001B[44m\u001B[37m\u001B[1m  INFO  \u001B[22m\u001B[39m\u001B[49m " : "|  INFO  | ") << logger::format(s, 9 + indent + (isTTY(std::cerr) ? 0 : 2));
    };

    static inline  void error(const std::string& s, std::size_t indent = 0)
    {
        std::cerr << std::string(isTTY(std::cerr) ? "\u001B[41m\u001B[37m\u001B[1m  ERR   \u001B[22m\u001B[39m\u001B[49m " : "|  ERROR  | ") << logger::format(s, 10 + indent + (isTTY(std::cerr) ? 0 : 2));
    };

    static inline  void warning(const std::string& s, std::size_t indent = 0)
    {
        std::cerr << std::string(isTTY(std::cerr) ? "\u001B[43m\u001B[30m\u001B[1m  WARN  \u001B[22m\u001B[39m\u001B[49m " : "|  WARN  | ") << logger::format(s, 9 + indent + (isTTY(std::cerr) ? 0 : 2));
    };

    static inline  void success(const std::string& s, std::size_t indent = 0)
    {
        std::cerr << std::string(isTTY(std::cerr) ? "\u001B[42m\u001B[37m\u001B[1m  SUCC  \u001B[22m\u001B[39m\u001B[49m " : "|  SUCCESS  | ") << logger::format(s, 12 + indent + (isTTY(std::cerr) ? 0 : 2));
    };
};

#endif