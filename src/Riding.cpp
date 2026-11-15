#include "Settings.h"
#include "State.h"
#include "Utils.h"

namespace Riding
{
	bool IsOnAFlyingMount(RE::Actor* actor)
	{
		if (!actor)
			return false;

		RE::ActorPtr mountPtr;
		if (!actor->GetMount(mountPtr))
			return false;

		auto mount = mountPtr.get();
		auto race = mount ? mount->GetRace() : nullptr;

		if (!race || !race->data.flags.all(RE::RACE_DATA::Flag::kFlies))
			return false;

		if (!State::GetSingleton()->isRidingDriven)
			return true;

		if (!actor->IsPlayerRef())
			return true;

		return false;
	}

	namespace movement
	{
		class DisableDragonRandomMove
		{
		public:
			inline static void Install()
			{
				REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(39637, 40724), 0x9 };  //IsOnAFlyingMount

				auto& trampoline = SKSE::GetTrampoline();
				trampoline.write_call<5>(target.address(), IsOnAFlyingMount);
			}
		};

		class DisableHorseLikeControl
		{
		public:
			inline static void Install()
			{
				REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(40692, 41703) };

				auto& trampoline = SKSE::GetTrampoline();
				_func = trampoline.write_call<5>(target.address() + OFFSET(0xAD, 0xB2), Func);

				_func1 = Utils::ReadDisp<5>(target.address() + OFFSET(0xB7, 0xBC));
			}

		private:
			inline static void Func(RE::MovementControllerNPC* movementController)
			{
				if (!movementController)
					return _func(movementController);

				auto actor = *(RE::Actor**)((uintptr_t)movementController + 0x1B8);
				if (Utils::IsDragonRace(actor))
					return _func1(movementController);

				return _func(movementController);
			}

			static inline REL::Relocation<decltype(Func)> _func;
			static inline REL::Relocation<decltype(Func)> _func1;
		};
	}

	class HookMount
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(39638, 40725), OFFSET(0x17A, 0x1FF) };

			auto& trampoline = SKSE::GetTrampoline();
			_func = trampoline.write_call<5>(target.address(), Func);
		}

	private:
		inline static bool Func(RE::Actor* actor)
		{
			if (_func(actor))
				return true;

			State::GetSingleton()->isRidingDriven = Settings::GetSingleton()->Get<bool>("RidingControl");

			if (Utils::IsDragonRace(actor) && Settings::GetSingleton()->Get<bool>("RidingControl")) {
				actor->SetGraphVariableBool("iIsPlayer", true);
				auto movementController = actor->movementController;
				if (movementController)
					movementController->SetControlsDriven();
			}

			return false;
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class HookDismount
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(39273, 40352), OFFSET(0x104, 0x105) };

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), Func);
		}

	private:
		inline static RE::BSTSmartPointer<RE::MovementControllerNPC>* Func(RE::Actor* actor)
		{
			if (!actor)
				return nullptr;

			actor->SetGraphVariableBool("iIsPlayer", false);
			return &actor->movementController;
		}
	};

	class DisableTargetEnemy
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(41349, 42423), 0x10 };  //IsOnAFlyingMount

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), IsOnAFlyingMount);
		}
	};

	class PlayerRidingCrashFix
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };  //PlayerCharacter

			_func = vtbl.write_vfunc(0x6C, Func);
		}

	private:
		inline static void Func(RE::PlayerCharacter* player, RE::NiAVObject* object, bool queue3DTasks)
		{
			if (!player || !player->IsOnMount())
				_func(player, object, queue3DTasks);
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class HookDrawWeaponMagicHands
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };  //PlayerCharacter

			_func = vtbl.write_vfunc(0xA6, Func);
		}

	private:
		inline static void Func(RE::PlayerCharacter* player, bool draw)
		{
			_func(player, draw);
			if (player && player->IsOnMount()) {
				auto extra = player->extraList.GetByType<RE::ExtraInteraction>();
				auto interaction = extra ? extra->interaction.get() : nullptr;
				auto actorPtr = interaction ? interaction->actor.get() : nullptr;
				auto mount = actorPtr ? actorPtr.get() : nullptr;
				if (Utils::IsPlayerControlledDragon(mount)) {
					mount->DrawWeaponMagicHands(draw);
				}
			}
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class HookDrawSheatheWeaponHandler
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_Character[6] };  //Character

			_func = vtbl.write_vfunc(0x15, Func);
		}

	private:
		inline static void Func(RE::ActorState* actorState, bool flag)
		{
			_func(actorState, flag);

			auto actor = actorState ? static_cast<RE::Actor*>(actorState) : nullptr;

			if (!Utils::IsPlayerControlledDragon(actor))
				return;

			auto player = RE::PlayerCharacter::GetSingleton();
			if (actor && player->IsOnMount() && actor->IsAMount() && actor->GetRace() && actor->GetRace()->data.flags.any(RE::RACE_DATA::Flag::kFlies)) {
				auto extra = player->extraList.GetByType<RE::ExtraInteraction>();
				auto interaction = extra ? extra->interaction.get() : nullptr;
				auto actorPtr = interaction ? interaction->actor.get() : nullptr;
				auto mount = actorPtr ? actorPtr.get() : nullptr;
				if (mount && actor == mount) {
					auto actorstate = static_cast<RE::ActorState*>(player);

					((void (*)(RE::ActorState*, bool)) * (uintptr_t*)((*(uintptr_t*)(actorstate)) + 0xA8))(actorstate, flag);
				}
			}
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	void Install()
	{
		movement::DisableDragonRandomMove::Install();
		movement::DisableHorseLikeControl::Install();

		HookMount::Install();
		HookDismount::Install();

		DisableTargetEnemy::Install();
		PlayerRidingCrashFix::Install();

		HookDrawWeaponMagicHands::Install();
		HookDrawSheatheWeaponHandler::Install();
	}
}
