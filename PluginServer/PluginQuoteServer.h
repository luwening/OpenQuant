#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct.h"
#include "JsonCpp/json.h"
#include "PluginBasicPrice.h"
#include "PluginGearPrice.h"
#include "PluginRTData.h"
#include "PluginKLData.h"
#include "PluginStockSub.h"
#include "PluginStockUnSub.h"
#include "PluginQueryStockSub.h"
#include "PluginTradeDate.h"
#include "PluginStockList.h"
#include "PluginBatchBasic.h"
#include "PluginTickerPrice.h"
#include "PluginSnapshot.h"
#include "PluginHistoryKL.h"
#include "PluginExRightInfo.h"
#include "PluginPushStockData.h"
#include "PluginPushBatchBasic.h"
#include "PluginPushGearPrice.h"
#include "PluginPushTickerPrice.h"
#include "PluginPushKLData.h"
#include "PluginPushRTData.h"
#include "PluginPlatesetIDs.h"
#include "PluginPlateSubIDs.h"
#include "PluginBrokerQueue.h"
#include "PluginGlobalState.h"

class CPluginNetwork;

enum QuoteServerType
{
	QuoteServer_BasicPrice = 1,
	QuoteServer_GearPrice = 2,
	QuoteServer_RTData = 3,
	QuoteServer_KLData = 4,
	QuoteServer_StockSub = 5,
	QuoteServer_StockUnSub = 6,
	QuoteServer_QueryStockSub = 7,
	QuoteServer_BatchBasic = 8,
	QuoteServer_TickerPrice = 9,
	QuoteServer_Snapshot = 10,
	QuoteServer_HistoryKL = 11,
	QuoteServer_ExRightInfo = 12,
	QuoteServer_PushStockData = 13,
	QuoteServer_PushBatchBasic = 14,
	QuoteServer_PushGearPrice = 15,
	QuoteServer_PushKLData = 16,
	QuoteServer_PushRTData = 17,
	QuoteServer_PushTickerPrice = 18,
};

struct tagRTKLDataRefresh
{
	INT64 ddwStockHash;
	StockSubType eStockSubType;
	DWORD dwReqCookie;
	int nTryCount;
	int nWaitSecs;
	tagRTKLDataRefresh();
};

class CPluginQuoteServer: 	
	public IQuoteInfoCallback,
	public IQuoteKLCallback,
	public CTimerWndInterface
{
public:
	CPluginQuoteServer();
	virtual ~CPluginQuoteServer();
	
	void InitQuoteSvr(IFTPluginCore* pPluginCore, CPluginNetwork *pNetwork);
	void UninitQuoteSvr();	
	void SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);
	void ReplyQuoteReq(int nCmdID, const char *pBuf, int nLen, SOCKET sock);

	StockSubErrCode SubscribeQuote(const std::string &strCode, StockMktType nMarketType, StockSubType eStockSubType, bool bSubOrUnsub, SOCKET sock);
	QueryDataErrCode QueryStockRTData(DWORD* pCookie, const std::string &strCode, StockMktType nMarketType, QuoteServerType type);
	QueryDataErrCode QueryStockKLData(DWORD* pCookie, const std::string &strCode, StockMktType nMarketType, QuoteServerType type, int nKLType);
	void CloseSocket(SOCKET sock);

	QueryDataErrCode QueryPlatesetSubIDList(DWORD* pdwCookie, INT64 nPlatesetID);
	QueryDataErrCode QueryPlateSubIDList(DWORD* pdwCookie, INT64 nPlateID);

protected:
	//IQuoteInfoCallback
	virtual void  OnChanged_PriceBase(INT64  ddwStockHash); 
	virtual void  OnChanged_OrderQueue(INT64 ddwStockHash); 
	virtual void  OnChanged_BrokerQueue(INT64 ddwStockHash);
	virtual void  OnChanged_RTData(INT64 ddwStockHash);
	virtual void  OnChanged_KLData(INT64 ddwStockHash, int nKLType);
	virtual void  OnReqStockSnapshot(DWORD dwCookie, PluginStockSnapshot *arSnapshot, int nSnapshotNum);

	virtual void  OnPushPriceBase(INT64 ddwStockHash, SOCKET sock);
	virtual void  OnPushGear(INT64 ddwStockHash, SOCKET sock);
	virtual void  OnPushTicker(INT64 ddwStockHash, SOCKET sock);
	virtual void  OnPushKL(INT64  ddwStockHash, SOCKET sock, StockSubType eStockSubType);
	virtual void  OnPushRT(INT64  ddwStockHash, SOCKET sock);
	virtual void  OnPushBrokerQueue(INT64 ddwStockHash, SOCKET sock);
	virtual void  OnPushMarketNewTrade(StockMktType eMkt, INT64 ddwLastTradeStamp, INT64 ddwNewTradeStamp);
	virtual void  OnPushHeartBeat(SOCKET sock, UINT64 nTimeStampNow);

	//IQuoteKLCallback
	virtual void  OnQueryStockRTData(DWORD dwCookie, int nCSResult);
	virtual void  OnQueryStockKLData(DWORD dwCookie, int nCSResult);

	//°å¿éÇëÇó
	virtual void  OnReqPlatesetIDs(int nCSResult, DWORD dwCookie);
	virtual void  OnReqPlateStockIDs(int nCSResult, DWORD dwCookie);

	//CTimerWndInterface
	virtual void OnTimeEvent(UINT nEventID);

private:
	bool DoKLSubTypeConvert(StockSubType eType, int& nKLType);
	void DoTryRefreshRTKLData(INT64 ddwStockHash, StockSubType eSubType);
	void DoCheckRTKLFreshReq(DWORD dwCookie, int nCSResult);

	void DoKillTimerRefreshRTKL();
	void DoStartTimerRefreshRTKL();

protected:
	IFTPluginCore		*m_pPluginCore;
	IFTQuoteData		*m_pQuoteData;
	IFTQuoteOperation	*m_pQuoteOp;
	IFTDataReport		*m_pDataReport;
	CPluginNetwork		*m_pNetwork;

	CPluginBasicPrice	m_BasicPrice;
	CPluginGearPrice	m_GearPrice;
	CPluginRTData		m_RTData;

	CPluginKLData		m_KLData;
	CPluginStockSub		m_StockSub;
	CPluginStockUnSub	m_StockUnSub;

	CPluginQueryStockSub m_QueryStockSub;
	CPluginTradeDate	m_TradeDate;
	CPluginStockList	m_StockList;

	CPluginBatchBasic   m_BatchBasic;
	CPluginTickerPrice  m_TickerPrice;
	CPluginSnapshot		m_Snapshot;

	CPluginHistoryKL	m_HistoryKL;
	CPluginExRightInfo  m_ExRightInfo;
	CPluginPushStockData m_PushStockData;

	CPluginPushBatchBasic m_PushBatchBasic;
	CPluginPushGearPrice m_PushGearPrice;
	CPluginPushTickerPrice m_PushTickerPrice;

	CPluginPushKLData	m_PushKLData;
	CPluginPushRTData	m_PushRTData;
	CPluginPlatesetIDs  m_platesetIDs;

	CPluginPlateSubIDs  m_plateSubIDs;
	CPluginBrokerQueue  m_BrokerQueue;
	CPluginGlobalState	m_GlobalState;

	CTimerMsgWndEx m_TimerWnd;
	UINT m_nTimerIDRefreshRTKL;
	std::vector<tagRTKLDataRefresh> m_vtRTKLRefresh;
	
};