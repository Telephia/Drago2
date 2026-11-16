#include "Settings.h"
#include "Utils.h"

#include <chrono>
#include <cmath>

namespace Movement
{
	class SpeedFeedback
	{
	public:
		inline static void Install()
		{
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36365, 37356) };

			auto setSpeedOffset = OFFSET(0x6B3, 0x6F9);
			auto setCurrentMovementSpeedOffset = OFFSET(0x2DD, 0x375);

			trampoline.write_call<5>(target.address() + setSpeedOffset, Func);
			std::uint8_t patch[] = { 0x48, 0x8B, 0xCF, 0x90, 0x90, 0x90, 0x90 };
			REL::safe_write(target.address() + setSpeedOffset - 7, patch, 7);

			trampoline.write_call<5>(target.address() + setCurrentMovementSpeedOffset, Func2);

			std::uint8_t patch2[] = { 0x48, 0x8B, 0xCF, 0x48, 0x8D, 0x54, 0x24, 0x5C, 0x90, 0x90, 0x90, 0x90, 0x90 };
			REL::safe_write(target.address() + setCurrentMovementSpeedOffset - 13, patch2, 13);
		}

	private:
		inline static void Func(RE::Actor* actor, float speed)
		{
			if (!actor)
				return;

			auto middleHigh = actor->currentProcess ? actor->currentProcess->middleHigh : nullptr;
			if (!Utils::IsPlayerControlledDragon(actor) && middleHigh)
				middleHigh->unk2AC = speed;
		}

		inline static void Func2(RE::Actor* actor, float* currentMovementSpeed)
		{
			if (!actor)
				return;

			auto middleHigh = actor->currentProcess ? actor->currentProcess->middleHigh : nullptr;
			if (!middleHigh)
				return;

			middleHigh->currentMovementSpeed = *currentMovementSpeed;

			if (!Utils::IsPlayerControlledDragon(actor))
				return;

			bool bSpeedSynced = false;
			actor->GetGraphVariableBool("bSpeedSynced", bSpeedSynced);

			if (!bSpeedSynced)
				return;

			float speed = 0.0f;
			actor->GetGraphVariableFloat("Speed", speed);

			middleHigh->currentMovementSpeed = speed;

			auto highProcess = actor->currentProcess->high;
			if (highProcess) {
				auto playerControls = RE::PlayerControls::GetSingleton();
				auto data = playerControls->data;
				auto movingForward = data.moveInputVec.y * Settings::GetSingleton()->Get<float>("FlySpeedMult") / 2;

				auto speeds = highProcess->currentMovementType.defaultData.speeds;

				if (data.running) {
					middleHigh->unk2AC = movingForward * speeds[RE::Movement::SPEED_DIRECTION::kForward][RE::Movement::MaxSpeeds::kRun];
				} else {
					middleHigh->unk2AC = movingForward * speeds[RE::Movement::SPEED_DIRECTION::kForward][RE::Movement::MaxSpeeds::kWalk];
				}
			}
		}
	};

	class EnableCollisionWhenFlying
	{
	public:
		inline static void Install()
		{
			struct Code : Xbyak::CodeGenerator
			{
				Code(std::uintptr_t target)
				{
					Xbyak::Label retnLabel, retnLabel2;

					mov(rcx, rsi);
					mov(rdx, r14);
					mov(rax, (uintptr_t)&Func);
					call(rax);
					test(al, al);
					jz(retnLabel2);
					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
#ifdef SKYRIM_AE
					dq(target + 0x2B2);
#else
					dq(target + 0x323);
#endif

					L(retnLabel2);
					jmp(ptr[rip]);
					dq(target + 0x1C);
				}
			};

			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(76436, 78275), OFFSET(0xED, 0xF0) };

			Code code(target.address());
			code.ready();

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_jmp<6>(target.address(), trampoline.allocate(code));
		}

	private:
		struct Arg
		{
			float        framerate;
			RE::NiPoint3 rot;
			RE::NiPoint3 vec;
			float        speed;
		};

		struct MomentumState
		{
			RE::Actor*   lastActor = nullptr;
			RE::NiPoint3 momentum;
		};

		inline static MomentumState                          _momentumState{};
		inline static std::chrono::steady_clock::time_point _momentumLastCall{};

		inline static bool Func(RE::bhkCharacterController* charController, Arg* arg)
		{
			charController->pitchAngle = arg->rot.x;
			charController->rollAngle = arg->rot.y;

			RE::Actor* actor = nullptr;

			auto worldObject = charController->GetBodyImpl();
			if (worldObject && worldObject->HasProperty(1000)) {
				auto property = worldObject->GetProperty(1000);
				if (property)
					actor = reinterpret_cast<RE::Actor*>(property->value.data);
			}

			if (!Utils::IsPlayerControlledDragon(actor))
				return false;

			bool isSynced;
			if (actor && ((actor->GetGraphVariableBool("bIsSynced", isSynced) && isSynced) || actor->GetFlyState() == RE::FLY_STATE::kHovering))
				charController->pitchAngle = 0;

			auto proxy = static_cast<RE::bhkCharProxyController*>(charController)->GetCharacterProxy();

			if (!proxy || !proxy->shapePhantom || !proxy->shapePhantom->world)
				return false;

			if (Settings::GetSingleton()->Get<bool>("AutoLand")) {
				charController->CheckSupportImpl();
				if (charController->surfaceInfo.supportedState.all(RE::hkpSurfaceInfo::SupportedState::kSupported) && charController->surfaceInfo.surfaceVelocity.SqrLength4() == 0.0f) {
					if (actor->GetFlyState() == RE::FLY_STATE::kCruising)
						actor->NotifyAnimationGraph("FlyStopHastyVertical");
				}
			}

			RE::NiMatrix3 mat;
			mat.EulerAnglesToAxesZXY(arg->rot);

			float delta = arg->framerate;
			float invDelta = 1.0f / delta;

			RE::NiPoint3 vec = mat * arg->vec;

			// --- Hover Momentum patch -------------------------------------------------
			// When the engine transitions from gliding/cruising into hovering it
			// drops the per-frame displacement to zero immediately, which makes
			// the dragon stop dead in the air. Instead, keep the last motion and
			// bleed it off exponentially. "HoverMomentum" is the decay time in
			// seconds (0 disables the feature entirely).
			const float hoverMomentum = Settings::GetSingleton()->Get<float>("HoverMomentum");
			if (hoverMomentum > 0.0f) {
				const auto now      = std::chrono::steady_clock::now();
				const bool  staleCall = (now - _momentumLastCall) > std::chrono::milliseconds(250);
				_momentumLastCall    = now;

				if (staleCall || _momentumState.lastActor != actor) {
					// fresh session (actor change or hook was not called recently)
					_momentumState.lastActor = actor;
					_momentumState.momentum  = vec;
				} else {
					const auto flyState = actor->GetFlyState();
					if (flyState == RE::FLY_STATE::kCruising || flyState == RE::FLY_STATE::kHovering) {
						if (vec.Length() < _momentumState.momentum.Length()) {
							// decelerating: exponential decay, independent of frame rate
							const float blend = 1.0f - std::exp(-delta / hoverMomentum);
							_momentumState.momentum += (vec - _momentumState.momentum) * blend;
						} else {
							// accelerating or steady flight: follow the engine 1:1
							_momentumState.momentum = vec;
						}

						if (_momentumState.momentum.Length() < 0.01f) {
							_momentumState.momentum = RE::NiPoint3();
						}
					} else {
						// landing, perching or grounded: no carry-over
						_momentumState.momentum = RE::NiPoint3();
					}
				}

				vec = _momentumState.momentum;
			}
			// --- end Hover Momentum patch -------------------------------------------------

			vec *= invDelta;
			*(RE::NiPoint3*)&charController->unk180 = vec;

			vec *= 0.0142875f;
			charController->outVelocity = RE::hkVector4(vec);

			charController->stepInfo.deltaTime = delta;
			charController->stepInfo.invDeltaTime = invDelta;

			charController->Unk_0B();

			return true;
		}
	};

	class EnableTcWhenFlying
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_MovementControllerNPC[2] };

			_func = vtbl.write_vfunc(0x4, Func);
		}

	private:
		inline static void* Func(RE::IMovementMotionDrivenControl* controller)
		{
			if (!controller)
				return _func(controller);

			auto actor = *reinterpret_cast<RE::Actor**>((std::uintptr_t)controller + 0x90);
			auto flags = reinterpret_cast<std::uint8_t*>((std::uintptr_t)controller + 0x9C);  // 8 byte

			if (Utils::IsDragonRace(actor)) {
				if (flags[1] == 1) {
					flags[0] = 0;
				}
			}

			return _func(controller);
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class ChangeFlyStateCrashFix
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36922, 37947), OFFSET(0xAB, 0xA2) };

			auto& trampoline = SKSE::GetTrampoline();
			_func = trampoline.write_call<5>(target.address(), Func);
		}

	private:
		inline static std::uint64_t Func(RE::Actor* actor)
		{
			if (actor == RE::PlayerCharacter::GetSingleton())
				return 0;
			return _func(actor);
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class DisableDeferredKill
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36922, 37947), OFFSET(0x222, 0x210) };

			auto& trampoline = SKSE::GetTrampoline();
			_func = trampoline.write_call<5>(target.address(), Func);
		}

	private:
		inline static void Func(RE::Actor* actor, bool deferredKillFlag)
		{
			if (actor == RE::PlayerCharacter::GetSingleton())
				return;
			_func(actor, deferredKillFlag);
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	class PreventPlayerPushed
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_bhkCollisionFilter[1] };

			_func = vtbl.write_vfunc(0x1, Func);
		}

	private:
		inline static bool* Func(RE::hkpCollidableCollidableFilter* filter, bool* outCollisionEnabled, RE::hkpCollidable* collidableA, RE::hkpCollidable* collidableB)
		{
			if (!collidableA || !collidableB)
				return _func(filter, outCollisionEnabled, collidableA, collidableB);

			auto isCharControllerA = collidableA->GetCollisionLayer() == RE::COL_LAYER::kCharController;
			auto isCharControllerB = collidableB->GetCollisionLayer() == RE::COL_LAYER::kCharController;
			if (isCharControllerA && isCharControllerB) {
				auto player = RE::PlayerCharacter::GetSingleton();
				if (Utils::IsDragonRace(player)) {
					auto charController = player->GetCharController();

					if (charController) {
						auto proxy = static_cast<RE::bhkCharProxyController*>(charController)->GetCharacterProxy();

						if (proxy && proxy->shapePhantom && proxy->shapePhantom->userData) {
							auto userdata = (RE::bhkCachingShapePhantom*)proxy->shapePhantom->userData;
							auto phantom = (RE::hkpCachingShapePhantom*)userdata->referencedObject.get();
							if (phantom) {
								if (phantom->GetCollidable() == collidableA || phantom->GetCollidable() == collidableB) {
									*outCollisionEnabled = false;
									return outCollisionEnabled;
								}
							}
						}
					}
				}
			}

			return _func(filter, outCollisionEnabled, collidableA, collidableB);
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	void Install()
	{
		SpeedFeedback::Install();
		PreventPlayerPushed::Install();

		EnableCollisionWhenFlying::Install();

		EnableTcWhenFlying::Install();
		ChangeFlyStateCrashFix::Install();
		DisableDeferredKill::Install();
	}
}
