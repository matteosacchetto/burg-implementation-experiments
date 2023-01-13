/**
 * https://stackoverflow.com/questions/18081392/discrimination-between-file-and-console-streams#answer-51660110
 */

#include <streambuf>
#include <iostream>

extern "C" {
#ifdef _WIN32
# include <io.h>        // for: _isatty()
#else
# include <unistd.h>    // for: isatty()
#endif
}

#include "tty.hpp"

// Stdio file descriptors.
#ifndef STDIN_FILENO
# define STDIN_FILENO   0
# define STDOUT_FILENO  1
# define STDERR_FILENO  2 
#endif

// Store start-up addresses of C++ stdio stream buffers as identifiers.
// These addresses differ per process and must be statically linked in.
// Assume that the stream buffers at these stored addresses
//  are always connected to their underlaying stdio files.
static const std::streambuf* const StdioBufs[] = {
    std::cin.rdbuf(),  std::cout.rdbuf(),  std::cerr.rdbuf(),  std::clog.rdbuf()
};
static const std::wstreambuf* const StdioWBufs[sizeof(StdioBufs)/sizeof(StdioBufs[0])] = {
    std::wcin.rdbuf(), std::wcout.rdbuf(), std::wcerr.rdbuf(), std::wclog.rdbuf()
};

// Store start-up terminal/TTY statuses of C++ stdio stream buffers.
// These statuses differ per process and must be statically linked in.
// Assume that the statuses don't change during the process life-time.
static const int StdioTtys[sizeof(StdioBufs)/sizeof(StdioBufs[0])] = {
#ifdef _WIN32
    _isatty(STDIN_FILENO), _isatty(STDOUT_FILENO), _isatty(STDERR_FILENO), _isatty(STDERR_FILENO)
#else
     isatty(STDIN_FILENO),  isatty(STDOUT_FILENO),  isatty(STDERR_FILENO),  isatty(STDERR_FILENO)
#endif
};

// Is a Terminal/Console/TTY connected to the C++ stream?
// Use on C++ stdio chararacter streams: cin, cout, cerr and clog.
bool isTTY(const std::ios& strm)
{
    for(unsigned int i = 0; i < sizeof(StdioBufs)/sizeof(StdioBufs[0]); ++i) {
        if(strm.rdbuf() == StdioBufs[i])
            return StdioTtys[i];
    }
    return false;
}

// Is a Terminal/Console/TTY connected to the C++ stream?
// Use on C++ stdio wide-chararacter streams: wcin, wcout, wcerr and wclog.
bool isTTY(const std::wios& strm)
{
    for(unsigned int i = 0; i < sizeof(StdioWBufs)/sizeof(StdioWBufs[0]); ++i) {
        if(strm.rdbuf() == StdioWBufs[i])
            return StdioTtys[i];
    }
    return false;
}