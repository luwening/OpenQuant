#include "stdafx.h"
#include "PluginSnapshot.h"
#include "PluginQuoteServer.h"
#include "Include/FTPluginQuoteDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern GUID PLUGIN_GUID;

#define TIMER_ID_CLEAR_CACHE		354
#define TIMER_ID_HANDLE_TIMEOUT_REQ	355

#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_SNAPSHOT
#define QUOTE_SERVER_TYPE	QuoteServer_Snapshot


//////////////////////////////////////////////////////////////////////////

CPluginSnapshot::CPluginSnapshot()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
	m_pQuoteOperator = NULL;

	m_bStartTimerClearCache = FALSE;
	m_bStartTimerHandleTimeout = FALSE;
}

CPluginSnapshot::~CPluginSnapshot()
{
	Uninit();
}

void CPluginSnapshot::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
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

void CPluginSnapshot::Uninit()
{
	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;
		m_TimerWnd.Destroy();
		m_TimerWnd.SetEventInterface(NULL);

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);

		ClearAllReqCache();
	}
}

void CPluginSnapshot::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QUOTE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pQuoteData && m_pQuoteServer, ReplyDataDefError(sock, PROTO_ERR_UNKNOWN_ERROR, L"内部状态错误"));
	
	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType	req;
	proto.SetProtoData_Req(&req);
	if ( !proto.ParseJson_Req(jsnVal) || req.head.nProtoID != nCmdID || req.body.vtReqSnapshot.empty() )
	{
		CHECK_OP(false, NORET);
		req.head.nProtoID = nCmdID;

		StockDataReq req_info;		
		req_info.sock = sock;
		req_info.req = req;
		req_info.dwReqTick = ::GetTickCount();
		ReplyDataReqError(&req_info, PROTO_ERR_PARAM_ERR, L"参数错误！");
		return;
	}
	
	int nStockNum = (int)req.body.vtReqSnapshot.size();
	std::vector<INT64> vtReqStockID;
	for (int n = 0; n < nStockNum; n++ )
	{
		INT64 nStockID = IFTStockUtil::GetStockHashVal(req.body.vtReqSnapshot[n].strStockCode.c_str(),
				(StockMktType)req.body.vtReqSnapshot[n].nStockMarket);
		if (0 == nStockID)
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
	}

	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->dwReqTick = ::GetTickCount();

	QueryDataErrCode queryErr = m_pQuoteOperator->QueryStockSnapshot(PLUGIN_GUID, _vect2Ptr(vtReqStockID), _vectIntSize(vtReqStockID), pReqInfo->nReqCookie);
	if ( queryErr != QueryData_Suc || pReqInfo->nReqCookie == 0 )
	{
		delete pReqInfo;
		
		int nErrCode = UtilPlugin::ConvertErrCode(queryErr);
		std::string strErr = UtilPlugin::GetErrStrByCode(queryErr);
		std::wstring strTmp;
		CA::UTF2Unicode(strErr.c_str(), strTmp);

		ReplyDataDefError(sock, nErrCode, strTmp.c_str());
		return;
	}

	VT_STOCK_DATA_REQ &vtReq = m_mapReqInfo[pReqInfo->nReqCookie];	
	vtReq.push_back(pReqInfo);
	

	ReplyAllReadyReq();
	//m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);
	SetTimerHandleTimeout(true);
}

void CPluginSnapshot::SetSnapshotOperator(IFTQuoteOperation	*pQuoteOp)
{
	m_pQuoteOperator = pQuoteOp;
}

