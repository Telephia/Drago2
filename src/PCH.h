#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>
#ifndef NDEBUG
#include <spdlog/sinks/msvc_sink.h>
#endif
#include <xbyak/xbyak.h>

#include <SimpleIni.h>

// SKSE::log::info/warn/error/... existieren in der libxse-CommonLibSSE nicht
// mehr; das Logging laeuft dort ueber REX::Impl::Log, das intern den
// spdlog-Default-Logger benutzt (den InitializeLogging() selbst konfiguriert).
namespace logger
{
    using SKSE::log::log_directory;

    namespace detail
    {
        inline void log_raw(const REX::ELogLevel a_level, const std::string_view a_msg)
        {
            REX::Impl::Log(std::source_location::current(), a_level, a_msg);
        }
    }

    inline void critical(const char* a_msg) { detail::log_raw(REX::ELogLevel::Critical, a_msg); }
    inline void error(const char* a_msg)   { detail::log_raw(REX::ELogLevel::Error, a_msg); }
    inline void warn(const char* a_msg)    { detail::log_raw(REX::ELogLevel::Warning, a_msg); }
    inline void info(const char* a_msg)    { detail::log_raw(REX::ELogLevel::Info, a_msg); }
    inline void debug(const char* a_msg)   { detail::log_raw(REX::ELogLevel::Debug, a_msg); }
    inline void trace(const char* a_msg)   { detail::log_raw(REX::ELogLevel::Trace, a_msg); }

    template <class... T>
    void critical(const std::format_string<T...> a_fmt, T&&... a_args)
    {
        REX::Impl::Log(std::source_location::current(), REX::ELogLevel::Critical, a_fmt, std::forward<T>(a_args)...);
    }

    template <class... T>
    void error(const std::format_string<T...> a_fmt, T&&... a_args)
    {
        REX::Impl::Log(std::source_location::current(), REX::ELogLevel::Error, a_fmt, std::forward<T>(a_args)...);
    }

    template <class... T>
    void warn(const std::format_string<T...> a_fmt, T&&... a_args)
    {
        REX::Impl::Log(std::source_location::current(), REX::ELogLevel::Warning, a_fmt, std::forward<T>(a_args)...);
    }

    template <class... T>
    void info(const std::format_string<T...> a_fmt, T&&... a_args)
    {
        REX::Impl::Log(std::source_location::current(), REX::ELogLevel::Info, a_fmt, std::forward<T>(a_args)...);
    }

    template <class... T>
    void debug(const std::format_string<T...> a_fmt, T&&... a_args)
    {
        REX::Impl::Log(std::source_location::current(), REX::ELogLevel::Debug, a_fmt, std::forward<T>(a_args)...);
    }

    template <class... T>
    void trace(const std::format_string<T...> a_fmt, T&&... a_args)
    {
        REX::Impl::Log(std::source_location::current(), REX::ELogLevel::Trace, a_fmt, std::forward<T>(a_args)...);
    }
}

using namespace std::literals;

// --- Kompatibilitaets-Shims fuer die libxse-CommonLibSSE ---

namespace RE
{
    // frueher (CommonLibSSE-NG): BSTSmartPointer<Actor>
    using ActorPtr = NiPointer<Actor>;
}

namespace SKSE
{
    // der Trampoline liegt in der neuen Bibliothek unter REL
    [[nodiscard]] inline REL::Trampoline& GetTrampoline() noexcept
    {
        return REL::GetTrampoline();
    }

    namespace stl
    {
        // Nachfolger ist REX::FAIL (loggt kritisch, zeigt Meldungsbox, beendet den Prozess)
        [[noreturn]] inline void report_and_fail(std::string_view a_message)
        {
            REX::FAIL<void>{a_message};
            std::unreachable();
        }
    }
}

// --- Ende der Shims ---

#ifdef SKYRIM_AE
#	define REL_ID(se, ae) REL::ID(ae)
#	define OFFSET(se, ae) ae
#	define OFFSET_3(se, ae, vr) ae
#elif SKYRIMVR
#	define REL_ID(se, ae) REL::ID(se)
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) vr
#else
#	define REL_ID(se, ae) REL::ID(se)
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) se
#endif

#define DLLEXPORT __declspec(dllexport)

#include "Plugin.h"
