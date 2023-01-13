#ifndef __TTY_HPP__
#define __TTY_HPP__

#include <iostream>

bool isTTY(const std::ios& strm);
bool isTTY(const std::wios& strm);

#endif