#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <chrono>

namespace measure
{
    class timer
    {
    private:
        std::chrono::nanoseconds _start;
        std::chrono::nanoseconds _end;

        bool _running;

        std::chrono::nanoseconds now();

    public:
        timer();

        void start();
        void stop();
        double get_duration_in_ns();
    };
}

#endif