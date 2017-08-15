#include "stdafx.h"
#include "PluginChangeOrder_US.h"
#include "PluginUSTradeServer.h"
#include "Protocol/ProtoChangeOrder.h"
#include "IManage_SecurityNum.h"


#include "stdafx.h"
#include "PluginChangeOrder_US.h"
#include "PluginUSTradeServer.h"
#include "Protocol/ProtoChangeOrder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_HANDLE_TIMEOUT_REQ	355
#define EVENT_ID_ACK_REQUEST		368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_TDUS_CHANGE_ORDER
typedef CProtoChangeOrder	CProtoQuote;



//////////////////////////////////////////////////////////////////////////

CPluginChangeOrder_US::CPluginChangeOrder_US()
{	
	m_pTradeOp = NULL;
	m_pTradeServer = NULL;
	m_bStartTimerHandleTimeout = FALSE;
}

CPluginChangeOrder_US::~CPluginChangeOrder_US()
{
	Uninit();
}

void CPluginChangeOrder_US::Init(CPluginUSTradeServer* pTradeServer, ITrade_US*  pTradeOp)
{
	if ( m_pTradeServer != NULL )
		return;

	if ( pTradeServer == NULL || pTradeOp == NULL )
	{
		ASSERT(false);
		return;
	}

	m_pTradeServer = pTradeServer;
	m_pTradeOp = pTradeOp;
	m_TimerWnd.SetEventInterface(this);
	m_TimerWnd.Create();

	m_MsgHandler.SetEventInterface(this);
	m_MsgHandler.Create();

	m_stOrderIDCvt.Init(m_pTradeOp, this);
}

void CPluginChangeOrder_US::Uninit()
{
	if ( m_pTradeServer != NULL )
	{
		m_pTradeServer = NULL;
		m_pTradeOp = NULL;

		m_TimerWnd.Destroy();
		m_TimerWnd.SetEventInterface(NULL);

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);

		ClearAllReqAckData();
	}
}

void CPluginChangeOrder_US::SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QUOTE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pTradeOp && m_pTradeServer, NORET);

	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType	req;
	proto.SetProtoData_Req(&req);
	if ( !proto.ParseJson_Req(jsnVal) )
	{
		CHECK_OP(false, NORET);
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = PROTO_ERR_PARAM_ERR;
		CA::Unicode2UTF(L"参数错误！", ack.head.strErrDesc);
		ack.body.nCookie = req.body.nCookie;
		ack.body.nSvrResult = Trade_SvrResult_Failed;
		HandleTradeAck(&ack, sock);
		return;
	}

	if (!IManage_SecurityNum::IsSafeSocket(sock))
	{
		CHECK_OP(false, NORET);
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = PROTO_ERR_UNKNOWN_ERROR;
		CA::Unicode2UTF(L"请重新解锁！", ack.head.strErrDesc);
		ack.body.nCookie = req.body.nCookie;
		ack.body.nSvrResult = Trade_SvrResult_Failed;
		HandleTradeAck(&ack, sock);
		return;
	}

	CHECK_RET(req.head.nProtoID == nCmdID && req.body.nCookie, NORET);
	ChangeOrderReqBody &body = req.body;		

	StockDataReq *pReq = new StockDataReq;
	CHECK_RET(pReq, NORET);
	pReq->sock = sock;
	pReq->dwReqTick = ::GetTickCount();
	pReq->req = req; 
	pReq->bWaitDelaySvrID = true; 

	DoTryProcessTradeOpt(pReq); 
}

void  CPluginChangeOrder_US::DoTryProcessTradeOpt(StockDataReq* pReq)
{
	CHECK_RET(m_pTradeOp && pReq, NORET); 

	bool bIsNewReq = !IsReqDataExist(pReq); 

	ChangeOrderReqBody &body = pReq->req.body;
	body.nChangeOrderStatus = Trade_SetOrderStatus_Change;
	TradeReqType& req = pReq->req; 
	SOCKET sock = pReq->sock; 

	//如果是新的调用， 通过本地id查找定单SvrID 
	if (bIsNewReq)
	{
		m_vtReqData.push_back(pReq); 

		if (0 == body.nSvrOrderID && 0 != body.nLocalOrderID) 
		{ 
			body.nSvrOrderID = m_stOrderIDCvt.FindSvrOrderID((Trade_Env)body.nEnvType, 
				body.nLocalOrderID, true);	

			// 等待svrid 取到后再实际调用接口
			if (0 == body.nSvrOrderID)
			{ 
				return; 
			}
		} 
	} 

	//减少不必要的请求， 避免超过无意义的超过调用频率
	if (IsNewStateNotNeedReq((Trade_Env)body.nEnvType, body.nSvrOrderID, (Trade_SetOrderStatus)body.nChangeOrderStatus))
	{
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = 0;
		ack.head.strErrDesc = "";

		ack.body.nEnvType = body.nEnvType;
		ack.body.nCookie = body.nCookie;
		ack.body.nLocalOrderID = body.nLocalOrderID;
		ack.body.nSvrOrderID = body.nSvrOrderID;
		ack.body.nSvrResult = Trade_SvrResult_Succeed;
		HandleTradeAck(&ack, sock);

		//清除req对象 
		DoRemoveReqData(pReq);
		return;
	}

	// 
	bool bRet = false;
	int nReqResult = 0;
	//美股只支持撤单， 只有真实交易环境!!
	if (body.nSvrOrderID != 0 && body.nEnvType == Trade_Env_Real)
	{  
		pReq->bWaitDelaySvrID = false;
		bRet = m_pTradeOp->ChangeOrder((UINT*)&pReq->dwLocalCookie, body.nSvrOrderID, 
			body.nPrice, body.nQty, &nReqResult);
	} 

	if ( !bRet )
	{
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = UtilPlugin::ConvertErrCode((QueryDataErrCode)nReqResult);
		CA::Unicode2UTF(L"发送失败", ack.head.strErrDesc);
		if (nReqResult != 0)
		{
			ack.head.strErrDesc = UtilPlugin::GetErrStrByCode((QueryDataErrCode)nReqResult);
		}

		ack.body.nEnvType = body.nEnvType;
		ack.body.nCookie = body.nCookie;
		ack.body.nLocalOrderID = body.nLocalOrderID;
		ack.body.nSvrOrderID = body.nSvrOrderID; 

		ack.body.nSvrResult = Trade_SvrResult_Failed;
		HandleTradeAck(&ack, sock);

		//清除req对象 
		DoRemoveReqData(pReq);
		return ;
	}

	SetTimerHandleTimeout(true);
} 

