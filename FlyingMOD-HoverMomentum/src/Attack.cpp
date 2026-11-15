#include "Attack.h"
#include "FormManager.h"
#include "Settings.h"
#include "Utils.h"

namespace Attack
{
	static inline int originalCollisionGroupB = 0;

	class FlightActionStartEnd
	{
	public:
		inline static void Install()
		{
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> targetBase{ RELOCATION_ID(36767, 37783) };

			{
				struct Code : Xbyak::CodeGenerator
				{
					Code(std::uintptr_t target)
					{
						Xbyak::Label retnLabel;
#ifdef SKYRIM_AE
						mov(ptr[rsp + 0x78 - 0x58], rdi);
#else
						mov(ptr[rsp + 0x88 - 0x60], rsi);
#endif

						mov(rax, (uintptr_t)&Utils::IsPlayerControlledDragon);
						call(rax);
						test(al, al);
						jz(retnLabel);
#ifdef SKYRIM_AE
						lea(r15d, ptr[rdi - 1]);
#else
						lea(r14d, ptr[rsi - 1]);
#endif
						jmp(ptr[rip]);
						dq(target + 0x3E);

						L(retnLabel);
#ifdef SKYRIM_AE
						mov(rcx, rsi);
#else
						mov(rcx, rdi);
#endif
						jmp(ptr[rip]);
						dq(target + 0x5);
					}
				};

				Code code(targetBase.address() + 0x2A);
				code.ready();

				trampoline.write_branch<5>(targetBase.address() + 0x2A, trampoline.allocate(code));
			}

			{
				struct Code : Xbyak::CodeGenerator
				{
					Code(std::uintptr_t target)
					{
						Xbyak::Label retnLabel;

						test(r13, r13);
						jnz(retnLabel);

						mov(rax, (uintptr_t)&originalCollisionGroupB);
#ifdef SKYRIM_AE
						mov(r14d, ptr[rax]);
#else
						mov(r15d, ptr[rax]);
#endif

						jmp(ptr[rip]);
						dq(target + 0x7);

						L(retnLabel);
#ifdef SKYRIM_AE
						mov(r14d, ptr[r13 + 0x15C]);
#else
						mov(r15d, ptr[r13 + 0x15C]);
#endif

						jmp(ptr[rip]);
						dq(target + 0x7);
					}
				};

				auto targetAddr = targetBase.address() + OFFSET(0xBC, 0xB1);
				Code code(targetAddr);
				code.ready();

				trampoline.write_branch<6>(targetAddr, trampoline.allocate(code));
			}
			{
				struct Code : Xbyak::CodeGenerator
				{
					Code(std::uintptr_t target)
					{
						Xbyak::Label retnLabel;

						test(r13, r13);
						jnz(retnLabel);

						mov(rax, (uintptr_t)&Manager::GetSingleton()->_targetHandle);
						lea(rcx, ptr[rax]);

						jmp(ptr[rip]);
						dq(target + 0x7);

						L(retnLabel);
						lea(rcx, ptr[r13 + 0x12C]);

						jmp(ptr[rip]);
						dq(target + 0x7);
					}
				};

				auto targetAddr = targetBase.address() + OFFSET(0xC8, 0xBD);
				Code code(targetAddr);
				code.ready();

				trampoline.write_branch<6>(targetAddr, trampoline.allocate(code));
			}
			{
				struct Code : Xbyak::CodeGenerator
				{
					Code(std::uintptr_t target)
					{
						Xbyak::Label retnLabel;

						test(r13, r13);
						jnz(retnLabel);

						mov(rax, (uintptr_t)&originalCollisionGroupB);
#ifdef SKYRIM_AE
						mov(ptr[rax], esi);
#else
						mov(ptr[rax], ebx);
#endif

						jmp(ptr[rip]);
						dq(target + 0x7);

						L(retnLabel);
#ifdef SKYRIM_AE
						mov(ptr[r13 + 0x15C], esi);
#else
						mov(ptr[r13 + 0x15C], ebx);
#endif

						jmp(ptr[rip]);
						dq(target + 0x7);
					}
				};

				auto targetAddr = targetBase.address() + OFFSET(0x185, 0x178);
				Code code(targetAddr);
				code.ready();

				trampoline.write_branch<6>(targetAddr, trampoline.allocate(code));
			}
		}
	};

