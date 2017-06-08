#include "stdafx.h"
#include "PluginPlatesetIDs.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoPlatesetIDs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_HANDLE_TIMEOUT_REQ	355
#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_PLATESETIDS

typedef CProtoPlatesetIDs		CProtoQuote;

//////////////////////////////////////////////////////////////////////////

CPluginPlatesetIDs::CPluginPlatesetIDs()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
	m_bStartTimerHandleTimeout = 0;
}

CPluginPlatesetIDs::~CPluginPlatesetIDs()
{
	Uninit();
}

void CPluginPlatesetIDs::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginPlatesetIDs::Uninit()
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

void CPluginPlatesetIDs::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
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
	INT64 nPlatesetID = DoCalReqPlatesetID((StockMktType)req.body.nStockMarket, (PlateClass)req.body.nPlateClassType);
	if (0 == nPlatesetID)
	{
		CHECK_OP(false, NOOP);
		StockDataReq req_info;
		req_info.nStockID = nPlatesetID;
		req_info.sock = sock;
		req_info.req = req;
		req_info.dwReqTick = ::GetTickCount();
		ReplyDataReqError(&req_info, PROTO_ERR_STOCK_NOT_FIND, L"找不到对应的板块集合！");
		return;
	}	
	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->nStockID = nPlatesetID;
	pReqInfo->dwReqTick = ::GetTickCount();
	pReqInfo->nReqCookie = 0;
	m_vtReqData.push_back(pReqInfo);

	int nItemCount = 0;
	if (!m_pQuoteData->GetPlatesetIDList(nPlatesetID, NULL, nItemCount))
	{
		//需要请求
		QueryDataErrCode  eCode = m_pQuoteServer->QueryPlatesetSubIDList(&pReqInfo->nReqCookie, nPlatesetID);
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

void CPluginPlatesetIDs::NotifyQueryPlatesetIDs(INT nCSResult, DWORD dwCookie)
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

void CPluginPlatesetIDs::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginPlatesetIDs::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllReadyReq();
	}	
}

void CPluginPlatesetIDs::OnTimeEvent(UINT nEventID)
{
	if (TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID)
	{
		HandleTimeoutReq();
	}
}

void CPluginPlatesetIDs::HandleTimeoutReq()
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

void CPluginPlatesetIDs::ReplyAllReadyReq()
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
		if (!m_pQuoteData->GetPlatesetIDList(pReqData->nStockID, parID, nItemNum))
		{
			//不存在的数据应该是在请求中, 
			CHECK_OP(pReqData->nReqCookie, NOOP);
			++it;
			continue;
		}

		if (nItemNum > 0)
		{
			parID = new INT64[nItemNum];
			m_pQuoteData->GetPlatesetIDList(pReqData->nStockID, parID, nItemNum);
		}

		CProtoQuote::ProtoAckBodyType ackBody;
		ackBody.nStockMarket = reqBody.nStockMarket;
		ackBody.nPlateClassType = reqBody.nPlateClassType;
	 
		ackBody.vtPlatesetIDs.clear();
		ackBody.vtPlatesetIDs.reserve(nItemNum);
		
		for (int n = 0; n < nItemNum; n++)
		{
			wchar_t szStockCode[16] = { 0 }, szStockName[128] = { 0 };
			StockMktType eMkt = StockMkt_None;
			if (!m_pQuoteData->GetStockInfoByHashVal(parID[n], eMkt, szStockCode, szStockName))
			{
				CHECK_OP(false, NOOP);
				continue;
			}
			
			PlatesetIDsAckItem AckItem;
			AckItem.nStockMarket = (int)eMkt;
			AckItem.nStockID = parID[n];

			//code
			std::string strTmp; 
			CA::Unicode2UTF(szStockCode, strTmp);
			AckItem.strStockCode = strTmp;

			//name
			CA::Unicode2UTF(szStockName, strTmp);
			AckItem.strStockName = strTmp;

			ackBody.vtPlatesetIDs.push_back(AckItem);
		}	
		SAFE_DELETE_ARR(parID);

		ReplyStockDataReq(pReqData, ackBody);

		it = vtReqData.erase(it);
		SAFE_DELETE(pReqData);

	}
}

void CPluginPlatesetIDs::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
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

void CPluginPlatesetIDs::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
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

void CPluginPlatesetIDs::SetTimerHandleTimeout(bool bStartOrStop)
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

void CPluginPlatesetIDs::ReleaseAllReqData()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for ( ; it != m_vtReqData.end(); ++it )
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}

INT64 CPluginPlatesetIDs::DoCalReqPlatesetID(StockMktType eMkt, PlateClass eClass)
{
	INT64 nPlatesetID = 0;

	//所有, 行业 , 地域, 概念 (使用内部hardcode stockID)
	INT64 arClass[] = { PlateClass_All, PlateClass_Industry, PlateClass_Region, PlateClass_Concept };
	INT64 arPlateHK[] = { 9700009, 9700000, -1, 9700001 };  // -1区别于0代表的id错误, 后续可能会更新，现在请求只会返回空
	INT64 arPlateUS[] = { 9700309, 9700300, -1, 9700301 };
	INT64 arPlateCN[] = { 9700609, 9700600, 9700602, 9700601 };
	CHECK_RET(_countof(arClass) == _countof(arPlateHK), nPlatesetID);
	CHECK_RET(_countof(arClass) == _countof(arPlateUS), nPlatesetID);
	CHECK_RET(_countof(arClass) == _countof(arPlateCN), nPlatesetID);

	INT64 *parPlate = NULL;
	switch (eMkt)
	{
	case StockMkt_None:
		break;
	case StockMkt_US:
		parPlate = arPlateUS;
		break;
	case StockMkt_SH:
	case StockMkt_SZ:
		parPlate = arPlateCN;
		break;
	case StockMkt_HK:
	case StockMkt_Feature_Old:
	case StockMkt_Feature_New:
		parPlate = arPlateHK;
		break;
	default:
		break;
	}
	if (parPlate)
	{
		for (int i = 0; i < _countof(arClass); i++)
		{
			if (eClass == arClass[i])
			{
				nPlatesetID = parPlate[i];
				break;
			}
		}
	}
	return nPlatesetID;
}

void CPluginPlatesetIDs::DoClearReqInfo(SOCKET socket)
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
