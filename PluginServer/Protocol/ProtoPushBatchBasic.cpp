#include "stdafx.h"
#include "ProtoPushBatchBasic.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json_op.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProtoPushBatchBasic::CProtoPushBatchBasic()
{
	m_pReqData = NULL;
	m_pAckData = NULL;
}

CProtoPushBatchBasic::~CProtoPushBatchBasic()
{

}

bool CProtoPushBatchBasic::ParseJson_Req(const Json::Value &jsnVal)
{
	CHECK_RET(m_pReqData != NULL, false);

	bool bSuc = true;
	do 
	{
		bSuc &= ParseProtoHead_Req(jsnVal, m_pReqData->head);
		CHECK_OP(bSuc, break);
		bSuc &= ParseProtoBody_Req(jsnVal, *m_pReqData);
		CHECK_OP(bSuc, break);
	} while (0);

	return bSuc;
}

bool CProtoPushBatchBasic::ParseJson_Ack(const Json::Value &jsnVal)
{
	CHECK_RET(m_pAckData != NULL, false);

	bool bSuc = true;
	do 
	{
		bSuc &= ParseProtoHead_Ack(jsnVal, m_pAckData->head);
		CHECK_OP(bSuc, break);

		if ( m_pAckData->head.ddwErrCode == PROTO_ERR_NO_ERROR )
		{
			bSuc &= ParseProtoBody_Ack(jsnVal, *m_pAckData);
			CHECK_OP(bSuc, break);
		}
	} while (0);

	return bSuc;
}


bool CProtoPushBatchBasic::MakeJson_Req(Json::Value &jsnVal)
{
	CHECK_RET(m_pReqData != NULL, false);

	bool bSuc = true;
	do 
	{
		bSuc &= MakeProtoHead_Req(jsnVal, m_pReqData->head);
		CHECK_OP(bSuc, break);
		bSuc &= MakeProtoBody_Req(jsnVal, *m_pReqData);
		CHECK_OP(bSuc, break);
	} while (0);

	return bSuc;
}

bool CProtoPushBatchBasic::MakeJson_Ack(Json::Value &jsnVal)
{
	CHECK_RET(m_pAckData != NULL, false);

	bool bSuc = true;
	do 
	{
		bSuc &= MakeProtoHead_Ack(jsnVal, m_pAckData->head);
		CHECK_OP(bSuc, break);

		if ( m_pAckData->head.ddwErrCode == PROTO_ERR_NO_ERROR )
		{
			bSuc &= MakeProtoBody_Ack(jsnVal, *m_pAckData);
			CHECK_OP(bSuc, break);
		}
	} while (0);

	return bSuc;
}

void CProtoPushBatchBasic::SetProtoData_Req(ProtoReqDataType *pData)
{
	m_pReqData = pData;
}

void CProtoPushBatchBasic::SetProtoData_Ack(ProtoAckDataType *pData)
{
	m_pAckData = pData;
}

//tomodify 3(数组等复杂结构或单层的结构体)
bool CProtoPushBatchBasic::ParseProtoBody_Req(const Json::Value &jsnVal, ProtoReqDataType &data)
{	
	if ( !warn_if_prop_not_set(jsnVal, KEY_REQ_PARAM) )
		return true;

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Req(vtField, data.body);

	const Json::Value &jsnBody = jsnVal[KEY_REQ_PARAM];
	bool bSuc = CProtoParseBase::ParseProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if ( bSuc )
	{
		bSuc &= ParseReqBatchBasicArr(jsnBody, data.body);
	}

	return true;
}

//tomodify 4(数组等复杂结构或单层的结构体)
bool CProtoPushBatchBasic::ParseProtoBody_Ack(const Json::Value &jsnVal, ProtoAckDataType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnVal, KEY_ACK_DATA), false);	

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Ack(vtField, data.body);

	const Json::Value &jsnBody = jsnVal[KEY_ACK_DATA];
	bool bSuc = CProtoParseBase::ParseProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if ( bSuc )
	{
		bSuc &= ParseAckBatchBasicArr(jsnBody, data.body);
	}

	return bSuc;
}

