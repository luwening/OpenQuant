#pragma once

#include "IManage_SecurityNum.h"
#include <map>

class Manage_SecurityNum
{
public:
	Manage_SecurityNum(void);
	virtual ~Manage_SecurityNum(void);

public:
	void Clear();

	void InsertCookieSocket(const UINT32 &nCookie, const SOCKET &sock);
	void DeleteCookieSocket(const UINT32 &nCookie);
	bool IsSafeSocket(const SOCKET &sock);
	void AddSafeSocket(const UINT32 &nCookie);

private:
	//OMCriticalSection m_safe;
	char strLocalSecurityNum[16];
	bool bLocalIsCheck;
	std::map<UINT32, SOCKET> m_mapCookieSocket;
	std::vector<SOCKET> m_vtSocket;
};
