#include "stdafx.h"
#include "PluginPushBatchBasic.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoPushBatchBasic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_CLEAR_CACHE		354
#define TIMER_ID_HANDLE_TIMEOUT_REQ	355

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_PUSH_BATCHPRICE
#define QUOTE_SERVER_TYPE	QuoteServer_PushBatchBasic
typedef CProtoPushBatchBasic	CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginPushBatchBasic::CPluginPushBatchBasic()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
}

CPluginPushBatchBasic::~CPluginPushBatchBasic()
{
	Uninit();
}

void CPluginPushBatchBasic::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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
}

void CPluginPushBatchBasic::Uninit()
{
	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;
	}
}

void CPluginPushBatchBasic::PushStockData(INT64 nStockID, SOCKET sock)
{
	CHECK_RET(m_pQuoteData && nStockID, NORET);

	QuoteAckDataBody ackbody;
	Quote_BatchBasic batchprice;
	if (m_pQuoteData->FillBatchBasic(nStockID, &batchprice))
	{
		PushBatchBasicAckItem Item;
		Item.nHigh = batchprice.dwHigh;
		Item.nOpen = batchprice.dwLastClose;
		Item.nLastClose = batchprice.dwLastClose;
		Item.nLow = batchprice.dwLow;
		Item.nCur = batchprice.dwCur;
		Item.nSuspension = batchprice.nSuspension;
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
		StockMktType eMkt = StockMkt_HK;
		wchar_t szStockCode[16] = {0}, szStockName[128] = {0};
		m_pQuoteData->GetStockInfoByHashVal(nStockID, eMkt, szStockCode, szStockName);
		Item.nStockMarket = (int)eMkt;
		CA::Unicode2UTF(szStockCode, Item.strStockCode);
		ackbody.vtAckBatchBasic.push_back(Item);

		CProtoQuote::ProtoAckDataType ack;
		ack.head.nProtoID = PROTO_ID_PUSH_BATCHPRICE;
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
			m_pQuoteServer->ReplyQuoteReq(PROTO_ID_PUSH_BATCHPRICE, strOut.c_str(), (int)strOut.size(), sock);
		}
		else
		{
			CHECK_OP(false, NOOP);
		}
	}
}