#include "stdafx.h"
#include "ProtoPlateSubIDs.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json_op.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProtoPlateSubIDs::CProtoPlateSubIDs()
{
	m_pReqData = NULL;
	m_pAckData = NULL;
}

CProtoPlateSubIDs::~CProtoPlateSubIDs()
{

}

bool CProtoPlateSubIDs::ParseJson_Req(const Json::Value &jsnVal)
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

bool CProtoPlateSubIDs::ParseJson_Ack(const Json::Value &jsnVal)
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


bool CProtoPlateSubIDs::MakeJson_Req(Json::Value &jsnVal)
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

bool CProtoPlateSubIDs::MakeJson_Ack(Json::Value &jsnVal)
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

void CProtoPlateSubIDs::SetProtoData_Req(ProtoReqDataType *pData)
{
	m_pReqData = pData;
}

void CProtoPlateSubIDs::SetProtoData_Ack(ProtoAckDataType *pData)
{
	m_pAckData = pData;
}

//tomodify 3(数组等复杂结构或单层的结构体)
bool CProtoPlateSubIDs::ParseProtoBody_Req(const Json::Value &jsnVal, ProtoReqDataType &data)
{	
	if ( !warn_if_prop_not_set(jsnVal, KEY_REQ_PARAM) )
		return true;

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Req(vtField, data.body);

	const Json::Value &jsnBody = jsnVal[KEY_REQ_PARAM];
	bool bSuc = CProtoParseBase::ParseProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	return true;
}

//tomodify 4(数组等复杂结构或单层的结构体)
bool CProtoPlateSubIDs::ParseProtoBody_Ack(const Json::Value &jsnVal, ProtoAckDataType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnVal,  KEY_ACK_DATA), false);	

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Ack(vtField, data.body);

	const Json::Value &jsnBody = jsnVal[KEY_ACK_DATA];
	bool bSuc = CProtoParseBase::ParseProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if (bSuc)
	{
		bSuc &= ParsePlateSubIDsArr(jsnBody, data.body);
	}
	return bSuc;
}

//tomodify 5(数组等复杂结构或单层的结构体)
bool CProtoPlateSubIDs::MakeProtoBody_Req(Json::Value &jsnVal, const ProtoReqDataType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnVal, KEY_REQ_PARAM), false);
	
	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Req(vtField, data.body);

	Json::Value &jsnBody = jsnVal[KEY_REQ_PARAM];
	bool bSuc = CProtoParseBase::MakeProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	return bSuc;
}

//tomodify 6(数组等复杂结构或单层的结构体)
bool CProtoPlateSubIDs::MakeProtoBody_Ack(Json::Value &jsnVal, const ProtoAckDataType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnVal, KEY_ACK_DATA), false);	

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Ack(vtField, data.body);

	Json::Value &jsnBody = jsnVal[KEY_ACK_DATA];
	bool bSuc = CProtoParseBase::MakeProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if (bSuc)
	{
		bSuc &= MakePlateSubIDsArr(jsnBody, data.body);
	}

	return bSuc;
}

//tomodify 7
void CProtoPlateSubIDs::GetProtoBodyField_Req(VT_PROTO_FIELD &vtField, const ProtoReqBodyType &reqData)
{
	static BOOL arOptional[] = {
		FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_StringA
	};
	static LPCSTR arFieldKey[] = {
		"Market", "StockCode"
	};

	ProtoReqBodyType &body = const_cast<ProtoReqBodyType &>(reqData);	
	void *arPtr[] = {
		&body.nStockMarket, &body.strStockCode
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

//tomodify 8
void CProtoPlateSubIDs::GetProtoBodyField_Ack(VT_PROTO_FIELD &vtField, const ProtoAckBodyType &ackData)
{
	static BOOL arOptional[] = {
		FALSE, FALSE
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_StringA
	};
	static LPCSTR arFieldKey[] = {
		"Market", "StockCode"
	};

	ProtoAckBodyType &body = const_cast<ProtoAckBodyType &>(ackData);
	void *arPtr[] = {
		&body.nStockMarket, &body.strStockCode
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

bool CProtoPlateSubIDs::ParsePlateSubIDsArr(const Json::Value &jsnRetData, ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnRetData, "PlateSubIDsArr"), false);

	const Json::Value &jsnPlateSubIDsArr = jsnRetData["PlateSubIDsArr"];
	CHECK_RET(jsnPlateSubIDsArr.isArray(), false);

	bool bSuc = true;
	int nArrSize = jsnPlateSubIDsArr.size();
	for (int n = 0; n < nArrSize; n++)
	{
		PlateSubIDsAckItem item;
		VT_PROTO_FIELD vtField;
		GetPlateSubIDsArrField(vtField, item);

		const Json::Value jsnItem = jsnPlateSubIDsArr[n];
		CHECK_OP(!jsnItem.isNull(), continue);
		bSuc = CProtoParseBase::ParseProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
		data.vtPlateSubIDs.push_back(item);
	}

	return bSuc;
}

bool CProtoPlateSubIDs::MakePlateSubIDsArr(Json::Value &jsnRetData, const ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "PlateSubIDsArr"), false);

	Json::Value &jsnPlateSubIDsArr = jsnRetData["PlateSubIDsArr"];

	bool bSuc = true;
	for (int n = 0; n < (int)data.vtPlateSubIDs.size(); n++)
	{
		const PlateSubIDsAckItem &item = data.vtPlateSubIDs[n];
		VT_PROTO_FIELD vtField;
		GetPlateSubIDsArrField(vtField, item);

		Json::Value &jsnItem = jsnPlateSubIDsArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
	}
	return bSuc;
}

void CProtoPlateSubIDs::GetPlateSubIDsArrField(VT_PROTO_FIELD &vtField, const PlateSubIDsAckItem &ackItem)
{
	static BOOL arOptional[] = {
		FALSE, FALSE,
		FALSE, FALSE,
		FALSE,
		TRUE, TRUE, TRUE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_Int32,
		ProtoFild_StringA, ProtoFild_StringA,
		ProtoFild_Int32,
		ProtoFild_Int32, ProtoFild_StringA, ProtoFild_Int32,

	};
	static LPCSTR arFieldKey[] = {
		"LotSize", "Market",
		"StockName", "StockCode",
		"StockType",
		"StockChildType", "OwnerStockCode", "OwnerMarketType"
	};

	PlateSubIDsAckItem &item = const_cast<PlateSubIDsAckItem &>(ackItem);
	void *arPtr[] = {
		&item.nLotSize, &item.nStockMarket,
		&item.strSimpName, &item.strStockCode,
		&item.nSecurityType,
		&item.nSubType, &item.strOwnerStockCode, &item.nOwnerMarketType,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for (int n = 0; n < _countof(arOptional); n++)
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