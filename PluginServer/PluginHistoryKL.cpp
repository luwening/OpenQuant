#include "stdafx.h"
#include "PluginHistoryKL.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoHistoryKL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_HISTORYKL

typedef CProtoHistoryKL		CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginHistoryKL::CPluginHistoryKL()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
}

CPluginHistoryKL::~CPluginHistoryKL()
{
	Uninit();
}

void CPluginHistoryKL::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

	m_MsgHandler.SetEventInterface(this);
	m_MsgHandler.Create();
}

void CPluginHistoryKL::Uninit()
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

void CPluginHistoryKL::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
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
	INT64 nStockID = IFTStockUtil::GetStockHashVal(req.body.strStockCode.c_str(), (StockMktType)req.body.nStockMarket);
	if ( nStockID == 0 )
	{
		CHECK_OP(false, NOOP);
		StockDataReq req_info;
		req_info.nStockID = nStockID;
		req_info.sock = sock;
		req_info.req = req;
		req_info.dwReqTick = ::GetTickCount();
		ReplyDataReqError(&req_info, PROTO_ERR_STOCK_NOT_FIND, L"找不到股票！");
		return;
	}	
	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->nStockID = nStockID;
	pReqInfo->dwReqTick = ::GetTickCount();
	m_vtReqData.push_back(pReqInfo);

	m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);
}

void CPluginHistoryKL::NotifyQuoteDataUpdate(int nCmdID, INT64 nStockID)
{
	CHECK_OP(false, NOOP);
	//CHECK_RET(nCmdID == PROTO_ID_QUOTE && nStockID, NORET);
	//CHECK_RET(m_pQuoteData, NORET);
}

void CPluginHistoryKL::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginHistoryKL::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllRequest();
	}	
}

void CPluginHistoryKL::ReplyAllRequest()
{
	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);

	VT_STOCK_DATA_REQ vtReqData;
	vtReqData.swap(m_vtReqData);

	//tomodify 3
	VT_STOCK_DATA_REQ::iterator it = vtReqData.begin();
	for ( ; it != vtReqData.end(); ++it )
	{
		StockDataReq *pReqData = *it;
		CHECK_OP(pReqData, continue);
		CProtoQuote::ProtoReqBodyType &reqBody = pReqData->req.body;

		LPCWSTR pszDateFrom = NULL, pszDateTo = NULL;
		std::wstring strDateFrom, strDateTo;
		if ( !reqBody.strStartDate.empty() )
		{
			CA::UTF2Unicode(reqBody.strStartDate.c_str(), strDateFrom);
			pszDateFrom = strDateFrom.c_str();
		}
		if ( !reqBody.strEndDate.empty() )
		{
			CA::UTF2Unicode(reqBody.strEndDate.c_str(), strDateTo);
			pszDateTo = strDateTo.c_str();
		}

		int nKLNum = 0;
		Quote_StockKLData *arKLData = NULL;
		bool bRet = m_pQuoteData->GetHistoryKLineTimeStr((StockMktType)reqBody.nStockMarket, pReqData->nStockID, reqBody.nKLType, reqBody.nRehabType, pszDateFrom, pszDateTo, arKLData, nKLNum);
		if ( !bRet )
		{
			ReplyDataReqError(pReqData, PROTO_ERR_UNKNOWN_ERROR, L"本地无数据！");
			break;
		}

		if ( arKLData == NULL )
			 nKLNum = 0;

		CProtoQuote::ProtoAckBodyType ackBody;
		ackBody.nStockMarket = reqBody.nStockMarket;
		ackBody.strStockCode = reqBody.strStockCode;
		ackBody.strStartDate = reqBody.strStartDate;
		ackBody.strEndDate = reqBody.strEndDate;
		ackBody.nRehabType = reqBody.nRehabType;
		ackBody.nKLType = reqBody.nKLType;
		
		ackBody.vtHistoryKL.clear();
		ackBody.vtHistoryKL.reserve(nKLNum);
		for ( int  n = 0; n < nKLNum; n++ )
		{	
			HistoryKLAckItem AckItem;
			//AckItem.nDataStatus = arKLData[n].nDataStatus;
			wchar_t szTime[64] = {}; 
			m_pQuoteData->TimeStampToStr(pReqData->nStockID, arKLData[n].dwTime, szTime);
			AckItem.strTime = szTime;
			AckItem.nOpenPrice = arKLData[n].nOpenPrice;
			AckItem.nClosePrice = arKLData[n].nClosePrice;
			AckItem.nHighestPrice = arKLData[n].nHighestPrice;
			AckItem.nLowestPrice = arKLData[n].nLowestPrice;
			AckItem.nPERatio = arKLData[n].nPERatio;
			AckItem.nTurnoverRate = arKLData[n].nTurnoverRate;
			AckItem.ddwTDVol = arKLData[n].ddwTDVol;
			AckItem.ddwTDVal = arKLData[n].ddwTDVal;
			ackBody.vtHistoryKL.push_back(AckItem);
		}	
		//m_pQuoteData->DeleteKLDataPointer(arKLData);
		ReplyStockDataReq(pReqData, ackBody);
	}
}

void CPluginHistoryKL::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
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

void CPluginHistoryKL::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
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

void CPluginHistoryKL::ReleaseAllReqData()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for ( ; it != m_vtReqData.end(); ++it )
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}

int  CPluginHistoryKL::GetMarketTimezone(StockMktType eMkt)
{
	switch (eMkt)
	{
	case StockMkt_US:
		return -5;
		break;
	default:
		return 8;
		break;
	}
}

void CPluginHistoryKL::FormatTimestampToDate(int nTimestamp, int nTimezone, std::string &strFmtTime)
{
	time_t nTimezoneTimestamp = nTimestamp + nTimezone * 3600;		
	struct tm *stTime = gmtime(&nTimezoneTimestamp);
	char szBuf[32];
	sprintf_s(szBuf, "%d-%02d-%02d", stTime->tm_year + 1900, stTime->tm_mon + 1, stTime->tm_mday);
	strFmtTime = szBuf;
}

void CPluginHistoryKL::DoClearReqInfo(SOCKET socket)
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
