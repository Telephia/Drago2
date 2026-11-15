#include "ArmorManager.h"
#include "FormManager.h"
#include "Utils.h"

namespace Armor
{
	void ClearDragonSkin(RE::PlayerCharacter* player)
	{
		if (!player)
			return;

		auto inventory = player->GetInventory();
		for (const auto& item : inventory) {
			auto armor = item.first->As<RE::TESObjectARMO>();
			if (!armor)
				continue;

			auto race = armor->race;
			if (!Utils::IsDragonRace(race))
				continue;

			RE::ActorEquipManager::GetSingleton()->UnequipObject(player, item.first);
			player->RemoveItem(item.first, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
		}
	}

	void ApplySkin(RE::PlayerCharacter* player, const std::string& name)
	{
		if (!player)
			return;

		ClearDragonSkin(player);

		if (name == "") {
			return;
		}

		if (!Utils::IsDragonRace(player)) {
			return;
		}

		auto dragonSkinList = Manager::GetSingleton()->_dragonSkinList;
		auto it = std::find_if(dragonSkinList.begin(), dragonSkinList.end(),
			[&name](const std::pair<std::string, std::vector<RE::TESObjectARMO*>>& element) {
				return element.first == name;
			});

		if (it != dragonSkinList.end()) {
			auto inventory = player->GetInventory();
			for (const auto& armorItem : it->second) {
				if (inventory.find(armorItem) == inventory.end()) {
					player->AddObjectToContainer(armorItem, nullptr, 1, nullptr);
				}

				RE::ActorEquipManager::GetSingleton()->EquipObject(player, armorItem);
			}
		}
	}

	std::vector<RE::BSFixedString> GetSkinNameList()
	{
		std::vector<RE::BSFixedString> names;
		for (const auto& [name, _] : Manager::GetSingleton()->_dragonSkinList) {
			names.push_back(name);
		}
		return names;
	}

	RE::TESObjectARMO* CreateARMO()
	{
		REL::Relocation<RE::TESObjectARMO* (*)(RE::TESObjectARMO*)> Ctor{ RELOCATION_ID(17384, 17784) };

		return Ctor(RE::malloc<RE::TESObjectARMO>());
	}

	RE::TESObjectARMA* CreateARMA()
	{
		REL::Relocation<RE::TESObjectARMA* (*)(RE::TESObjectARMA*)> Ctor{ RELOCATION_ID(17354, 17752) };

		return Ctor(RE::malloc<RE::TESObjectARMA>());
	}

	RE::TESObjectARMA* CopyARMA(RE::TESObjectARMA* src)
	{
		if (!src)
			return nullptr;

		auto dest = CreateARMA();

		static_cast<RE::TESRaceForm*>(dest)->CopyComponent(static_cast<RE::TESRaceForm*>(src));
		static_cast<RE::BGSBipedObjectForm*>(dest)->CopyComponent(static_cast<RE::BGSBipedObjectForm*>(src));
		dest->data = src->data;
		dest->bipedModels[0] = src->bipedModels[0];
		dest->bipedModels[1] = src->bipedModels[1];
		dest->bipedModel1stPersons[0] = src->bipedModel1stPersons[0];
		dest->bipedModel1stPersons[1] = src->bipedModel1stPersons[1];
		dest->skinTextures[0] = src->skinTextures[0];
		dest->skinTextures[1] = src->skinTextures[1];
		dest->skinTextureSwapLists[0] = src->skinTextureSwapLists[0];
		dest->skinTextureSwapLists[1] = src->skinTextureSwapLists[1];
		dest->footstepSet = src->footstepSet;
		dest->artObject = src->artObject;
		return dest;
	}

	std::vector<RE::TESObjectARMO*> PatchArmor(const std::vector<RE::TESObjectARMO*> src)
	{
		auto                            dragonRace = Form::Manager::GetSingleton()->DragonRace.GetPtr();
		std::vector<RE::TESObjectARMO*> patchedArmorList;
		if (!dragonRace)
			return patchedArmorList;

		for (const auto& armorObj : src) {
			if (!armorObj)
				continue;

			RE::BSTArray<RE::TESObjectARMA*> updatedAddonList;
			bool                             isChanged = false;

			for (const auto& addonObj : armorObj->armorAddons) {
				if (!addonObj)
					continue;
				RE::TESObjectARMA* newArmorAddon = nullptr;

				if (armorObj->bipedModelData.bipedObjectSlots.all(RE::BIPED_MODEL::BipedObjectSlot::kLongHair)) {
					if (!newArmorAddon)
						newArmorAddon = CopyARMA(addonObj);
					if (newArmorAddon)
						newArmorAddon->bipedModelData.bipedObjectSlots.set(RE::BIPED_MODEL::BipedObjectSlot::kBody, RE::BIPED_MODEL::BipedObjectSlot::kHead);
				}
				if (addonObj->race != dragonRace && armorObj->armorAddons.size() == 1) {
					if (!newArmorAddon)
						newArmorAddon = CopyARMA(addonObj);
					if (newArmorAddon)
						newArmorAddon->race = dragonRace;
				}

				updatedAddonList.push_back(newArmorAddon ? newArmorAddon : addonObj);
				isChanged |= newArmorAddon != nullptr;
			}
			if (isChanged == true || armorObj->race != dragonRace) {
				auto newArmorObject = CreateARMO();
				newArmorObject->Copy(armorObj);
				newArmorObject->race = dragonRace;
				newArmorObject->armorAddons = updatedAddonList;
				patchedArmorList.push_back(newArmorObject);

			} else {
				patchedArmorList.push_back(armorObj);
			}
		}

		return patchedArmorList;
	}

	void Install()
	{
		auto armorManager = Manager::GetSingleton();
		auto formManager = Form::Manager::GetSingleton();
		const auto& [map, lock] = RE::TESForm::GetAllForms();
		const RE::BSReadWriteLock l{ lock };

		if (!map)
			return;

		std::map<std::set<RE::TESObjectARMO*>, std::vector<std::string>> armorToNames;

		for (const auto& [_, form] : *map) {
			auto* npc = form->As<RE::TESNPC>();
			if (!npc || npc->fullName == nullptr)
				continue;

			auto* race = npc->GetRace();
			if (!Utils::IsDragonRace(race))
				continue;

			std::set<RE::TESObjectARMO*> armorSet;

			if (npc->defaultOutfit) {
				for (const auto& item : npc->defaultOutfit->outfitItems) {
					if (auto* armor = item->As<RE::TESObjectARMO>()) {
						if (armor->race && armor->race->formID == formManager->DefaultRace.GetID())
							continue;
						armorSet.insert(armor);
					}
				}
			}

			RE::TESObjectARMO* skin = nullptr;
			if (npc->skin) {
				skin = npc->skin;
			} else if (race->skin && race->formID != formManager->DragonRace.GetID()) {
				skin = race->skin;
			}
			if (skin)
				armorSet.insert(skin);

			if (!armorSet.empty()) {
				armorToNames[armorSet].push_back(npc->fullName.c_str());
			}
		}

		std::unordered_map<std::string, std::vector<RE::TESObjectARMO*>> finalArmorMap;

		for (const auto& [armoSet, names] : armorToNames) {
			std::unordered_map<std::string, size_t> counts;
			std::string                             bestName;
			size_t                                  maxCount = 0;

			for (const auto& name : names) {
				if (++counts[name] > maxCount) {
					maxCount = counts[name];
					bestName = name;
				}
			}

			std::string finalName = bestName + " ";
			int         suffix = 0;
			while (finalArmorMap.contains(finalName)) {
				suffix++;
				finalName = bestName + " " + std::to_string(suffix);
			}
			finalArmorMap[finalName] = std::vector<RE::TESObjectARMO*>(armoSet.begin(), armoSet.end());
		}

		SkinList dragonSkinList;
		for (const auto& [name, armors] : finalArmorMap) {
			dragonSkinList.push_back({ name, PatchArmor(armors) });
		}

		std::sort(dragonSkinList.begin(), dragonSkinList.end());
		armorManager->_dragonSkinList = std::move(dragonSkinList);
	}
}