	class FlightActionGrabRelease
	{
	public:
		inline static void Install()
		{
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> targetBase{ RELOCATION_ID(41826, 42907) };

			{
				struct Code : Xbyak::CodeGenerator
				{
					Code(std::uintptr_t target)
					{
						Xbyak::Label retnLabel;
#ifdef SKYRIM_AE
						mov(ptr[rbp + 0x57 - 0x80], rdi);
#else
						mov(ptr[rbp + 0x57 - 0x80], r14);
#endif

						mov(rax, (uintptr_t)&Utils::IsPlayerControlledDragon);
						call(rax);
						test(al, al);
						jz(retnLabel);
#ifdef SKYRIM_AE
						lea(r12d, ptr[rdi - 1]);
#else
						lea(r12d, ptr[r14 - 1]);
#endif

						jmp(ptr[rip]);
						dq(target + 0x33);

						L(retnLabel);
						mov(rcx, r15);
						lea(rdx, ptr[rbp + 0x57 - 0x80]);

						jmp(ptr[rip]);
						dq(target + 0x8);
					}
				};

				auto targetAddr = targetBase.address() + OFFSET(0x43, 0x40);
				Code code(targetAddr);
				code.ready();

				trampoline.write_branch<6>(targetAddr, trampoline.allocate(code));
			}

			{
				struct Code : Xbyak::CodeGenerator
				{
					Code(std::uintptr_t target)
					{
						Xbyak::Label retnLabel;

						test(rax, rax);
						jnz(retnLabel);

						mov(rax, (uintptr_t)&Manager::GetSingleton()->_targetHandle);
						lea(rcx, ptr[rax]);

						jmp(ptr[rip]);
						dq(target + 0x7);

						L(retnLabel);
						lea(rcx, ptr[rax + 0x12C]);

						jmp(ptr[rip]);
						dq(target + 0x7);
					}
				};

				auto targetAddr = targetBase.address() + OFFSET(0x82, 0x7F);
				Code code(targetAddr);
				code.ready();

				trampoline.write_branch<6>(targetAddr, trampoline.allocate(code));
			}

			{
				struct Code : Xbyak::CodeGenerator
				{
					Code(std::uintptr_t target)
					{
						Xbyak::Label retnLabel;

						test(rax, rax);
						jnz(retnLabel);

						mov(rax, (uintptr_t)&originalCollisionGroupB);
						mov(eax, ptr[rax]);

						jmp(ptr[rip]);
						dq(target + 0x6);

						L(retnLabel);
						mov(eax, ptr[rax + 0x15C]);

						jmp(ptr[rip]);
						dq(target + 0x6);
					}
				};

				auto targetAddr = targetBase.address() + OFFSET(0x257, 0x259);
				Code code(targetAddr);
				code.ready();

				trampoline.write_branch<6>(targetAddr, trampoline.allocate(code));
			}
			{
				struct Code : Xbyak::CodeGenerator
				{
					Code(std::uintptr_t target)
					{
						Xbyak::Label retnLabel;

						test(rax, rax);
						jnz(retnLabel);

						mov(rax, (uintptr_t)&originalCollisionGroupB);

#ifdef SKYRIM_AE
						mov(ebx, ptr[rax]);
#else
						mov(edi, ptr[rax]);
#endif

						jmp(ptr[rip]);
						dq(target + 0x6);

						L(retnLabel);

#ifdef SKYRIM_AE
						mov(ebx, ptr[rax + 0x15C]);
#else
						mov(edi, ptr[rax + 0x15C]);
#endif

						jmp(ptr[rip]);
						dq(target + 0x6);
					}
				};

				auto targetAddr = targetBase.address() + OFFSET(0x30E, 0x310);
				Code code(targetAddr);
				code.ready();

				trampoline.write_branch<6>(targetAddr, trampoline.allocate(code));
			}
		}
	};

	class FlightActionEntryEnd
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(41769, 42850), 0x7 };

			auto& trampoline = SKSE::GetTrampoline();
			_func = trampoline.write_call<5>(target.address(), Func);
		}

	private:
		inline static void Func(RE::Actor* actor)
		{
			if (!Utils::IsPlayerControlledDragon(actor))
				return _func(actor);

			auto action = RE::BGSDefaultObjectManager::GetSingleton()->objects[RE::DEFAULT_OBJECTS::kActionRightPowerAttack]->As<RE::BGSAction>();

			auto actorPtr = Attack::Manager::GetSingleton()->_targetHandle.get();
			auto targetActor = actorPtr ? actorPtr.get() : nullptr;

			auto actionData = RE::TESActionData::Create();
			actionData->action = action;
			actionData->source = RE::TESObjectREFRPtr(actor);
			actionData->target = RE::TESObjectREFRPtr(targetActor);

			auto mgr = Form::Manager::GetSingleton();
			auto idleForm = mgr->KillMoveFlightDragonKillSnatchStandardActionFailed.GetPtr();
			if (targetActor && actor->data.location.GetSquaredDistance(targetActor->data.location) < 500 * 500) {
				float damage = actor->CalcUnarmedDamage() * Settings::GetSingleton()->Get<float>("AirSwipeDamageMult");

				targetActor->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -damage);

				if (!targetActor->IsEssential() && targetActor->GetActorValue(RE::ActorValue::kHealth) <= 0) {
					idleForm = mgr->KillMoveFlightDragonKillSnatchStandardAction.GetPtr();
				}
			}
			if (idleForm)
				actionData->animEvent = idleForm->animEventName;

			Utils::TriggerAction(actionData);
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class UnarmedReachMultiplier
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36479, 37478) };

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_branch<6>(target.address(), Func);
		}

	private:
		inline static float Func(RE::Actor* actor)
		{
			auto race = actor ? actor->GetRace() : nullptr;

			if (!race)
				return 0;

			float reach = race->data.unarmedReach;
			if (Utils::IsPlayerControlledDragon(actor))
				return reach * Settings::GetSingleton()->Get<float>("BiteReachMult");
			return reach;
		}
	};

	void Install()
	{
		FlightActionStartEnd::Install();
		FlightActionGrabRelease::Install();
		FlightActionEntryEnd::Install();
		UnarmedReachMultiplier::Install();
	}
}
