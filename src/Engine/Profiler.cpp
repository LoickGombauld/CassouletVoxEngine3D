#include "Profiler.hpp"

#include <iostream>
#include <iomanip>

Profiler& Profiler::instance()
{
    static Profiler profiler;
    return profiler;
}

void Profiler::begin(const std::string& name)
{
    auto& timer = m_timers[name];

    timer.start = Clock::now();
}

void Profiler::end(const std::string& name)
{
    auto it = m_timers.find(name);

    if (it == m_timers.end())
        return;

    const auto now = Clock::now();

    const double elapsed =
        std::chrono::duration<double, std::milli>(
            now - it->second.start
        ).count();

    it->second.totalMilliseconds += elapsed;
    it->second.calls++;
}

void Profiler::reset()
{
    for (auto& [name, timer] : m_timers)
    {
        timer.totalMilliseconds = 0.0;
        timer.calls = 0;
    }
}

void Profiler::print() const
{
    std::cout << "\n========== PERFORMANCE ==========\n";

    for (const auto& [name, timer] : m_timers)
    {
        if (timer.calls == 0)
            continue;

        const double average =
            timer.totalMilliseconds /
            static_cast<double>(timer.calls);

        std::cout
            << std::left
            << std::setw(24)
            << name
            << " total: "
            << std::setw(10)
            << timer.totalMilliseconds
            << " ms | avg: "
            << average
            << " ms | calls: "
            << timer.calls
            << '\n';
    }

    std::cout << "=================================\n";
}

ProfileScope::ProfileScope(const std::string& name)
    : m_name(name)
{
    Profiler::instance().begin(m_name);
}

ProfileScope::~ProfileScope()
{
    Profiler::instance().end(m_name);
}