/*********************************************************************
创建日期：2007年8月10日
作    者：kvls
文件描述：同步对象
使用说明：
历史版本：1.00 － 创建
*********************************************************************/
#ifndef _CMLOCK_H_A4D5A30D_C533_49d0_A1F6_8650D7979BB8
#define _CMLOCK_H_A4D5A30D_C533_49d0_A1F6_8650D7979BB8

#include "CMHelper.h"

_CMHELPER_BEGIN
template <typename T>
class CAutoLock
{
public:
	explicit CAutoLock(T *pLock)
	{
		if(pLock)
			pLock->Lock();
		m_pLock = pLock;
	}
	~CAutoLock()
	{
		if(m_pLock)
			m_pLock->Unlock();
	}

private:
	T *m_pLock;
};

class CCritSec
{
public:
	CCritSec()
	{
		InitializeCriticalSection(&m_cs);
	}
	~CCritSec()
	{
		DeleteCriticalSection(&m_cs);
	}
	void Lock()
	{
		EnterCriticalSection(&m_cs);
	}
	void Unlock()
	{
		LeaveCriticalSection(&m_cs);
	}

private:
	CRITICAL_SECTION m_cs;
};

class CMutex
{
public:
	CMutex(LPCTSTR lpstrName = NULL)
	{
		m_hMutex = ::CreateMutex(NULL, FALSE, lpstrName);
	}
	~CMutex()
	{
		if(m_hMutex)
			::CloseHandle(m_hMutex);
	}
	void Lock()
	{
		if(m_hMutex)
			WaitForSingleObject(m_hMutex, INFINITE);
	}
	void Unlock()
	{
		if(m_hMutex)
			ReleaseMutex(m_hMutex);
	}

private:
	HANDLE m_hMutex;
};

class CEvent
{
public:
	CEvent(BOOL bManualReset = FALSE, BOOL bInitState = FALSE, LPCTSTR lpstrName = NULL)
	{
		m_hEvent = ::CreateEvent(NULL, bManualReset, bInitState, lpstrName);
	}
	~CEvent()
	{
		if(m_hEvent)
			::CloseHandle(m_hEvent);
	}
	HANDLE Get() const
	{
		return m_hEvent;
	}
	void Set()
	{
		if(m_hEvent)
			::SetEvent(m_hEvent);
	}
	void Reset()
	{
		if(m_hEvent)
			::ResetEvent(m_hEvent);
	}
	unsigned long Wait(unsigned int nMilliseconds = INFINITE)
	{
		if(!m_hEvent)
			return WAIT_OBJECT_0;
		return WaitForSingleObject(m_hEvent, nMilliseconds);
	}
	unsigned long Wait(HANDLE h, unsigned int nMilliseconds = INFINITE)
	{
		if(!m_hEvent)
			return WAIT_OBJECT_0;
		if(!h)
			return WAIT_OBJECT_0+1;

		HANDLE ha[2] = {m_hEvent, h};
		return WaitForMultipleObjects(2, ha, FALSE, nMilliseconds);
	}
	unsigned long Wait(HANDLE h1, HANDLE h2, unsigned int nMilliseconds = INFINITE)
	{
		if(!m_hEvent)
			return WAIT_OBJECT_0;
		if(!h1)
			return WAIT_OBJECT_0+1;
		if(!h2)
			return WAIT_OBJECT_0+2;

		HANDLE ha[3] = {m_hEvent, h1, h2};
		return WaitForMultipleObjects(3, ha, FALSE, nMilliseconds);
	}
	unsigned long Wait(HANDLE h1, HANDLE h2, HANDLE h3, unsigned int nMilliseconds = INFINITE)
	{
		if(!m_hEvent)
			return WAIT_OBJECT_0;
		if(!h1)
			return WAIT_OBJECT_0+1;
		if(!h2)
			return WAIT_OBJECT_0+2;
		if(!h3)
			return WAIT_OBJECT_0+3;

		HANDLE ha[4] = {m_hEvent, h1, h2, h3};
		return WaitForMultipleObjects(4, ha, FALSE, nMilliseconds);
	}
	unsigned long Wait(HANDLE h1, HANDLE h2, HANDLE h3, HANDLE h4, unsigned int nMilliseconds = INFINITE)
	{
		if(!m_hEvent)
			return WAIT_OBJECT_0;
		if(!h1)
			return WAIT_OBJECT_0+1;
		if(!h2)
			return WAIT_OBJECT_0+2;
		if(!h3)
			return WAIT_OBJECT_0+3;
		if(!h4)
			return WAIT_OBJECT_0+4;

		HANDLE ha[5] = {m_hEvent, h1, h2, h3, h4};
		return WaitForMultipleObjects(5, ha, FALSE, nMilliseconds);
	}
	unsigned long Wait(HANDLE h1, HANDLE h2, HANDLE h3, HANDLE h4, HANDLE h5, unsigned int nMilliseconds = INFINITE)
	{
		if(!m_hEvent)
			return WAIT_OBJECT_0;
		if(!h1)
			return WAIT_OBJECT_0+1;
		if(!h2)
			return WAIT_OBJECT_0+2;
		if(!h3)
			return WAIT_OBJECT_0+3;
		if(!h4)
			return WAIT_OBJECT_0+4;
		if(!h5)
			return WAIT_OBJECT_0+5;

		HANDLE ha[6] = {m_hEvent, h1, h2, h3, h4, h5};
		return WaitForMultipleObjects(6, ha, FALSE, nMilliseconds);
	}

private:
	HANDLE m_hEvent;
};

