#include "Settings.h"
#include "Ability.h"
#include "Animation.h"
#include "ArmorManager.h"
#include "FormManager.h"
#include "State.h"
#include "Utils.h"

void Settings::Load()
{
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(_iniPath.c_str());

	std::unordered_map<std::string, SettingValue> defaults = {
		{ "ChosenSkin", std::string("Dragon ") },
		{ "AutoLand", true },
		{ "HeadTracking", true },
		{ "RidingControl", true },
		{ "FastTransform", false },
		{ "FriendlyNPCs", true },
		{ "FriendlyDragons", false },
		{ "AutoEquipOutfit", true },
		{ "UseDynamicSpellSwap", false },
		{ "FlySpeedMult", 1.0f },
		{ "HoverMomentum", 0.4f },
		{ "BiteReachMult", 1.0f },
		{ "AirSwipeDamageMult", 10.0f },
		{ "DragonSizeScale", 1.0f },
		{ "FortifyCarryWeightMag", 2000.0f },
		{ "FortifyHealthMag", 0.0f },
		{ "FortifyUnarmedDamageMag", 0.0f },
		{ "DragonShoutDamageMag", 150.0f },
		{ "LandKey", std::string("Sneak") },
		{ "PerchKey", std::string("Sprint") },
		{ "TakeOffKey", std::string("Jump") },
		{ "LayDownKey", std::string("Sneak") },
	};

	for (auto const& [name, defaultValue] : defaults) {
		const char* valueStr = ini.GetValue("Settings", name.c_str(), nullptr);
		if (!valueStr) {
			_values[name] = defaultValue;
			continue;
		}

		if (std::holds_alternative<bool>(defaultValue)) {
			_values[name] = (std::string(valueStr) == "1" || std::string(valueStr) == "true");
		} else if (std::holds_alternative<float>(defaultValue)) {
			_values[name] = std::stof(valueStr);
		} else if (std::holds_alternative<int>(defaultValue)) {
			_values[name] = std::stoi(valueStr);
		} else if (std::holds_alternative<std::string>(defaultValue)) {
			std::string val = std::string(valueStr);

			if (name == "ChosenSkin") {
				if (!val.empty() && val.back() != ' ') {
					val += ' ';
				}
			}

			_values[name] = val;
		}
	}
}

void Settings::Save()
{
	if (!_dirty)
		return;

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(_iniPath.c_str());

	for (auto const& [name, value] : _values) {
		std::visit([&](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, bool>)
				ini.SetValue("Settings", name.c_str(), arg ? "1" : "0");
			else if constexpr (std::is_same_v<T, float>)
				ini.SetValue("Settings", name.c_str(), std::to_string(arg).c_str());
			else if constexpr (std::is_same_v<T, int>)
				ini.SetValue("Settings", name.c_str(), std::to_string(arg).c_str());
			else if constexpr (std::is_same_v<T, std::string>)
				ini.SetValue("Settings", name.c_str(), arg.c_str());
		},
			value);
	}
	ini.SaveFile(_iniPath.c_str());
	_dirty = false;
}

void SetNthSpellMagnitude(RE::SpellItem* spell, uint8_t index, float mag)
{
	if (spell == nullptr)
		return;

	if (spell->effects.size() <= index)
		return;

	spell->effects[index]->SetMagnitude(mag);
}

void SetShoutMagnitude(float mag)
{
	auto mgr = Form::Manager::GetSingleton();
	SetNthSpellMagnitude(mgr->PDVoiceFireball2.GetPtr(), 0, mag);
	SetNthSpellMagnitude(mgr->PDVoiceIceStorm2.GetPtr(), 0, mag);
	SetNthSpellMagnitude(mgr->PDVoiceFireBreath3.GetPtr(), 0, mag);
	SetNthSpellMagnitude(mgr->PDVoiceFrostBreath3.GetPtr(), 0, mag);
}

void Settings::ApplyAll()
{
	auto player = RE::PlayerCharacter::GetSingleton();

	auto arg = RE::MakeFunctionArguments();
	Utils::CallQuestFunction("OnSettingsApplied", arg);

	bool         ridingControl = Get<bool>("RidingControl");
	RE::ActorPtr ptr;

	if (player && player->GetMount(ptr)) {
		auto mount = ptr.get();
		if (mount && mount->movementController && mount->GetRace() && mount->GetRace()->data.flags.all(RE::RACE_DATA::Flag::kFlies)) {
			if (ridingControl) {
				mount->SetGraphVariableBool("iIsPlayer", true);
				mount->movementController->SetControlsDriven();
				State::GetSingleton()->isRidingDriven = true;
			} else {
				mount->SetGraphVariableBool("iIsPlayer", false);
				Animation::OnDefault(mount);
				mount->movementController->SetAIDriven();
				mount->NotifyAnimationGraph("HoverStart");
			}
		}
	}

	if (Utils::IsDragonRace(player)) {
		player->SetGraphVariableBool("bPDHeadTracking", Get<bool>("HeadTracking"));
		Ability::UpdateBreathShouts(true);
		auto name = Get<std::string>("ChosenSkin");
		Armor::ApplySkin(player, name);
	}

	SetShoutMagnitude(Get<float>("DragonShoutDamageMag"));

	logger::info("Settings: Applied all configurations from INI to game.");
}
