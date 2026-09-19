#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

#define PROFILE_SCOPE(name) \
    ProfileScope profileScope##__LINE__(name)

static double profilerTimer = 0.0;
static int frameCount = 0;
static double fpsTimer = 0.0;

class ProfileScope
{
public:
    explicit ProfileScope(const std::string& name);
    ~ProfileScope();

private:
    std::string m_name;
};

class Profiler
{
public:
    static Profiler& instance();

    void begin(const std::string& name);
    void end(const std::string& name);

    void reset();

    void print() const;

private:
    using Clock = std::chrono::high_resolution_clock;

    struct Timer
    {
        Clock::time_point start;
        double totalMilliseconds = 0.0;
        std::uint64_t calls = 0;
    };

    std::unordered_map<std::string, Timer> m_timers;
};