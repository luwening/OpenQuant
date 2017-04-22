#include "stdafx.h"
#include "PluginBrokerQueue.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoBrokerQueue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_CLEAR_CACHE		354
#define TIMER_ID_HANDLE_TIMEOUT_REQ	355

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_GEAR_PRICE
#define QUOTE_SERVER_TYPE	QuoteServer_BrokerQueue
typedef CProtoBrokerQueue		CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginBrokerQueue::CPluginBrokerQueue()
{
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
	m_bStartTimerHandleTimeout = FALSE;
}

CPluginBrokerQueue::~CPluginBrokerQueue()
{
	Uninit();
}

void CPluginBrokerQueue::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginBrokerQueue::Uninit()
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

void CPluginBrokerQueue::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QT_GET_BROKER_QUEUE && sock != INVALID_SOCKET, NORET);
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
		ReplyDataReqError(&req_info, PROTO_ERR_PARAM_ERR, L"参数错误！");
		return;
	}

	CHECK_RET(req.head.nProtoID == nCmdID, NORET);
	
	std::wstring strCode;
	CA::UTF2Unicode(req.body.strStockCode.c_str(), strCode);
	INT64 nStockID = m_pQuoteData->GetStockHashVal(strCode.c_str(), (StockMktType)req.body.nStockMarket);
	if ( nStockID == 0 )
	{
		CHECK_OP(false, NOOP);
		StockDataReq req_info;
		req_info.nStockID = nStockID;
		req_info.sock = sock;
		req_info.req = req;
		ReplyDataReqError(&req_info, PROTO_ERR_STOCK_NOT_FIND, L"找不到股票！");
		return;
	}	
	bool bIsSub = m_pQuoteData->IsSubStockOneType(nStockID, StockSubType_Broker);
	if (!bIsSub)
	{
		StockDataReq req_info;
		req_info.nStockID = nStockID;
		req_info.sock = sock;
		req_info.req = req;
		ReplyDataReqError(&req_info, PROTO_ERR_VER_NOT_SUPPORT, L"股票未订阅！");
		return;
	}

	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);
	pReqInfo->nStockID = nStockID;
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->dwReqTick = ::GetTickCount();
	m_vtReqData.push_back(pReqInfo);

	int nCount = 0;
	if (m_pQuoteData->GetBrokerQueueList(nStockID, NULL, nCount))
	{
		m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);
	}
	SetTimerHandleTimeout(true);
}

void CPluginBrokerQueue::NotifyQuoteDataUpdate(int nCmdID, INT64 nStockID)
{
	CHECK_RET(nCmdID == PROTO_ID_QT_GET_BROKER_QUEUE && nStockID, NORET);
	CHECK_RET(m_pQuoteData, NORET);
	
	bool bIsSub = m_pQuoteData->IsSubStockOneType(nStockID, StockSubType_Broker);
	if ( !bIsSub )
	{
		return;
	}
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	StockDataReq* pReqItem = NULL;
	while (it != m_vtReqData.end())
	{
		if (*it && (*it)->nStockID == nStockID)
		{
			pReqItem = *it;
			break;
		}
		++it;
	}
	if (pReqItem)
	{
		ReplyAllReadyReq();
	}
}

void CPluginBrokerQueue::OnTimeEvent(UINT nEventID)
{
	if ( TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID )
	{
		HandleTimeoutReq();
	}
}

void CPluginBrokerQueue::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllReadyReq();
	}	
}

void CPluginBrokerQueue::HandleTimeoutReq()
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
		if (int(dwTickNow - pReq->dwReqTick) > 10000)
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

bool CPluginBrokerQueue::DoFillAckDataBody(INT64 nStockID, QuoteAckDataBody& ackBody)
{
	CHECK_RET(m_pQuoteData, false);

	//股票信息
	StockMktType eMkt = StockMkt_HK;
	wchar_t szStockCode[16] = { 0 }, szStockName[128] = { 0 };
	if (!m_pQuoteData->GetStockInfoByHashVal(nStockID, eMkt, szStockCode, szStockName))
	{
		return false;
	}
	ackBody.nStockMarket = (int)eMkt;
	CA::Unicode2UTF(szStockCode, ackBody.strStockCode);

	//数据项
	ackBody.vtBrokerAsk.clear();
	ackBody.vtBrokerBid.clear();

	int nItemNum = 0;
	Quote_BrokerItem* parItem = NULL;

	//找到数据已经准备好的请求项
	if (!m_pQuoteData->GetBrokerQueueList(nStockID, parItem, nItemNum))
	{
		//不存在数据, 等定阅通知
		return false;
	}

	if (nItemNum > 0)
	{
		parItem = new Quote_BrokerItem[nItemNum];
		m_pQuoteData->GetBrokerQueueList(nStockID, parItem, nItemNum);
	}

	for (int n = 0; n < nItemNum; n++)
	{
		Quote_BrokerItem& stItem = parItem[n];

		BrokerQueueAckItem AckItem;
		AckItem.nBrokerID = stItem.nBrokerID;
		AckItem.nBrokerPos = stItem.nBrokerPos;
		AckItem.strBrokerName = stItem.strBrokerName;

		if (stItem.bAskOrBid)
		{
			ackBody.vtBrokerAsk.push_back(AckItem);
		}
		else
		{
			ackBody.vtBrokerBid.push_back(AckItem);
		}
	}
	SAFE_DELETE_ARR(parItem);

	return true;
}

void CPluginBrokerQueue::ReplyAllReadyReq()
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

		if (!DoFillAckDataBody(pReqData->nStockID, ackBody))
		{
			continue;
		}

		ReplyStockDataReq(pReqData, ackBody);

		it = vtReqData.erase(it);
		SAFE_DELETE(pReqData);
	}
}

void CPluginBrokerQueue::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
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

void CPluginBrokerQueue::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
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

void CPluginBrokerQueue::SetTimerHandleTimeout(bool bStartOrStop)
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

void CPluginBrokerQueue::ReleaseAllReqData()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for (; it != m_vtReqData.end(); ++it)
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}


void CPluginBrokerQueue::PushStockData(INT64 nStockID, SOCKET sock)
{
	CHECK_RET(m_pQuoteData, NORET);

	QuoteAckDataBody ackbody;
	if (!DoFillAckDataBody(nStockID, ackbody))
	{
		return;
	}

	CProtoQuote::ProtoAckDataType ack;
	ack.head.nProtoID = PROTO_ID_PUSH_BROKER_QUEUE;
	ack.head.ddwErrCode = 0;
	ack.head.nProtoVer = 1;
	ack.body = ackbody;

	CProtoQuote proto;
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if (proto.MakeJson_Ack(jsnAck))
	{
		std::string strOut;
		CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
		m_pQuoteServer->ReplyQuoteReq(PROTO_ID_PUSH_BROKER_QUEUE, strOut.c_str(), (int)strOut.size(), sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}

}
