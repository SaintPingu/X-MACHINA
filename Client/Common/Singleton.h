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
	static std::unique_ptr<T, Deleter> const I;

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
inline std::unique_ptr<T, typename Singleton<T>::Deleter> const Singleton<T>::I(new T);