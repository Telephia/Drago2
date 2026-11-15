#pragma once
#include <string>
#include <unordered_map>
#include <variant>

using SettingValue = std::variant<bool, float, int, std::string>;

class Settings : public REX::Singleton<Settings>
{
public:
	void Load();
	void Save();
	void ApplyAll();

	template <typename T>
	void Set(const std::string& name, T value)
	{
		if (_values.count(name)) {
			_values[name] = value;
			_dirty = true;
		}
	}

	template <typename T>
	T Get(const std::string& name, T defaultValue = T{})
	{
		if (_values.count(name)) {
			return std::get<T>(_values[name]);
		}
		return defaultValue;
	}

private:
	std::unordered_map<std::string, SettingValue> _values;
	bool                                          _dirty = false;
	const std::wstring                            _iniPath = L"Data/SKSE/Plugins/FlyingMOD.ini";
};
