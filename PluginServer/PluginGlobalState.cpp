#include "stdafx.h"
#include "PluginGlobalState.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoGlobalState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_CLEAR_CACHE		354
#define TIMER_ID_HANDLE_TIMEOUT_REQ	355

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2

typedef CProtoGlobalState		CProtoQuote;

//////////////////////////////////////////////////////////////////////////

CPluginGlobalState::CPluginGlobalState()
{
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
	m_bStartTimerHandleTimeout = FALSE;
}

CPluginGlobalState::~CPluginGlobalState()
{
	Uninit();
}

void CPluginGlobalState::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
{
	if ( m_pQuoteServer != NULL )
		return;

	if ( pQuoteServer == NULL || pQuoteData == NULL )
	{
		ASSERT(false);
		return;
	}

	m_pQuoteServer = pQuoteServer;
	m_pQuoteData = pQuoteData;
	m_TimerWnd.SetEventInterface(this);
	m_TimerWnd.Create();

	m_MsgHandler.SetEventInterface(this);
	m_MsgHandler.Create();
}

void CPluginGlobalState::Uninit()
{
	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;
		m_TimerWnd.Destroy();
		m_TimerWnd.SetEventInterface(NULL);

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);

		ReleaseAllReqData();
	}
}

void CPluginGlobalState::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QT_GET_GLOBAL_STATE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);
	
	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType	req;
	proto.SetProtoData_Req(&req);
	if ( !proto.ParseJson_Req(jsnVal) )
	{
		CHECK_OP(false, NORET);
		StockDataReq req_info;
		req_info.sock = sock;
		req_info.req = req;
		req_info.dwReqTick = ::GetTickCount();
		ReplyDataReqError(&req_info, PROTO_ERR_PARAM_ERR, L"参数错误！");
		return;
	}

	CHECK_RET(req.head.nProtoID == nCmdID, NORET);

	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->dwReqTick = ::GetTickCount();
	m_vtReqData.push_back(pReqInfo);

	//状态数据接口始终是ready状态
	m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);

	//SetTimerHandleTimeout(true);
}

void CPluginGlobalState::NotifyQuoteDataUpdate(int nCmdID)
{
	CHECK_RET(nCmdID == PROTO_ID_QT_GET_GLOBAL_STATE, NORET);
	CHECK_RET(m_pQuoteData, NORET);
}

void CPluginGlobalState::OnTimeEvent(UINT nEventID)
{
	if ( TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID )
	{
		HandleTimeoutReq();
	}
}

void CPluginGlobalState::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllReadyReq();
	}	
}

void CPluginGlobalState::HandleTimeoutReq()
{
	if (m_vtReqData.empty())
	{
		SetTimerHandleTimeout(false);
		return;
	}

	ReplyAllReadyReq();

	DWORD dwTickNow = ::GetTickCount();
	VT_STOCK_DATA_REQ &vtReq = m_vtReqData;
	VT_STOCK_DATA_REQ::iterator it_req = vtReq.begin();

	for (; it_req != vtReq.end();)
	{
		StockDataReq *pReq = *it_req;
		if (pReq == NULL)
		{
			CHECK_OP(false, NOOP);
			it_req = vtReq.erase(it_req);
			continue;
		}
		if (int(dwTickNow - pReq->dwReqTick) > REQ_TIMEOUT_MILLISECOND)
		{
			//tomodify timeout
			CStringA  strTimeout = "req timeout";
			OutputDebugStringA(strTimeout.GetString());
			ReplyDataReqError(pReq, PROTO_ERR_SERVER_TIMEROUT, L"请求超时！");

			it_req = vtReq.erase(it_req);
			delete pReq;
		}
		else
		{
			++it_req;
		}
	}

	if (m_vtReqData.size() == 0)
	{
		SetTimerHandleTimeout(false);
		return;
	}
}

bool CPluginGlobalState::DoFillAckDataBody(QuoteAckDataBody& ackBody)
{
	CHECK_RET(m_pQuoteData, false);

	NNGlobalState stState;
	m_pQuoteData->GetNNGlobalState(&stState);

	ackBody.nMarketStateHK = stState.eMktHK;
	ackBody.nMarketStateHKFuture = stState.eMktHKFuture;
	ackBody.nMarketStateSH = stState.eMktSH;
	ackBody.nMarketStateSZ = stState.eMktSZ;
	ackBody.nMarketStateUS = stState.eMktUS;
	
	ackBody.nQuoteLogined = stState.bQuoteSvrLogined;
	ackBody.nTradeLogined = stState.bTradeSvrLogined;
	 
	return true;
}

void CPluginGlobalState::ReplyAllReadyReq()
{
	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);

	VT_STOCK_DATA_REQ& vtReqData = m_vtReqData;

	//tomodify 3
	VT_STOCK_DATA_REQ::iterator it = vtReqData.begin();
	for (; it != vtReqData.end();)
	{
		StockDataReq *pReqData = *it;
		CHECK_OP(pReqData, continue);
		CProtoQuote::ProtoReqBodyType &reqBody = pReqData->req.body;
		QuoteAckDataBody ackBody;

		if (!DoFillAckDataBody(ackBody))
		{
			continue;
		}

		ReplyStockDataReq(pReqData, ackBody);

		it = vtReqData.erase(it);
		SAFE_DELETE(pReqData);
	}
}

void CPluginGlobalState::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
{
	CHECK_RET(pReq && m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReq->req.head;
	ack.head.ddwErrCode = 0;
	ack.body = data;

	//tomodify 4
	CProtoQuote proto;	
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if ( proto.MakeJson_Ack(jsnAck) )
	{
		std::string strOut;
		CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
		m_pQuoteServer->ReplyQuoteReq(pReq->req.head.nProtoID, strOut.c_str(), (int)strOut.size(), pReq->sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}
}

void CPluginGlobalState::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
{
	CHECK_RET(pReq && m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReq->req.head;
	ack.head.ddwErrCode = nErrCode;

	if ( pErrDesc )
	{
		CA::Unicode2UTF(pErrDesc, ack.head.strErrDesc);		 
	}

	CProtoQuote proto;	
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if ( proto.MakeJson_Ack(jsnAck) )
	{
		std::string strOut;
		CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
		m_pQuoteServer->ReplyQuoteReq(pReq->req.head.nProtoID, strOut.c_str(), (int)strOut.size(), pReq->sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}

}

void CPluginGlobalState::SetTimerHandleTimeout(bool bStartOrStop)
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
			m_TimerWnd.StartMillionTimer(1000, TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = TRUE;
		}
	}
}

void CPluginGlobalState::ReleaseAllReqData()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for (; it != m_vtReqData.end(); ++it)
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}

void CPluginGlobalState::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginGlobalState::DoClearReqInfo(SOCKET socket)
{
	VT_STOCK_DATA_REQ& vtReq = m_vtReqData;

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
