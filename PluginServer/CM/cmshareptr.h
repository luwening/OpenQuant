/*********************************************************************
创建日期：2007年8月10日
作    者：kvls
文件描述：智能指针
使用说明：
历史版本：1.00 － 创建
*********************************************************************/

#ifndef _CMSHAREPTR_H_9AF6ECFB_0B2A_4388_9B15_1CC13AE668F3
#define _CMSHAREPTR_H_9AF6ECFB_0B2A_4388_9B15_1CC13AE668F3

#include "CMHelper.h"

_CMHELPER_BEGIN
template<int x>
struct Int2Type
{
	enum {type = x};
};

template<typename T, bool bArray = false, bool bMultiThread = false>
class CCMSharePtr
{
	class XCount
	{
	public:
		XCount(long nCount = 0)
			: m_nCount(nCount)
		{
		}
		XCount(const XCount &v)
			: m_nCount(v.m_nCount)
		{
		}
		long AddRef()
		{
			return Inc(Int2Type<bMultiThread>());
		}
		long Release()
		{
			return Dec(Int2Type<bMultiThread>());
		}

	private:
		inline long Inc(Int2Type<true>)
		{
			return InterlockedIncrement(&m_nCount);
		}
		inline long Inc(Int2Type<false>)
		{
			return ++m_nCount;
		}
		inline long Dec(Int2Type<true>)
		{
			return InterlockedDecrement(&m_nCount);
		}
		inline long Dec(Int2Type<false>)
		{
			return --m_nCount;
		}

	private:
		long m_nCount;
	};

	class XData
	{
		friend CCMSharePtr;

	public:
		XData()
			: m_p(NULL), m_pCount(NULL)
		{
		}
		XData(T *p)
			: m_p(p), m_pCount(NULL)
		{
			if(m_p)
				m_pCount = new XCount(1);
		}
		XData(const XData &v)
			: m_p(v.m_p), m_pCount(v.m_pCount)
		{
			if(m_p)
			{
				assert(m_pCount);
				m_pCount->AddRef();
			}
		}
		~XData()
		{
			Release();
		}
		void operator=(T *p)
		{
			if(m_p != p)
			{
				Release();
				m_p = p;
				if(m_p)
					m_pCount = new XCount(1);
			}
		}
		void operator=(const XData &data)
		{
			if(this != &data && m_p != data.m_p)
			{
				Release();
				m_p = data.m_p;
				if(m_p)
				{
					m_pCount = data.m_pCount;
					m_pCount->AddRef();
				}
			}
		}
		long Release(bool bFreePoint = true)
		{
			if(m_pCount)
			{
				long nCount = m_pCount->Release();
				if(nCount == 0)
				{
					if(bFreePoint)
						Free(Int2Type<bArray>());
					delete m_pCount;
				}

				m_p = NULL;
				m_pCount = NULL;

				return nCount;
			}

			return -1;
		}

	private:
		inline void Free(Int2Type<true>)
		{
			if(m_p)
			{
				delete [] m_p;
				m_p = NULL;
			}
		}
		inline void Free(Int2Type<false>)
		{
			if(m_p)
			{
				delete m_p;
				m_p = NULL;
			}
		}

	private:
		T *m_p;
		XCount *m_pCount;
	};

public:
	explicit CCMSharePtr()
	{
	}
	explicit CCMSharePtr(T *p)
		: m_Data(p)
	{
	}
	CCMSharePtr(const CCMSharePtr &sp)
		: m_Data(sp.m_Data)
	{
	}
	void operator=(const CCMSharePtr &v)
	{
		if(this != &v)
			m_Data = v.m_Data;
	}
	void operator=(T *p)
	{
		m_Data = p;
	}
	T& operator[](int nIndex) const
	{
		return GetElement(nIndex, Int2Type<bArray>());
	}
	T& operator*() const
	{
		return *m_Data.m_p;
	}
	T* operator->() const
	{
		return m_Data.m_p;
	}
	bool IsValid() const
	{
		return (m_Data.m_p != NULL);
	}
	const T* Get() const
	{
		return m_Data.m_p;
	}
	T* Release()
	{
		T *p = m_Data.m_p;
		m_Data.Release(false);
		return p;
	}

private:
	inline T& GetElement(int nIndex, Int2Type<true>) const
	{
		return m_Data.m_p[nIndex];
	}

private:
	XData m_Data;
};
_CMHELPER_END

#endif	// _CMSHAREPTR_H_9AF6ECFB_0B2A_4388_9B15_1CC13AE668F3