void CPluginSnapshot::NotifySnapshotResult(DWORD dwCookie, PluginStockSnapshot *arSnapshot, int nSnapshotNum)
{	
	MAP_STOCK_DATA_REQ::iterator it_find = m_mapReqInfo.find(dwCookie);
	if ( it_find == m_mapReqInfo.end() )
	{
		return ;
	}
	VT_STOCK_DATA_REQ &vtTmpReq = it_find->second;	
	VT_STOCK_DATA_REQ vtReq;
	vtReq.swap(vtTmpReq);
	m_mapReqInfo.erase(it_find);
	if ( vtReq.empty() )
	{
		return ;
	}
	StockDataReq *pReqInfo = vtReq[0];
	if ( pReqInfo == NULL )
	{
		return ;
	}
	nSnapshotNum = max(nSnapshotNum, 0);
	if (nSnapshotNum != 0 && arSnapshot == NULL)
	{
		nSnapshotNum = 0;
	}
	
	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReqInfo->req.head;	
	ack.head.nProtoID = PROTO_ID_QUOTE;
	ack.head.ddwErrCode = 0;	
	ack.body.vtSnapshot.resize(nSnapshotNum);
	//[3/29/2017 python脚本中约定的还是3位精度，等后台修改后这里会再修改ysq]
	const INT64 c_priceMultiply = 1000L; 

	for (int n = 0; n < nSnapshotNum; n++)
	{
		const PluginStockSnapshot &snapSrc = arSnapshot[n];
		SnapshotAckItem &snapDst = ack.body.vtSnapshot[n];
		StockMktCodeEx stMktCode;
		IFTStockUtil::GetStockMktCode(snapSrc.stock_id, stMktCode);

		snapDst.nStockID = snapSrc.stock_id;
		snapDst.strStockCode = stMktCode.strCode;
		snapDst.nStockMarket = stMktCode.eMarketType;
		snapDst.instrument_type = snapSrc.instrument_type;

		snapDst.last_close_price = INT64(snapSrc.last_close_price * c_priceMultiply);
		snapDst.nominal_price = INT64(snapSrc.nominal_price * c_priceMultiply);
		snapDst.open_price = INT64(snapSrc.open_price * c_priceMultiply);
		snapDst.update_time = snapSrc.update_time;
		snapDst.suspend_flag = snapSrc.suspend_flag;
		snapDst.listing_status = snapSrc.listing_status;
		snapDst.listing_date = snapSrc.listing_date;
		snapDst.shares_traded = snapSrc.shares_traded;
		snapDst.turnover = INT64(snapSrc.turnover * 1000);
		snapDst.highest_price = INT64(snapSrc.highest_price * c_priceMultiply);
		snapDst.lowest_price = INT64(snapSrc.lowest_price * c_priceMultiply);
		snapDst.turnover_ratio = int(snapSrc.turnover_ratio * 10000);
		
		snapDst.nLostSize = snapSrc.nLotSize;

		wchar_t szTime[64] = {0};
		if (m_pQuoteData)
		{
			m_pQuoteData->TimeStampToStr(snapDst.nStockID, snapDst.update_time, szTime);
		}
		CA::Unicode2UTF(szTime, snapDst.strUpdateTime);
		
		if (snapSrc.stEquitiesData.bDataValid)
		{
			snapDst.nTatalMarketVal = ((INT64)(snapSrc.nominal_price * c_priceMultiply)) * snapSrc.stEquitiesData.nIssuedShares;
			snapDst.nCircularMarketVal = ((INT64)(snapSrc.nominal_price * c_priceMultiply)) * snapSrc.stEquitiesData.nOutStandingShares;
		}
		else
		{
			snapDst.nTatalMarketVal = 0;
			snapDst.nCircularMarketVal = 0;
		}
		snapDst.ret_err = snapSrc.ret;
		//涡轮信息
		snapDst.stWrtData.bDataValid = snapSrc.stWrtData.bDataValid;
		snapDst.stWrtData.nWarrantType = snapSrc.stWrtData.nWarrantType;
		snapDst.stWrtData.nConversionRatio = snapSrc.stWrtData.nConversionRatio;
		snapDst.stWrtData.nStrikePrice = snapSrc.stWrtData.dbStrikePrice * c_priceMultiply;
		snapDst.stWrtData.nMaturityDate = snapSrc.stWrtData.nMaturityDate;
		snapDst.stWrtData.nEndtradeDate = snapSrc.stWrtData.nEndtradeDate;
		
		StockMktType eMkt = StockMkt_None; wchar_t szCode[16] = { 0 }, szStockName[128] = { 0 };
		if (m_pQuoteData && snapDst.stWrtData.bDataValid)
		{
			m_pQuoteData->GetStockInfoByHashVal(snapSrc.stWrtData.nWarrantOwnerID, eMkt, szCode, szStockName);
		}
		CA::Unicode2UTF(szCode, snapDst.stWrtData.strOwnerStockCode);
		snapDst.stWrtData.nOwnerStockMarket = (int)eMkt;

		snapDst.stWrtData.nRecoveryPrice = snapSrc.stWrtData.dbRecoveryPrice * c_priceMultiply;
		snapDst.stWrtData.nStreetVol = snapSrc.stWrtData.nStreetVol;
		snapDst.stWrtData.nIssueVol = snapSrc.stWrtData.nIssueVol;
		snapDst.stWrtData.nOwnerStockPrice = snapSrc.stWrtData.dbOwnerStockPrice * c_priceMultiply;
		snapDst.stWrtData.nStreetRatio = snapSrc.stWrtData.dbStreetRatio * 100000;
		snapDst.stWrtData.nDelta = snapSrc.stWrtData.dbDelta * 1000;
		snapDst.stWrtData.nImpliedVolatility = snapSrc.stWrtData.dbImpliedVolatility * 1000;
		snapDst.stWrtData.nPremium = snapSrc.stWrtData.dbPremiun * 100000;

		if (snapDst.stWrtData.bDataValid)
		{
			szTime[0] = 0;
			m_pQuoteData->TimeStampToStr(snapDst.nStockID, snapDst.stWrtData.nEndtradeDate, szTime);
			CA::Unicode2UTF(szTime, snapDst.stWrtData.strEndtradeDate);

			szTime[0] = 0;
			m_pQuoteData->TimeStampToStr(snapDst.nStockID, snapDst.stWrtData.nMaturityDate, szTime);
			CA::Unicode2UTF(szTime, snapDst.stWrtData.strMaturityData);
		}
	}
	
	CProtoQuote proto;	
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if ( proto.MakeJson_Ack(jsnAck) && m_pQuoteServer )
	{
		std::string strOut;
		CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
		m_pQuoteServer->ReplyQuoteReq(PROTO_ID_QUOTE, strOut.c_str(), (int)strOut.size(), pReqInfo->sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}

	VT_STOCK_DATA_REQ::iterator it_vt = vtReq.begin();
	for ( ; it_vt != vtReq.end(); ++it_vt )
	{
		StockDataReq *pReq = *it_vt;
		delete pReq;
	}
	vtReq.clear();
}

void CPluginSnapshot::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginSnapshot::OnTimeEvent(UINT nEventID)
{
	if ( TIMER_ID_CLEAR_CACHE == nEventID )
	{
		ClearQuoteDataCache();
	}
	else if ( TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID )
	{
		HandleTimeoutReq();
	}
}

void CPluginSnapshot::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{
		ReplyAllReadyReq();
	}	
}

