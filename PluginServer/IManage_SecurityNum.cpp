#include "stdafx.h"
#include "IManage_SecurityNum.h"
#include "Manage_SecurityNum.h"

Manage_SecurityNum* IManage_SecurityNum::ms_pImpl = NULL;

IManage_SecurityNum::IManage_SecurityNum(void)
{
}

IManage_SecurityNum::~IManage_SecurityNum(void)
{
}

void IManage_SecurityNum::Init()
{
	if (ms_pImpl == NULL)
	{
		ms_pImpl = new Manage_SecurityNum();
	}
}

void IManage_SecurityNum::UnInit()
{
	if ((ms_pImpl) != NULL)
	{ 
		delete (ms_pImpl);
		(ms_pImpl) = NULL;
	}
}

void IManage_SecurityNum::Clear()
{
	Init();
	ms_pImpl->Clear();
}

void IManage_SecurityNum::InsertCookieSocket(const UINT32 &nCookie, const SOCKET &sock)
{
	Init();
	ms_pImpl->InsertCookieSocket(nCookie, sock);
}

void IManage_SecurityNum::DeleteCookieSocket(const UINT32 &nCookie)
{
	Init();
	ms_pImpl->DeleteCookieSocket(nCookie);
}

bool IManage_SecurityNum::IsSafeSocket(const SOCKET &sock)
{
	Init();
	return ms_pImpl->IsSafeSocket(sock);
}

void IManage_SecurityNum::AddSafeSocket(const UINT32 &nCookie)
{
	Init();
	ms_pImpl->AddSafeSocket(nCookie);
}
