#pragma once

// usage :
// class SampleClass : public Singleton<SampleClass> {
//		friend Singleton;
// };
template<class T>
class Singleton {
private:
	struct Deleter {
		void operator()(T* instance) const
		{
			delete instance;
		}
	};

public:
	static const std::unique_ptr<T, Deleter> I;

protected:
	Singleton() = default;
	~Singleton() = default;
		
public:
	Singleton(const Singleton&)				= delete;
	Singleton(Singleton&&)					= delete;
	Singleton& operator=(const Singleton&)	= delete;
	Singleton& operator=(Singleton&&)		= delete;
};
template<class T>
const std::unique_ptr<T, typename Singleton<T>::Deleter> Singleton<T>::I(new T);