#include "stdafx.h"
#include "PluginPlateSubIDs.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoPlateSubIDs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_HANDLE_TIMEOUT_REQ	355
#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_PLATESUBIDS

typedef CProtoPlateSubIDs		CProtoQuote;

//////////////////////////////////////////////////////////////////////////

CPluginPlateSubIDs::CPluginPlateSubIDs()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
	m_bStartTimerHandleTimeout = 0;
}

CPluginPlateSubIDs::~CPluginPlateSubIDs()
{
	Uninit();
}

void CPluginPlateSubIDs::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginPlateSubIDs::Uninit()
{
	ReleaseAllReqData();

	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;		

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);
	}
}

void CPluginPlateSubIDs::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QUOTE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);

	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType  req;	
	proto.SetProtoData_Req(&req);
	if ( !proto.ParseJson_Req(jsnVal) )
	{
		CHECK_OP(false, NORET);
		return;
	}
	CHECK_RET(req.head.nProtoID == nCmdID, NORET);
	
	INT64 nPlateID = IFTStockUtil::GetStockHashVal(req.body.strStockCode.c_str(), (StockMktType)req.body.nStockMarket);
	if (0 == nPlateID)
	{
		CHECK_OP(false, NOOP);
		StockDataReq req_info;
		req_info.nStockID = nPlateID;
		req_info.sock = sock;
		req_info.req = req;
		req_info.dwReqTick = ::GetTickCount();
		ReplyDataReqError(&req_info, PROTO_ERR_STOCK_NOT_FIND, L"找不到板块！");
		return;
	}	
	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->nStockID = nPlateID;
	pReqInfo->dwReqTick = ::GetTickCount();
	pReqInfo->nReqCookie = 0;
	m_vtReqData.push_back(pReqInfo);

	int nItemCount = 0;
	if (!m_pQuoteData->GetPlateStockIDList(nPlateID, NULL, nItemCount))
	{
		//需要请求
		QueryDataErrCode  eCode = m_pQuoteServer->QueryPlateSubIDList(&pReqInfo->nReqCookie, nPlateID);
		if (eCode != QueryData_Suc)
		{
			m_vtReqData.erase(--m_vtReqData.end());
			ReplyDataReqError(pReqInfo, PROTO_ERR_UNKNOWN_ERROR, L"拉取数据失败！");
			SAFE_DELETE(pReqInfo);
		}
		else
		{
			//wait callback
		}
	}
	else
	{
		m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);
	}
	SetTimerHandleTimeout(true);
}

void CPluginPlateSubIDs::NotifyQueryPlateSubIDs(INT nCSResult, DWORD dwCookie)
{
	if (0 != nCSResult)
	{
		VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
		StockDataReq* pReqItem = NULL;
		while (it != m_vtReqData.end())
		{
			if (*it && (*it)->nReqCookie == dwCookie)
			{
				pReqItem = *it;
				m_vtReqData.erase(it);
				break;
			}
			++it;
		}
		if (!pReqItem)
		{
			return;
		}

		ReplyDataReqError(pReqItem, PROTO_ERR_UNKNOWN_ERROR, L"拉取数据失败");
		SAFE_DELETE(pReqItem);
	}
	else
	{
		ReplyAllReadyReq();
	}
}

void CPluginPlateSubIDs::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginPlateSubIDs::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllReadyReq();
	}	
}

void CPluginPlateSubIDs::OnTimeEvent(UINT nEventID)
{
	if (TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID)
	{
		HandleTimeoutReq();
	}
}

void CPluginPlateSubIDs::HandleTimeoutReq()
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
			CStringA strTimeout;
			strTimeout.Format("req timeout, cookie=%d", pReq->nReqCookie);
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

void CPluginPlateSubIDs::ReplyAllReadyReq()
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

		int nItemNum = 0;
		INT64* parID = NULL;
	
		//找到数据已经准备好的请求项
		if (!m_pQuoteData->GetPlateStockIDList(pReqData->nStockID, parID, nItemNum))
		{
			//不存在的数据应该是在请求中, 
			CHECK_OP(pReqData->nReqCookie, NOOP);
			++it;
			continue;
		}

		if (nItemNum > 0)
		{
			parID = new INT64[nItemNum];
			m_pQuoteData->GetPlateStockIDList(pReqData->nStockID, parID, nItemNum);
		}

		CProtoQuote::ProtoAckBodyType ackBody;
		ackBody.nStockMarket = reqBody.nStockMarket;
		ackBody.strStockCode = reqBody.strStockCode;
	 
		ackBody.vtPlateSubIDs.clear();
		ackBody.vtPlateSubIDs.reserve(nItemNum);
		
		for (int n = 0; n < nItemNum; n++)
		{
			wchar_t szStockCode[16] = { 0 }, szStockName[128] = { 0 };
			PlateSubIDsAckItem AckItem;
			StockMktType eMkt = StockMkt_None;
			INT64 nOwnerID = 0;

			if (!m_pQuoteData->GetStockInfoByHashVal(parID[n], eMkt, szStockCode, szStockName,
				&AckItem.nLotSize, &AckItem.nSecurityType, &AckItem.nSubType, &nOwnerID))
			{
				CHECK_OP(false, NOOP);
				continue;
			}
			AckItem.nStockMarket = (int)eMkt;
			//code
			std::string strTmp; 
			CA::Unicode2UTF(szStockCode, strTmp);
			AckItem.strStockCode = strTmp;
			//name
			CA::Unicode2UTF(szStockName, strTmp);
			AckItem.strSimpName = strTmp;

			AckItem.nOwnerMarketType = 0;
			if (nOwnerID != 0)
			{
				StockMktType eOwnerMkt = StockMkt_None;
				szStockCode[0] = 0; szStockName[0] = 0;
				m_pQuoteData->GetStockInfoByHashVal(nOwnerID, eOwnerMkt, szStockCode, szStockName);
				AckItem.nOwnerMarketType = (int)eOwnerMkt;
				CA::Unicode2UTF(szStockCode, strTmp);
				AckItem.strOwnerStockCode = strTmp;
			}
			ackBody.vtPlateSubIDs.push_back(AckItem);
		}	
		SAFE_DELETE_ARR(parID);

		ReplyStockDataReq(pReqData, ackBody);

		it = vtReqData.erase(it);
		SAFE_DELETE(pReqData);

	}
}

void CPluginPlateSubIDs::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
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

void CPluginPlateSubIDs::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
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

void CPluginPlateSubIDs::SetTimerHandleTimeout(bool bStartOrStop)
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

void CPluginPlateSubIDs::ReleaseAllReqData()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for ( ; it != m_vtReqData.end(); ++it )
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}

void CPluginPlateSubIDs::DoClearReqInfo(SOCKET socket)
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
