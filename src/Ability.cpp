#include "Ability.h"
#include "FormManager.h"
#include "Settings.h"
#include "Utils.h"

namespace Ability
{
	void UpdateBreathShouts(bool isBeast)
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		auto mgr = Form::Manager::GetSingleton();
		if (!player)
			return;

		bool useDynamicSwap = Settings::GetSingleton()->Get<bool>("UseDynamicSpellSwap");

		auto fireBreathShout = mgr->FireBreathShout.GetPtr();
		auto frostBreathShout = mgr->FrostBreathShout.GetPtr();

		if (fireBreathShout) {
			auto targetSpell2 = mgr->originalFireSpell2;
			auto targetSpell3 = mgr->originalFireSpell3;

			if (isBeast && useDynamicSwap) {
				auto firePerk = mgr->PDselfFire2.GetPtr();
				if (firePerk && player->HasPerk(firePerk)) {
					targetSpell2 = mgr->PDVoiceFireball2.GetPtr();
					targetSpell3 = mgr->PDVoiceFireBreath3.GetPtr();
				}
			}

			if (fireBreathShout->variations[1].spell != targetSpell2)
				fireBreathShout->variations[1].spell = targetSpell2;
			if (fireBreathShout->variations[2].spell != targetSpell3)
				fireBreathShout->variations[2].spell = targetSpell3;
		}

		if (frostBreathShout) {
			auto targetSpell2 = mgr->originalFrostSpell2;
			auto targetSpell3 = mgr->originalFrostSpell3;

			if (isBeast && useDynamicSwap) {
				auto frostPerk = mgr->PDselfFrost2.GetPtr();
				if (frostPerk && player->HasPerk(frostPerk)) {
					targetSpell2 = mgr->PDVoiceIceStorm2.GetPtr();
					targetSpell3 = mgr->PDVoiceFrostBreath3.GetPtr();
				}
			}

			if (frostBreathShout->variations[1].spell != targetSpell2)
				frostBreathShout->variations[1].spell = targetSpell2;
			if (frostBreathShout->variations[2].spell != targetSpell3)
				frostBreathShout->variations[2].spell = targetSpell3;
		}

		auto PDfireBreathShout = mgr->PDDragonFireBreathShout.GetPtr();
		auto PDfrostBreathShout = mgr->PDDragonFrostBreathShout.GetPtr();

		if (PDfireBreathShout) {
			auto firePerk = mgr->PDselfFire2.GetPtr();
			bool shouldHaveShout = (isBeast && !useDynamicSwap && firePerk && player->HasPerk(firePerk));
			bool currentlyHasShout = player->HasShout(PDfireBreathShout);

			if (shouldHaveShout && !currentlyHasShout) {
				player->AddShout(PDfireBreathShout);
				auto fireWord = mgr->PDWordYol.GetPtr();
				if (fireWord) {
					player->UnlockWord(fireWord);
					fireWord->formFlags |= RE::TESForm::RecordFlags::kKnown;
					fireWord->AddChange(RE::TESForm::ChangeFlags::kFlags);
				}
			} else if (!shouldHaveShout && currentlyHasShout) {
				Utils::RemoveShout(player, PDfireBreathShout);
			}
		}

		if (PDfrostBreathShout) {
			auto frostPerk = mgr->PDselfFrost2.GetPtr();
			bool shouldHaveShout = (isBeast && !useDynamicSwap && frostPerk && player->HasPerk(frostPerk));
			bool currentlyHasShout = player->HasShout(PDfrostBreathShout);

			if (shouldHaveShout && !currentlyHasShout) {
				player->AddShout(PDfrostBreathShout);
				auto frostWord = mgr->PDWordFo.GetPtr();
				if (frostWord) {
					player->UnlockWord(frostWord);
					frostWord->formFlags |= RE::TESForm::RecordFlags::kKnown;
					frostWord->AddChange(RE::TESForm::ChangeFlags::kFlags);
				}
			} else if (!shouldHaveShout && currentlyHasShout) {
				Utils::RemoveShout(player, PDfrostBreathShout);
			}
		}
	}
}
