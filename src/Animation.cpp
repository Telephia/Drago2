#include "Animation.h"
#include "Attack.h"
#include "FormManager.h"
#include "Settings.h"
#include "Utils.h"

namespace Animation
{

	class CrosshairDistanceChecker
	{
	public:
		CrosshairDistanceChecker() = delete;

		CrosshairDistanceChecker(float autoAimScreenPercentage) :
			_minScreenBound(autoAimScreenPercentage * -0.005f),
			_maxScreenBound(autoAimScreenPercentage * 0.005f),
			_minDistanceX(1),
			_minDistanceY(1),
			_nearestObject(nullptr)

		{}

		void AddCompare(RE::TESObjectREFR* objectRef)
		{
			if (!objectRef)
				return;

			auto        node3D = objectRef->Get3D2();
			RE::NiBound worldBound;
			if (node3D) {
				worldBound = node3D->worldBound;

			} else {
				worldBound = RE::NiBound(RE::NiPoint3(objectRef->data.location), 1.0f);
			}

			auto node = RE::Main::WorldRootNode();
			auto camera = node ? node->camera.get() : nullptr;

			if (!camera)
				return;

			RE::NiPoint3 smallBoundMin, smallBoundMax, largeBoundMin, largeBoundMax;

			ProjectBoundingSphereToNDC(camera, &worldBound, &smallBoundMin, &smallBoundMax, 0.00001f);

			worldBound.radius = objectRef->GetBoundMax().x;
			ProjectBoundingSphereToNDC(camera, &worldBound, &largeBoundMin, &largeBoundMax, 0.00001f);

			if ((fabs(largeBoundMin.x + largeBoundMax.x) < _minDistanceX || fabs(smallBoundMin.y + smallBoundMax.y) < _minDistanceY) && (largeBoundMin.x != -1.0 || largeBoundMax.x != 1.0) && (largeBoundMin.x >= _minScreenBound || largeBoundMax.x >= _minScreenBound) && (largeBoundMin.x <= _maxScreenBound || largeBoundMax.x <= _maxScreenBound) && (smallBoundMin.y != -1.0 || smallBoundMax.y != 1.0) && (smallBoundMin.y >= _minScreenBound || smallBoundMax.y >= _minScreenBound) && (smallBoundMin.y <= _maxScreenBound || smallBoundMax.y <= _maxScreenBound)) {
				_minDistanceX = fabs(largeBoundMin.x + largeBoundMax.x);
				_minDistanceY = fabs(smallBoundMin.y + smallBoundMax.y);
				_nearestObject = objectRef;
			}
		}

		template <class T>
		T* GetResult()
		{
			return reinterpret_cast<T*>(_nearestObject);
		}

	private:
		inline static void ProjectBoundingSphereToNDC(RE::NiCamera* camera, RE::NiBound* sphere, RE::NiPoint3* outMin, RE::NiPoint3* outMax, float minDepthThreshold)
		{
			using func_t = decltype(ProjectBoundingSphereToNDC);

			REL::Relocation<func_t> func{ RELOCATION_ID(69265, 70632) };

			return func(camera, sphere, outMin, outMax, minDepthThreshold);
		}

		float              _minScreenBound, _maxScreenBound, _minDistanceX, _minDistanceY;
		RE::TESObjectREFR* _nearestObject;
	};

