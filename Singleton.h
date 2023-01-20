#pragma once

template <typename T>
class ISingleton
{
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}

	ISingleton(const ISingleton&) = delete;
	ISingleton& operator=(const ISingleton&) = delete;
protected:
	uintptr_t m_ModuleBase = (uintptr_t)(GetModuleHandle(NULL));
	ISingleton() {};
	~ISingleton() {};
};