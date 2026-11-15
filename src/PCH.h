#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

// REX::Singleton gibt es erst in neueren CommonLibSSE-NG-Versionen als v3.7.0,
// das xmake-Paket liefert sie nicht mit. Funktionstüchtige, eigenständige
// Umsetzung derselben Vorlage:
namespace REX
{
    template <class T>
    class Singleton
    {
    public:
        Singleton(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;

        [[nodiscard]] static T* GetSingleton()
        {
            static T singleton{};
            return std::addressof(singleton);
        }

    protected:
        Singleton() = default;
        ~Singleton() = default;
    };
}

#include <spdlog/sinks/basic_file_sink.h>
#ifndef NDEBUG
#include <spdlog/sinks/msvc_sink.h>
#endif
#include <xbyak/xbyak.h>

#include <ClibUtil/simpleINI.hpp>
#include <ClibUtil/string.hpp>

namespace logger = SKSE::log;
namespace string = clib_util::string;
namespace ini = clib_util::ini;

using namespace std::literals;

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
