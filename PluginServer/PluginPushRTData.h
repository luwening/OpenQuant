#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Quote.h"
#include "JsonCpp/json.h"

class CPluginQuoteServer;

class CPluginPushRTData
{
public:
	CPluginPushRTData();
	virtual ~CPluginPushRTData();

	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData* pQuoteData);
	void Uninit();	
	void PushStockData(INT64 nStockID, SOCKET sock);

	void NotifySocketClosed(SOCKET sock);
	void NotifyMarketNewTrade(StockMktType eMkt);

protected:
	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		DWORD	dwLocalCookie;
		INT64	nStockID;
		PushRTData_Req req;
	};
	struct StockMktCode
	{
		int nMarketType;
		std::string strCode;
	};

	//tomodify 1
	typedef PushRTDataAckBody	QuoteAckDataBody;

	DWORD GetLastPushRT(INT64 ddwStockHash, SOCKET sock);
	bool SetLastPushRT(INT64 ddwStockHash, SOCKET sock, DWORD dwTime);

protected:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteData*		m_pQuoteData;

	std::map<SOCKET, std::vector<Stock_PushInfo>> m_mapPushInfo;
};