void CPluginChangeOrder_US::NotifyOnChangeOrder(Trade_Env enEnv, UINT nCookie, 
												Trade_SvrResult enSvrRet, UINT64 nLocalID, INT64 nErrCode)
{
	CHECK_RET(nCookie, NORET);
	CHECK_RET(m_pTradeOp && m_pTradeServer, NORET);

	VT_REQ_TRADE_DATA::iterator itReq = m_vtReqData.begin();
	StockDataReq *pFindReq = NULL;
	for ( ; itReq != m_vtReqData.end(); ++itReq )
	{
		StockDataReq *pReq = *itReq;
		CHECK_OP(pReq, continue);
		if ( pReq->dwLocalCookie == nCookie )
		{
			pFindReq = pReq;
			break;
		}
	}
	if (!pFindReq)
		return; 

	TradeAckType ack;
	ack.head = pFindReq->req.head;
	ack.head.ddwErrCode = nErrCode;
	if (nErrCode != 0 || enSvrRet != Trade_SvrResult_Succeed)
	{
		WCHAR szErr[256] = L"发送请求失败!";
		if (nErrCode != 0)
			m_pTradeOp->GetErrDescV2(nErrCode, szErr);

		CA::Unicode2UTF(szErr, ack.head.strErrDesc);
	}

	//tomodify 4
	ack.body.nEnvType = enEnv;
	ack.body.nCookie = pFindReq->req.body.nCookie;
	ack.body.nLocalOrderID = pFindReq->req.body.nLocalOrderID;
	ack.body.nSvrOrderID = pFindReq->req.body.nSvrOrderID; 

	ack.body.nSvrResult = enSvrRet;
	HandleTradeAck(&ack, pFindReq->sock);

	m_vtReqData.erase(itReq);
	delete pFindReq;
}

void CPluginChangeOrder_US::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginChangeOrder_US::OnTimeEvent(UINT nEventID)
{
	if ( TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID )
	{
		HandleTimeoutReq();
	}
}

void CPluginChangeOrder_US::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{		
	}	
}

void CPluginChangeOrder_US::HandleTimeoutReq()
{
	if ( m_vtReqData.empty() )
	{
		SetTimerHandleTimeout(false);
		return;
	}

	DWORD dwTickNow = ::GetTickCount();	
	VT_REQ_TRADE_DATA::iterator it_req = m_vtReqData.begin();
	for ( ; it_req != m_vtReqData.end(); )
	{
		StockDataReq *pReq = *it_req;	
		if ( pReq == NULL )
		{
			CHECK_OP(false, NOOP);
			++it_req;
			continue;
		}		
		//
		if (pReq->bWaitDelaySvrID)
		{
			continue;;
		}

		if ( int(dwTickNow - pReq->dwReqTick) > 8000 )
		{
			TradeAckType ack;
			ack.head = pReq->req.head;
			ack.head.ddwErrCode= PROTO_ERR_SERVER_TIMEROUT;
			CA::Unicode2UTF(L"协议超时", ack.head.strErrDesc);

			//tomodify 5
			ack.body.nEnvType = pReq->req.body.nEnvType;
			ack.body.nCookie = pReq->req.body.nCookie;
			ack.body.nSvrOrderID = pReq->req.body.nSvrOrderID;
			ack.body.nLocalOrderID = pReq->req.body.nLocalOrderID;

			ack.body.nSvrResult = Trade_SvrResult_Failed;
			HandleTradeAck(&ack, pReq->sock);

			it_req = m_vtReqData.erase(it_req);
			delete pReq;
		}
		else
		{
			++it_req;
		}
	}

	if ( m_vtReqData.empty() )
	{
		SetTimerHandleTimeout(false);
		return;
	}
}

