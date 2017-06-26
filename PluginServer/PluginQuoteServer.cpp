#include "stdafx.h"
#include "PluginQuoteServer.h"
#include "PluginNetwork.h"
#include "Protocol/ProtoBasicPrice.h"
#include "IFTStockUtil.h"
#include "Protocol/ProtoPushHeartBeat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern GUID PLUGIN_GUID;

#define TimerID_FreshRTKLData  100
#define Max_RTKL_Fresh_Try_Count 3

//////////////////////////////////////////////////////////////////////////

CPluginQuoteServer::CPluginQuoteServer()
{
	m_pPluginCore = NULL;
	m_pQuoteData = NULL;
	m_pQuoteOp = NULL;
	m_pDataReport = NULL;
	m_pNetwork = NULL;
	m_nTimerIDRefreshRTKL = 0;
}

CPluginQuoteServer::~CPluginQuoteServer()
{
	UninitQuoteSvr();
}

void CPluginQuoteServer::InitQuoteSvr(IFTPluginCore* pPluginCore, CPluginNetwork *pNetwork)
{
	if ( m_pPluginCore != NULL )
		return;

	if ( pPluginCore == NULL || pNetwork == NULL )
	{
		ASSERT(false);
		return;
	}

	m_pNetwork = pNetwork;
	m_pPluginCore = pPluginCore;
	pPluginCore->QueryFTInterface(IID_IFTQuoteData, (void**)&m_pQuoteData);
	pPluginCore->QueryFTInterface(IID_IFTQuoteOperation, (void**)&m_pQuoteOp);
	pPluginCore->QueryFTInterface(IID_IFTDataReport, (void**)&m_pDataReport);

	if ( m_pQuoteData == NULL || m_pQuoteOp == NULL  || m_pDataReport == NULL)
	{
		ASSERT(false);
		m_pQuoteData = NULL;
		m_pQuoteOp = NULL;
		m_pDataReport = NULL;
		m_pPluginCore = NULL;
		m_pNetwork = NULL;
		return;
	}
	IFTStockUtil::Init(m_pQuoteData);

	m_BasicPrice.Init(this, m_pQuoteData);
	m_GearPrice.Init(this, m_pQuoteData);
	m_RTData.Init(this, m_pQuoteData);
	m_KLData.Init(this, m_pQuoteData);
	m_StockSub.Init(this, m_pQuoteData);
	m_StockUnSub.Init(this, m_pQuoteData);
	m_QueryStockSub.Init(this, m_pQuoteData);
	m_TradeDate.Init(this, m_pQuoteData);
	m_StockList.Init(this, m_pQuoteData);
	m_BatchBasic.Init(this, m_pQuoteData);
	m_TickerPrice.Init(this, m_pQuoteData);
	m_Snapshot.Init(this, m_pQuoteData);
	m_Snapshot.SetSnapshotOperator(m_pQuoteOp);
	m_ExRightInfo.Init(this, m_pQuoteData);
	m_HistoryKL.Init(this, m_pQuoteData);
	m_PushStockData.Init(this, m_pQuoteData);
	m_PushBatchBasic.Init(this, m_pQuoteData);
	m_PushGearPrice.Init(this, m_pQuoteData);
	m_PushTickerPrice.Init(this, m_pQuoteData);
	m_PushKLData.Init(this, m_pQuoteData);
	m_PushRTData.Init(this, m_pQuoteData);
	m_platesetIDs.Init(this, m_pQuoteData);
	m_plateSubIDs.Init(this, m_pQuoteData);
	m_BrokerQueue.Init(this, m_pQuoteData);
	m_GlobalState.Init(this, m_pQuoteData);

}

void CPluginQuoteServer::UninitQuoteSvr()
{
	if ( m_pPluginCore != NULL )
	{	
		m_BasicPrice.Uninit();
		m_GearPrice.Uninit();
		m_RTData.Uninit();
		m_KLData.Uninit();
		m_StockSub.Uninit();
		m_StockUnSub.Uninit();
		m_QueryStockSub.Uninit();
		m_TradeDate.Uninit();
		m_StockList.Uninit();
		m_BatchBasic.Uninit();
		m_TickerPrice.Uninit();
		m_Snapshot.Uninit();
		m_HistoryKL.Uninit();
		m_ExRightInfo.Uninit();
		m_PushStockData.Uninit();
		m_PushBatchBasic.Uninit();
		m_PushGearPrice.Uninit();
		m_PushTickerPrice.Uninit();
		m_PushKLData.Uninit();
		m_PushRTData.Uninit();
		m_platesetIDs.Uninit();
		m_plateSubIDs.Uninit();
		m_BrokerQueue.Uninit();

		m_pQuoteData = NULL;
		m_pQuoteOp = NULL;
		m_pPluginCore = NULL;
		m_pNetwork = NULL;

		IFTStockUtil::Uninit();
	}
}

