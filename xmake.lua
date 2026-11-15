set_xmakever("2.8.2")

set_project("FlyingMOD")
set_arch("x64")
set_languages("c++20")
set_warnings("allextra")
set_encodings("utf-8")

add_rules("mode.debug", "mode.release")

add_requires("commonlibsse-ng v3.7.0", {
    configs = {
        skyrim_se = true,
        skyrim_ae = true,
        skyrim_vr = true,
        skse_xbyak = true
    }
})

add_requires("simpleini")

target("FlyingMOD")
    set_kind("shared")
    set_basename("FlyingMOD")

    add_files("src/**.cpp")
    add_headerfiles("src/(**.h)")
    add_includedirs("src")

    add_packages("commonlibsse-ng")
    add_packages("simpleini")

    set_pcxxheader("src/PCH.h")
target_end()
