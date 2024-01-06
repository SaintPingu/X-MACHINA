#pragma once

// usage :
// class SampleClass : public Singleton<SampleClass> {
//		friend Singleton
// };
template<class T>
class Singleton {
private:
	struct Deleter
	{
		void operator()(T* instance)
		{
			delete instance;
		}
	};

	static std::unique_ptr<T, Deleter> mSingletonInstance;
	static std::once_flag mSingletonCallFlag;

protected:
	Singleton() { std::atexit(CheckDestroy); }
	~Singleton() = default;
		
public:
	Singleton(const Singleton&)				= delete;
	Singleton(Singleton&&)					= delete;
	Singleton& operator=(const Singleton&)	= delete;
	Singleton& operator=(Singleton&&)		= delete;

public:
	static T* Inst()
	{
		std::call_once(mSingletonCallFlag, []() {	// thread-safe
			mSingletonInstance.reset(new T);
			});
		return mSingletonInstance.get();
	}

	static void Destroy()
	{
		mSingletonInstance = nullptr;
	}

private:
	static void CheckDestroy()
	{
		assert(mSingletonInstance == nullptr);
	}
};
template<class T>
std::unique_ptr<T, typename Singleton<T>::Deleter> Singleton<T>::mSingletonInstance;
template<class T>
std::once_flag Singleton<T>::mSingletonCallFlag;