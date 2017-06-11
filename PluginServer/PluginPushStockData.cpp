#include "stdafx.h"
#include "PluginPushStockData.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoPushStockData.h"
#include "include/FTPluginQuoteDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_PushStockData

typedef CProtoPushStockData		CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginPushStockData::CPluginPushStockData()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
}

CPluginPushStockData::~CPluginPushStockData()
{
	Uninit();
}

void CPluginPushStockData::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginPushStockData::Uninit()
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

void CPluginPushStockData::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
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
	
	if (req.body.nStockPushType <= 0 || req.body.nStockPushType >= StockSubType_Max)
	{
		CHECK_OP(false, NOOP);
		StockDataReq req_info;
		req_info.nStockID = nStockID;
		req_info.sock = sock;
		req_info.req = req;
		ReplyDataReqError(&req_info, PROTO_ERR_PARAM_ERR, L"股票推送类型错误！");
		return;
	}

	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->dwReqTick = ::GetTickCount();
	pReqInfo->nStockID = nStockID;
	m_vtReqData.push_back(pReqInfo);

	m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);
}

void CPluginPushStockData::NotifyQuoteDataUpdate(int nCmdID, INT64 nStockID)
{
	CHECK_OP(false, NOOP);
	//CHECK_RET(nCmdID == PROTO_ID_QUOTE && nStockID, NORET);
	//CHECK_RET(m_pQuoteData, NORET);
}

void CPluginPushStockData::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginPushStockData::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllRequest();
	}	
}

void CPluginPushStockData::ReplyAllRequest()
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
		bool bUnpush = pReqData->req.body.nUnPush == 0 ? false : true;
		m_pQuoteData->RecordPushRequest(pReqData->sock, pReqData->nStockID, (StockSubType)pReqData->req.body.nStockPushType, bUnpush);

		CProtoQuote::ProtoAckBodyType ackBody;
		ackBody.nStockMarket = pReqData->req.body.nStockMarket;
		ackBody.nStockPushType = pReqData->req.body.nStockPushType;
		ackBody.strStockCode = pReqData->req.body.strStockCode;

		ReplyStockDataReq(pReqData, ackBody);
	}
}

void CPluginPushStockData::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
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

void CPluginPushStockData::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
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

void CPluginPushStockData::ReleaseAllReqData()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for ( ; it != m_vtReqData.end(); ++it )
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}

void CPluginPushStockData::DoClearReqInfo(SOCKET socket)
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
