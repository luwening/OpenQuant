#include "stdafx.h"
#include "ProtoExRightInfo.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json_op.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProtoExRightInfo::CProtoExRightInfo()
{
	m_pReqData = NULL;
	m_pAckData = NULL;
}

CProtoExRightInfo::~CProtoExRightInfo()
{

}

bool CProtoExRightInfo::ParseJson_Req(const Json::Value &jsnVal)
{
	CHECK_RET(m_pReqData != NULL, false);

	bool bSuc = ParseJsonProtoStruct(jsnVal, true, "", m_pReqData);
	return bSuc;
}

bool CProtoExRightInfo::ParseJson_Ack(const Json::Value &jsnVal)
{
	CHECK_RET(m_pAckData != NULL, false);

	bool bSuc = ParseJsonProtoStruct(jsnVal, false, "", m_pAckData);
	return bSuc;
}


bool CProtoExRightInfo::MakeJson_Req(Json::Value &jsnVal)
{
	CHECK_RET(m_pReqData != NULL, false);

	bool bSuc = MakeJsonProtoStruct(jsnVal, true, "", m_pReqData);
	return bSuc;
}

bool CProtoExRightInfo::MakeJson_Ack(Json::Value &jsnVal)
{
	CHECK_RET(m_pAckData != NULL, false);

	bool bSuc = MakeJsonProtoStruct(jsnVal, false, "", m_pAckData);
	return bSuc;
}

void CProtoExRightInfo::SetProtoData_Req(ProtoReqDataType *pData)
{
	m_pReqData = pData;
}

void CProtoExRightInfo::SetProtoData_Ack(ProtoAckDataType *pData)
{
	m_pAckData = pData;
}

void CProtoExRightInfo::GetStructField4ParseJson_v0(bool bReqOrAck, int nLevel, const std::string &strStructName, VT_PROTO_FIELD &vtField, void *pStruct)
{
	CHECK_RET(nLevel >= 0 && pStruct != NULL && vtField.empty(), NORET);

	if ( nLevel == 0 && bReqOrAck )
	{
		ProtoReqDataType *pReqData = (ProtoReqDataType *)pStruct;		
		CProtoParseBase::GetProtoHeadField_Req(vtField, pReqData->head);

		PROTO_FIELD field;
		CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, KEY_REQ_PARAM, &pReqData->body);
		vtField.push_back(field);
	}
	else if ( nLevel == 0 && !bReqOrAck )
	{
		ProtoReqDataType *pAckData = (ProtoReqDataType *)pStruct;		
		CProtoParseBase::GetProtoHeadField_Ack(vtField, pAckData->head);

		PROTO_FIELD field;
		CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, KEY_ACK_DATA, &pAckData->body);
		vtField.push_back(field);
	}
	else if ( nLevel == 1 && bReqOrAck && strStructName == KEY_REQ_PARAM )
	{
		GetProtoReqBodyFields(vtField, pStruct);
	}	
	else if ( nLevel == 1 && !bReqOrAck && strStructName == KEY_ACK_DATA )
	{
		GetProtoAckBodyFields(vtField, pStruct);
	}
	//tomodify 7-0 如果协议有其它内嵌结构，在这继续添加	
	else if ( nLevel == 3 && bReqOrAck && strStructName == "StockArr" )
	{
		static BOOL arOptional[] = {
			FALSE, FALSE, 
		};
		static EProtoFildType arFieldType[] = {
			ProtoFild_Int32, ProtoFild_StringA,
		};
		static LPCSTR arFieldKey[] = {
			"Market",	"StockCode",
		};

		ExRightInfoReqItem *pReqItem = (ExRightInfoReqItem *)pStruct;
		void *arPtr[] = {
			&pReqItem->nStockMarket, &pReqItem->strStockCode,
		};

		CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

		PROTO_FIELD field;
		for ( int n = 0; n < _countof(arOptional); n++ )
		{
			bool bFill = CProtoParseBase::FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
			CHECK_OP(bFill, continue);
			vtField.push_back(field);
		}
	}
	else if ( nLevel == 3 && !bReqOrAck && strStructName == "ExRightInfoArr" )
	{
		static BOOL arOptional[] = {
			FALSE, FALSE, FALSE, 
			FALSE, FALSE, FALSE, 
			FALSE, FALSE, FALSE, 

			FALSE, FALSE, FALSE, 
			FALSE, FALSE, FALSE, 
		};
		static EProtoFildType arFieldType[] = {
			ProtoFild_StringA, ProtoFild_Int32, ProtoFild_StringA,
			ProtoFild_Int32, ProtoFild_Int64, ProtoFild_Int32, 
			ProtoFild_Int32, ProtoFild_Int32, ProtoFild_Int64, 
			ProtoFild_Int32, ProtoFild_Int64, ProtoFild_Int64, 
			ProtoFild_Int64, ProtoFild_Int64, ProtoFild_Int64,
		};
		static LPCSTR arFieldKey[] = {
			"StockCode",  "Market", "ExDivDate", 
			"SplitRatio", "PerCashDiv", "PerShareDivRatio",
			"PerShareTransRatio", "AllotmentRatio", "AllotmentPrice",
			"StkSpoRatio", "StkSpoPrice", "ForwardAdjFactorA",
			"ForwardAdjFactorB", "BackwardAdjFactorA", "BackwarAdjFactorB",
		};

		ExRightInfoAckItem *pAckItem = (ExRightInfoAckItem *)pStruct;
		void *arPtr[] = {
			&pAckItem->strStockCode, &pAckItem->nStockMarket, &pAckItem->str_ex_date,
			&pAckItem->split_ratio, &pAckItem->per_cash_div, &pAckItem->per_share_ratio,
			&pAckItem->per_share_trans_ratio, &pAckItem->allotment_ratio, &pAckItem->allotment_price,

			&pAckItem->stk_spo_ratio, &pAckItem->stk_spo_price, &pAckItem->fwd_factor_a,
			&pAckItem->fwd_factor_b, &pAckItem->bwd_factor_a, &pAckItem->bwd_factor_b,
		};

		CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

		PROTO_FIELD field;
		for ( int n = 0; n < _countof(arOptional); n++ )
		{
			bool bFill = CProtoParseBase::FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
			CHECK_OP(bFill, continue);
			vtField.push_back(field);
		}
	}	
}

