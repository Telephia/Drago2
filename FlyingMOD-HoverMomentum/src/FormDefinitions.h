#pragma once

#define PD_ESP_NAME				"PlayableDragon.esp"

#define PD_FORM_LIST(V)                                          \
	V(PDabDragon, RE::SpellItem, 0x0EF996, PD_ESP_NAME)          \
	V(PDTransformQST, RE::TESQuest, 0x0EA8C0, PD_ESP_NAME)       \
	V(PDtransformEffect, RE::EffectSetting, 0x0EA8BE, PD_ESP_NAME) \
	V(PDselfMassHP2, RE::BGSPerk, 0x0F8B26, PD_ESP_NAME)         \
	V(PDselfFire2, RE::BGSPerk, 0x0FA0D5, PD_ESP_NAME)           \
	V(PDselfFrost2, RE::BGSPerk, 0x0FA0D7, PD_ESP_NAME)          \
	V(PDVoiceFireball2, RE::SpellItem, 0x0F0466, PD_ESP_NAME)    \
	V(PDVoiceFireBreath3, RE::SpellItem, 0x0F0462, PD_ESP_NAME)  \
	V(PDVoiceIceStorm2, RE::SpellItem, 0x0F0467, PD_ESP_NAME)    \
	V(PDVoiceFrostBreath3, RE::SpellItem, 0x0F0463, PD_ESP_NAME) \
	V(PDskillPerkPoints, RE::TESGlobal, 0x0F066D, PD_ESP_NAME)   \
	V(PDselfFearfulPresence2, RE::BGSPerk, 0x0F9B63, PD_ESP_NAME)   \
	V(PDperkFearfulPresence1, RE::SpellItem, 0x0FAB9E, PD_ESP_NAME)   \
	V(PDperkFearfulPresence2, RE::SpellItem, 0x0FAB9F, PD_ESP_NAME)   \
	V(PDperkFearfulPresence3, RE::SpellItem, 0x0FABA2, PD_ESP_NAME)   \
	V(PDDragonFireBreathShout, RE::TESShout, 0x20CC69, PD_ESP_NAME)   \
	V(PDDragonFrostBreathShout, RE::TESShout, 0x20CC6A, PD_ESP_NAME)   \
	V(PDWordYol, RE::TESWordOfPower, 0x211D6B, PD_ESP_NAME)   \
	V(PDWordFo, RE::TESWordOfPower, 0x211D6C, PD_ESP_NAME)   

#define PD_VANILLA_FORM_LIST(V)                                                 \
	V(DragonRace, RE::TESRace, 0x012E82, "Skyrim.esm")   \
	V(DefaultRace, RE::TESRace, 0x000019, "Skyrim.esm")   \
	V(ActorTypeDragon, RE::BGSKeyword, 0x035D59, "Skyrim.esm")   \
	V(DragonPerchRockL02, RE::TESFurniture, 0x0AA934, "Skyrim.esm")   \
	V(DragonPerchTower, RE::TESFurniture, 0x01E595, "Skyrim.esm")   \
	V(DragonPerchWordwall, RE::TESFurniture, 0x01CC4A, "Skyrim.esm")   \
	V(KillMoveFlightDragonKillSnatchStandardActionFailed, RE::TESIdleForm, 0x0C9A21, "Skyrim.esm")   \
	V(KillMoveFlightDragonKillSnatchStandardAction, RE::TESIdleForm, 0x0C9A20, "Skyrim.esm")   \
	V(AbFortifyHealth, RE::EffectSetting, 0x0A1A3F, "Skyrim.esm")   \
	V(AbFortifyCarryWeight, RE::EffectSetting, 0x10CED9, "Skyrim.esm")   \
	V(AbFortifyUnarmedDamage, RE::EffectSetting, 0x0424E2, "Skyrim.esm")   \
	V(FXDragonTakeoffImpactSet, RE::BGSImpactDataSet, 0x019A01, "Skyrim.esm")   \
	V(FXDragonLandingImpactSet, RE::BGSImpactDataSet, 0x02BD39, "Skyrim.esm")   \
	V(FXDragonTailstompImpactSet, RE::BGSImpactDataSet, 0x03F819, "Skyrim.esm") \
	V(NPCDragonFlyby, RE::TESSound, 0x018B58, "Skyrim.esm") \
	V(FireBreathShout, RE::TESShout, 0x03F9EA, "Skyrim.esm")                    \
	V(FrostBreathShout, RE::TESShout, 0x05D16B, "Skyrim.esm")\
	V(VoiceDismayingShout1, RE::SpellItem, 0x02395B, "Skyrim.esm")\
	V(VoiceDismayingShout2, RE::SpellItem, 0x02395E, "Skyrim.esm")\
	V(VoiceDismayingShout3, RE::SpellItem, 0x023967, "Skyrim.esm")

