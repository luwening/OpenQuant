#ifndef _CA_LOCK_H
#define _CA_LOCK_H

#include "CommonAssist.h"

_CA_BEGIN
template <typename T>
class CAutoLock
{
public:
	explicit CAutoLock(T* pLock)
	{
		if (pLock)
			pLock->Lock();
		m_pLock = pLock;
	}
	~CAutoLock()
	{
		if (m_pLock)
			m_pLock->Unlock();
	}
	
private:
	T*	m_pLock;
};

class CCriticalSection
{
public:
	CCriticalSection()
	{
		InitializeCriticalSection(&m_cs);
	}
	~CCriticalSection()
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

class CEvent
{
public:
	CEvent(BOOL bManualReset = FALSE, BOOL bInitState = FALSE, LPCTSTR lpstrName = NULL)
	{
		m_hEvent = ::CreateEvent(NULL, bManualReset, bInitState, lpstrName);
	}
	~CEvent()
	{
		if (m_hEvent)
			::CloseHandle(m_hEvent);
	}
	HANDLE Get() const
	{
		return m_hEvent;
	}
	void Set()
	{
		if (m_hEvent)
			::SetEvent(m_hEvent);
	}
	void Reset()
	{
		if (m_hEvent)
			::ResetEvent(m_hEvent);
	}
	unsigned long Wait(unsigned int nMilliseconds = INFINITE)
	{
		if (!m_hEvent)
			return WAIT_OBJECT_0;
		return WaitForSingleObject(m_hEvent, nMilliseconds);
	}
	unsigned long Wait(HANDLE h, unsigned int nMilliseconds = INFINITE)
	{
		if (!m_hEvent)
			return WAIT_OBJECT_0;
		if (!h)
			return WAIT_OBJECT_0+1;

		HANDLE ha[2] = {m_hEvent, h};
		return WaitForMultipleObjects(2, ha, FALSE, nMilliseconds);
	}
	unsigned long Wait(HANDLE h1, HANDLE h2, unsigned int nMilliseconds = INFINITE)
	{
		if (!m_hEvent)
			return WAIT_OBJECT_0;
		if (!h1)
			return WAIT_OBJECT_0+1;
		if (!h2)
			return WAIT_OBJECT_0+2;

		HANDLE ha[3] = {m_hEvent, h1, h2};
		return WaitForMultipleObjects(3, ha, FALSE, nMilliseconds);
	}
	unsigned long Wait(HANDLE h1, HANDLE h2, HANDLE h3, unsigned int nMilliseconds = INFINITE)
	{
		if (!m_hEvent)
			return WAIT_OBJECT_0;
		if (!h1)
			return WAIT_OBJECT_0+1;
		if (!h2)
			return WAIT_OBJECT_0+2;
		if (!h3)
			return WAIT_OBJECT_0+3;

		HANDLE ha[4] = {m_hEvent, h1, h2, h3};
		return WaitForMultipleObjects(4, ha, FALSE, nMilliseconds);
	}
	unsigned long Wait(HANDLE h1, HANDLE h2, HANDLE h3, HANDLE h4, unsigned int nMilliseconds = INFINITE)
	{
		if (!m_hEvent)
			return WAIT_OBJECT_0;
		if (!h1)
			return WAIT_OBJECT_0+1;
		if (!h2)
			return WAIT_OBJECT_0+2;
		if (!h3)
			return WAIT_OBJECT_0+3;
		if (!h4)
			return WAIT_OBJECT_0+4;
		
		HANDLE ha[5] = {m_hEvent, h1, h2, h3, h4};
		return WaitForMultipleObjects(5, ha, FALSE, nMilliseconds);
	}
	unsigned long Wait(HANDLE h1, HANDLE h2, HANDLE h3, HANDLE h4, HANDLE h5, unsigned int nMilliseconds = INFINITE)
	{
		if (!m_hEvent)
			return WAIT_OBJECT_0;
		if (!h1)
			return WAIT_OBJECT_0+1;
		if (!h2)
			return WAIT_OBJECT_0+2;
		if (!h3)
			return WAIT_OBJECT_0+3;
		if (!h4)
			return WAIT_OBJECT_0+4;
		if (!h5)
			return WAIT_OBJECT_0+5;
		
		HANDLE ha[6] = {m_hEvent, h1, h2, h3, h4, h5};
		return WaitForMultipleObjects(6, ha, FALSE, nMilliseconds);
	}

private:
	HANDLE m_hEvent;
};

_CA_END

#endif