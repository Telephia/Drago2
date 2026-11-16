#include "Papyrus.h"
#include "Ability.h"
#include "ArmorManager.h"
#include "Settings.h"

namespace Papyrus
{
	float GetRefScale(RE::StaticFunctionTag*, RE::TESObjectREFR* ref)
	{
		if (!ref)
			return 1;
		return ref->refScale * 0.01f;
	}

	void ChangeDragonSkin(RE::StaticFunctionTag*, std::string name)
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		Armor::ApplySkin(player, name);
	}

	std::vector<RE::BSFixedString> GetDragonSkinList(RE::StaticFunctionTag*)
	{
		return Armor::GetSkinNameList();
	}

	void SetFloat(RE::StaticFunctionTag*, std::string name, float value)
	{
		Settings::GetSingleton()->Set(name, value);
		Settings::GetSingleton()->Save();
	}

	void SetInt(RE::StaticFunctionTag*, std::string name, int value)
	{
		Settings::GetSingleton()->Set(name, value);
		Settings::GetSingleton()->Save();
	}

	void SetBool(RE::StaticFunctionTag*, std::string name, bool value)
	{
		Settings::GetSingleton()->Set(name, value);
		Settings::GetSingleton()->Save();
	}

	void SetString(RE::StaticFunctionTag*, std::string name, std::string value)
	{
		Settings::GetSingleton()->Set(name, value);
		Settings::GetSingleton()->Save();
	}

	float GetFloat(RE::StaticFunctionTag*, std::string name)
	{
		return Settings::GetSingleton()->Get<float>(name);
	}

	int GetInt(RE::StaticFunctionTag*, std::string name)
	{
		return Settings::GetSingleton()->Get<int>(name);
	}

	bool GetBool(RE::StaticFunctionTag*, std::string name)
	{
		return Settings::GetSingleton()->Get<bool>(name);
	}

	std::string GetString(RE::StaticFunctionTag*, std::string name)
	{
		return Settings::GetSingleton()->Get<std::string>(name);
	}

	void ApplyAll(RE::StaticFunctionTag*)
	{
		Settings::GetSingleton()->ApplyAll();
	}

	void UpdateBreathShouts(RE::StaticFunctionTag*, bool isBeast)
	{
		Ability::UpdateBreathShouts(isBeast);
	}

	bool Bind(VM* vm)
	{
		if (!vm) {
			logger::critical("couldn't get VM State");
			return false;
		}

		vm->RegisterFunction("ChangeDragonSkin", "PDDragon", ChangeDragonSkin);
		vm->RegisterFunction("GetDragonSkinList", "PDDragon", GetDragonSkinList);
		vm->RegisterFunction("GetRefScale", "PDDragon", GetRefScale);
		vm->RegisterFunction("SetFloat", "PDDragon", SetFloat);
		vm->RegisterFunction("SetInt", "PDDragon", SetInt);
		vm->RegisterFunction("SetBool", "PDDragon", SetBool);
		vm->RegisterFunction("SetString", "PDDragon", SetString);
		vm->RegisterFunction("GetFloat", "PDDragon", GetFloat);
		vm->RegisterFunction("GetInt", "PDDragon", GetInt);
		vm->RegisterFunction("GetBool", "PDDragon", GetBool);
		vm->RegisterFunction("GetString", "PDDragon", GetString);
		vm->RegisterFunction("ApplyAll", "PDDragon", ApplyAll);
		vm->RegisterFunction("UpdateBreathShouts", "PDDragon", UpdateBreathShouts);
		return true;
	}

}
