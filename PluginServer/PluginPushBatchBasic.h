#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Quote.h"
#include "JsonCpp/json.h"
#include <set>

class CPluginQuoteServer;

class CPluginPushBatchBasic
{
public:
	CPluginPushBatchBasic();
	virtual ~CPluginPushBatchBasic();

	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData);
	void Uninit();	
	void PushStockData(INT64 nStockID, SOCKET sock);

protected:
	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;		
		PushBatchBasic_Req req;
	};
	struct StockMktCode
	{
		int nMarketType;
		std::string strCode;
	};

	//tomodify 1
	typedef PushBatchBasicAckBody	QuoteAckDataBody;

protected:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteData*		m_pQuoteData;
};