void CProtoExRightInfo::GetStructField4ParseJson(bool bReqOrAck, int nLevel, const std::string &strStructName, VT_PROTO_FIELD &vtField, void *pStruct)
{
	GetStructField4ParseJson_v0(bReqOrAck, nLevel, strStructName, vtField, pStruct);
	return ;

	CHECK_RET(nLevel >= 0 && pStruct != NULL && vtField.empty(), NORET);

	if ( nLevel == 0 && bReqOrAck )
	{
		static BOOL arOptional[] = {
			FALSE, FALSE, 
		};
		static EProtoFildType arFieldType[] = {
			ProtoFild_Struct, ProtoFild_Struct,
		};
		static LPCSTR arFieldKey[] = {
			FIELD_KEY_HEAD,	FIELD_KEY_BODY,
		};

		ProtoReqDataType *pReqData = (ProtoReqDataType *)pStruct;		
		void *arPtr[] = {
			&pReqData->head, &pReqData->body,
		};

		CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

		PROTO_FIELD field;
		for ( int n = 0; n < _countof(arOptional); n++ )
		{
			bool bFill = CProtoParseBase::FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
			CHECK_OP(bFill, continue);
			vtField.push_back(field);
		}
	}
	else if ( nLevel == 0 && !bReqOrAck )
	{
		static BOOL arOptional[] = {
			FALSE, TRUE, 
		};
		static EProtoFildType arFieldType[] = {
			ProtoFild_Struct, ProtoFild_Struct,
		};
		static LPCSTR arFieldKey[] = {
			FIELD_KEY_HEAD,	FIELD_KEY_BODY,
		};

		ProtoReqDataType *pAckData = (ProtoReqDataType *)pStruct;		
		void *arPtr[] = {
			&pAckData->head, &pAckData->body,
		};

		CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

		PROTO_FIELD field;
		for ( int n = 0; n < _countof(arOptional); n++ )
		{
			bool bFill = CProtoParseBase::FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
			CHECK_OP(bFill, continue);
			vtField.push_back(field);
		}
	}
	else if ( nLevel == 1 && bReqOrAck && strStructName == FIELD_KEY_HEAD )
	{
		ProtoHead *pHead = (ProtoHead *)pStruct;
		CProtoParseBase::GetProtoHeadField_Req(vtField, *pHead);
	}
	else if ( nLevel == 1 && !bReqOrAck && strStructName == FIELD_KEY_HEAD )
	{
		ProtoHead *pHead = (ProtoHead *)pStruct;
		CProtoParseBase::GetProtoHeadField_Ack(vtField, *pHead);
	}	
	else if ( nLevel == 1 && bReqOrAck && strStructName == FIELD_KEY_BODY )
	{
		GetProtoReqBodyFields(vtField, pStruct);
	}	
	else if ( nLevel == 1 && !bReqOrAck && strStructName == FIELD_KEY_BODY )
	{
		GetProtoAckBodyFields(vtField, pStruct);
	}
	//tomodify 7-1 如果协议有其它内嵌结构，在这继续添加
}

