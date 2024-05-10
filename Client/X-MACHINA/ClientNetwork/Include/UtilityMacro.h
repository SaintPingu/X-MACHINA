
#pragma once

/// +----------------------------
///			 SINGLETON
/// ----------------------------+
#pragma region SINGLETON
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
#pragma endregion
