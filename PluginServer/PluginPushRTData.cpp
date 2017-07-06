#include "stdafx.h"
#include "PluginPushRTData.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoPushRTData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_CLEAR_CACHE		354
#define TIMER_ID_HANDLE_TIMEOUT_REQ	355

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_PUSH_RTDATA
#define QUOTE_SERVER_TYPE	QuoteServer_PushRTData
typedef CProtoPushRTData		CProtoQuote;


#define  MsgEvent_Push_Triger_RTKLData_Error  400
#define  Min_Tick_Interval_Req_RTKLData 5000  //5秒 


//////////////////////////////////////////////////////////////////////////

CPluginPushRTData::CPluginPushRTData()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;

	m_dwTickLastTrigerKLError = 0;
}

CPluginPushRTData::~CPluginPushRTData()
{
	Uninit();
}

void CPluginPushRTData::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

	m_MsgHandler.Create();
	m_MsgHandler.SetEventInterface(this);
}

void CPluginPushRTData::Uninit()
{
	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;
	}
}

void CPluginPushRTData::OnMsgEvent(int nEvent, WPARAM wParam, LPARAM lParam)
{
	if (nEvent == MsgEvent_Push_Triger_RTKLData_Error)
	{
		OnPushTrigerRTKLDataError();
	}
}

void CPluginPushRTData::OnPushTrigerRTKLDataError()
{
	CHECK_RET(m_pQuoteServer, NORET);

	if (m_setPushTrigerRTKLError.size() == 0)
	{
		return;
	}
	DWORD dwTick = GetTickCount();
	if (0 == m_dwTickLastTrigerKLError)
	{
		m_dwTickLastTrigerKLError = dwTick;
		return;
	}
	if (dwTick - m_dwTickLastTrigerKLError < Min_Tick_Interval_Req_RTKLData)
	{
		return;
	}

	std::set<std::pair<INT64, int>> vtStock = m_setPushTrigerRTKLError;
	m_setPushTrigerRTKLError.clear();
	m_dwTickLastTrigerKLError = dwTick;

	std::set<std::pair<INT64, int>>::iterator it = vtStock.begin();
	for (; it != vtStock.end(); it++)
	{
		INT64 nStockID = it->first;

		StockMktCodeEx stMktCode;
		if (IFTStockUtil::GetStockMktCode(nStockID, stMktCode))
		{
			std::string strCode = stMktCode.strCode;
			DWORD dwCookie = 0;
			m_pQuoteServer->QueryStockRTData(&dwCookie, strCode, stMktCode.eMarketType, QuoteServer_RTData);
		}
	}

}

DWORD CPluginPushRTData::GetLastPushRT(INT64 ddwStockHash, SOCKET sock)
{
	std::vector<Stock_PushInfo> &vtInfoSingleSocket = m_mapPushInfo[sock];
	for (std::vector<Stock_PushInfo>::iterator iter_find = vtInfoSingleSocket.begin(); iter_find != vtInfoSingleSocket.end(); iter_find++)
	{
		if (iter_find->ddwStockID == ddwStockHash)
		{
			return iter_find->dwRTTime;
		}
	}
	Stock_PushInfo NewItem;
	NewItem.ddwStockID = ddwStockHash;
	vtInfoSingleSocket.push_back(NewItem);
	return NewItem.dwRTTime;
}

bool CPluginPushRTData::SetLastPushRT(INT64 ddwStockHash, SOCKET sock, DWORD dwTime)
{
	std::vector<Stock_PushInfo> &vtInfoSingleSocket = m_mapPushInfo[sock];
	for (std::vector<Stock_PushInfo>::iterator iter_find = vtInfoSingleSocket.begin(); iter_find != vtInfoSingleSocket.end(); iter_find++)
	{
		if (iter_find->ddwStockID == ddwStockHash)
		{
			iter_find->dwRTTime = dwTime;
			return true;
		}
	}
	return false;
}