void CProtoExRightInfo::GetStructField4MakeJson(bool bReqOrAck, int nLevel, const std::string &strStructName, VT_PROTO_FIELD &vtField, void *pStruct)
{
	GetStructField4ParseJson_v0(bReqOrAck, nLevel, strStructName, vtField, pStruct);
	return ;

	CHECK_RET(nLevel >= 0 && pStruct != NULL && vtField.empty(), NORET);

	if ( nLevel == 0 && bReqOrAck )
	{
		static BOOL arOptional[] = {
			FALSE, FALSE, 
		};
		static EProtoFildType arFieldType[] = {
			ProtoFild_Struct, ProtoFild_Struct,
		};
		static LPCSTR arFieldKey[] = {
			FIELD_KEY_HEAD,	FIELD_KEY_BODY,
		};

		ProtoReqDataType *pReqData = (ProtoReqDataType *)pStruct;		
		void *arPtr[] = {
			&pReqData->head, &pReqData->body,
		};

		CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

		PROTO_FIELD field;
		for ( int n = 0; n < _countof(arOptional); n++ )
		{
			bool bFill = CProtoParseBase::FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
			CHECK_OP(bFill, continue);
			vtField.push_back(field);
		}
	}
	else if ( nLevel == 0 && !bReqOrAck )
	{
		static BOOL arOptional[] = {
			FALSE, TRUE, 
		};
		static EProtoFildType arFieldType[] = {
			ProtoFild_Struct, ProtoFild_Struct,
		};
		static LPCSTR arFieldKey[] = {
			FIELD_KEY_HEAD,	FIELD_KEY_BODY,
		};

		ProtoReqDataType *pAckData = (ProtoReqDataType *)pStruct;		
		void *arPtr[] = {
			&pAckData->head, &pAckData->body,
		};

		CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
		CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

		PROTO_FIELD field;
		for ( int n = 0; n < _countof(arOptional); n++ )
		{
			bool bFill = CProtoParseBase::FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
			CHECK_OP(bFill, continue);
			vtField.push_back(field);
		}
	}
	else if ( nLevel == 1 && bReqOrAck && strStructName == FIELD_KEY_HEAD )
	{
		ProtoHead *pHead = (ProtoHead *)pStruct;
		CProtoParseBase::GetProtoHeadField_Req(vtField, *pHead);
	}
	else if ( nLevel == 1 && !bReqOrAck && strStructName == FIELD_KEY_HEAD )
	{
		ProtoHead *pHead = (ProtoHead *)pStruct;
		CProtoParseBase::GetProtoHeadField_Ack(vtField, *pHead);
	}
	else if ( nLevel == 1 && bReqOrAck && strStructName == FIELD_KEY_BODY )
	{
		GetProtoReqBodyFields(vtField, pStruct);
	}	
	else if ( nLevel == 1 && !bReqOrAck && strStructName == FIELD_KEY_BODY )
	{
		GetProtoAckBodyFields(vtField, pStruct);
	}
	//tomodify 7-2 如果协议有其它内嵌结构，在这继续添加
}