class CSemaphore
{
public:
	CSemaphore(long MaxCount = 0x7fffffff, long InitCount = 0, LPCTSTR lpstrName = NULL)
	{
		m_Semaphore = ::CreateSemaphore(NULL, InitCount, MaxCount, lpstrName);
	}
	~CSemaphore()
	{
		if(m_Semaphore)
			::CloseHandle(m_Semaphore);
	};
	bool Inc(long l = 1)
	{
		if(!m_Semaphore)
			return false;

		if(::ReleaseSemaphore(m_Semaphore, l, NULL))
			return true;

		return false;
	}
	unsigned long Wait(unsigned int nMilliseconds = INFINITE)
	{
		if(!m_Semaphore)
			return WAIT_OBJECT_0;

		return ::WaitForSingleObject(m_Semaphore, nMilliseconds);
	}
	unsigned long Wait(CMHelper::CEvent &e, unsigned int nMilliseconds = INFINITE)
	{
		if(!m_Semaphore)
			return WAIT_OBJECT_0;

		HANDLE h = e.Get();
		if(!h)
			return WAIT_OBJECT_0+1;

		HANDLE ha[2] = {m_Semaphore, h};
		return ::WaitForMultipleObjects(2, ha, FALSE, nMilliseconds);
	}

private:
	HANDLE m_Semaphore;
};

class CMutexRun
{
public:
	CMutexRun()
	{
		m_hMutex = NULL;
	}
	~CMutexRun()
	{
		Release();
	}
	bool SetRunName(LPCTSTR lpstrName)
	{
		if((!lpstrName) || m_hMutex)
			return false;

		m_hMutex = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpstrName);
		if(!m_hMutex)	// 打开失败，试图创建
		{
			m_hMutex = ::CreateMutex(NULL, TRUE, lpstrName);
		}
		else
		{
			DWORD dwWaitResult = ::WaitForSingleObject(m_hMutex, 0);
			if(dwWaitResult != WAIT_OBJECT_0 && dwWaitResult != WAIT_ABANDONED)
				Release();
		}

		return true;
	}
	bool HasRun()
	{
		return (m_hMutex == NULL);
	}

private:
	void Release()
	{
		if(m_hMutex)
		{
			::ReleaseMutex(m_hMutex);
			m_hMutex = NULL;
		}
	}

private:
	HANDLE m_hMutex;
};

typedef CMHelper::CAutoLock<CMHelper::CCritSec>	CAutoLock_CritSec;
typedef CMHelper::CAutoLock<CMHelper::CMutex> CAutoLock_Mutex;
_CMHELPER_END

#endif	// _CMLOCK_H_A4D5A30D_C533_49d0_A1F6_8650D7979BB8