#include "Input.h"
#include "Settings.h"
#include "Utils.h"

namespace Input
{
	void Install()
	{
		if (const auto manager = RE::BSInputDeviceManager::GetSingleton()) {
			manager->AddEventSink<RE::InputEvent*>(Manager::GetSingleton());
		}
	}

	EventResult Manager::ProcessEvent(RE::InputEvent* const* event, RE::BSTEventSource<RE::InputEvent*>*)
	{
		using InputType = RE::INPUT_EVENT_TYPE;

		if (!event) {
			return EventResult::kContinue;
		}

		const auto ui = RE::UI::GetSingleton();
		if (ui->GameIsPaused()) {
			return EventResult::kContinue;
		}

		const auto ctrlMap = RE::ControlMap::GetSingleton();
		if (!ctrlMap || ctrlMap->contextPriorityStack.empty() || ctrlMap->contextPriorityStack.back() != RE::UserEvents::INPUT_CONTEXT_ID::kGameplay)
			return EventResult::kContinue;

		auto                    settings = Settings::GetSingleton();
		const RE::BSFixedString landKey = settings->Get<std::string>("LandKey");
		const RE::BSFixedString perchKey = settings->Get<std::string>("PerchKey");
		const RE::BSFixedString takeOffKey = settings->Get<std::string>("TakeOffKey");
		const RE::BSFixedString layDownKey = settings->Get<std::string>("LayDownKey");

		for (auto it = *event; it; it = it->next) {
			if (const auto button = it->AsButtonEvent(); button) {
				auto userEvent = button->QUserEvent();

				if (button->IsDown()) {
					if (userEvent == perchKey || userEvent == takeOffKey || userEvent == landKey) {
						auto controls = RE::PlayerControls::GetSingleton();
						if (controls) {
							RE::BSSpinLockGuard locker(controls->actorArrayLock);

							for (auto itr : controls->actionInterestedActor) {
								auto actor = itr.get();
								if (!actor)
									continue;

								auto flyState = actor->GetFlyState();

								if (userEvent == perchKey) {
									if (flyState == RE::FLY_STATE::kCruising || flyState == RE::FLY_STATE::kHovering) {
										actor->NotifyAnimationGraph("FlyStopPerch");
									}
								}

								if (userEvent == takeOffKey) {
									if (flyState == RE::FLY_STATE::kPerching) {
										actor->NotifyAnimationGraph("TakeOff");
									}
									if (flyState == RE::FLY_STATE::kNone) {
										float speed = 0.0f;
										actor->GetGraphVariableFloat("Speed", speed);
										if (speed < 1.0f) {
											actor->NotifyAnimationGraph("TakeOff_Vertical");
										} else {
											actor->NotifyAnimationGraph("TakeOff");
										}
									}
								}

								if (userEvent == landKey) {
									if (flyState == RE::FLY_STATE::kCruising || flyState == RE::FLY_STATE::kHovering) {
										bool injured = false;
										actor->GetGraphVariableBool("Injured", injured);
										if (injured) {
											actor->NotifyAnimationGraph("FlyStopInjured");
										} else {
											actor->NotifyAnimationGraph("FlyStopDefault");
										}
									}
								}
								if (userEvent == layDownKey) {
									if (flyState == RE::FLY_STATE::kNone) {
										bool animationDriven = true;
										actor->GetGraphVariableBool("bAnimationDriven", animationDriven);
										if (!animationDriven) {
											actor->NotifyAnimationGraph("IdleFurnitureNestEnter");
										} else {
											actor->NotifyAnimationGraph("to_Furniture_Exit");
										}
									}
								}
							}
						}
					}
				}
			}
		}

		return EventResult::kContinue;
	}

}
