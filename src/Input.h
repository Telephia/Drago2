#pragma once

namespace Input
{
	using EventResult = RE::BSEventNotifyControl;

	class Manager : public REX::Singleton<Manager>, public RE::BSTEventSink<RE::InputEvent*>
	{
	private:
		EventResult ProcessEvent(RE::InputEvent* const* event, RE::BSTEventSource<RE::InputEvent*>*) override;
	};
}
