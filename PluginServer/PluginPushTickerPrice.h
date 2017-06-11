#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Quote.h"
#include "TimerWnd.h"
#include "MsgHandler.h"
#include "JsonCpp/json.h"
#include "Protocol/ProtoPushTickerPrice.h"

class CPluginQuoteServer;

class CPluginPushTickerPrice
{
	//tomodify 1
	typedef CProtoPushTickerPrice	CProtoQuote;

public:
	CPluginPushTickerPrice();
	virtual ~CPluginPushTickerPrice();
	
	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData);
	void Uninit();
	void PushStockData(INT64 ddwStockHash, SOCKET sock);
	
	void NotifySocketClosed(SOCKET sock);
	void NotifyMarketNewTrade(StockMktType eMkt);

protected:
	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick; 
		INT64	nStockID;		
		CProtoQuote::ProtoReqDataType req;
	};
	struct StockMktCode
	{
		int nMarketType;
		std::string strCode;
	};
	
	typedef CProtoQuote::ProtoAckBodyType		QuoteAckDataBody;

protected:
	INT64 GetLastPushTicker(INT64 ddwStockHash, SOCKET sock);
	bool SetLastPushTicker(INT64 ddwStockHash, SOCKET sock, INT64 nSequence);
	
protected:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteData*		m_pQuoteData;

	std::map<SOCKET, std::vector<Stock_PushInfo>> m_mapPushInfo;
};