#pragma once

namespace Attack
{
	class Manager : public REX::Singleton<Manager>
	{
	public:
		RE::ActorHandle _targetHandle;
	};
}
