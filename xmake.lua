includes("deps/commonlibsse")

set_project("FlyingMOD")
set_version("1.9.7")
set_arch("x64")
set_languages("c++23")
set_warnings("allextra")
set_encodings("utf-8")

add_rules("mode.debug", "mode.release")

add_requires("simpleini")

target("FlyingMOD")
    set_kind("shared")
    set_basename("FlyingMOD")

    add_deps("commonlibsse")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")

    add_packages("simpleini")

    add_defines("SKYRIM_AE=1")

    set_pcxxheader("src/PCH.h")
target_end()
