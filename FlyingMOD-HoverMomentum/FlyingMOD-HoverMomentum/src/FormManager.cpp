#include "FormManager.h"
#include <iomanip>
#include <sstream>
#include <string>

namespace Form
{
	std::string FormatFormString(uint32_t id, const char* esp)
	{
		std::ostringstream oss;
		oss << "0x" << std::hex << std::uppercase << id << "," << esp;
		return oss.str();
	}

	template <typename T>
	bool InitFormEntry(FormEntry<T>& entry, const std::string& name, uint32_t defaultId, const char* defaultEsp, CSimpleIniA& ini, RE::TESDataHandler* dataHandler, bool& iniDirty)
	{
		const char* value = ini.GetValue("Forms", name.c_str(), nullptr);
		uint32_t    finalId = defaultId;
		std::string finalEsp = defaultEsp;
		bool        parseError = false;
		bool        isMissing = (value == nullptr);

		if (value) {
			std::string valueStr(value);
			size_t      commaPos = valueStr.find(',');

			if (commaPos != std::string::npos) {
				std::string idPart = valueStr.substr(0, commaPos);
				std::string espPart = valueStr.substr(commaPos + 1);
				try {
					finalId = std::stoul(idPart, nullptr, 16);
					finalEsp = espPart;
				} catch (...) {
					parseError = true;
				}
			} else {
				try {
					finalId = std::stoul(valueStr, nullptr, 16);
				} catch (...) {
					parseError = true;
				}
			}
		}

		entry.id = dataHandler->LookupFormID(finalId, finalEsp.c_str());
		entry.ptr = RE::TESForm::LookupByID<T>(entry.id);

		if (isMissing || parseError || !entry.ptr) {
			if (isMissing) {
				logger::info("Adding missing form entry: {}.", name);
			} else if (parseError) {
				logger::warn("Invalid INI format for {}. Resetting to default.", name);
			} else {
				logger::warn("Form {} ({:X} in {}) not found. Resetting to default.", name, finalId, finalEsp);
			}

			ini.SetValue("Forms", name.c_str(), FormatFormString(defaultId, defaultEsp).c_str());
			iniDirty = true;

			entry.id = dataHandler->LookupFormID(defaultId, defaultEsp);
			entry.ptr = RE::TESForm::LookupByID<T>(entry.id);

			if (!entry.ptr) {
				logger::error("Default form {} not found. Critical error.", name);
				return false;
			}
		}

		return true;
	}

	bool Install()
	{
		auto manager = Manager::GetSingleton();
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler)
			return false;

		CSimpleIniA ini;
		ini.SetUnicode();
		const std::wstring iniPath = L"Data/SKSE/Plugins/FlyingMOD.ini";
		ini.LoadFile(iniPath.c_str());

		bool iniDirty = false;

#define GEN_INIT(name, type, id, esp)                                                    \
	if (!InitFormEntry<type>(manager->name, #name, id, esp, ini, dataHandler, iniDirty)) \
		return false;
		PD_FORM_LIST(GEN_INIT)
		PD_VANILLA_FORM_LIST(GEN_INIT)
#undef GEN_INIT

		if (iniDirty) {
			logger::info("Correcting malformed/missing form entries in FlyingMOD.ini...");
			ini.SaveFile(iniPath.c_str());
		}

		auto fireBreathShout = manager->FireBreathShout.GetPtr();
		auto frostBreathShout = manager->FrostBreathShout.GetPtr();
		manager->originalFireSpell2 = fireBreathShout ? fireBreathShout->variations[1].spell : 0;
		manager->originalFireSpell3 = fireBreathShout ? fireBreathShout->variations[2].spell : 0;
		manager->originalFrostSpell2 = frostBreathShout ? frostBreathShout->variations[1].spell : 0;
		manager->originalFrostSpell3 = frostBreathShout ? frostBreathShout->variations[2].spell : 0;

		return true;
	}
}
