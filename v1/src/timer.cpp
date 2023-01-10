
#include "timer.hpp"

namespace measure
{
    std::chrono::nanoseconds timer::now()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch());
    }

    timer::timer() : _start(now()), _end(_start), _running(true) {}

    void timer::start()
    {
        _start = now();
        _end = _start;
        _running = true;
    }

    void timer::stop()
    {
        _end = now();
        _running = false;
    }

    double timer::get_duration_in_ns()
    {
        if (_running)
        {
            stop();
        }

        return (_end - _start).count();
    }
}
