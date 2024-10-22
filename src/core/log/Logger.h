#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <mutex>

namespace lgr {
    class Logger {
    private:    
        enum LogLevel { DEBUG, INFO, WARNING, ERR };

        bool m_consoleAvailable;
        std::ofstream m_logFile;
        std::mutex m_mtx;

        std::string getCurrentTime() const noexcept;
        void log(const LogLevel& llevel, const std::string& msg) noexcept;

        // Delete copy constructor and assignment operator
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

    public:
        Logger() noexcept;
        ~Logger() noexcept;

        void debug(const std::string& msg) noexcept;
        void info(const std::string& msg) noexcept;
        void warn(const std::string& msg) noexcept;
        void error(const std::string& msg) noexcept;
    };

    extern Logger lout;
}

#endif