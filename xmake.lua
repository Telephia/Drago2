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

target("FlyingMOD")
	set_kind("shared")
	set_basename("FlyingMOD")

	add_files("src/**.cpp")
	add_headerfiles("src/(**.h)")
	add_includedirs("src")

	-- clib-util is header-only; cloned by the CI workflow into deps/CLibUtil
	add_includedirs("deps/CLibUtil/include")

	add_packages("commonlibsse-ng")

	set_pcxxheader("src/PCH.h")

	-- module definition not required: SKSE finds the plugin via
	-- SKSEPlugin_Query / SKSEPlugin_Load exports (declared in main.cpp)
target_end()
