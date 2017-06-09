#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Trade.h"
#include "TimerWnd.h"
#include "MsgHandler.h"
#include "JsonCpp/json.h"
#include "DelayOrderIDCvt_US.h"

class CPluginUSTradeServer;

class CPluginPlaceOrder_US : public CTimerWndInterface, public CMsgHandlerEventInterface, public IOrderIDCvtNotify_US
{
public:
	CPluginPlaceOrder_US();
	virtual ~CPluginPlaceOrder_US();
	
	void Init(CPluginUSTradeServer* pTradeServer, ITrade_US*  pTradeOp);
	void Uninit();	
	void SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);
	void NotifyOnPlaceOrder(Trade_Env enEnv, UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nLocalID, INT64 nErrCode);

	void NotifySocketClosed(SOCKET sock);

protected:
	//CTimerWndInterface 
	virtual void OnTimeEvent(UINT nEventID);

	//CMsgHandlerEventInterface
	virtual void OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam);

protected:
	//IOrderIDCvtNotify_US
	virtual void OnCvtOrderID_Local2Svr(int nResult, Trade_Env eEnv, INT64 nLocalID, INT64 nServerID);

protected:
	//tomodify 1
	typedef PlaceOrder_Req	TradeReqType;
	typedef PlaceOrder_Ack	TradeAckType;

	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		DWORD	dwLocalCookie;
		TradeReqType req;
		bool bWaitSvrIDAfterPlacedOK;
		StockDataReq()
		{
			dwReqTick = 0;
			dwLocalCookie = 0;
			bWaitSvrIDAfterPlacedOK = false;
		}
	};
	
	typedef std::vector<StockDataReq*>		VT_REQ_TRADE_DATA;	
	
protected:	
	void HandleTimeoutReq();
	void HandleTradeAck(TradeAckType *pAck, SOCKET	sock);
	void SetTimerHandleTimeout(bool bStartOrStop);
	void ClearAllReqAckData();
	
private:
	void DoClearReqInfo(SOCKET socket);

protected:
	CPluginUSTradeServer	*m_pTradeServer;
	ITrade_US				*m_pTradeOp;	
	BOOL					m_bStartTimerHandleTimeout;
	
	CTimerMsgWndEx		m_TimerWnd;
	CMsgHandler			m_MsgHandler;

	VT_REQ_TRADE_DATA	m_vtReqData;

	CDelayOrderIDCvt_US  m_stOrderIDCvt;
};