void CPluginChangeOrder_US::HandleTradeAck(TradeAckType *pAck, SOCKET sock)
{
	CHECK_RET(pAck && pAck->body.nCookie && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pTradeServer, NORET);

	CProtoQuote proto;
	proto.SetProtoData_Ack(pAck);

	Json::Value jsnValue;
	bool bRet = proto.MakeJson_Ack(jsnValue);
	CHECK_RET(bRet, NORET);

	std::string strBuf;
	CProtoParseBase::ConvJson2String(jsnValue, strBuf, true);
	m_pTradeServer->ReplyTradeReq(PROTO_ID_QUOTE, strBuf.c_str(), (int)strBuf.size(), sock);
}

void CPluginChangeOrder_US::SetTimerHandleTimeout(bool bStartOrStop)
{
	if ( m_bStartTimerHandleTimeout )
	{
		if ( !bStartOrStop )
		{			
			m_TimerWnd.StopTimer(TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = FALSE;
		}
	}
	else
	{
		if ( bStartOrStop )
		{
			m_TimerWnd.StartMillionTimer(500, TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = TRUE;
		}
	}
}

void CPluginChangeOrder_US::ClearAllReqAckData()
{
	VT_REQ_TRADE_DATA::iterator it_req = m_vtReqData.begin();
	for ( ; it_req != m_vtReqData.end(); )
	{
		StockDataReq *pReq = *it_req;
		delete pReq;
	}

	m_vtReqData.clear();
}

bool CPluginChangeOrder_US::IsReqDataExist( StockDataReq* pReq )
{
	VT_REQ_TRADE_DATA::iterator it = m_vtReqData.begin();
	while (it != m_vtReqData.end())
	{
		StockDataReq* pItem = *it;
		if (pItem && pItem == pReq) 
		{ 
			return true; 
		}
		++it; 
	}
	return false; 
}

void CPluginChangeOrder_US::DoRemoveReqData(StockDataReq* pReq)
{
	VT_REQ_TRADE_DATA::iterator it = m_vtReqData.begin();
	while (it != m_vtReqData.end())
	{
		StockDataReq* pItem = *it;
		if (pItem && pItem == pReq) 
		{ 
			it = m_vtReqData.erase(it);
			SAFE_DELETE(pItem); 
			return; 
		}
		++it; 
	}
}

void CPluginChangeOrder_US::OnCvtOrderID_Local2Svr( int nResult, Trade_Env eEnv, 
												   INT64 nLocalID, INT64 nServerID )
{
	VT_REQ_TRADE_DATA vtTmp = m_vtReqData;

	VT_REQ_TRADE_DATA::iterator it = vtTmp.begin();
	while (it != vtTmp.end())
	{
		StockDataReq* pItem = *it; 
		if (!pItem)
			continue;

		if (pItem->req.body.nEnvType == eEnv && pItem->req.body.nLocalOrderID == nLocalID)
		{ 
			if (0 == pItem->req.body.nSvrOrderID)
				pItem->req.body.nSvrOrderID = nServerID;  
			else 
				CHECK_OP(false, NOOP); 

			DoTryProcessTradeOpt(pItem);
		}
		++it; 
	}
}

void CPluginChangeOrder_US::DoClearReqInfo(SOCKET socket)
{
	VT_REQ_TRADE_DATA& vtReq = m_vtReqData;

	//清掉socket对应的请求信息
	auto itReq = vtReq.begin();
	while (itReq != vtReq.end())
	{
		if (*itReq && (*itReq)->sock == socket)
		{
			delete *itReq;
			itReq = vtReq.erase(itReq);
		}
		else
		{
			++itReq;
		}
	}
}

bool CPluginChangeOrder_US::IsNewStateNotNeedReq(Trade_Env eEnv, INT64 nSvrOrderID, Trade_SetOrderStatus eNewStatus)
{
	if (0 == nSvrOrderID || eEnv != Trade_Env_Real)
	{
		return false;
	}
	Trade_OrderStatus eCurStatus = Trade_OrderStatus_Processing;
	if (!m_pTradeOp->GetOrderStatus(nSvrOrderID, eCurStatus))
	{
		return false;
	}

	bool bRet = false;
	switch (eNewStatus)
	{
	case Trade_SetOrderStatus_Change:
		bRet = Trade_OrderStatus_Cancelled == eCurStatus || Trade_OrderStatus_Deleted == eCurStatus || Trade_OrderStatus_Processing == eCurStatus;
		break;
	case Trade_SetOrderStatus_Disable:
	case Trade_SetOrderStatus_Enable:
	case Trade_SetOrderStatus_Delete:
	case Trade_SetOrderStatus_HK_SplitLargeOrder:
	case Trade_SetOrderStatus_HK_PriceTooFar:
	case Trade_SetOrderStatus_HK_BuyWolun:
	case Trade_SetOrderStatus_HK_BuyGuQuan:
	case Trade_SetOrderStatus_HK_BuyLowPriceStock:
	default:
		break;
	}
	return bRet;
}