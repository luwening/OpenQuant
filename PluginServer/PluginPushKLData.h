#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Quote.h"
#include "JsonCpp/json.h"
#include "MsgHandler.h"
#include <set>

class CPluginQuoteServer;

class CPluginPushKLData: public CMsgHandlerEventInterface
{
public:
	CPluginPushKLData();
	virtual ~CPluginPushKLData();

	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData* pQuoteData);
	void Uninit();	
	void PushStockData(INT64 nStockID, SOCKET sock, StockSubType eStockSubType);

	void NotifySocketClosed(SOCKET sock);
	void NotifyMarketNewTrade(StockMktType eMkt);

protected:
	// CMsgHandlerEventInterface
	virtual void OnMsgEvent(int nEvent, WPARAM wParam, LPARAM lParam);

private:
	void OnPushTrigerRTKLDataError();

protected:
	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		DWORD	dwLocalCookie;
		INT64	nStockID;
		PushKLData_Req req;
	};
	struct StockMktCode
	{
		int nMarketType;
		std::string strCode;
	};

	//tomodify 1
	typedef PushKLDataAckBody	QuoteAckDataBody;

protected:
	DWORD GetLastPushKL(INT64 ddwStockHash, SOCKET sock, StockSubType eStockSubType);
	bool SetLastPushKL(INT64 ddwStockHash, SOCKET sock, StockSubType eStockSubType, DWORD dwTime);

protected:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteData*		m_pQuoteData;
	CMsgHandler			m_MsgHandler;
	std::set<std::pair<INT64, int>> m_setPushTrigerRTKLError;
	DWORD  m_dwTickLastTrigerKLError;

	std::map<SOCKET, std::vector<Stock_PushInfo>> m_mapPushInfo;
};