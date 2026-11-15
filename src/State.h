#pragma once

class State : public REX::Singleton<State>
{
public:
	bool isRidingDriven = false;
};
