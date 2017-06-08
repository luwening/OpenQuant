#include "stdafx.h"
#include "PluginBatchBasic.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoBatchBasic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_HANDLE_TIMEOUT_REQ	355
#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_BATCHBASIC
typedef CProtoBatchBasic		CProtoQuote;

//股票状态 
enum
{
	STOCK_STATUS_NONE = 0,
	STOCK_STATUS_SUSPENSION = 1,  //停牌
	STOCK_STATUS_NORMAL = 2,
	STOCK_STATUS_PAUSE_Temp = 3, //熔断(可恢复) 
	STOCK_STATUS_PAUSE_Break = 4, //熔断(不可恢复) 
};

//////////////////////////////////////////////////////////////////////////

CPluginBatchBasic::CPluginBatchBasic()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
	m_bStartTimerHandleTimeout = 0;
}

CPluginBatchBasic::~CPluginBatchBasic()
{
	Uninit();
}

void CPluginBatchBasic::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginBatchBasic::Uninit()
{
	ReleaseAllReqData();

	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;		

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);

		m_TimerWnd.Destroy();
		m_TimerWnd.SetEventInterface(NULL);
	}
}

void CPluginBatchBasic::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QUOTE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);

	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType	req;
	proto.SetProtoData_Req(&req);
	if (!proto.ParseJson_Req(jsnVal))
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

	if ((int)req.body.vtReqBatchBasic.size() > 50 || (int)req.body.vtReqBatchBasic.size() < 1)
	{
		//////参数错误
		StockDataReq req_info;
		//req_info.nStockID = nStockID;
		req_info.sock = sock;
		req_info.req = req;
		req_info.dwReqTick = ::GetTickCount();
		ReplyDataReqError(&req_info, PROTO_ERR_PARAM_ERR, L"参数错误！");
		return;
	}
	
	//检查 stockID 及定阅状态 
	for (int i = 0; i < (int)req.body.vtReqBatchBasic.size(); i++)
	{
		INT64 nStockID = IFTStockUtil::GetStockHashVal(req.body.vtReqBatchBasic[i].strStockCode.c_str(),
					(StockMktType)req.body.vtReqBatchBasic[i].nStockMarket);
		if (nStockID == 0)
		{
			StockDataReq req_info;
			req_info.sock = sock;
			req_info.req = req;
			req_info.dwReqTick = ::GetTickCount();
			ReplyDataReqError(&req_info, PROTO_ERR_STOCK_NOT_FIND, L"找不到股票！");
			return;
		}
		if (!m_pQuoteData->IsSubStockOneType(nStockID, StockSubType_Simple))
		{
			StockDataReq req_info;
			req_info.sock = sock;
			req_info.req = req;
			req_info.dwReqTick = ::GetTickCount();
			ReplyDataReqError(&req_info, PROTO_ERR_UNSUB_ERR, L"股票未订阅！");
			return;
		}
	}
 
	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->dwReqTick = ::GetTickCount();
	m_vtReqData.push_back(pReqInfo);

	m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);
	
	SetTimerHandleTimeout(true);
}

void CPluginBatchBasic::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginBatchBasic::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllReadyReq();
	}	
}

void CPluginBatchBasic::OnTimeEvent(UINT nEventID)
{
	if (TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID)
	{
		HandleTimeoutReq();
	}
}

void CPluginBatchBasic::HandleTimeoutReq()
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

void CPluginBatchBasic::ReplyAllReadyReq()
{
	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);

	VT_STOCK_DATA_REQ& vtReqData = m_vtReqData;
	
	//tomodify 3
	VT_STOCK_DATA_REQ::iterator it = vtReqData.begin();
	for ( ; it != vtReqData.end(); )
	{
		StockDataReq *pReqData = *it;
		CHECK_OP(pReqData, continue);
		CProtoQuote::ProtoReqBodyType &reqBody = pReqData->req.body;
		CProtoQuote::ProtoAckBodyType ackBody;
	  
		if (InnerTryFillReplyData(pReqData, ackBody))
		{
			ReplyStockDataReq(pReqData, ackBody);

			it = vtReqData.erase(it);
			SAFE_DELETE(pReqData);
		}
		else
		{
			++it;
		}
	}
}

//tomodify 4
bool CPluginBatchBasic::InnerTryFillReplyData(const StockDataReq* pReq, QuoteAckDataBody& ackBody)
{
	CHECK_RET(pReq, false);
	const BatchBasic_Req& req = pReq->req;

	Quote_BatchBasic batchprice;
	bool bFillSuccess = false;
	for (int i = 0; i < (int)req.body.vtReqBatchBasic.size(); i++)
	{
		const BatchBasicReqItem& reqItem = req.body.vtReqBatchBasic[i];
		INT64 nStockID = IFTStockUtil::GetStockHashVal(reqItem.strStockCode.c_str(), (StockMktType)reqItem.nStockMarket);
		if (m_pQuoteData->FillBatchBasic(nStockID, &batchprice))
		{
			bFillSuccess = true;
			BatchBasicAckItem Item;
			Item.nStockMarket = reqItem.nStockMarket;
			Item.strStockCode = reqItem.strStockCode;
			Item.nHigh = batchprice.dwHigh;
			Item.nOpen = batchprice.dwOpen;
			Item.nLastClose = batchprice.dwLastClose;
			Item.nLow = batchprice.dwLow;
			Item.nCur = batchprice.dwCur;

			//统一返回， 1表求停牌， 0表示正常
			Item.nSuspension = (STOCK_STATUS_SUSPENSION == batchprice.nSuspension) ? 1 : 0;

			Item.nVolume = batchprice.ddwVolume;
			Item.nValue = batchprice.ddwTurnover;
			Item.nAmpli = batchprice.dwAmpli;
			Item.nTurnoverRate = batchprice.nTurnoverRate;
			wchar_t szListDate[64] = {};
			m_pQuoteData->TimeStampToStrDate(nStockID, batchprice.dwListTime, szListDate);
			Item.strListTime = szListDate;
			wchar_t szDate[64] = {};
			wchar_t szTime[64] = {};
			m_pQuoteData->TimeStampToStrDate(nStockID, batchprice.dwTime, szDate);
			m_pQuoteData->TimeStampToStrTime(nStockID, batchprice.dwTime, szTime);
			Item.strDate = szDate;
			Item.strTime = szTime;

			ackBody.vtAckBatchBasic.push_back(Item);
		}
		else
		{
			bFillSuccess = false;
			break;
		}
	}
	return bFillSuccess;
}

void CPluginBatchBasic::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
{
	CHECK_RET(pReq && m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReq->req.head;
	ack.head.ddwErrCode = 0;
	ack.body = data;

	//tomodify 5
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

void CPluginBatchBasic::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
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

void CPluginBatchBasic::SetTimerHandleTimeout(bool bStartOrStop)
{
	if (m_bStartTimerHandleTimeout)
	{
		if (!bStartOrStop)
		{
			m_TimerWnd.StopTimer(TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = FALSE;
		}
	}
	else
	{
		if (bStartOrStop)
		{
			m_TimerWnd.StartMillionTimer(1000, TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = TRUE;
		}
	}
}

void CPluginBatchBasic::ReleaseAllReqData()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for ( ; it != m_vtReqData.end(); ++it )
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}

void CPluginBatchBasic::DoClearReqInfo(SOCKET socket)
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
