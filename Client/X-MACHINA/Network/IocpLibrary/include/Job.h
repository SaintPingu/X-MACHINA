#pragma once
#include <functional>

/*---------
	Job
----------*/

using CallbackType = std::function<void()>;

class Job
{
private:
	CallbackType mCallback;

public:
	Job(CallbackType&& callback) : mCallback(std::move(callback))
	{
	}

	template<typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args) // Owner - Member Func - arguments 
	{
		mCallback = [owner, memFunc, args...]() // onwer �� shared_ptr �� ĸ���ϸ� Ref �� �����Ѵ�. 
		{
			(owner.get()->*memFunc)(args...);
		};
	}

	void Execute()
	{
		mCallback();
	}


};

