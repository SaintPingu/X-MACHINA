#pragma once
#include <Windows.h>

#define DWORD_OVERLOADER( className )					\
public:													\
    className() = default;								\
	className(DWORD value) : DwordOverloader(value) {}


// usage :
//	class SampleClass : public DwordOverloader<SampleClass> {
//		DWORD_OVERLOADER(SampleClass)
// 
//		static const DWORD None   = 0x00;
//		static const DWORD First  = 0x01;
//		static const DWORD Second = 0x02;
//		static const DWORD Third  = 0x04;

template<class T>
class DwordOverloader {
private:
	DWORD mValue;

public:
	DwordOverloader() = default;
	DwordOverloader(DWORD value) : mValue(value) {}
	~DwordOverloader() = default;

	DwordOverloader(const DwordOverloader& other) : mValue(other.mValue) {}

public:
	inline constexpr operator DWORD () const	{ return mValue; }
	inline constexpr operator DWORD& ()         { return mValue; }

	T& operator=(T rhs)							{ mValue = rhs.mValue;	return static_cast<T>(*this); }
	T& operator=(DWORD value)					{ mValue = value;		return static_cast<T>(*this); }

	bool operator==(T other) const				{ return mValue == other.mValue; }
	bool operator==(DWORD value) const			{ return mValue == value; }

	friend DWORD operator|(T lhs, DWORD rhs)	{ return lhs.mValue | rhs; }
	friend DWORD operator|(T lhs, int rhs)		{ return lhs.mValue | rhs; }
	DWORD operator|=(DWORD rhs)					{ return mValue |= rhs; }

	friend DWORD operator&(T lhs, DWORD rhs)	{ return lhs.mValue & rhs; }
	friend DWORD operator&(T lhs, int rhs)		{ return lhs.mValue & rhs; }
	DWORD operator&=(DWORD rhs)					{ return mValue &= rhs; }

	friend DWORD operator^(T lhs, DWORD rhs)	{ return lhs.mValue ^ rhs; }
	friend DWORD operator^(T lhs, int rhs)		{ return lhs.mValue ^ rhs; }
	DWORD operator^=(DWORD rhs)					{ return mValue ^= rhs; }

	friend DWORD operator~(T obj)				{ return ~obj.mValue; }

	friend DWORD operator<<(T obj, int shift)	{ return obj.mValue << shift; }
	friend DWORD operator>>(T obj, int shift)	{ return obj.mValue >> shift; }
};