	RE::ActorHandle GetNearCrosshairActor(RE::Actor* originActor, float distance, float percent)
	{
		if (!originActor)
			return RE::ActorHandle();

		auto player = RE::PlayerCharacter::GetSingleton();
		if (!player)
			return RE::ActorHandle();

		float autoAimMaxDistance = distance;  //10000;
		float maxDistanceSq = autoAimMaxDistance * autoAimMaxDistance;

		CrosshairDistanceChecker hostileChecker(percent);
		CrosshairDistanceChecker anyChecker(percent);

		for (auto handle : RE::ProcessLists::GetSingleton()->highActorHandles) {
			auto targetActorPtr = handle.get();
			auto targetActor = targetActorPtr ? targetActorPtr.get() : nullptr;

			if (!targetActor || targetActor->IsDead(false) || !targetActor->currentProcess || !targetActor->currentProcess->InHighProcess())
				continue;

			auto distSq = originActor->data.location.GetSquaredDistance(targetActor->data.location);
			if (distSq >= maxDistanceSq)
				continue;

			if (targetActor == player)
				continue;

			if (player->IsOnMount() && targetActor->IsAMount()) {
				auto extra = player->extraList.GetByType<RE::ExtraInteraction>();
				auto interaction = extra ? extra->interaction.get() : nullptr;
				auto actorPtr = interaction ? interaction->actor.get() : nullptr;
				auto actor = actorPtr ? actorPtr.get() : nullptr;
				if (targetActor == actor)
					continue;
			}

			anyChecker.AddCompare(targetActor);

			if (targetActor->IsHostileToActor(player)) {
				hostileChecker.AddCompare(targetActor);
			}
		}

		auto hostileActor = hostileChecker.GetResult<RE::Actor>();
		if (hostileActor) {
			return hostileActor->GetHandle();
		}

		auto anyActor = anyChecker.GetResult<RE::Actor>();
		if (anyActor) {
			return anyActor->GetHandle();
		}

		return RE::ActorHandle();
	}

	void SetTween(RE::Actor* actor, RE::NiPoint3& location, RE::NiPoint3& angle)
	{
		using func_t = decltype(SetTween);

		REL::Relocation<func_t> func{ RELOCATION_ID(37033, 38061) };

		return func(actor, location, angle);
	}

	void OnDefault(RE::Actor* actor)
	{
		RE::NiPoint3 rot = { 0, 0, actor->data.angle.z };
		SetTween(actor, actor->data.location, rot);
	}

	void OnFlightKill(RE::Actor* actor)
	{
		auto handle = GetNearCrosshairActor(actor, 10000, 20);
		Attack::Manager::GetSingleton()->_targetHandle = handle;

		auto targetPtr = handle.get();
		auto target = targetPtr ? targetPtr.get() : nullptr;
		if (target) {
			RE::NiPoint3 rotationVec = { 0, 0, actor->data.angle.z };
			RE::NiPoint3 targetPos = { 0, 0, 0 };

			float dx = target->data.location.x - actor->data.location.x;
			float dy = target->data.location.y - actor->data.location.y;
			float dist2D = sqrtf(dx * dx + dy * dy);

			if (dist2D > 0.000001f) {
				float normX = dx / dist2D;
				float normY = dy / dist2D;
				if (dist2D > 0.1) {
					rotationVec.z = atan2f(normX, normY);
				}
			}

			targetPos = target->data.location;

			SetTween(actor, targetPos, rotationVec);

		} else {
			OnDefault(actor);
		}
	}

	void OnPerch(RE::Actor* actor)
	{
		float autoAimMaxDistance = 10000;
		float autoAimScreenPercentage = 50;

		auto mgr = Form::Manager::GetSingleton();

		CrosshairDistanceChecker checker(autoAimScreenPercentage);

		RE::TES::GetSingleton()->ForEachReferenceInRange(actor, autoAimMaxDistance, [&](RE::TESObjectREFR* ref) {
			auto baseObj = ref->GetBaseObject();

			if (!baseObj)
				return RE::BSContainer::ForEachResult::kContinue;

			if (baseObj->GetFormID() != mgr->DragonPerchRockL02.GetID() && baseObj->GetFormID() != mgr->DragonPerchTower.GetID() && baseObj->GetFormID() != mgr->DragonPerchWordwall.GetID())
				return RE::BSContainer::ForEachResult::kContinue;

			checker.AddCompare(ref);

			return RE::BSContainer::ForEachResult::kContinue;
		});

		auto nearest = checker.GetResult<RE::TESObjectREFR>();

		if (nearest) {
			auto         pos = nearest->GetPosition();
			auto         rot = nearest->GetAngle();
			RE::TESForm* form = nearest->GetBaseObject();
			SetTween(actor, pos, rot);
			if (form) {
				if (form->GetFormID() == mgr->DragonPerchRockL02.GetID()) {
					actor->NotifyAnimationGraph("FlyStopPerchRockL02");
				} else if (form->GetFormID() == mgr->DragonPerchTower.GetID()) {
					actor->NotifyAnimationGraph("FlyStopPerchRockL02");
				} else if (form->GetFormID() == mgr->DragonPerchWordwall.GetID()) {
					actor->NotifyAnimationGraph("FlyStopPerchRockL02");
				}
			}
		} else {
			OnDefault(actor);
		}
	}