void CPluginSnapshot::ClearQuoteDataCache()
{
	if ( m_mapCacheToDel.empty() )
	{
		SetTimerClearCache(false);
		return ;
	}

	DWORD dwTickNow = ::GetTickCount();

	MAP_CACHE_TO_DESTROY::iterator it_todel = m_mapCacheToDel.begin();
	for ( ; it_todel != m_mapCacheToDel.end(); )
	{
		INT64 nStockID = it_todel->first;
		DWORD dwToDelTick = it_todel->second;

		MAP_STOCK_DATA_REQ::iterator it_req = m_mapReqInfo.find(nStockID);
		if ( it_req != m_mapReqInfo.end() )
		{
			it_todel = m_mapCacheToDel.erase(it_todel);
		}
		else
		{
			if ( int(dwTickNow - dwToDelTick) > 60*1000  )
			{
				m_mapCacheData.erase(nStockID);
				it_todel = m_mapCacheToDel.erase(it_todel);

				StockMktCodeEx stkMktCode;
				if ( m_pQuoteServer && IFTStockUtil::GetStockMktCode(nStockID, stkMktCode) )
				{				
					//m_pQuoteServer->SubscribeQuote(stkMktCode.strCode, (StockMktType)stkMktCode.nMarketType, QUOTE_SERVER_TYPE, false);					
				}
				else
				{
					CHECK_OP(false, NOOP);
				}
			}
			else
			{
				++it_todel;
			}			
		}
	}

	if ( m_mapCacheToDel.empty() )
	{
		SetTimerClearCache(false);		
	}
}

void CPluginSnapshot::HandleTimeoutReq()
{
	if ( m_mapReqInfo.empty() )
	{
		SetTimerHandleTimeout(false);
		return;
	}

	ReplyAllReadyReq();

	DWORD dwTickNow = ::GetTickCount();	
	MAP_STOCK_DATA_REQ::iterator it_stock = m_mapReqInfo.begin();
	for ( ; it_stock != m_mapReqInfo.end(); )
	{
		INT64 nStockID = it_stock->first;
		VT_STOCK_DATA_REQ &vtReq = it_stock->second;
		VT_STOCK_DATA_REQ::iterator it_req = vtReq.begin();

		for ( ; it_req != vtReq.end(); )
		{
			StockDataReq *pReq = *it_req;
			if ( pReq == NULL )
			{
				CHECK_OP(false, NOOP);
				it_req = vtReq.erase(it_req);
				continue;
			}

			if (int(dwTickNow - pReq->dwReqTick) > REQ_TIMEOUT_MILLISECOND)
			{
				//tomodify timeout
				CStringA strTimeout;
				strTimeout.Format("snapshot req timeout, cookie=%d", pReq->nReqCookie);
				OutputDebugStringA(strTimeout.GetString());				
				ReplyDataReqError(pReq, PROTO_ERR_SERVER_TIMEROUT, L"请求超时！");
				if ( m_pQuoteOperator )
				{
					m_pQuoteOperator->CancelQuerySnapshot(pReq->nReqCookie);
				}
				it_req = vtReq.erase(it_req);
				delete pReq;
			}
			else
			{
				++it_req;
			}
		}

		if ( vtReq.empty() )
		{
			//这里不用启动清缓定时器，因为这时没有缓存当前股票的数据
			it_stock = m_mapReqInfo.erase(it_stock);			
		}
		else
		{
			++it_stock;
		}
	}

	if ( m_mapReqInfo.empty() )
	{
		SetTimerHandleTimeout(false);
		return;
	}
}

