#include "Hooks.h"
#include "Settings.h"
#include "Utils.h"

static bool g_IsModLoaded = false;

void Hooks::DataLoaded::Install()
{
	if (!GetModuleHandle(L"MRCrashFix.dll")) {
		logger::error("Dependency 'Monster Race Crash Fix' (MRCrashFix.dll) not found. Skipping hooks.");
		RE::DebugMessageBox("Play as a Dragon \n 'Monster Race Crash Fix' is required for this mod to work.\n Please install 'Monster Race Crash Fix' and try again.");
		return;
	}

	auto handler = RE::TESDataHandler::GetSingleton();
	auto pd_esp = handler->LookupModByName(PD_ESP_NAME);

	if (!pd_esp) {
		logger::error("ESP {} NOT found, skipping hooks", PD_ESP_NAME);
		RE::DebugMessageBox(std::format("Play as a Dragon \n ESP {} NOT found, skipping hooks", PD_ESP_NAME).c_str());
		return;
	}

	if (!Form::Install()) {
		logger::error("Critical FormIDs missing. Aborting all hooks installation.");
		RE::DebugMessageBox(std::format("Play as a Dragon \n Critical FormIDs missing. Aborting all hooks installation.").c_str());
		return;
	}

	logger::info("ESP {} found and data verified, loading hooks", PD_ESP_NAME);
	PlayerEvents::Install();
	Armor::Install();
	Riding::Install();
	Animation::Install();
	Attack::Install();
	Camera::Install();
	Movement::Install();
	Input::Install();

	g_IsModLoaded = true;
}

void Hooks::PostLoadGame::Install()
{
	if (g_IsModLoaded) {
		auto arg = RE::MakeFunctionArguments();
		Utils::CallQuestFunction("OnGameLoaded", arg);
		Settings::GetSingleton()->ApplyAll();
	}
}
