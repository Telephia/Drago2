#pragma once

namespace Armor
{
	using SkinList = std::vector<std::pair<std::string, std::vector<RE::TESObjectARMO*>>>;

	class Manager : public REX::Singleton<Manager>
	{
	public:
		SkinList _dragonSkinList;
	};

	void                           ApplySkin(RE::PlayerCharacter* player, const std::string& name);
	std::vector<RE::BSFixedString> GetSkinNameList();
}
