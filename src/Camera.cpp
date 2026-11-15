#include "Utils.h"

namespace Camera
{
	class CameraLockingFix
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36365, 37356), OFFSET(0xA50, 0xBF4) };  //TESObjectREFR__SetRotationX

			auto& trampoline = SKSE::GetTrampoline();
			_func = trampoline.write_call<5>(target.address(), Func);
		}

	private:
		inline static void Func(RE::Actor* actor, float rotationX)
		{
			if (actor && actor->IsPlayerRef() && Utils::IsDragonRace(actor)) {
				auto thirdPersonState = static_cast<RE::ThirdPersonState*>(RE::PlayerCamera::GetSingleton()->cameraStates[RE::CameraState::kThirdPerson].get());
				if (RE::PlayerCamera::GetSingleton()->currentState.get() == thirdPersonState && thirdPersonState->freeRotationEnabled) {
					bool isSynced;
					if (actor->GetGraphVariableBool("bIsSynced", isSynced) && isSynced)
						return _func(actor, actor->data.angle.x);
				}
			}

			_func(actor, rotationX);
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class CameraDistance
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(49975, 50911), OFFSET(0x111, 0x115) };

			auto& trampoline = SKSE::GetTrampoline();
			_func = trampoline.write_call<5>(target.address(), Func);
		}

	private:
		inline static bool Func(RE::PlayerCharacter* player)
		{
			if (!Utils::IsDragonRace(player))
				return _func(player);

			return true;
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class InjectCameraNode
	{
	public:
		inline static void Install()
		{
			auto& trampoline = SKSE::GetTrampoline();

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t target)
				{
#ifdef SKYRIM_AE
					mov(rcx, r12);
#else
					mov(rcx, r15);
#endif
					mov(rdx, rdi);
					mov(rax, (uintptr_t)&Func);
					call(rax);
#ifdef SKYRIM_AE
					cmp(qword[rbp + 0x300 + 0x18], 0);
#else
					cmp(qword[rbp + 0x310 + 0x28], 0);
#endif

					jmp(ptr[rip]);
					dq(target + 0x7);
				}
			};

			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(17260, 17653) };
			auto                            targetAdr = target.address() + OFFSET(0x34F, 0x43F);

			Code code(targetAdr);
			code.ready();

			trampoline.write_branch<6>(targetAdr, trampoline.allocate(code));
		}

	private:
		using ArgsType = RE::BSModelDB::DBTraits::ArgsType;

		inline static void Func(const char* path, RE::NiNode* node)
		{
			if (path && node && !strcmp(path, "Actors\\Dragon\\Character Assets\\Skeleton.nif")) {
				if (!_cameraNode) {
					RE::NiPointer<RE::NiNode> ptr = RE::NiPointer<RE::NiNode>();
					ArgsType                  arg = ArgsType();
					if (RE::BSModelDB::Demand("Actors\\Dragon\\Character Assets\\PDSkeleton.nif", ptr, arg) != RE::BSResource::ErrorCode::kNone)
						return;

					auto rootNode = ptr.get();
					if (!rootNode)
						return;

					auto childNode = rootNode->GetObjectByName("Camera3rd [Cam3]");

					if (!childNode)
						return;

					_cameraNode = childNode;
					_cameraNode->IncRefCount();

					if (auto npcEyeNode = rootNode->GetObjectByName("NPCEyeBone")) {
						_eyeNode = npcEyeNode;
						_eyeNode->IncRefCount();
					}
				}

				if (_cameraNode)
					node->AttachChild(_cameraNode);

				if (_eyeNode)
					node->AttachChild(_eyeNode);
			}
		}

		static inline RE::NiAVObject* _eyeNode = nullptr;
		static inline RE::NiAVObject* _cameraNode = nullptr;
	};

	void Install()
	{
		CameraLockingFix::Install();
		InjectCameraNode::Install();
		CameraDistance::Install();
	}
}
