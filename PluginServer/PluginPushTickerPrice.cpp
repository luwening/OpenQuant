#include "stdafx.h"
#include "PluginPushTickerPrice.h"
#include "PluginQuoteServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_CLEAR_CACHE		354
#define TIMER_ID_HANDLE_TIMEOUT_REQ	355

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_PUSH_TICKER
#define QUOTE_SERVER_TYPE	QuoteServer_PushTickerPrice


//////////////////////////////////////////////////////////////////////////

CPluginPushTickerPrice::CPluginPushTickerPrice()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
}

CPluginPushTickerPrice::~CPluginPushTickerPrice()
{
	Uninit();
}

void CPluginPushTickerPrice::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginPushTickerPrice::Uninit()
{
	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;
	}
}

void CPluginPushTickerPrice::PushStockData(INT64 ddwStockHash, SOCKET sock)
{
	INT64 nLastSequnce = GetLastPushTicker(ddwStockHash, sock);
	std::vector<PluginTickItem> vtTickPrice;
	vtTickPrice.resize(200);
	int nTickFillCount = m_pQuoteData->FillTickArr(ddwStockHash, _vect2Ptr(vtTickPrice), _vectIntSize(vtTickPrice), nLastSequnce);
	if ( nTickFillCount > 0)
	{
		QuoteAckDataBody ackbody;
		ackbody.nNextSequence = -1;

		CProtoQuote::ProtoAckDataType ack;
		ack.head.nProtoID = PROTO_ID_PUSH_TICKER;
		ack.head.ddwErrCode = 0;
		ack.body = ackbody;

		StockMktType eMkt = StockMkt_HK;
		wchar_t szStockCode[16] = {}, szStockName[128] = { 0 };
		m_pQuoteData->GetStockInfoByHashVal(ddwStockHash, eMkt, szStockCode, szStockName);
		ack.body.nStockMarket = (int)eMkt;
		CA::Unicode2UTF(szStockCode, ack.body.strStockCode);

		int nValidNum = min(nTickFillCount, 200);
		INT64 nMaxSequence = nLastSequnce;
		for ( int n = 0; n < nValidNum; n++ )
		{
			PushTickerAckItem tickItem;
			PluginTickItem &srcItem = vtTickPrice[n];					
			tickItem.nPrice = (int)srcItem.dwPrice;
			tickItem.nDealType = srcItem.nDealType;
			tickItem.nSequence = srcItem.nSequence;
			if (tickItem.nSequence >= nMaxSequence)
			{
				nMaxSequence = tickItem.nSequence;
			}
			tickItem.nVolume = srcItem.nVolume;
			tickItem.nTurnover = srcItem.nTurnover;
			tickItem.strTickTime = UtilPlugin::FormatMktTimestamp((int)srcItem.dwTime, (StockMktType)ack.body.nStockMarket, FormatTime_YMDHMS);

			ack.body.vtTicker.push_back(tickItem);
		}
		SetLastPushTicker(ddwStockHash, sock, nMaxSequence);
		if (ack.body.vtTicker.size() > 0)
		{
			CProtoQuote proto;
			proto.SetProtoData_Ack(&ack);

			Json::Value jsnAck;
			if (proto.MakeJson_Ack(jsnAck))
			{
				std::string strOut;
				CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
				m_pQuoteServer->ReplyQuoteReq(ack.head.nProtoID, strOut.c_str(), (int)strOut.size(), sock);
			}
			else
			{
				CHECK_OP(false, NOOP);
			}
		}
	}
}

void CPluginPushTickerPrice::NotifySocketClosed(SOCKET sock)
{
	auto itmap = m_mapPushInfo.find(sock);
	if (itmap != m_mapPushInfo.end())
	{
		m_mapPushInfo.erase(itmap);
	}
}

//市场切换了交易日，记录的推送信息要清掉 
void CPluginPushTickerPrice::NotifyMarketNewTrade(StockMktType eMkt)
{
	CHECK_RET(m_pQuoteData, NORET);

	std::map<SOCKET, std::vector<Stock_PushInfo>>::iterator itmap = m_mapPushInfo.begin();
	for (; itmap != m_mapPushInfo.end(); itmap++)
	{
		std::vector<Stock_PushInfo>& vtStock = itmap->second;
		std::vector<Stock_PushInfo>::iterator itStock = vtStock.begin();
		while (itStock != vtStock.end())
		{
			StockMktType eStockMkt = StockMkt_None;
			wchar_t szStockCode[16] = {}, szStockName[128] = { 0 };
			m_pQuoteData->GetStockInfoByHashVal(itStock->ddwStockID, eStockMkt, szStockCode, szStockName);
			if (eMkt == eStockMkt)
			{
				itStock = vtStock.erase(itStock);
			}
			else
			{
				++itStock;
			}
		}
	}
}

INT64 CPluginPushTickerPrice::GetLastPushTicker(INT64 ddwStockHash, SOCKET sock)
{
	std::vector<Stock_PushInfo> &vtInfoSingleSocket = m_mapPushInfo[sock];
	for (std::vector<Stock_PushInfo>::iterator iter_find = vtInfoSingleSocket.begin(); iter_find != vtInfoSingleSocket.end(); iter_find++)
	{
		if (iter_find->ddwStockID == ddwStockHash)
		{
			return iter_find->nSequence;
		}
	}
	Stock_PushInfo NewItem;
	NewItem.ddwStockID = ddwStockHash;
	vtInfoSingleSocket.push_back(NewItem);
	return NewItem.nSequence;
}

bool CPluginPushTickerPrice::SetLastPushTicker(INT64 ddwStockHash, SOCKET sock, INT64 nSequence)
{
	std::vector<Stock_PushInfo> &vtInfoSingleSocket = m_mapPushInfo[sock];
	for (std::vector<Stock_PushInfo>::iterator iter_find = vtInfoSingleSocket.begin(); iter_find != vtInfoSingleSocket.end(); iter_find++)
	{
		if (iter_find->ddwStockID == ddwStockHash)
		{
			iter_find->nSequence = nSequence;
			return true;
		}
	}
	return false;
}