	RE::NiAVObject* IsPositionInLineOfSight(RE::Actor* actor, RE::NiPoint3* targetPosition, RE::NiPoint3* rayHitPosition, float viewCone = 6.28)
	{
		using func_t = decltype(&IsPositionInLineOfSight);
		REL::Relocation<func_t> func{ RELOCATION_ID(36754, 37770) };
		return func(actor, targetPosition, rayHitPosition, viewCone);
	}

	void OnLand(RE::Actor* actor)
	{
		auto actorPosition = actor->GetPosition();
		auto currentAngle = actor->GetAngle();

		RE::NiMatrix3 mat;
		mat.EulerAnglesToAxesZXY(0, 0, currentAngle.z);

		RE::NiPoint3 vec = mat * RE::NiPoint3(0, 0.707106f, -0.707106f);

		auto targetPos = actorPosition + vec * 100 * 69.991249f;
		auto hitPos = RE::NiPoint3();

		auto obj = IsPositionInLineOfSight(actor, &targetPos, &hitPos);
		auto rot = RE::NiPoint3(0, 0, actor->data.angle.z);

		if (obj && hitPos.z < actor->data.location.z) {
			SetTween(actor, hitPos, rot);
		} else {
			SetTween(actor, targetPos, rot);
		}
	}

	void KnockAreaEffect(RE::Actor* reference, float magnitude, float radius)
	{
		using func_t = void (*)(void*, RE::Actor*, float, float);
		static REL::Relocation<func_t> func{ RELOCATION_ID(35321, 36216) };
		return func(nullptr, reference, magnitude, radius);
	}

	void OnDragonLandEffect(RE::Actor* actor)
	{
		auto objectLength = actor->GetBoundMax().y - actor->GetBoundMin().y;  //ObjectReference.GetLength()
		KnockAreaEffect(actor, 1, objectLength);

		auto impactDataSet = Form::Manager::GetSingleton()->FXDragonTakeoffImpactSet.GetPtr();
		auto point = RE::NiPoint3(0, 0, -1);
		if (impactDataSet)
			RE::BGSImpactManager::GetSingleton()->PlayImpactEffect(actor, impactDataSet, "NPC Pelvis", point, 512, false, false);
	}

	void OnDragonForcefulLandEffect(RE::Actor* actor)
	{
		auto objectLength = actor->GetBoundMax().y - actor->GetBoundMin().y;
		KnockAreaEffect(actor, 1, 2 * objectLength);

		auto impactDataSet = Form::Manager::GetSingleton()->FXDragonLandingImpactSet.GetPtr();
		auto point = RE::NiPoint3(0, 0, -1);
		if (impactDataSet)
			RE::BGSImpactManager::GetSingleton()->PlayImpactEffect(actor, impactDataSet, "NPC Pelvis", point, 512, false, false);
	}

	void OnDragonTakeoffEffect(RE::Actor* actor)
	{
		auto impactDataSet = Form::Manager::GetSingleton()->FXDragonTakeoffImpactSet.GetPtr();
		auto point = RE::NiPoint3(0, 0, -1);
		if (impactDataSet)
			RE::BGSImpactManager::GetSingleton()->PlayImpactEffect(actor, impactDataSet, "NPC Tail8", point, 2048, false, false);
	}

	void OnDragonTailAttackEffect(RE::Actor* actor)
	{
		auto impactDataSet = Form::Manager::GetSingleton()->FXDragonTailstompImpactSet.GetPtr();
		auto point = RE::NiPoint3(0, 0, -1);
		if (impactDataSet)
			RE::BGSImpactManager::GetSingleton()->PlayImpactEffect(actor, impactDataSet, "NPC Tail8", point, 512, false, false);
	}

	void OnDragonLeftWingAttackEffect(RE::Actor* actor)
	{
		auto impactDataSet = Form::Manager::GetSingleton()->FXDragonTailstompImpactSet.GetPtr();
		auto point = RE::NiPoint3(0, 0, -1);
		if (impactDataSet)
			RE::BGSImpactManager::GetSingleton()->PlayImpactEffect(actor, impactDataSet, "NPC LHand", point, 512, false, false);
	}