void CPluginSnapshot::ReplyAllReadyReq()
{
	DWORD dwTickNow = ::GetTickCount();
	MAP_STOCK_DATA_REQ::iterator it_stock = m_mapReqInfo.begin();
	for ( ; it_stock != m_mapReqInfo.end(); )
	{
		INT64 nStockID = it_stock->first;
		VT_STOCK_DATA_REQ &vtReq = it_stock->second;
		MAP_STOCK_CACHE_DATA::iterator it_data = m_mapCacheData.find(nStockID);

		if ( it_data == m_mapCacheData.end() )
		{
			++it_stock;
			continue;
		}
		
		VT_STOCK_DATA_REQ::iterator it_req = vtReq.begin();
		for ( ; it_req != vtReq.end(); ++it_req )
		{
			StockDataReq *pReq = *it_req;
			CHECK_OP(pReq, NOOP);
			ReplyStockDataReq(pReq, it_data->second);
			delete pReq;
		}

		vtReq.clear();
		m_mapCacheData.erase(it_data);

		it_stock = m_mapReqInfo.erase(it_stock);
		m_mapCacheToDel[nStockID] = dwTickNow;
		SetTimerClearCache(true);
	}

	if ( m_mapReqInfo.empty() )
	{
		SetTimerHandleTimeout(false);
		return;
	}
}

void CPluginSnapshot::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
{
	CHECK_RET(pReq && m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReq->req.head;
	ack.head.ddwErrCode = 0;
	ack.body = data;

	//tomodify 4 ack中的重要字段直接替换成请包时的字段

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

void CPluginSnapshot::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
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

void CPluginSnapshot::ReplyDataDefError(SOCKET sock, int nErrCode, LPCWSTR pErrDesc)
{
	CHECK_RET(m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head.nProtoID = PROTO_ID_QUOTE;	
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
		m_pQuoteServer->ReplyQuoteReq(PROTO_ID_QUOTE, strOut.c_str(), (int)strOut.size(), sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}
}

void CPluginSnapshot::SetTimerHandleTimeout(bool bStartOrStop)
{
	if ( m_bStartTimerHandleTimeout )
	{
		if ( !bStartOrStop )
		{			
			m_TimerWnd.StopTimer(TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = FALSE;
		}
	}
	else
	{
		if ( bStartOrStop )
		{
			m_TimerWnd.StartMillionTimer(500, TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = TRUE;
		}
	}
}

void CPluginSnapshot::SetTimerClearCache(bool bStartOrStop)
{
	if ( m_bStartTimerClearCache )
	{
		if ( !bStartOrStop )
		{
			m_TimerWnd.StopTimer(TIMER_ID_CLEAR_CACHE);
			m_bStartTimerClearCache = FALSE;
		}
	}
	else
	{
		if ( bStartOrStop )
		{
			m_TimerWnd.StartMillionTimer(50, TIMER_ID_CLEAR_CACHE);
			m_bStartTimerClearCache = TRUE;
		}
	}
}


void CPluginSnapshot::ClearAllReqCache()
{
	MAP_STOCK_DATA_REQ::iterator it_stock = m_mapReqInfo.begin();
	for ( ; it_stock != m_mapReqInfo.end(); ++it_stock )
	{
		VT_STOCK_DATA_REQ &vtReq = it_stock->second;
		VT_STOCK_DATA_REQ::iterator it_req = vtReq.begin();
		for ( ; it_req != vtReq.end(); ++it_req )
		{
			StockDataReq *pReq = *it_req;
			delete pReq;
		}
	}

	m_mapReqInfo.clear();
	m_mapCacheData.clear();
	m_mapCacheToDel.clear();
}

void CPluginSnapshot::DoClearReqInfo(SOCKET socket)
{
	auto itmap = m_mapReqInfo.begin();
	while (itmap != m_mapReqInfo.end())
	{
		VT_STOCK_DATA_REQ& vtReq = itmap->second;

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
		if (vtReq.size() == 0)
		{
			itmap = m_mapReqInfo.erase(itmap);
		}
		else
		{
			++itmap;
		}
	}
}
