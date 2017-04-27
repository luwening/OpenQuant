#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Protocol/ProtoDataStruct_Quote.h"
#include "TimerWnd.h"
#include "MsgHandler.h"
#include "JsonCpp/json.h"

class CPluginQuoteServer;

class CPluginExRightInfo : public CMsgHandlerEventInterface
{
public:
	CPluginExRightInfo();
	virtual ~CPluginExRightInfo();
	
	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData);
	void Uninit();	
	void SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);
	void NotifyQuoteDataUpdate(int nCmdID, INT64 nStockID);

	void NotifySocketClosed(SOCKET sock);

protected:	
	//CMsgHandlerEventInterface
	virtual void OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam);

protected:
	//tomodify 1
	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		std::vector<INT64> vtStockID;
		ExRightInfo_Req req;
	};

	struct StockMktCode
	{
		int nMarketType;
		std::string strCode;
	};

	typedef std::vector<StockDataReq*>	VT_STOCK_DATA_REQ;
	typedef ExRightInfoAckBody	QuoteAckDataBody;	
	typedef std::map<INT64, StockMktCode>		MAP_STOCK_ID_CODE;

protected:	
	void ReplyAllRequest();
	void ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data);
	void ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc);

	void ReleaseAllReqData();
	int  GetMarketTimezone(StockMktType eMkt);
	void FormatTimestampToDate(int nTimestamp, int nTimezone, std::string &strFmtTime);
	
private:
	void DoClearReqInfo(SOCKET socket);

protected:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteData*		m_pQuoteData;
	CMsgHandler			m_MsgHandler;

	VT_STOCK_DATA_REQ	m_vtReqData;
	MAP_STOCK_ID_CODE	m_mapStockIDCode;
};