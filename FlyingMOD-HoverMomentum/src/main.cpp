#include "Hooks.h"
#include "Papyrus.h"
#include "Settings.h"
#ifndef NDEBUG
#	include "DebugUtils.h"
#endif

void MessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Hooks::DataLoaded::Install();
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		Hooks::PostLoadGame::Install();
		break;
	default:
		break;
	}
}

void InitializeLogging()
{
#ifndef NDEBUG
	LaunchDebugger();
#endif
	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
	}
	*path /= Plugin::NAME;
	*path += L".log";

	std::shared_ptr<spdlog::logger> log;
#ifndef NDEBUG
	if (IsDebuggerPresent()) {
		log = std::make_shared<spdlog::logger>(
			"Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
		log->set_level(spdlog::level::trace);
	} else {
#endif
		log = std::make_shared<spdlog::logger>(
			"Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);
#ifndef NDEBUG
	}
#endif
	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%v");
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData version{};
	version.PluginVersion(Plugin::VERSION);
	version.PluginName(Plugin::NAME);
	version.AuthorName("z65536"sv);
	version.CompatibleVersions({});
	version.UsesAddressLibrary();
	version.UsesUpdatedStructs();
	return version;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* skse, SKSE::PluginInfo* info)
{
	info->infoVersion = SKSE::PluginInfo::kVersion;
	info->name = Plugin::NAME.data();
	info->version = Plugin::VERSION.pack();

	if (skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}
#endif
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse)
{
	InitializeLogging();

	SKSE::Init(skse);

	SKSE::AllocTrampoline(1 << 11);

	try {
		Settings::GetSingleton()->Load();
	} catch (...) {
		logger::error("Exception caught when loading settings! Default settings will be used");
	}

	auto papyrus = SKSE::GetPapyrusInterface();
	papyrus->Register(Papyrus::Bind);

	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);

	const auto runtimeVer = REL::Module::get().version();

	logger::info("Skyrim SSE v{}.{}.{}", runtimeVer[0], runtimeVer[1], runtimeVer[2]);
	logger::info("{} v{}.{}.{}", Plugin::NAME, Plugin::VERSION[0], Plugin::VERSION[1], Plugin::VERSION[2]);

	return true;
}