//tomodify 3 得到req body的所有字段
void  CProtoExRightInfo::GetProtoReqBodyFields(VT_PROTO_FIELD &vtField, void *pStruct)
{
	ProtoReqBodyType *pBody = (ProtoReqBodyType *)pStruct;
	CHECK_RET(pBody != NULL, NORET);

	static BOOL arOptional[] = {
		FALSE, 
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Vector
	};
	static LPCSTR arFieldKey[] = {
		"StockArr", 
	};

	void *arPtr[] = {
		&pBody->vtReqExRightInfo,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		bool bFill = CProtoParseBase::FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
		CHECK_OP(bFill, continue);
		vtField.push_back(field);
	}
}

//tomodify 4 得到ack body的所有字段
void  CProtoExRightInfo::GetProtoAckBodyFields(VT_PROTO_FIELD &vtField, void *pStruct)
{
	ProtoAckBodyType *pBody = (ProtoAckBodyType*)pStruct;
	CHECK_RET(pBody != NULL, NORET);

	static BOOL arOptional[] = {
		FALSE, 		
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Vector,		
	};
	static LPCSTR arFieldKey[] = {
		"ExRightInfoArr",
	};

	void *arPtr[] = {
		&pBody->vtAckExRightInfo, 
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		bool bFill = CProtoParseBase::FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
		CHECK_OP(bFill, continue);
		vtField.push_back(field);
	}
}

//tomodify 5 如果协议有其它内嵌数组，在这更改实现
void CProtoExRightInfo::GetArrayField4ParseJson(bool bReqOrAck, int nLevel, const std::string &strArrayName, int nJsnArrSize, VT_PROTO_FIELD &vtField, void *pVector)
{
	CHECK_RET(nLevel >= 2 && nJsnArrSize >= 0 && pVector != NULL, NORET);

	if ( bReqOrAck && nLevel == 2 && strArrayName == "StockArr" )
	{
		vtField.clear();

		//注意vector的类型
		VT_REQ_EXRIGHTINFO &vtReqExRightInfo = *(VT_REQ_EXRIGHTINFO*)pVector;
		vtReqExRightInfo.resize(nJsnArrSize);
		for ( int n = 0; n < nJsnArrSize; n++ )
		{
			PROTO_FIELD field;
			CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, strArrayName, &vtReqExRightInfo[n]);
			vtField.push_back(field);
		}
	}
	else if ( !bReqOrAck && nLevel == 2 && strArrayName == "ExRightInfoArr" )
	{
		vtField.clear();

		//注意vector的类型
		VT_ACK_EXRIGHTINFO &vtAckExRightInfo = *(VT_ACK_EXRIGHTINFO*)pVector;
		vtAckExRightInfo.resize(nJsnArrSize);
		for ( int n = 0; n < nJsnArrSize; n++ )
		{
			PROTO_FIELD field;
			CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, strArrayName, &vtAckExRightInfo[n]);
			vtField.push_back(field);
		}
	}
}

//tomodify 6 如果协议有其它内嵌数组，在这更改实现
void CProtoExRightInfo::GetArrayField4MakeJson(bool bReqOrAck, int nLevel, const std::string &strArrayName, VT_PROTO_FIELD &vtField, void *pVector)
{
	CHECK_RET(nLevel >= 2 && pVector != NULL, NORET);

	if ( bReqOrAck && nLevel == 2 && strArrayName == "StockArr" )
	{
		vtField.clear();

		//注意vector的类型
		VT_REQ_EXRIGHTINFO &vtReqExRightInfo = *(VT_REQ_EXRIGHTINFO*)pVector;		
		for ( int n = 0; n < (int)vtReqExRightInfo.size(); n++ )
		{
			PROTO_FIELD field;
			CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, strArrayName, &vtReqExRightInfo[n]);
			vtField.push_back(field);
		}
	}else if ( !bReqOrAck && nLevel == 2 && strArrayName == "ExRightInfoArr" )
	{
		vtField.clear();

		//注意vector的类型
		VT_ACK_EXRIGHTINFO &vtAckExRightInfo = *(VT_ACK_EXRIGHTINFO*)pVector;		
		for ( int n = 0; n < (int)vtAckExRightInfo.size(); n++ )
		{
			PROTO_FIELD field;
			CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, strArrayName, &vtAckExRightInfo[n]);
			vtField.push_back(field);
		}
	}	
}
