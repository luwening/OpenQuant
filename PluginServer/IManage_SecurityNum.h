#pragma once

class Manage_SecurityNum;
class CPluginHKTradeServer;
class IManage_SecurityNum
{
public:
	IManage_SecurityNum(void);
	virtual ~IManage_SecurityNum(void);

public:
	static void Clear();

	static void InsertCookieSocket(const UINT32 &Cookie, const SOCKET &sock);
	static void DeleteCookieSocket(const UINT32 &Cookie);
	static bool IsSafeSocket(const SOCKET &sock);
	static void AddSafeSocket(const UINT32 &nCookie);

private:
	static void Init();
	static void UnInit();

private:
	static Manage_SecurityNum* ms_pImpl;
};
