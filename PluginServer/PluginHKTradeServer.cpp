#include "stdafx.h"
#include "PluginHKTradeServer.h"
#include "PluginNetwork.h"
#include "Protocol/ProtoOrderErrorPush.h"
#include "Protocol/ProtoOrderUpdatePush.h"
#include "Protocol/ProtoBasicPrice.h"
#include "IManage_SecurityNum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern GUID PLUGIN_GUID;

//////////////////////////////////////////////////////////////////////////

CPluginHKTradeServer::CPluginHKTradeServer()
{
	m_pPluginCore = NULL;
	m_pTradeOp = NULL;	
	m_pNetwork = NULL;
	//IManage_SecurityNum::Init();
}

CPluginHKTradeServer::~CPluginHKTradeServer()
{
	UninitTradeSvr();
}

void CPluginHKTradeServer::InitTradeSvr(IFTPluginCore* pPluginCore, CPluginNetwork *pNetwork)
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
	pPluginCore->QueryFTInterface(IID_IFTTrade_HK, (void**)&m_pTradeOp);
	pPluginCore->QueryFTInterface(IID_IFTDataReport, (void**)&m_pDataReport);

	if ( m_pTradeOp == NULL || m_pDataReport == NULL )
	{
		ASSERT(false);		
		m_pTradeOp = NULL;
		m_pDataReport = NULL;
		m_pPluginCore = NULL;
		m_pNetwork = NULL;
		return;
	}	

	m_PlaceOrder.Init(this, m_pTradeOp);
	m_ChangeOrder.Init(this, m_pTradeOp);
	m_SetOrderStatus.Init(this, m_pTradeOp);
	m_UnlockTrade.Init(this, m_pTradeOp);
	m_QueryAccInfo.Init(this, m_pTradeOp);
	m_QueryHKOrder.Init(this, m_pTradeOp);
	m_QueryHKPos.Init(this, m_pTradeOp);
	m_QueryHKDeal.Init(this, m_pTradeOp);
}

void CPluginHKTradeServer::UninitTradeSvr()
{
	if ( m_pPluginCore != NULL )
	{
		m_QueryHKPos.Uninit();
		m_QueryHKOrder.Uninit();
		m_QueryAccInfo.Uninit();
		m_UnlockTrade.Uninit();
		m_PlaceOrder.Uninit();
		m_ChangeOrder.Uninit();
		m_SetOrderStatus.Uninit();
		m_QueryHKDeal.Uninit();

		m_pTradeOp = NULL;
		m_pDataReport = NULL;
		m_pPluginCore = NULL;
		m_pNetwork = NULL;
	}	
}

void CPluginHKTradeServer::SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	m_pDataReport->PLSCmdIDReport(PLUGIN_GUID, nCmdID);
	switch (nCmdID)
	{
	case PROTO_ID_TDHK_UNLOCK_TRADE:
		m_UnlockTrade.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_ACC_INFO:
		m_QueryAccInfo.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_ORDER:
		m_QueryHKOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_POSITION:
		m_QueryHKPos.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_PLACE_ORDER:
		m_PlaceOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_SET_ORDER_STATUS:
		m_SetOrderStatus.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_CHANGE_ORDER:
		m_ChangeOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_DEAL:
		m_QueryHKDeal.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	default:
		CHECK_OP(false, NOOP);
		BasicPrice_Ack Ack;
		Ack.head.ddwErrCode = PROTO_ERR_PARAM_ERR;
		Ack.head.nProtoID = nCmdID;
		CA::Unicode2UTF(L"Э��Ŵ���", Ack.head.strErrDesc);
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

void CPluginHKTradeServer::ReplyTradeReq(int nCmdID, const char *pBuf, int nLen, SOCKET sock)
{
	CHECK_RET(nCmdID && pBuf && nLen && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pNetwork, NORET);
	m_pNetwork->SendData(sock, pBuf, nLen);
}

void CPluginHKTradeServer::CloseSocket(SOCKET sock)
{
	m_PlaceOrder.NotifySocketClosed(sock);
	m_ChangeOrder.NotifySocketClosed(sock);
	m_SetOrderStatus.NotifySocketClosed(sock);

	m_UnlockTrade.NotifySocketClosed(sock);
	m_QueryAccInfo.NotifySocketClosed(sock);
	m_QueryHKOrder.NotifySocketClosed(sock);

	m_QueryHKPos.NotifySocketClosed(sock);
	m_QueryHKDeal.NotifySocketClosed(sock);
}

void CPluginHKTradeServer::OnUnlockTrade(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nErrCode)
{
	if (enSvrRet == Trade_SvrResult_Failed)
	{
		IManage_SecurityNum::DeleteCookieSocket(nCookie);
	}
	else
	{
		IManage_SecurityNum::AddSafeSocket(nCookie);
	}

	m_UnlockTrade.NotifyOnUnlockTrade(nCookie, enSvrRet, nErrCode);
}

void CPluginHKTradeServer::OnQueryOrderList(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder)
{
	m_QueryHKOrder.NotifyOnQueryHKOrder(enEnv, nCookie, nCount, pArrOrder);
}

void CPluginHKTradeServer::OnQueryDealList(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_DealItem* pArrOrder)
{
	m_QueryHKDeal.NotifyOnQueryHKDeal(enEnv, nCookie, nCount, pArrOrder);
}

void CPluginHKTradeServer::OnQueryPositionList(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_PositionItem* pArrPosition)
{
	m_QueryHKPos.NotifyOnQueryPosition(enEnv, nCookie, nCount, pArrPosition);
}

void CPluginHKTradeServer::OnQueryAccInfo(Trade_Env enEnv, UINT32 nCookie, const Trade_AccInfo& accInfo, int nResult)
{
	m_QueryAccInfo.NotifyOnQueryHKAccInfo(enEnv, nCookie, accInfo, nResult);
}

void CPluginHKTradeServer::OnPlaceOrder(Trade_Env enEnv, UINT nCookie, Trade_SvrResult enSvrRet, UINT64 nLocalID, UINT16 nErrCode)
{
	m_PlaceOrder.NotifyOnPlaceOrder(enEnv, nCookie, enSvrRet, nLocalID, nErrCode);
}

void CPluginHKTradeServer::OnOrderUpdate(Trade_Env enEnv, const Trade_OrderItem& orderItem)
{ 
 
}

void CPluginHKTradeServer::OnSetOrderStatus(Trade_Env enEnv, UINT nCookie, Trade_SvrResult enSvrRet, UINT64 nOrderID, UINT16 nErrCode)
{
	m_SetOrderStatus.NotifyOnSetOrderStatus(enEnv, nCookie, enSvrRet, nOrderID, nErrCode);
}

void CPluginHKTradeServer::OnChangeOrder(Trade_Env enEnv, UINT nCookie, Trade_SvrResult enSvrRet, UINT64 nOrderID, UINT16 nErrCode)
{
	m_ChangeOrder.NotifyOnChangeOrder(enEnv, nCookie, enSvrRet, nOrderID, nErrCode);
}

void CPluginHKTradeServer::OnOrderErrNotify(Trade_Env enEnv, UINT64 nOrderID, Trade_OrderErrNotify_HK enErrNotify, UINT16 nErrCode)
{
	 
}
