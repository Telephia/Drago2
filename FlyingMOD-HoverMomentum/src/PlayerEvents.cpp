#include "Ability.h"
#include "FormManager.h"
#include "Settings.h"
#include "Utils.h"

namespace PlayerEvents
{
	class SpellCastEventHandler : public RE::BSTEventSink<RE::TESSpellCastEvent>
	{
	public:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent* event, RE::BSTEventSource<RE::TESSpellCastEvent>*) override
		{
			auto player = RE::PlayerCharacter::GetSingleton();
			auto mgr = Form::Manager::GetSingleton();
			auto selfFearfulPerk = mgr->PDselfFearfulPresence2.GetPtr();
			if (event->object.get() == player && Utils::IsDragonRace(player) && selfFearfulPerk && player->HasPerk(selfFearfulPerk)) {
				float dismayRecoverTime1 = 3.0;
				float dismayRecoverTime2 = 6.0;
				float dismayRecoverTime3 = 9.0;

				auto magicCaster = player->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);

				if (magicCaster) {
					if (event->spell == mgr->VoiceDismayingShout1.GetID()) {
						if (auto perk = mgr->PDperkFearfulPresence1.GetPtr()) {
							magicCaster->CastSpellImmediate(perk, false, nullptr, 1, false, 0, nullptr);
						}

						if (player->GetVoiceRecoveryTime() > dismayRecoverTime1) {
							SetVoiceRecoveryTime(player, dismayRecoverTime1);
						}
					} else if (event->spell == mgr->VoiceDismayingShout2.GetID()) {
						if (auto perk = mgr->PDperkFearfulPresence2.GetPtr()) {
							magicCaster->CastSpellImmediate(perk, false, nullptr, 1, false, 0, nullptr);
						}

						if (player->GetVoiceRecoveryTime() > dismayRecoverTime2) {
							SetVoiceRecoveryTime(player, dismayRecoverTime2);
						}
					} else if (event->spell == mgr->VoiceDismayingShout3.GetID()) {
						if (auto perk = mgr->PDperkFearfulPresence3.GetPtr()) {
							magicCaster->CastSpellImmediate(perk, false, nullptr, 1, false, 0, nullptr);
						}

						if (player->GetVoiceRecoveryTime() > dismayRecoverTime3) {
							SetVoiceRecoveryTime(player, dismayRecoverTime3);
						}
					}
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}

		void SetVoiceRecoveryTime(RE::Actor* actor, float recoveryTime)
		{
			if (!actor)
				return;

			auto high = actor->GetHighProcess();
			if (high)
				high->voiceRecoveryTime = recoveryTime;
		}
	};

	class RaceSwitchEventHandler : public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent>
	{
	public:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override
		{
			if (event->subject.get() == RE::PlayerCharacter::GetSingleton()) {
				auto arg = RE::MakeFunctionArguments();
				Utils::CallQuestFunction("OnPlayerRaceSwitchComplete", arg);
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	class MenuOpenCloseEventHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
		{
			if (event->menuName == "StatsMenu") {
				auto perkPoints = Form::Manager::GetSingleton()->PDskillPerkPoints.GetPtr();
				auto player = RE::PlayerCharacter::GetSingleton();
				if (perkPoints && player) {
					float dragonSouls = player->GetActorValue(RE::ActorValue::kDragonSouls);
					if (event->opening) {
						perkPoints->value = dragonSouls;
					} else if (perkPoints->value != dragonSouls) {
						Ability::UpdateBreathShouts(true);
						player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kDragonSouls, perkPoints->value - dragonSouls);
					}
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	class OnEffectAdded
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[4] };  //PlayerCharacter

			_func = vtbl.write_vfunc(0x8, Func);
		}

	private:
		inline static void Func(RE::MagicTarget* magicTarget, RE::ActiveEffect* activeEffect)
		{
			auto baseEffect = activeEffect ? activeEffect->GetBaseObject() : nullptr;
			if (baseEffect) {
				auto player = RE::PlayerCharacter::GetSingleton();
				auto actor = magicTarget ? magicTarget->GetTargetAsActor() : nullptr;

				if (player && actor && actor == player) {
					if (baseEffect->GetFormID() == Form::Manager::GetSingleton()->PDtransformEffect.GetID()) {
						if (player->GetFlyState() != RE::FLY_STATE::kNone) {
							auto                                                     arg = RE::MakeFunctionArguments(std::move(std::string("pddragon")));
							RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
							RE::BSScript::Internal::VirtualMachine::GetSingleton()->DispatchStaticCall("CustomSkills", "OpenCustomSkillMenu", arg, callback);

							activeEffect->Dispel(true);

						} else {
							auto arg = RE::MakeFunctionArguments();
							Utils::CallQuestFunction("StartTransform", arg);
						}
					}

					auto spell = activeEffect->spell;

					if (spell) {
						auto mgr = Form::Manager::GetSingleton();
						if (spell->GetFormID() == mgr->PDabDragon.GetID()) {
							if (baseEffect->GetFormID() == mgr->AbFortifyHealth.GetID()) {
								float addHealth = 0;
								auto  perk = mgr->PDselfMassHP2.GetPtr();
								if (player->HasPerk(perk))
									addHealth += 54.161f * player->GetLevel() + 138.71f;

								addHealth += Settings::GetSingleton()->Get<float>("FortifyHealthMag");
								activeEffect->magnitude = addHealth;
							} else if (baseEffect->GetFormID() == mgr->AbFortifyCarryWeight.GetID()) {
								activeEffect->magnitude = Settings::GetSingleton()->Get<float>("FortifyCarryWeightMag");
							} else if (baseEffect->GetFormID() == mgr->AbFortifyUnarmedDamage.GetID()) {
								activeEffect->magnitude = Settings::GetSingleton()->Get<float>("FortifyUnarmedDamageMag");
							}
						}
					}
				}
			}

			_func(magicTarget, activeEffect);
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	RaceSwitchEventHandler    g_raceSwitchEventHandler;
	SpellCastEventHandler     g_spellCastEventHandler;
	MenuOpenCloseEventHandler g_menuOpenCloseEventHandler;

	void Install()
	{
		RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(&g_raceSwitchEventHandler);
		RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(&g_spellCastEventHandler);

		RE::UI::GetSingleton()->AddEventSink(&g_menuOpenCloseEventHandler);

		OnEffectAdded::Install();
	}
}