void CPluginQuoteServer::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	m_pDataReport->PLSCmdIDReport(PLUGIN_GUID, nCmdID);
	switch (nCmdID)
	{
	case PROTO_ID_QT_GET_BASIC_PRICE:
		m_BasicPrice.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_GEAR_PRICE:
		m_GearPrice.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_RTDATA:
		m_RTData.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_KLDATA:
		m_KLData.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_SUBSTOCK:
		m_StockSub.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_UNSUBSTOCK:
		m_StockUnSub.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_QueryStockSub:
		m_QueryStockSub.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_TRADE_DATE:
		m_TradeDate.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_STOCK_LIST:
		m_StockList.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_BATCHBASIC:
		m_BatchBasic.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_TICKER:
		m_TickerPrice.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_SNAPSHOT:
		m_Snapshot.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_HISTORYKL:
		m_HistoryKL.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_GET_EXRIGHTINFO:
		m_ExRightInfo.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_QT_PushStockData:
		m_PushStockData.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;
	case PROTO_ID_QT_GET_PLATESETIDS:
		m_platesetIDs.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;
	case PROTO_ID_QT_GET_PLATESUBIDS:
		m_plateSubIDs.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;
	case PROTO_ID_QT_GET_BROKER_QUEUE:
		m_BrokerQueue.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;
	case PROTO_ID_QT_GET_GLOBAL_STATE:
		m_GlobalState.SetQuoteReqData(nCmdID, jsnVal, sock);
		break;
	default:
		CHECK_OP(false, NOOP);
		BasicPrice_Ack Ack;
		Ack.head.ddwErrCode = PROTO_ERR_PARAM_ERR;
		Ack.head.nProtoID = nCmdID;
		CA::Unicode2UTF(L"协议号错误！", Ack.head.strErrDesc);
		CProtoBasicPrice proto;	
		proto.SetProtoData_Ack(&Ack);

		Json::Value jsnAck;
		if ( proto.MakeJson_Ack(jsnAck) )
		{
			std::string strOut;
			CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
			m_pNetwork->SendData(sock, strOut.c_str(), (int)strOut.size());
		}
		break;
	}
}

void CPluginQuoteServer::ReplyQuoteReq(int nCmdID, const char *pBuf, int nLen, SOCKET sock)
{
	CHECK_RET(nCmdID && pBuf && nLen && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pNetwork, NORET);
	m_pNetwork->SendData(sock, pBuf, nLen);
}

StockSubErrCode CPluginQuoteServer::SubscribeQuote(const std::string &strCode, StockMktType nMarketType, StockSubType eStockSubType, bool bSubOrUnsub, SOCKET sock)
{
	CHECK_RET(m_pQuoteOp, StockSub_FailUnknown);

	StockSubErrCode err_code = StockSub_Suc;
	std::wstring strUnicode;
	CA::UTF2Unicode(strCode.c_str(), strUnicode);

	switch (eStockSubType)
	{
	case StockSubType_Simple:
		err_code = m_pQuoteOp->Subscribe_PriceBase(PLUGIN_GUID, strUnicode.c_str(), nMarketType, bSubOrUnsub, sock);
		break;

	case StockSubType_Gear:
		err_code = m_pQuoteOp->Subscribe_OrderQueue(PLUGIN_GUID, strUnicode.c_str(), nMarketType, bSubOrUnsub, sock);
		break;

	case StockSubType_RT:
		err_code = m_pQuoteOp->Subscribe_RTData(PLUGIN_GUID, strUnicode.c_str(), nMarketType, bSubOrUnsub, sock);
		break;

	case StockSubType_Ticker:
		err_code = m_pQuoteOp->Subscribe_Ticker(PLUGIN_GUID, strUnicode.c_str(), nMarketType, bSubOrUnsub, sock);
		break;
	case StockSubType_Broker:
		err_code = m_pQuoteOp->Subscribe_BrokerQueue(PLUGIN_GUID, strUnicode.c_str(), nMarketType, bSubOrUnsub, sock);
		break;

	case StockSubType_KL_DAY:
	case StockSubType_KL_MIN1:
	case StockSubType_KL_MIN5:
	case StockSubType_KL_MIN15:
	case StockSubType_KL_MIN30:
	case StockSubType_KL_MIN60:
	case StockSubType_KL_WEEK:
	case StockSubType_KL_MONTH:
		err_code = m_pQuoteOp->Subscribe_KLData(PLUGIN_GUID, strUnicode.c_str(), nMarketType, bSubOrUnsub, eStockSubType, sock);
		break;

	default:
		err_code = StockSub_FailUnknown;
		break;
	}

	return err_code;
}

