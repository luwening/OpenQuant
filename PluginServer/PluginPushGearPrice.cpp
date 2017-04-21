#include "stdafx.h"
#include "PluginPushGearPrice.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoPushGearPrice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_CLEAR_CACHE		354
#define TIMER_ID_HANDLE_TIMEOUT_REQ	355

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_PUSH_GEARPRICE
#define QUOTE_SERVER_TYPE	QuoteServer_GearPrice
typedef CProtoPushGearPrice		CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginPushGearPrice::CPluginPushGearPrice()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
}

CPluginPushGearPrice::~CPluginPushGearPrice()
{
	Uninit();
}

void CPluginPushGearPrice::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginPushGearPrice::Uninit()
{
	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;
	}
}

void CPluginPushGearPrice::PushStockData(INT64 nStockID, SOCKET sock)
{
	CHECK_RET(m_pQuoteData, NORET);
	Quote_OrderItem price[10];
	if (m_pQuoteData->FillOrderQueue(nStockID, price, _countof(price)))
	{
		QuoteAckDataBody ackbody;
		ackbody.vtGear.clear();
		for ( int n = 0; n < _countof(price); n++ )
		{
			PushGearPriceAckItem item;
			item.nBuyOrder = price[n].nBuyOrders;
			item.nBuyPrice = price[n].dwBuyPrice;
			item.nBuyVolume = price[n].ddwBuyVol;

			item.nSellOrder = price[n].nSellOrders;
			item.nSellPrice = price[n].dwSellPrice;
			item.nSellVolume = price[n].ddwSellVol;
			ackbody.vtGear.push_back(item);
		}

		CProtoQuote::ProtoAckDataType ack;
		ack.head.nProtoID = PROTO_ID_PUSH_GEARPRICE;
		ack.head.ddwErrCode = 0;
		ack.head.nProtoVer = 1;
		ack.body = ackbody;

		StockMktType eMkt = StockMkt_HK;
		wchar_t szStockCode[16] = {0}, szStockName[128] = {0};
		m_pQuoteData->GetStockInfoByHashVal(nStockID, eMkt, szStockCode, szStockName);
		ack.body.nStockMarket = (int)eMkt;
		CA::Unicode2UTF(szStockCode, ack.body.strStockCode);

		CProtoQuote proto;	
		proto.SetProtoData_Ack(&ack);

		Json::Value jsnAck;
		if ( proto.MakeJson_Ack(jsnAck) )
		{
			std::string strOut;
			CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
			m_pQuoteServer->ReplyQuoteReq(PROTO_ID_PUSH_GEARPRICE, strOut.c_str(), (int)strOut.size(), sock);
		}
		else
		{
			CHECK_OP(false, NOOP);
		}

	}
}