//tomodify 5(数组等复杂结构或单层的结构体)
bool CProtoPushBatchBasic::MakeProtoBody_Req(Json::Value &jsnVal, const ProtoReqDataType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnVal, KEY_REQ_PARAM), false);

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Req(vtField, data.body);

	Json::Value &jsnBody = jsnVal[KEY_REQ_PARAM];
	bool bSuc = CProtoParseBase::MakeProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if ( bSuc )
	{
		bSuc &= MakeReqBatchBasicArr(jsnBody, data.body);
	}

	return bSuc;
}

//tomodify 6(数组等复杂结构或单层的结构体)
bool CProtoPushBatchBasic::MakeProtoBody_Ack(Json::Value &jsnVal, const ProtoAckDataType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnVal, KEY_ACK_DATA), false);	

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Ack(vtField, data.body);

	Json::Value &jsnBody = jsnVal[KEY_ACK_DATA];
	bool bSuc = CProtoParseBase::MakeProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if ( bSuc )
	{
		bSuc &= MakeAckBatchBasicArr(jsnBody, data.body);
	}

	return bSuc;
}

//tomodify 7
void CProtoPushBatchBasic::GetProtoBodyField_Req(VT_PROTO_FIELD &vtField, const ProtoReqBodyType &reqData)
{

}

//tomodify 8
void CProtoPushBatchBasic::GetProtoBodyField_Ack(VT_PROTO_FIELD &vtField, const ProtoAckBodyType &ackData)
{

}

bool CProtoPushBatchBasic::ParseAckBatchBasicArr(const Json::Value &jsnRetData, ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnRetData, "SubSnapshotArr"), false);	

	const Json::Value &jsnGearArr = jsnRetData["SubSnapshotArr"];
	CHECK_RET(jsnGearArr.isArray(), false);

	bool bSuc = true;
	int nArrSize = jsnGearArr.size();
	for ( int n = 0; n < nArrSize; n++ )
	{		
		PushBatchBasicAckItem item;
		VT_PROTO_FIELD vtField;
		GetAckBatchBasicArrField(vtField, item);

		const Json::Value jsnItem = jsnGearArr[n];
		CHECK_OP(!jsnItem.isNull(), continue);
		bSuc = CProtoParseBase::ParseProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
		data.vtAckBatchBasic.push_back(item);
	}

	return bSuc;
}