QueryDataErrCode CPluginQuoteServer::QueryStockRTData(DWORD* pCookie, const std::string &strCode, StockMktType nMarketType, QuoteServerType type)
{
	CHECK_RET(m_pQuoteOp, QueryData_FailUnknown);

	QueryDataErrCode err_code = QueryData_Suc;
	std::wstring strUnicode;
	CA::UTF2Unicode(strCode.c_str(), strUnicode);
	switch (type)
	{
	case QuoteServer_RTData:
		err_code = m_pQuoteOp->QueryStockRTData(PLUGIN_GUID, pCookie, strUnicode.c_str(), nMarketType);
		break;
	
	default:
		err_code = QueryData_FailUnknown;
	}
	
	return err_code;
}

QueryDataErrCode CPluginQuoteServer::QueryStockKLData(DWORD* pCookie, const std::string &strCode, StockMktType nMarketType, QuoteServerType type, int nKLType)
{
	CHECK_RET(m_pQuoteOp, QueryData_FailUnknown);

	QueryDataErrCode err_code = QueryData_Suc;
	std::wstring strUnicode;
	CA::UTF2Unicode(strCode.c_str(), strUnicode);
	switch (type)
	{
	case QuoteServer_KLData:
		err_code = m_pQuoteOp->QueryStockKLData(PLUGIN_GUID, pCookie, strUnicode.c_str(), nMarketType, nKLType);
		break;

	default:
		err_code = QueryData_FailUnknown;
	}

	return err_code;
}

QueryDataErrCode CPluginQuoteServer::QueryPlatesetSubIDList(DWORD* pdwCookie, INT64 nPlatesetID)
{
	CHECK_RET(m_pQuoteOp, QueryData_FailUnknown);

	DWORD dwCookie = 0;
	QueryDataErrCode eRet = m_pQuoteOp->QueryPlatesetSubIDList(PLUGIN_GUID, nPlatesetID, dwCookie);
	
	if (pdwCookie)
	{
		*pdwCookie = dwCookie;
	}
	return eRet;
}

QueryDataErrCode CPluginQuoteServer::QueryPlateSubIDList(DWORD* pdwCookie, INT64 nPlatesetID)
{
	CHECK_RET(m_pQuoteOp, QueryData_FailUnknown);

	DWORD dwCookie = 0;
	QueryDataErrCode eRet = m_pQuoteOp->QueryPlateStockIDList(PLUGIN_GUID, nPlatesetID, dwCookie);

	if (pdwCookie)
	{
		*pdwCookie = dwCookie;
	}
	return eRet;
}

