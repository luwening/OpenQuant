#include "stdafx.h"
#include "ProtoSnapshot.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json_op.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProtoSnapshot::CProtoSnapshot()
{
	m_pReqData = NULL;
	m_pAckData = NULL;
}

CProtoSnapshot::~CProtoSnapshot()
{

}

bool CProtoSnapshot::ParseJson_Req(const Json::Value &jsnVal)
{
	CHECK_RET(m_pReqData != NULL, false);

	bool bSuc = ParseJsonProtoStruct(jsnVal, true, "", m_pReqData);
	return bSuc;
}

bool CProtoSnapshot::ParseJson_Ack(const Json::Value &jsnVal)
{
	CHECK_RET(m_pAckData != NULL, false);

	bool bSuc = ParseJsonProtoStruct(jsnVal, false, "", m_pAckData);
	return bSuc;
}


bool CProtoSnapshot::MakeJson_Req(Json::Value &jsnVal)
{
	CHECK_RET(m_pReqData != NULL, false);

	bool bSuc = MakeJsonProtoStruct(jsnVal, true, "", m_pReqData);
	return bSuc;
}

bool CProtoSnapshot::MakeJson_Ack(Json::Value &jsnVal)
{
	CHECK_RET(m_pAckData != NULL, false);

	bool bSuc = MakeJsonProtoStruct(jsnVal, false, "", m_pAckData);
	return bSuc;
}

void CProtoSnapshot::SetProtoData_Req(ProtoReqDataType *pData)
{
	m_pReqData = pData;
}

void CProtoSnapshot::SetProtoData_Ack(ProtoAckDataType *pData)
{
	m_pAckData = pData;
}

