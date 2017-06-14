//
//  Stopwatch.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/13/17.
//
//

#ifndef Stopwatch_hpp
#define Stopwatch_hpp

#include <cstdint>

// Measures system time with nanosecond precision for high resolution timers.
//
// We use uint64_t instead of double because it uint64_t will maintain precision
// until it overflows, which will not happen for more than 500 years. This is
// clearly long enough for our purposes. Double, however, will eventually start
// to lose precision in the lower digits, which is where we need it most.
class Stopwatch
{
public:
    static constexpr uint64_t NANOS_PER_USEC = 1000ULL;
    static constexpr uint64_t NANOS_PER_MILLISEC = 1000ULL * NANOS_PER_USEC;
    static constexpr uint64_t NANOS_PER_SEC = 1000ULL * NANOS_PER_MILLISEC;

    Stopwatch();
    uint64_t getCurrentTimeInNanos() const;
    void waitUntil(uint64_t wakeUpTime) const;

private:
    double _absToNanos, _nanosToAbs;
};

#endif /* Stopwatch_hpp */
