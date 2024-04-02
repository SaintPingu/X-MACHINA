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
		mCallback = [owner, memFunc, args...]() // onwer 는 shared_ptr 로 캡쳐하면 Ref 가 증가한다. 
		{
			(owner.get()->*memFunc)(args...);
		};
	}

	void Execute()
	{
		mCallback();
	}


};