bool CProtoPushBatchBasic::MakeAckBatchBasicArr(Json::Value &jsnRetData, const ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "SubSnapshotArr"), false);	

	Json::Value &jsnGearArr = jsnRetData["SubSnapshotArr"];	

	bool bSuc = true;
	for ( int n = 0; n < (int)data.vtAckBatchBasic.size(); n++ )
	{
		const PushBatchBasicAckItem &item = data.vtAckBatchBasic[n];
		VT_PROTO_FIELD vtField;
		GetAckBatchBasicArrField(vtField, item);

		Json::Value &jsnItem = jsnGearArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

void CProtoPushBatchBasic::GetAckBatchBasicArrField(VT_PROTO_FIELD &vtField, const PushBatchBasicAckItem &ackItem)
{
	static BOOL arOptional[] = {
		FALSE, FALSE, FALSE,
		FALSE, FALSE, FALSE,
		FALSE, FALSE, FALSE,
		FALSE, FALSE, FALSE,
		FALSE, FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32,   ProtoFild_StringA,   ProtoFild_Int32, 
		ProtoFild_Int32,   ProtoFild_Int32,     ProtoFild_Int32, 
		ProtoFild_Int32,   ProtoFild_Int32,     ProtoFild_Int32,
		ProtoFild_Int64,   ProtoFild_Int64,     ProtoFild_Int64,   
		ProtoFild_StringW, ProtoFild_StringW,   ProtoFild_StringW, 
	};
	static LPCSTR arFieldKey[] = {
		"Market",  "StockCode",  "High", 
		"Open",	  "LastClose",    "Low",
		"CurPrice",	  "Suspension", "TurnoverRate",
		"Turnover",  "Volume",   "Amplitude",     
		"Date",	  "Time",     "ListTime", 
	};

	PushBatchBasicAckItem &item = const_cast<PushBatchBasicAckItem &>(ackItem);
	void *arPtr[] = {
		&item.nStockMarket,	&item.strStockCode,	&item.nHigh,
		&item.nOpen,	  	&item.nLastClose,   &item.nLow,	
		&item.nCur,	        &item.nSuspension,  &item.nTurnoverRate,
		&item.nValue,		&item.nVolume,		&item.nAmpli,
		&item.strDate,      &item.strTime,	    &item.strListTime,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		field.bOptional = arOptional[n];
		field.eFieldType = arFieldType[n];
		field.strFieldKey = arFieldKey[n];
		switch (field.eFieldType)
		{
		case ProtoFild_Int32:
			field.pInt32 = (int*)arPtr[n];
			break;
		case ProtoFild_Int64:
			field.pInt64 = (INT64*)arPtr[n];
			break;
		case ProtoFild_StringA:
			field.pStrA = (std::string*)arPtr[n];
			break;
		case ProtoFild_StringW:
			field.pStrW = (std::wstring*)arPtr[n];
			break;
		default:
			CHECK_OP(FALSE, NOOP);
			break;
		}

		vtField.push_back(field);
	}	
}

bool CProtoPushBatchBasic::ParseReqBatchBasicArr(const Json::Value &jsnRetData, ProtoReqBodyType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnRetData, "ReqArr"), false);	

	const Json::Value &jsnReqBatchBasicArr = jsnRetData["ReqArr"];
	CHECK_RET(jsnReqBatchBasicArr.isArray(), false);

	bool bSuc = true;
	int nArrSize = jsnReqBatchBasicArr.size();
	for ( int n = 0; n < nArrSize; n++ )
	{		
		PushBatchBasicReqItem item;
		VT_PROTO_FIELD vtField;
		GetReqBatchBasicArrField(vtField, item);

		const Json::Value jsnItem = jsnReqBatchBasicArr[n];
		CHECK_OP(!jsnItem.isNull(), continue);
		bSuc = CProtoParseBase::ParseProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
		data.vtReqBatchBasic.push_back(item);
	}

	return bSuc;
}

bool CProtoPushBatchBasic::MakeReqBatchBasicArr(Json::Value &jsnRetData, const ProtoReqBodyType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "ReqArr"), false);	

	Json::Value &jsnReqBatchBasicArr = jsnRetData["ReqArr"];	

	bool bSuc = true;
	for ( int n = 0; n < (int)data.vtReqBatchBasic.size(); n++ )
	{
		const PushBatchBasicReqItem &item = data.vtReqBatchBasic[n];
		VT_PROTO_FIELD vtField;
		GetReqBatchBasicArrField(vtField, item);

		Json::Value &jsnItem = jsnReqBatchBasicArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

void CProtoPushBatchBasic::GetReqBatchBasicArrField(VT_PROTO_FIELD &vtField, const PushBatchBasicReqItem &ackItem)
{
	static BOOL arOptional[] = {
		FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32,   ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"Market",  "StockCode",
	};

	PushBatchBasicReqItem &item = const_cast<PushBatchBasicReqItem &>(ackItem);
	void *arPtr[] = {
		&item.nStockMarket,	&item.strStockCode,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		field.bOptional = arOptional[n];
		field.eFieldType = arFieldType[n];
		field.strFieldKey = arFieldKey[n];
		switch (field.eFieldType)
		{
		case ProtoFild_Int32:
			field.pInt32 = (int*)arPtr[n];
			break;
		case ProtoFild_Int64:
			field.pInt64 = (INT64*)arPtr[n];
			break;
		case ProtoFild_StringA:
			field.pStrA = (std::string*)arPtr[n];
			break;
		case ProtoFild_StringW:
			field.pStrW = (std::wstring*)arPtr[n];
			break;
		default:
			CHECK_OP(FALSE, NOOP);
			break;
		}

		vtField.push_back(field);
	}	
}