void CPluginQuoteServer::CloseSocket(SOCKET sock)
{
	if (m_pQuoteOp)
	{
		m_pQuoteOp->NotifyFTPluginSocketClosed(PLUGIN_GUID, sock);
	}

	m_BasicPrice.NotifySocketClosed(sock);
	m_GearPrice.NotifySocketClosed(sock);
	m_RTData.NotifySocketClosed(sock);

	m_KLData.NotifySocketClosed(sock);
	m_StockSub.NotifySocketClosed(sock);
	m_StockUnSub.NotifySocketClosed(sock);

	m_QueryStockSub.NotifySocketClosed(sock);
	m_TradeDate.NotifySocketClosed(sock);
	m_StockList.NotifySocketClosed(sock);

	m_BatchBasic.NotifySocketClosed(sock);
	m_TickerPrice.NotifySocketClosed(sock);
	m_Snapshot.NotifySocketClosed(sock);

	m_HistoryKL.NotifySocketClosed(sock);
	m_ExRightInfo.NotifySocketClosed(sock);
	m_PushStockData.NotifySocketClosed(sock);

	m_PushBatchBasic.NotifySocketClosed(sock);
	m_PushGearPrice.NotifySocketClosed(sock);
	m_PushTickerPrice.NotifySocketClosed(sock);

	m_PushKLData.NotifySocketClosed(sock);
	m_PushRTData.NotifySocketClosed(sock);
	m_platesetIDs.NotifySocketClosed(sock);

	m_plateSubIDs.NotifySocketClosed(sock);
	m_BrokerQueue.NotifySocketClosed(sock);
	m_GlobalState.NotifySocketClosed(sock);
}

void  CPluginQuoteServer::OnChanged_PriceBase(INT64  ddwStockHash)
{
	m_BasicPrice.NotifyQuoteDataUpdate(PROTO_ID_QT_GET_BASIC_PRICE, ddwStockHash);
}

void  CPluginQuoteServer::OnChanged_OrderQueue(INT64 ddwStockHash)
{
	m_GearPrice.NotifyQuoteDataUpdate(PROTO_ID_QT_GET_GEAR_PRICE, ddwStockHash);
}

void  CPluginQuoteServer::OnChanged_BrokerQueue(INT64 ddwStockHash)
{
	m_BrokerQueue.NotifyQuoteDataUpdate(PROTO_ID_QT_GET_BROKER_QUEUE, ddwStockHash);
}

void CPluginQuoteServer::OnChanged_RTData(INT64 ddwStockHash)
{
	m_RTData.NotifyQuoteDataUpdate(PROTO_ID_QT_GET_RTDATA, ddwStockHash);
}

void CPluginQuoteServer::OnChanged_KLData(INT64 ddwStockHash, int nKLType)
{
	m_KLData.NotifyQuoteDataUpdate(PROTO_ID_QT_GET_KLDATA, ddwStockHash, nKLType);
}

void CPluginQuoteServer::OnPushPriceBase(INT64 ddwStockHash, SOCKET sock)
{
	m_PushBatchBasic.PushStockData(ddwStockHash, sock);
}

void CPluginQuoteServer::OnPushGear(INT64 ddwStockHash, SOCKET sock)
{
	m_PushGearPrice.PushStockData(ddwStockHash, sock);
}

void CPluginQuoteServer::OnPushBrokerQueue(INT64 ddwStockHash, SOCKET sock)
{
	m_BrokerQueue.PushStockData(ddwStockHash, sock);
}

void CPluginQuoteServer::OnPushMarketNewTrade(StockMktType eMkt, INT64 ddwLastTradeStamp, INT64 ddwNewTradeStamp)
{
	CHECK_RET(m_pQuoteData, NORET);

	// 断线重连也会触发这个回调， 前后的stamp是相同
	if (ddwLastTradeStamp != ddwNewTradeStamp)
	{
		m_PushTickerPrice.NotifyMarketNewTrade(eMkt);
		m_PushKLData.NotifyMarketNewTrade(eMkt);
		m_PushRTData.NotifyMarketNewTrade(eMkt);
	}
	
	//分时k线数据请求一次
	//For Fix BUG: 盘前定阅或者牛牛断线重连后，不再收到k线推送的BUG
	Quote_SubInfo* parSubInfo = NULL;
	int nSubCount = 0;
	m_pQuoteData->GetStockSubInfoList(parSubInfo, nSubCount);
	
	for (int i = 0; i < nSubCount && parSubInfo; i++)
	{
		Quote_SubInfo& info = parSubInfo[i];
		if (IsStockSubType_RTKL(info.eStockSubType))
		{
			StockMktType eMktType = StockMkt_None;
			wchar_t wstrCode[16] = { 0 }, szStockName[128] = { 0 };
			m_pQuoteData->GetStockInfoByHashVal(info.ddwStockHash, eMktType, wstrCode, szStockName);

			if (eMktType == eMkt)
			{
				DoTryRefreshRTKLData(info.ddwStockHash, info.eStockSubType);
			}
		}
	}
}

