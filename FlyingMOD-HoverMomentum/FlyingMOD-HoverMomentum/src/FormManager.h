#pragma once

#include "FormDefinitions.h"

namespace Form
{
	template <typename T>
	struct FormEntry
	{
		RE::FormID id = 0;
		T*         ptr = nullptr;

		RE::FormID GetID() const { return id; }

		T* GetPtr() const { return ptr; }
	};

	class Manager : public REX::Singleton<Manager>
	{
	public:
#define GEN_MEMBER(name, type, id, esp) FormEntry<type> name;
		PD_FORM_LIST(GEN_MEMBER)
		PD_VANILLA_FORM_LIST(GEN_MEMBER)
#undef GEN_MEMBER

		RE::SpellItem* originalFireSpell2 = nullptr;
		RE::SpellItem* originalFireSpell3 = nullptr;
		RE::SpellItem* originalFrostSpell2 = nullptr;
		RE::SpellItem* originalFrostSpell3 = nullptr;
	};
}