	void OnDragonRightWingAttackEffect(RE::Actor* actor)
	{
		auto impactDataSet = Form::Manager::GetSingleton()->FXDragonTailstompImpactSet.GetPtr();
		auto point = RE::NiPoint3(0, 0, -1);
		if (impactDataSet)
			RE::BGSImpactManager::GetSingleton()->PlayImpactEffect(actor, impactDataSet, "NPC RHand", point, 512, false, false);
	}

	void OnDragonPassByEffect(RE::Actor*)
	{
		auto sound = Form::Manager::GetSingleton()->NPCDragonFlyby.GetPtr();
		if (sound)
			RE::BSAudioManager::GetSingleton()->Play(sound->descriptor);
	}

	void OnDragonKnockbackEvent(RE::Actor* actor)
	{
		auto objectLength = actor->GetBoundMax().y - actor->GetBoundMin().y;
		KnockAreaEffect(actor, 1, 1.5f * objectLength);
	}

	void NoOp(RE::Actor*)
	{
		// do nothing
	}

	class AnimationEventHandler
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(32157, 32901), OFFSET(0x28, 0x28) };

			struct Code : Xbyak::CodeGenerator
			{
				Code(std::uintptr_t targetAddr)
				{
					Xbyak::Label retnLabel;

					mov(r8, rdi);
					mov(rax, (uintptr_t)&Func);
					call(rax);

					movzx(ebx, al);

					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(targetAddr + 6);
				}
			};

			Code code(target.address());
			code.ready();

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_jmp<6>(target.address(), trampoline.allocate(code));

			static constexpr std::array EVENTS{
				std::make_pair("DragonLandEffect", OnDragonLandEffect),
				std::make_pair("DragonForcefulLandEffect", OnDragonForcefulLandEffect),
				std::make_pair("DragonTakeoffEffect", OnDragonTakeoffEffect),
				std::make_pair("DragonTailAttackEffect", OnDragonTailAttackEffect),
				std::make_pair("DragonLeftWingAttackEffect", OnDragonLeftWingAttackEffect),
				std::make_pair("DragonRightWingAttackEffect", OnDragonRightWingAttackEffect),
				std::make_pair("DragonPassByEffect", OnDragonPassByEffect),
				std::make_pair("DragonKnockbackEvent", OnDragonKnockbackEvent),

				std::make_pair("FlightTakeOff", OnDefault),
				std::make_pair("FlightHovering", NoOp),
				std::make_pair("FlightCruising", NoOp),
				std::make_pair("FlightLanding", NoOp),
				std::make_pair("FlightPerching", NoOp),
				std::make_pair("FlightAction", NoOp),

				std::make_pair("TakeOff", OnDefault),
				std::make_pair("TakeOff_Vertical", OnDefault),
				std::make_pair("HoverStart", OnDefault),
				std::make_pair("HoverStartSafe", OnDefault),
				std::make_pair("HoverStop", OnDefault),
				std::make_pair("HoverStopVertical", OnDefault),
				std::make_pair("FlyStopDefault", OnLand),
				std::make_pair("FlyStopDefaultSafe", OnLand),
				std::make_pair("FlyStopInjured", OnLand),
				std::make_pair("FlyStopInjuredSafe", OnLand),
				std::make_pair("FlyStopCrash", OnDefault),
				std::make_pair("FlyStopHasty", OnLand),
				std::make_pair("FlyStopPerch", OnPerch),
				std::make_pair("FlyStopHastyVertical", OnLand),
				std::make_pair("FlyStopPerchVertical", OnDefault),

				std::make_pair("FlyStopPerchTowerAttack", NoOp),
				std::make_pair("FlyStopPerchTower", NoOp),
				std::make_pair("FlyStopPerchAmp", NoOp),
				std::make_pair("FlyStopPerchRockL02", NoOp),

				std::make_pair("FlyStopPerchTowerVertical", OnDefault),
				std::make_pair("FlyStopPerchAmpVertical", OnDefault),
				std::make_pair("FlyStopPerchRockL02Vertical", OnDefault),
				std::make_pair("to_Flight_Kill_Grab_Enter", OnFlightKill)
			};

			for (const auto& event : EVENTS) {
				_animationEventCallbacks.emplace(event.first, event.second);
			}
		}

	private:
		inline static bool Func(RE::IAnimationGraphManagerHolder* holder, RE::BSTSmartPointer<RE::BSAnimationGraphManager>& out, RE::BSFixedString& eventName)
		{
			OnAnimationEvent(skyrim_cast<RE::Actor*>(holder), eventName);

			return holder->GetAnimationGraphManager(out);
		}

		class AnimationEventSink : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
		{
			virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*) override
			{
				OnAnimationEvent(skyrim_cast<RE::Actor*>(const_cast<RE::TESObjectREFR*>(event->holder)), event->tag);
				return RE::BSEventNotifyControl::kContinue;
			}
		};

		inline static void OnAnimationEvent(RE::Actor* actor, const RE::BSFixedString& eventName)
		{
			if (!Utils::IsPlayerControlledDragon(actor))
				return;

			actor->AddAnimationGraphEventSink(&_sink);

			auto it = _animationEventCallbacks.find(eventName.c_str());
			if (it != _animationEventCallbacks.end()) {
				it->second(actor);
			}
		}

		static inline AnimationEventSink                                    _sink;
		static inline std::unordered_map<std::string, void (*)(RE::Actor*)> _animationEventCallbacks;
	};

	class OnPlayerUpdateHook
	{
	public:
		inline static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };  //PlayerCharacter

			_func = vtbl.write_vfunc(0xAD, Func);
		}

	private:
		inline static void Func(RE::PlayerCharacter* player, float delta)
		{
			_func(player, delta);

			if (Utils::IsPlayerControlledDragon(player)) {
				EnableControls();
			}

			if (player && player->IsOnMount()) {
				auto extra = player->extraList.GetByType<RE::ExtraInteraction>();
				if (extra) {
					auto interaction = extra->interaction.get();
					auto actorPtr = interaction ? interaction->actor.get() : nullptr;
					auto actor = actorPtr ? actorPtr.get() : nullptr;
					if (actor && Utils::IsPlayerControlledDragon(actor)) {
						EnableControls();
						UpdateMountRotationFromCamera(actor);
					}
				}
			}
		}

		inline static void EnableControls()
		{
			auto controlMap = RE::ControlMap::GetSingleton();

			if (!controlMap->AreControlsEnabled(RE::ControlMap::UEFlag::kMovement))
				controlMap->ToggleControls(RE::ControlMap::UEFlag::kMovement, true, false);

			if (!controlMap->AreControlsEnabled(RE::ControlMap::UEFlag::kJumping))
				controlMap->ToggleControls(RE::ControlMap::UEFlag::kJumping, true, false);

			if (!controlMap->AreControlsEnabled(RE::ControlMap::UEFlag::kFighting))
				controlMap->ToggleControls(RE::ControlMap::UEFlag::kFighting, true, false);
		}

		inline static void UpdateMountRotationFromCamera(RE::Actor* mount)
		{
			auto dragonCameraState = static_cast<RE::DragonCameraState*>(RE::PlayerCamera::GetSingleton()->cameraStates[RE::CameraState::kDragon].get());
			if (RE::PlayerCamera::GetSingleton()->currentState.get() != dragonCameraState || !Settings::GetSingleton()->Get<bool>("RidingControl"))
				return;

			bool isSynced;
			if (mount->GetGraphVariableBool("bIsSynced", isSynced) && isSynced)
				return;

			if (auto currentProcess = mount->currentProcess) {
				if (currentProcess->middleHigh && currentProcess->middleHigh->currentMovementSpeed != 0.0f) {
					float cameraYaw = mount->GetHeading(false) + dragonCameraState->freeRotation.x;

					float cameraPitch = -dragonCameraState->freeRotation.y;

					mount->SetHeading(cameraYaw);
					mount->data.angle.x = cameraPitch;
				}
			}
		}

		static inline REL::Relocation<decltype(Func)> _func;
	};

	void Install()
	{
		OnPlayerUpdateHook::Install();
		AnimationEventHandler::Install();
	}
}