void CPluginQuoteServer::OnPushHeartBeat(SOCKET sock, UINT64 nTimeStampNow)
{
	PushHeartBeat_Ack ack;
	ack.head.nProtoID = PROTO_ID_PUSH_HEART_BEAT;
	ack.body.nTimeStamp = nTimeStampNow;
	CProtoPushHeartBeat proto;
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if (proto.MakeJson_Ack(jsnAck))
	{
		std::string strOut;
		CProtoPushHeartBeat::ConvJson2String(jsnAck, strOut, true);
		ReplyQuoteReq(ack.head.nProtoID, strOut.c_str(), (int)strOut.size(), sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}
}

void CPluginQuoteServer::OnPushTicker(INT64 ddwStockHash, SOCKET sock)
{
    m_PushTickerPrice.PushStockData(ddwStockHash, sock);
}

void CPluginQuoteServer::OnPushKL(INT64 ddwStockHash, SOCKET sock, StockSubType eStockSubType)
{
	m_PushKLData.PushStockData(ddwStockHash, sock, eStockSubType);
}

void CPluginQuoteServer::OnPushRT(INT64 ddwStockHash, SOCKET sock)
{
	m_PushRTData.PushStockData(ddwStockHash, sock);
}

void CPluginQuoteServer::OnQueryStockRTData(DWORD dwCookie, int nCSResult)
{
	DoCheckRTKLFreshReq(dwCookie, nCSResult);

	m_RTData.SendAck(dwCookie, nCSResult);
}

void CPluginQuoteServer::OnQueryStockKLData(DWORD dwCookie, int nCSResult)
{
	DoCheckRTKLFreshReq(dwCookie, nCSResult);

	m_KLData.SendAck(dwCookie, nCSResult);
}

void CPluginQuoteServer::OnReqPlatesetIDs(int nCSResult, DWORD dwCookie)
{
	m_platesetIDs.NotifyQueryPlatesetIDs(nCSResult, dwCookie);
}

void CPluginQuoteServer::OnReqPlateStockIDs(int nCSResult, DWORD dwCookie)
{
	m_plateSubIDs.NotifyQueryPlateSubIDs(nCSResult, dwCookie);
}

void CPluginQuoteServer::OnTimeEvent(UINT nEventID)
{
	if (0 == m_nTimerIDRefreshRTKL || m_nTimerIDRefreshRTKL != nEventID)
	{
		return;
	}

	CHECK_RET(m_pQuoteData, NORET);
	bool bRTKLFreshing = false;
	std::vector<tagRTKLDataRefresh>::iterator it = m_vtRTKLRefresh.begin();
	for (; it != m_vtRTKLRefresh.end(); it++)
	{
		if (it->dwReqCookie != 0)
		{
			it->nWaitSecs++;
			//超时处理
			if (it->nWaitSecs > REQ_TIMEOUT_MILLISECOND/1000)
			{
				it->nWaitSecs = 0;
				if (it->eStockSubType == StockSubType_RT)
				{
					OnQueryStockRTData(it->dwReqCookie, -1);
				}
				else
				{
					OnQueryStockKLData(it->dwReqCookie, -1);
				}
				return;
			}
			bRTKLFreshing = true;
			break;
		}
	}
	if (bRTKLFreshing)
	{
		return;
	}
	it = m_vtRTKLRefresh.begin();
	CHECK_RET(it != m_vtRTKLRefresh.end(), NORET);

	StockMktType eMktType = StockMkt_None;
	wchar_t wstrCode[16] = { 0 }, szStockName[128] = { 0 };
	if (m_pQuoteData->GetStockInfoByHashVal(it->ddwStockHash, eMktType, wstrCode, szStockName))
	{
		std::string strCode = CW2A(wstrCode);
		if (it->eStockSubType == StockSubType_RT)
		{
			QueryStockRTData(&it->dwReqCookie, strCode, eMktType, QuoteServer_RTData);
		}
		else
		{
			int nKLType = 0;
			if (DoKLSubTypeConvert(it->eStockSubType, nKLType))
			{
				CHECK_OP(nKLType != 0, NOOP);
				QueryStockKLData(&it->dwReqCookie, strCode, eMktType, QuoteServer_KLData, nKLType);
			}
		}
	}
}

bool CPluginQuoteServer::DoKLSubTypeConvert(StockSubType eType, int& nKLType)
{
	bool bRet = false;
	switch (eType)
	{
	case StockSubType_Simple:
	case StockSubType_Gear:
	case StockSubType_Ticker:
	case StockSubType_RT:
		break;
	case StockSubType_KL_DAY:
		nKLType = FT_KL_CLASS_DAY;
		bRet = true;
		break;
	case StockSubType_KL_MIN5:
		nKLType = FT_KL_CLASS_MIN_5;
		bRet = true;
		break;
	case StockSubType_KL_MIN15:
		nKLType = FT_KL_CLASS_MIN_15;
		bRet = true;
		break;
	case StockSubType_KL_MIN30:
		nKLType = FT_KL_CLASS_MIN_30;
		bRet = true;
		break;
	case StockSubType_KL_MIN60:
		nKLType = FT_KL_CLASS_MIN_60;
		bRet = true;
		break;
	case StockSubType_KL_MIN1:
		nKLType = FT_KL_CLASS_MIN_1;
		bRet = true;
		break;
	case StockSubType_KL_WEEK:
		nKLType = FT_KL_CLASS_WEEK;
		bRet = true;
		break;
	case StockSubType_KL_MONTH:
		nKLType = FT_KL_CLASS_MONTH;
		bRet = true;
		break;
	case StockSubType_Broker:
		break;
	case StockSubType_Max:
		break;
	default:
		break;
	}
	return bRet;
}

void CPluginQuoteServer::OnReqStockSnapshot(DWORD dwCookie, PluginStockSnapshot *arSnapshot, int nSnapshotNum)
{
	m_Snapshot.NotifySnapshotResult(dwCookie, arSnapshot, nSnapshotNum);
}

void CPluginQuoteServer::DoTryRefreshRTKLData(INT64 ddwStockHash, StockSubType eSubType)
{
	std::vector<tagRTKLDataRefresh>::iterator it = m_vtRTKLRefresh.begin();

	for (; it != m_vtRTKLRefresh.end(); it++)
	{
		if (it->ddwStockHash == ddwStockHash && it->eStockSubType == eSubType)
		{
			it->nTryCount = 0;
			return;
		}
	}
	tagRTKLDataRefresh stNew;
	stNew.ddwStockHash = ddwStockHash;
	stNew.eStockSubType = eSubType;
	stNew.dwReqCookie = 0;
	stNew.nTryCount = 0;
	m_vtRTKLRefresh.push_back(stNew);

	DoCheckRTKLFreshReq(0, 0);
}

void CPluginQuoteServer::DoCheckRTKLFreshReq(DWORD dwCookie, int nCSResult)
{
	if (dwCookie != 0)
	{
		std::vector<tagRTKLDataRefresh>::iterator it = m_vtRTKLRefresh.begin();
		for (; it != m_vtRTKLRefresh.end(); it++)
		{
			if (it->dwReqCookie == dwCookie)
			{
				if (0 == nCSResult || ++it->nTryCount >= Max_RTKL_Fresh_Try_Count)
				{
					m_vtRTKLRefresh.erase(it);
				}
				break;
			}
		}
	}
	if (m_vtRTKLRefresh.size() != 0)
	{
		DoStartTimerRefreshRTKL();
	}
	else
	{
		DoKillTimerRefreshRTKL();
	}
}

void CPluginQuoteServer::DoKillTimerRefreshRTKL()
{
	if (m_nTimerIDRefreshRTKL != 0)
	{
		m_TimerWnd.StopTimer(m_nTimerIDRefreshRTKL);
		m_nTimerIDRefreshRTKL = 0;
	}
}

void CPluginQuoteServer::DoStartTimerRefreshRTKL()
{
	if (0 == m_nTimerIDRefreshRTKL)
	{
		if (!m_TimerWnd.GetSafeHWnd())
		{
			m_TimerWnd.Create();
			m_TimerWnd.SetEventInterface(this);
		}
		m_nTimerIDRefreshRTKL = TimerID_FreshRTKLData;
		m_TimerWnd.StartTimer(1, m_nTimerIDRefreshRTKL);
	}
}

tagRTKLDataRefresh::tagRTKLDataRefresh()
{
	ddwStockHash = 0;
	eStockSubType = StockSubType_None;
	dwReqCookie = 0;
	nTryCount = 0;
	nWaitSecs = 0;
}
