// =======================================================================================
// C++ implementation of drewCo.Logging
// Copyright 2026 Andrew A. Ritz
// =======================================================================================

#pragma once
#ifndef DREWCO_LOGGING
#define DREWCO_LOGGING

#include <string>
#include <unordered_set>
#include <list>
#include <cstdarg>
#include <stdexcept>

typedef const std::string& CSTRING;


namespace drewCo::Logging
{
    namespace Levels {
        constexpr char* Info = "Info";
    }
    //    static std::hash<std::string> _hasher;

    // =======================================================================================
    class ILogger
    {
    private:
        std::unordered_set<std::string> _ActiveLevels;

    public:
        virtual void WriteLine(CSTRING level, CSTRING msg) = 0;
        //virtual void WriteLine(CSTRING level, const char* msg, ...) = 0;

        virtual void Info(CSTRING msg) = 0;
        //virtual void Info(const char* msg, ...) = 0;

        // -----------------------------------------------------------------------------------
        void AddLevel(CSTRING level)
        {
            _ActiveLevels.insert(level);
        }

        // -----------------------------------------------------------------------------------
        bool HasLevel(CSTRING level)
        {
            auto match = _ActiveLevels.find(level);
            return match != _ActiveLevels.end();
        }
    };

    // =======================================================================================
    class CConsoleLogger : public ILogger
    {
        // -----------------------------------------------------------------------------------
        virtual void WriteLine(CSTRING level, CSTRING msg) override
        {
            if (!HasLevel(level))
            {
                return;
            }
            printf(msg.data());

            // Always include the newline....
            printf("\n");
        }

        // -----------------------------------------------------------------------------------
        virtual void Info(CSTRING msg) override
        {
            WriteLine(Levels::Info, msg);
        }
    };

    // =======================================================================================
    class CLogMgr
    {
    private:
        std::list<ILogger*> _Loggers;

        // -----------------------------------------------------------------------------------
        // Thanks Internet!
        // https://stackoverflow.com/questions/3919995/determining-sprintf-buffer-size-whats-the-standard
        int bufferSize(const char* format, ...) {
            va_list args;
            va_start(args, format);
            int result = vsnprintf(NULL, 0, format, args);
            va_end(args);
            return result + 1; // safe byte for \0
        }

        // -----------------------------------------------------------------------------------
        void WriteIt(CSTRING level, const char* fmt, va_list args)
        {
            int bufSize = bufferSize(fmt, args);

            // NOTE: We could have some kind of static buffer that grows to meet the current largest
            // args size or whatver.  That will prevent doing malloc / free all of the time.
            char* buffer = (char*)malloc(bufSize);
            if (buffer == nullptr) {
                throw std::runtime_error("could not allocate buffer!");
            }

            vsprintf(buffer, fmt, args);

            for (auto it = _Loggers.begin(); it != _Loggers.end(); it++)
            {
                auto l = *it;
                l->WriteLine(level, buffer);
            }

            free(buffer);
        }

    public:
        // -----------------------------------------------------------------------------------
        void AddLogger(ILogger* logger__)
        {
            // TODO: Check for and reject existing logger!
            _Loggers.push_back(logger__);
        }

        // -----------------------------------------------------------------------------------
        void Info(const char* fmt, ...)
        {
            std::va_list args;
            va_start(args, fmt);
            WriteIt(Levels::Info, fmt, args);
            va_end(args);
        }

        // -----------------------------------------------------------------------------------
        void Info(CSTRING msg)
        {
            Info(msg.data());
            //for (auto it = _Loggers.begin(); it != _Loggers.end(); it++)
            //{
            //    auto l = *it;
            //    l->Info(msg);
            //}
        }
    };

    static CLogMgr Log;
}

#endif