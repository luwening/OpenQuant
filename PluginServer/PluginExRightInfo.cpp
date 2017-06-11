#include "stdafx.h"
#include "PluginExRightInfo.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoExRightInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_EXRIGHTINFO

typedef CProtoExRightInfo		CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginExRightInfo::CPluginExRightInfo()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
}

CPluginExRightInfo::~CPluginExRightInfo()
{
	Uninit();
}

void CPluginExRightInfo::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginExRightInfo::Uninit()
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

void CPluginExRightInfo::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
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
	
	int nStockNum = (int)req.body.vtReqExRightInfo.size();
	std::vector<INT64> vtReqStockID;
	vtReqStockID.reserve(nStockNum);
	for (int n = 0; n < nStockNum; n++ )
	{
		INT64 nStockID = IFTStockUtil::GetStockHashVal(req.body.vtReqExRightInfo[n].strStockCode.c_str(), 
				(StockMktType)req.body.vtReqExRightInfo[n].nStockMarket);
		if ( nStockID == 0 )
		{
			CHECK_OP(false, NOOP);
			StockDataReq req_info;			
			req_info.sock = sock;
			req_info.req = req;
			req_info.dwReqTick = ::GetTickCount();
			ReplyDataReqError(&req_info, PROTO_ERR_STOCK_NOT_FIND, L"找不到股票！");
			return;
		}
		vtReqStockID.push_back(nStockID);
		StockMktCode &mc = m_mapStockIDCode[nStockID];
		mc.strCode = req.body.vtReqExRightInfo[n].strStockCode;
		mc.nMarketType = req.body.vtReqExRightInfo[n].nStockMarket;
	}

	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->vtStockID = vtReqStockID;
	pReqInfo->dwReqTick = ::GetTickCount();
	m_vtReqData.push_back(pReqInfo);

	m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);
}

void CPluginExRightInfo::NotifyQuoteDataUpdate(int nCmdID, INT64 nStockID)
{
	CHECK_OP(false, NOOP);
	//CHECK_RET(nCmdID == PROTO_ID_QUOTE && nStockID, NORET);
	//CHECK_RET(m_pQuoteData, NORET);
}

void CPluginExRightInfo::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginExRightInfo::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllRequest();
	}	
}

void CPluginExRightInfo::ReplyAllRequest()
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
		
		PluginExRightItem* parItem = NULL;
		int nItemNum = 0;
		bool bRet = m_pQuoteData->GetExRightInfo(_vect2Ptr(pReqData->vtStockID), _vectIntSize(pReqData->vtStockID), parItem, nItemNum);
		if ( !bRet )
		{
			ReplyDataReqError(pReqData, PROTO_ERR_UNKNOWN_ERROR, L"未知错误！");
			break;
		}
	
		CProtoQuote::ProtoAckBodyType ackBody;
		const INT64 c_priceMultiply= 100000L; ///需要修改？
		for ( int  n = 0; n < nItemNum; n++ )
		{	
			ExRightInfoAckItem AckItem;
			StockMktCode mc = m_mapStockIDCode[parItem[n].stock_id];
			AckItem.strStockCode = mc.strCode;
			AckItem.nStockMarket = mc.nMarketType;
			char buf[16] = {0};
			sprintf_s(buf, "%d-%02d-%02d", parItem[n].ex_date / 10000, parItem[n].ex_date / 100 % 100, parItem[n].ex_date % 100);
			AckItem.str_ex_date = buf;
			
			if ( parItem[n].split_base != 0 )
			{
				AckItem.split_ratio = INT32((parItem[n].split_ert / parItem[n].split_base) * c_priceMultiply);//拆合股比例
			}
			else
			{
				AckItem.split_ratio = 0;
			}
			
			if ( parItem[n].dividend_base != 0)
			{
				AckItem.per_cash_div = INT64((parItem[n].dividend_amount / parItem[n].dividend_base) * c_priceMultiply);//现金分红
			}
			else
			{
				AckItem.per_cash_div = 0;
			}

			if ( parItem[n].bonus_stk_base != 0 )
			{
				AckItem.per_share_ratio = INT32((parItem[n].bonus_stk_ert / parItem[n].bonus_stk_base) * c_priceMultiply);//送股比例
			}
			else
			{
				AckItem.per_share_ratio = 0;
			}
			
			if ( parItem[n].into_shr_base != 0)
			{
				AckItem.per_share_trans_ratio = INT32((parItem[n].into_ert / parItem[n].into_shr_base) * c_priceMultiply);//转赠股比例
			}
			else
			{
				AckItem.per_share_trans_ratio = 0;
			}
			
			if ( parItem[n].allot_base != 0 )
			{
				AckItem.allotment_ratio = INT32((parItem[n].allot_ert / parItem[n].allot_base) * c_priceMultiply);//配股比例
			}
			else
			{
				AckItem.allotment_ratio = 0;
			}
			
			if ( parItem[n].stk_add_base != 0 )
			{
				AckItem.stk_spo_ratio = INT32((parItem[n].stk_add_ert / parItem[n].stk_add_base) * c_priceMultiply);//增发比例
			}
			else
			{
				AckItem.stk_spo_ratio = 0;
			}

			AckItem.allotment_price = INT64(parItem[n].allot_price * c_priceMultiply);//配股价格
			AckItem.stk_spo_price = INT64(parItem[n].stk_add_price * c_priceMultiply);//增发价格
			AckItem.fwd_factor_a = INT64(parItem[n].fwd_factor_a * c_priceMultiply);
			AckItem.fwd_factor_b = INT64(parItem[n].fwd_factor_b * c_priceMultiply);
			AckItem.bwd_factor_a = INT64(parItem[n].bwd_factor_a * c_priceMultiply);
			AckItem.bwd_factor_b = INT64(parItem[n].bwd_factor_b * c_priceMultiply);
			ackBody.vtAckExRightInfo.push_back(AckItem);
		}	
		
		ReplyStockDataReq(pReqData, ackBody);
	}
}

void CPluginExRightInfo::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
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

void CPluginExRightInfo::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
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

void CPluginExRightInfo::ReleaseAllReqData()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for ( ; it != m_vtReqData.end(); ++it )
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}

int  CPluginExRightInfo::GetMarketTimezone(StockMktType eMkt)
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

void CPluginExRightInfo::FormatTimestampToDate(int nTimestamp, int nTimezone, std::string &strFmtTime)
{
	time_t nTimezoneTimestamp = nTimestamp + nTimezone * 3600;		
	struct tm *stTime = gmtime(&nTimezoneTimestamp);
	char szBuf[32];
	sprintf_s(szBuf, "%d-%02d-%02d", stTime->tm_year + 1900, stTime->tm_mon + 1, stTime->tm_mday);
	strFmtTime = szBuf;
}

void CPluginExRightInfo::DoClearReqInfo(SOCKET socket)
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