void CPluginPushRTData::PushStockData(INT64 nStockID, SOCKET sock)
{
	if (!m_pQuoteData->IsRTDataExist(nStockID))
	{
		//容错处理
		m_setPushTrigerRTKLError.insert(std::make_pair(nStockID, 0));
		m_MsgHandler.RaiseEvent(MsgEvent_Push_Triger_RTKLData_Error, 0, 0);
	}
	else
	{
		Quote_StockRTData* pQuoteRT = NULL;
		int nCount = 0;
		if ( m_pQuoteData->FillRTData(nStockID, pQuoteRT, nCount) )
		{
			QuoteAckDataBody ackbody;
			ackbody.vtRTData.clear();
			int nNum = 0;
			DWORD dwLastPushTime = GetLastPushRT(nStockID, sock);
			DWORD dwMaxPushTime = dwLastPushTime;
			for ( int n = 0; n < nCount; n++ )
			{
				PushRTDataAckItem item;
				item.nDataStatus = pQuoteRT[n].nDataStatus;
				if (!IS_RTKL_VALID_PUSH_DATA_STATUS(pQuoteRT[n].nDataStatus))
				{
					break;
				}
				if (pQuoteRT[n].dwTime < dwLastPushTime)
				{
					continue;
				}
				if (pQuoteRT[n].dwTime > dwMaxPushTime)
				{
					dwMaxPushTime = pQuoteRT[n].dwTime;
				}
				wchar_t szTime[64] = {}; 
				m_pQuoteData->TimeStampToStr(nStockID, pQuoteRT[n].dwTime,szTime);
				item.strTime = szTime;
				item.dwOpenedMins = pQuoteRT[n].dwOpenedMins;
				item.nCurPrice = pQuoteRT[n].nCurPrice;
				item.nLastClosePrice = pQuoteRT[n].nLastClosePrice;
				item.nAvgPrice = pQuoteRT[n].nAvgPrice;
				item.ddwTDVolume= pQuoteRT[n].ddwTDVolume;
				item.ddwTDValue = pQuoteRT[n].ddwTDValue;
				ackbody.vtRTData.push_back(item);
				nNum++;
			}
			SetLastPushRT(nStockID, sock, dwMaxPushTime);
			ackbody.nNum = nNum;
			if (ackbody.vtRTData.size() > 5)
			{
				std::reverse(ackbody.vtRTData.begin(), ackbody.vtRTData.end());
				ackbody.vtRTData.resize(1);
				ackbody.nNum = 1;
			}
			CProtoQuote::ProtoAckDataType ack;
			ack.head.nProtoID = PROTO_ID_PUSH_RTDATA;
			ack.head.ddwErrCode = 0;
			ack.body = ackbody;

			StockMktType eMkt = StockMkt_HK;
			wchar_t szStockCode[16] = {}, szStockName[128] = { 0 };
			m_pQuoteData->GetStockInfoByHashVal(nStockID, eMkt, szStockCode, szStockName);
			ack.body.nStockMarket = (int)eMkt;
			CA::Unicode2UTF(szStockCode, ack.body.strStockCode);

			if (ack.body.vtRTData.size() > 0)
			{
				CProtoQuote proto;
				proto.SetProtoData_Ack(&ack);

				Json::Value jsnAck;
				if (proto.MakeJson_Ack(jsnAck))
				{
					std::string strOut;
					CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
					m_pQuoteServer->ReplyQuoteReq(PROTO_ID_PUSH_RTDATA, strOut.c_str(), (int)strOut.size(), sock);
				}
				else
				{
					CHECK_OP(false, NOOP);
				}
			}
		}
		m_pQuoteData->DeleteRTDataPointer(pQuoteRT);
	}
}

void CPluginPushRTData::NotifySocketClosed(SOCKET sock)
{
	auto itmap = m_mapPushInfo.find(sock);
	if (itmap != m_mapPushInfo.end())
	{
		m_mapPushInfo.erase(itmap);
	}
}

//市场切换了交易日，记录的推送信息要清掉 
void CPluginPushRTData::NotifyMarketNewTrade(StockMktType eMkt)
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
