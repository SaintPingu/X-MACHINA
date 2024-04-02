#pragma once

#define OUT

/*---------------
	  Lock
---------------*/

#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define	READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0)
#define	WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0)

/*---------------
	  Crash
---------------*/

#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

#define ASSERT_CRASH(expr)			\
{									\
	if (!(expr))					\
	{								\
		CRASH("ASSERT_CRASH");		\
		__analysis_assume(expr);	\
	}								\
}


/// +----------------------------
///			 SINGLETON
/// ----------------------------+
#pragma region SINGLETON
#define INSTANCE(TYPE) TYPE::GetInst()

#define DECLARE_SINGLETON(TYPE)             \
private:									\
	 static TYPE* m_pInst;					\
public:										\
	static TYPE* GetInst()					\
	{										\
		if (!m_pInst) m_pInst = new TYPE;	\
		return m_pInst;						\
	}										\
	static void Destroy() {					\
		if (nullptr != m_pInst) {			\
			delete m_pInst;					\
			m_pInst = nullptr;				\
		}									\
	}						

#define DEFINE_SINGLETON(TYPE)  \
	 TYPE* TYPE::m_pInst = nullptr;
#pragma endregion

#pragma region Delete
#define SAFE_DELETE(ptr) \
if(ptr != nullptr){		 \
	delete ptr;			 \
	ptr = nullptr;		 \
}	

#define SAFE_DELETE_VECTOR(Vector)			\
for (int i = 0; i < Vector.size(); ++i) {	\
	if(Vector[i] != nullptr) {				\
		delete Vector[i];					\
	}										\
}

#define SAFE_DELETE_UNORDERED_MAP(UnMap)				\
for(auto it = UnMap.begin(); it != UnMap.end(); ++it) {	\
	if (it->second != nullptr)							\
		delete it->second;								\
}														\
UnMap.clear();

#pragma endregion


