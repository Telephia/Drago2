#pragma once
#include "FormManager.h"

namespace Utils
{

	template <std::size_t N>
	static inline uintptr_t ReadDisp(uintptr_t src)
	{
		return src + N + *reinterpret_cast<int32_t*>(src + N - 4);
	}

	static inline bool EqualsIgnoreCase(const char* strA, const char* strB)
	{
		if (!strA || !strB)
			return false;
		size_t len_a = std::strlen(strA);
		size_t len_b = std::strlen(strB);
		if (len_a != len_b)
			return false;

		return std::equal(strA, strA + len_a, strB,
			[](char charA, char charB) {
				return std::tolower(static_cast<unsigned char>(charA)) ==
			           std::tolower(static_cast<unsigned char>(charB));
			});
	}

	static inline bool IsDragonRace(const RE::TESRace* race)
	{
		if (!race)
			return false;

		auto actorTypeDragon = Form::Manager::GetSingleton()->ActorTypeDragon.GetPtr();
		if (race->HasKeyword(actorTypeDragon))
			return true;

		return EqualsIgnoreCase(race->behaviorGraphs[0].GetModel(), "Actors\\Dragon\\DragonProject.hkx");
	}

	static inline bool IsDragonRace(const RE::Actor* actor)
	{
		if (!actor)
			return false;

		auto actorTypeDragon = Form::Manager::GetSingleton()->ActorTypeDragon.GetPtr();
		if (actor->HasKeyword(actorTypeDragon))
			return true;

		return IsDragonRace(actor->GetRace());
	}

	static inline bool IsPlayerControlledDragon(const RE::Actor* actor)
	{
		return IsDragonRace(actor) && actor->movementController && actor->movementController->controlsDriven;
	}

	static inline void TriggerAction(RE::TESActionData* action)
	{
		using func_t = decltype(&TriggerAction);

		REL::Relocation<func_t> func{ RELOCATION_ID(40551, 41557) };
		func(action);
	}

	static inline float RemoveShout(RE::Actor* actor, RE::TESShout* shout)
	{
		using func_t = decltype(&RemoveShout);
		static REL::Relocation<func_t> func{ RELOCATION_ID(37774, 38719) };
		return func(actor, shout);
	}

	static inline void CallQuestFunction(std::string name, RE::BSScript::IFunctionArguments* args)
	{
		RE::BSFixedString fn = name;
		const auto        form = Form::Manager::GetSingleton()->PDTransformQST.GetPtr();

		if (form) {
			const auto quest = form->As<RE::TESQuest>();

			if (quest && quest->IsRunning()) {
				auto vm = RE::SkyrimVM::GetSingleton();
				auto vmHandle = vm->handlePolicy.GetHandleForObject((RE::VMTypeID)quest->GetFormType(), quest);

				if (vmHandle && args) {
					vm->SendAndRelayEvent(vmHandle, &fn, args, nullptr);
				}
			}
		}
	}

}