void CProtoSnapshot::GetStructField4ParseJson_v0(bool bReqOrAck, int nLevel, const std::string &strStructName, VT_PROTO_FIELD &vtField, void *pStruct)
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

		SnapshotReqItem *pReqItem = (SnapshotReqItem *)pStruct;
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
	else if ( nLevel == 3 && !bReqOrAck && strStructName == "SnapshotArr" )
	{
		static BOOL arOptional[] = {
			FALSE, FALSE, FALSE, 
			FALSE, FALSE, FALSE, 
			FALSE, FALSE, FALSE, 

			FALSE, FALSE, FALSE, 
			FALSE, FALSE, FALSE, 
			FALSE, FALSE, TRUE,
			
			TRUE, TRUE, TRUE,

			TRUE, TRUE, TRUE,
			TRUE, TRUE, TRUE,
			TRUE, TRUE, TRUE,
			TRUE, TRUE, TRUE,
			TRUE, TRUE, TRUE,
		};
		static EProtoFildType arFieldType[] = {
			ProtoFild_Int64, ProtoFild_StringA, ProtoFild_Int32,
			ProtoFild_Int32, ProtoFild_Int64, ProtoFild_Int64,
			ProtoFild_Int64, ProtoFild_Int64, ProtoFild_Int64,

			ProtoFild_Int64, ProtoFild_Int64, ProtoFild_Int64,
			ProtoFild_Int64, ProtoFild_Int64, ProtoFild_Int64,
			ProtoFild_Int32, ProtoFild_Int32, ProtoFild_Int64, 

			ProtoFild_Int64, ProtoFild_Int32, ProtoFild_StringA,

			ProtoFild_Int32, ProtoFild_Int32, ProtoFild_Int32,
			ProtoFild_Int64, ProtoFild_StringA, ProtoFild_StringA,
			ProtoFild_StringA, ProtoFild_Int32, ProtoFild_Int64,
			ProtoFild_Int64, ProtoFild_Int64, ProtoFild_Int32,
			ProtoFild_Int32, ProtoFild_Int32, ProtoFild_Int32,

		};
		static LPCSTR arFieldKey[] = { 
			"StockID",	"StockCode",  "MarketType",
			"InstrumentType", "LastClose",  "NominalPrice",
			"OpenPrice",  "UpdateTime", "SuspendFlag",

			"ListingStatus", "ListingDate", "SharesTraded",
			"Turnover",  "HighestPrice", "LowestPrice",
			"TurnoverRatio", "RetErrCode", "TotalMarketVal",

			"CircularMarketVal", "LotSize", "UpdateTimeStr",

			//涡轮信息
			"Wrt_Valid", "Wrt_Type", "Wrt_ConversionRatio", 
			"Wrt_StrikePrice", "Wrt_MaturityDateStr", "Wrt_EndTradeDateStr", 
			"Wrt_OwnerStockCode", "Wrt_OwnerMarketType", "Wrt_RecoveryPrice", 
			"Wrt_StreetVol", "Wrt_IssueVol", "Wrt_StreetRatio",
			"Wrt_Delta", "Wrt_ImpliedVolatility", "Wrt_Premium",
		};

		SnapshotAckItem *pAckItem = (SnapshotAckItem *)pStruct;
		void *arPtr[] = {
			&pAckItem->nStockID, &pAckItem->strStockCode, &pAckItem->nStockMarket,
			&pAckItem->instrument_type, &pAckItem->last_close_price, &pAckItem->nominal_price,
			&pAckItem->open_price, &pAckItem->update_time, &pAckItem->suspend_flag,

			&pAckItem->listing_status, &pAckItem->listing_date, &pAckItem->shares_traded,
			&pAckItem->turnover, &pAckItem->highest_price, &pAckItem->lowest_price,
			&pAckItem->turnover_ratio, &pAckItem->ret_err, &pAckItem->nTatalMarketVal, 
			
			&pAckItem->nCircularMarketVal, &pAckItem->nLostSize, &pAckItem->strUpdateTime,

			&pAckItem->stWrtData.bDataValid, &pAckItem->stWrtData.nWarrantType, &pAckItem->stWrtData.nConversionRatio,
			&pAckItem->stWrtData.nStrikePrice, &pAckItem->stWrtData.strMaturityData, &pAckItem->stWrtData.strEndtradeDate,
			&pAckItem->stWrtData.strOwnerStockCode, &pAckItem->stWrtData.nOwnerStockMarket, &pAckItem->stWrtData.nRecoveryPrice,
			&pAckItem->stWrtData.nStreetVol, &pAckItem->stWrtData.nIssueVol, &pAckItem->stWrtData.nStreetRatio, 
			&pAckItem->stWrtData.nDelta, &pAckItem->stWrtData.nImpliedVolatility, &pAckItem->stWrtData.nPremium,
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

void CProtoSnapshot::GetStructField4ParseJson(bool bReqOrAck, int nLevel, const std::string &strStructName, VT_PROTO_FIELD &vtField, void *pStruct)
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

void CProtoSnapshot::GetStructField4MakeJson(bool bReqOrAck, int nLevel, const std::string &strStructName, VT_PROTO_FIELD &vtField, void *pStruct)
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
void  CProtoSnapshot::GetProtoReqBodyFields(VT_PROTO_FIELD &vtField, void *pStruct)
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
		&pBody->vtReqSnapshot,
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
void  CProtoSnapshot::GetProtoAckBodyFields(VT_PROTO_FIELD &vtField, void *pStruct)
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
		"SnapshotArr",
	};

	void *arPtr[] = {
		&pBody->vtSnapshot, 
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
void CProtoSnapshot::GetArrayField4ParseJson(bool bReqOrAck, int nLevel, const std::string &strArrayName, int nJsnArrSize, VT_PROTO_FIELD &vtField, void *pVector)
{
	CHECK_RET(nLevel >= 2 && nJsnArrSize >= 0 && pVector != NULL, NORET);

	if ( bReqOrAck && nLevel == 2 && strArrayName == "StockArr" )
	{
		vtField.clear();

		//注意vector的类型
		VT_REQ_SNAPSHOT &vtStockList = *(VT_REQ_SNAPSHOT*)pVector;
		vtStockList.resize(nJsnArrSize);
		for ( int n = 0; n < nJsnArrSize; n++ )
		{
			PROTO_FIELD field;
			CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, strArrayName, &vtStockList[n]);
			vtField.push_back(field);
		}
	}
	else if ( !bReqOrAck && nLevel == 2 && strArrayName == "SnapshotArr" )
	{
		vtField.clear();

		//注意vector的类型
		VT_ACK_SNAPSHOT &vtSnapshotList = *(VT_ACK_SNAPSHOT*)pVector;
		vtSnapshotList.resize(nJsnArrSize);
		for ( int n = 0; n < nJsnArrSize; n++ )
		{
			PROTO_FIELD field;
			CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, strArrayName, &vtSnapshotList[n]);
			vtField.push_back(field);
		}
	}
}

//tomodify 6 如果协议有其它内嵌数组，在这更改实现
void CProtoSnapshot::GetArrayField4MakeJson(bool bReqOrAck, int nLevel, const std::string &strArrayName, VT_PROTO_FIELD &vtField, void *pVector)
{
	CHECK_RET(nLevel >= 2 && pVector != NULL, NORET);

	if ( bReqOrAck && nLevel == 2 && strArrayName == "StockArr" )
	{
		vtField.clear();

		//注意vector的类型
		VT_REQ_SNAPSHOT &vtStockList = *(VT_REQ_SNAPSHOT*)pVector;		
		for ( int n = 0; n < (int)vtStockList.size(); n++ )
		{
			PROTO_FIELD field;
			CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, strArrayName, &vtStockList[n]);
			vtField.push_back(field);
		}
	}else if ( !bReqOrAck && nLevel == 2 && strArrayName == "SnapshotArr" )
	{
		vtField.clear();

		//注意vector的类型
		VT_ACK_SNAPSHOT &vtSnapshotList = *(VT_ACK_SNAPSHOT*)pVector;		
		for ( int n = 0; n < (int)vtSnapshotList.size(); n++ )
		{
			PROTO_FIELD field;
			CProtoParseBase::FillFieldMembers(field, FALSE, ProtoFild_Struct, strArrayName, &vtSnapshotList[n]);
			vtField.push_back(field);
		}
	}	
}
