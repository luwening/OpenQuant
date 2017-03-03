#include "stdafx.h"
#include "ProtoParseBase.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CProtoParseBase::CProtoParseBase()
{

}

CProtoParseBase::~CProtoParseBase()	
{

}

bool  CProtoParseBase::ConvBuffer2Json(const char *pBuf, int nBufLen, Json::Value &jsnVal)
{
	CHECK_RET(pBuf && nBufLen >= 2, false);

	int nValidLen = nBufLen;
	char chTmp = pBuf[nValidLen - 1];
	while ( chTmp == 0xd || chTmp == 0xA )
	{
		nValidLen--;
		if ( nValidLen < 2 )
		{
			CHECK_RET(false, false);
		}
		chTmp = pBuf[nValidLen - 1];
	}

	Json::Reader reader;
	bool bRet = reader.parse(pBuf, pBuf + nValidLen, jsnVal);
	return bRet;
}

bool  CProtoParseBase::ConvJson2String(const Json::Value &jsnVal, std::string &strOut, bool bAppendCRLF)
{
	Json::FastWriter writer;
	strOut = writer.write(jsnVal);

	if ( strOut.empty() )
		return false;
	
	strOut.append("\r\n\r\n");
	return true;
}

int	CProtoParseBase::GetProtoID(const Json::Value &jsnVal)
{
	CppJsonConv conv;
	conv.SetJsonValue(jsnVal);
	int nProID = 0;
	bool bSuc = conv.GetInt32Value("Protocol", nProID);
	CHECK_OP(bSuc, NOOP);
	return nProID;
}

bool CProtoParseBase::ParseProtoHead_Req(const Json::Value &jsnVal, ProtoHead &head)
{
	//将不用解析的项初始化
	head.ddwErrCode = 0;
	head.strErrDesc = "";

	VT_PROTO_FIELD vtField;	
	GetProtoHeadField_Req(vtField, head);
	
	return ParseProtoFields(jsnVal, vtField);
}

bool CProtoParseBase::ParseProtoHead_Ack(const Json::Value &jsnVal, ProtoHead &head)
{	
	VT_PROTO_FIELD vtField;
	GetProtoHeadField_Ack(vtField, head);

	return ParseProtoFields(jsnVal, vtField);
}

bool CProtoParseBase::MakeProtoHead_Req(Json::Value &jsnVal, const ProtoHead &head)
{
	VT_PROTO_FIELD vtField;
	ProtoHead hd = const_cast<ProtoHead&>(head);
	GetProtoHeadField_Req(vtField, hd);

	return MakeProtoFields(jsnVal, vtField);
}

bool CProtoParseBase::MakeProtoHead_Ack(Json::Value &jsnVal, const ProtoHead &head)
{
	VT_PROTO_FIELD vtField;
	ProtoHead hd = const_cast<ProtoHead&>(head);
	GetProtoHeadField_Ack(vtField, hd);

	return MakeProtoFields(jsnVal, vtField);
}

bool CProtoParseBase::ParseProtoFields(const Json::Value &jsnVal, const VT_PROTO_FIELD &vtField)
{
	CppJsonConv conv;
	conv.SetJsonValue(jsnVal);

	bool bRet = true;
	VT_PROTO_FIELD::const_iterator it = vtField.begin();
	for ( ; it != vtField.end(); ++it )
	{
		const PROTO_FIELD &field = *it;
		bool bFieldSuc = false;	

		switch ( field.eFieldType )
		{
		case ProtoFild_Int32:
			if ( field.pInt32 )
				bFieldSuc = conv.GetInt32Value(field.strFieldKey, *field.pInt32);
			break;

		case ProtoFild_Int64:
			if ( field.pInt32 )
				bFieldSuc = conv.GetInt64Value(field.strFieldKey, *field.pInt64);
			break;

		case ProtoFild_StringA:
			if ( field.pStrA )
				bFieldSuc = conv.GetStringValueA(field.strFieldKey, *field.pStrA);
			break;

		case ProtoFild_StringW:
			if ( field.pStrW )
				bFieldSuc = conv.GetStringValueW(field.strFieldKey, *field.pStrW);
			break;

		default:
			CHECK_OP(FALSE, NOOP);
			break;
		}

		if ( !field.bOptional && !bFieldSuc )
		{
			bRet = false;
			CHECK_OP(false, break);
		}		
	}

	return bRet;
}

bool CProtoParseBase::MakeProtoFields(Json::Value &jsnVal, const VT_PROTO_FIELD &vtField)
{
	CppJsonConv conv;
	conv.SetJsonValue(jsnVal);

	bool bRet = true;
	VT_PROTO_FIELD::const_iterator it = vtField.begin();
	for ( ; it != vtField.end(); ++it )
	{
		const PROTO_FIELD &field = *it;
		bool bFieldSuc = false;	

		switch ( field.eFieldType )
		{
		case ProtoFild_Int32:
			if ( field.pInt32 )
				bFieldSuc = conv.SetInt32Value(field.strFieldKey, *field.pInt32);
			break;

		case ProtoFild_Int64:
			if ( field.pInt32 )
				bFieldSuc = conv.SetInt64Value(field.strFieldKey, *field.pInt64);
			break;

		case ProtoFild_StringA:
			if ( field.pStrA )
				bFieldSuc = conv.SetStringValueA(field.strFieldKey, *field.pStrA);
			break;

		case ProtoFild_StringW:
			if ( field.pStrW )
				bFieldSuc = conv.SetStringValueW(field.strFieldKey, *field.pStrW);
			break;

		default:
			CHECK_OP(FALSE, NOOP);
			break;
		}

		if ( !field.bOptional && !bFieldSuc )
		{
			bRet = false;
			CHECK_OP(false, break);
		}		
	}

	return bRet;
}

bool CProtoParseBase::ParseJsonProtoStruct(const Json::Value &jsnVal, bool bReqOrAck, const std::string &strStructName, void *pStruct, int nLevel)
{
	CHECK_RET(pStruct != NULL && nLevel >= 0, false);
	CHECK_RET(jsnVal.isObject() || jsnVal.isArray(), false);
	if ( nLevel == 0 )
	{
		CHECK_RET(jsnVal.isObject(), false);
	}
	else
	{
		CHECK_RET(strStructName != "", false);
	}
	if ( nLevel > 10 )
	{
		CHECK_RET(false, false);
	}

	bool bRet = true;
	int nNewLevel = nLevel + 1;
	if ( jsnVal.isArray() )
	{		
		int nArrSize = jsnVal.size();		
		VT_PROTO_FIELD vtArrField;
		GetArrayField4ParseJson(bReqOrAck, nLevel, strStructName, nArrSize, vtArrField, pStruct);
		CHECK_RET((int)vtArrField.size() == nArrSize, false);

		CppJsonConv conv;
		conv.SetJsonValue(jsnVal);
	
		for ( int n = 0; n < nArrSize; n++ )
		{
			bool bFieldSuc = false;	
			const PROTO_FIELD &fldItem = vtArrField[n];
			switch ( fldItem.eFieldType )
			{
			case ProtoFild_Int32:
				if ( fldItem.pInt32 )
					bFieldSuc = conv.GetArrItemInt32Value(n, *fldItem.pInt32);
				break;

			case ProtoFild_Int64:
				if ( fldItem.pInt32 )
					bFieldSuc = conv.GetArrItemInt64Value(n, *fldItem.pInt64);
				break;

			case ProtoFild_StringA:
				if ( fldItem.pStrA )
					bFieldSuc = conv.GetArrItemStringValueA(n, *fldItem.pStrA);
				break;

			case ProtoFild_StringW:
				if ( fldItem.pStrW )
					bFieldSuc = conv.GetArrItemStringValueW(n, *fldItem.pStrW);
				break;

			case ProtoFild_Struct:
				{
					Json::Value jsnItem = jsnVal[n];
					CHECK_RET(jsnItem.isObject(), false);
					bFieldSuc = ParseJsonProtoStruct(jsnItem, bReqOrAck, strStructName, fldItem.pStruct, nNewLevel);
				}			
				break;
			case ProtoFild_Vector:
				{
					Json::Value jsnItem = jsnVal[n];
					CHECK_RET(jsnItem.isArray(), false);
					bFieldSuc = ParseJsonProtoStruct(jsnItem, bReqOrAck, strStructName, fldItem.pVector, nNewLevel);
				}			
				break;
			default:
				CHECK_OP(false, NOOP);
				break;					
			}
			if ( !fldItem.bOptional && !bFieldSuc )
			{
				bRet = false;
				CHECK_OP(false, break);
			}
		}				
	}
	else
	{
		CppJsonConv conv;
		conv.SetJsonValue(jsnVal);

		VT_PROTO_FIELD vtField;
		GetStructField4ParseJson(bReqOrAck, nLevel, strStructName, vtField, pStruct);
		
		VT_PROTO_FIELD::const_iterator it = vtField.begin();
		for ( ; it != vtField.end(); ++it )
		{
			const PROTO_FIELD &field = *it;
			bool bFieldSuc = false;	

			switch ( field.eFieldType )
			{
			case ProtoFild_Int32:
				if ( field.pInt32 )
					bFieldSuc = conv.GetInt32Value(field.strFieldKey, *field.pInt32);
				break;

			case ProtoFild_Int64:
				if ( field.pInt32 )
					bFieldSuc = conv.GetInt64Value(field.strFieldKey, *field.pInt64);
				break;

			case ProtoFild_StringA:
				if ( field.pStrA )
					bFieldSuc = conv.GetStringValueA(field.strFieldKey, *field.pStrA);
				break;

			case ProtoFild_StringW:
				if ( field.pStrW )
					bFieldSuc = conv.GetStringValueW(field.strFieldKey, *field.pStrW);
				break;

			case ProtoFild_Struct:
				{
					const Json::Value &jsnEmbedStruct = jsnVal[field.strFieldKey];
					CHECK_RET(jsnEmbedStruct.isObject(), false);
					bFieldSuc = ParseJsonProtoStruct(jsnEmbedStruct, bReqOrAck, field.strFieldKey, field.pStruct, nNewLevel);
				}			
				break;

			case ProtoFild_Vector:
				{
					const Json::Value &jsnEmbedVector = jsnVal[field.strFieldKey];
					CHECK_RET(jsnEmbedVector.isArray(), false);
					bFieldSuc = ParseJsonProtoStruct(jsnEmbedVector, bReqOrAck, field.strFieldKey, field.pVector, nNewLevel);
				}
				break;
			default:
				CHECK_OP(FALSE, NOOP);
				break;
			}
			if ( !field.bOptional && !bFieldSuc )
			{
				bRet = false;
				CHECK_OP(false, break);
			}
		}
	}

	return bRet;
}

bool CProtoParseBase::MakeJsonProtoStruct(Json::Value &jsnVal, bool bReqOrAck, const std::string &strStructName, void *pStruct, int nLevel, bool bObjectOrArray)
{
	CHECK_RET(nLevel >= 0 && pStruct != NULL, false);
	if ( nLevel == 0 )
	{
		CHECK_RET(jsnVal.isNull() && bObjectOrArray, false);
	}
	else
	{
		CHECK_RET(strStructName != "", false);
	}
	if ( nLevel > 10 )
	{
		CHECK_RET(false, false);
	}

	bool bRet = true;
	int nNewLevel = nLevel + 1;
	if ( bObjectOrArray )
	{
		CppJsonConv conv;
		conv.SetJsonValue(jsnVal);
		
		VT_PROTO_FIELD vtField;
		GetStructField4MakeJson(bReqOrAck, nLevel, strStructName, vtField, pStruct);
		VT_PROTO_FIELD::const_iterator it = vtField.begin();
		for ( ; it != vtField.end(); ++it )
		{
			const PROTO_FIELD &field = *it;
			bool bFieldSuc = false;	

			switch ( field.eFieldType )
			{
			case ProtoFild_Int32:
				if ( field.pInt32 )
					bFieldSuc = conv.SetInt32Value(field.strFieldKey, *field.pInt32);
				break;

			case ProtoFild_Int64:
				if ( field.pInt32 )
					bFieldSuc = conv.SetInt64Value(field.strFieldKey, *field.pInt64);
				break;

			case ProtoFild_StringA:
				if ( field.pStrA )
					bFieldSuc = conv.SetStringValueA(field.strFieldKey, *field.pStrA);
				break;

			case ProtoFild_StringW:
				if ( field.pStrW )
					bFieldSuc = conv.SetStringValueW(field.strFieldKey, *field.pStrW);
				break;
			case ProtoFild_Struct:
				{
					Json::Value &jsnEmbed = jsnVal[field.strFieldKey];
					bFieldSuc = MakeJsonProtoStruct(jsnEmbed, bReqOrAck, field.strFieldKey, field.pStruct, nNewLevel, true);
				}
				break;
			case ProtoFild_Vector:
				{
					Json::Value &jsnEmbed = jsnVal[field.strFieldKey];
					bFieldSuc = MakeJsonProtoStruct(jsnEmbed, bReqOrAck, field.strFieldKey, field.pVector, nNewLevel, false);
				}				
				break;
			default:
				CHECK_OP(FALSE, NOOP);
				break;
			}

			if ( !field.bOptional && !bFieldSuc )
			{
				bRet = false;
				CHECK_OP(false, break);
			}		
		}		
	}
	else
	{
		CppJsonConv conv;
		conv.SetJsonValue(jsnVal);
		
		VT_PROTO_FIELD vtField;
		GetArrayField4MakeJson(bReqOrAck, nLevel, strStructName, vtField, pStruct);
		VT_PROTO_FIELD::const_iterator it = vtField.begin();
		for ( ; it != vtField.end(); ++it )
		{
			const PROTO_FIELD &field = *it;
			bool bFieldSuc = false;	
			int nArrIndex = int(it - vtField.begin());

			switch ( field.eFieldType )
			{
			case ProtoFild_Int32:
				if ( field.pInt32 )
					bFieldSuc = conv.SetArrItemInt32Value(nArrIndex, *field.pInt32);
				break;

			case ProtoFild_Int64:
				if ( field.pInt32 )
					bFieldSuc = conv.SetArrItemInt64Value(nArrIndex, *field.pInt64);
				break;

			case ProtoFild_StringA:
				if ( field.pStrA )
					bFieldSuc = conv.SetArrItemStringValueA(nArrIndex, *field.pStrA);
				break;

			case ProtoFild_StringW:
				if ( field.pStrW )
					bFieldSuc = conv.SetArrItemStringValueW(nArrIndex, *field.pStrW);
				break;
			case ProtoFild_Struct:
				{
					Json::Value &jsnEmbed = jsnVal[nArrIndex];
					bFieldSuc = MakeJsonProtoStruct(jsnEmbed, bReqOrAck, strStructName, field.pStruct, nNewLevel, true);
				}
				break;
			case ProtoFild_Vector:
				{
					Json::Value &jsnEmbed = jsnVal[nArrIndex];
					bFieldSuc = MakeJsonProtoStruct(jsnEmbed, bReqOrAck, strStructName, field.pVector, nNewLevel, false);
				}
				break;
			default:
				CHECK_OP(FALSE, NOOP);
				break;
			}

			if ( !field.bOptional && !bFieldSuc )
			{
				bRet = false;
				CHECK_OP(false, break);
			}		
		}
	}	

	return bRet;
}

void CProtoParseBase::GetProtoHeadField_Ack(VT_PROTO_FIELD &vtField, const ProtoHead &head)
{
	static BOOL arOptional[] = {
		FALSE, FALSE, 
		FALSE, TRUE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_Int32, 
		ProtoFild_Int64, ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"Version",	"Protocol",		
		"ErrCode",	"ErrDesc",
	};

	ProtoHead &hd = const_cast<ProtoHead &>(head);
	void *arPtr[] = {
		&hd.nProtoVer,	&hd.nProtoID, 
		&hd.ddwErrCode, &hd.strErrDesc,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		bool bFill = FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
		CHECK_OP(bFill, continue);
		vtField.push_back(field);
	}	
}


void CProtoParseBase::GetProtoHeadField_Req(VT_PROTO_FIELD &vtField, const ProtoHead &head)
{
	static BOOL arOptional[] = {
		FALSE, FALSE, 		
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_Int32, 
	};
	static LPCSTR arFieldKey[] = {
		"Version",	"Protocol",
	};

	ProtoHead &hd = const_cast<ProtoHead &>(head);
	void *arPtr[] = {
		&hd.nProtoVer,	&hd.nProtoID, 
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		bool bFill = FillFieldMembers(field, arOptional[n], arFieldType[n], arFieldKey[n], arPtr[n]);
		CHECK_OP(bFill, continue);
		vtField.push_back(field);
	}	
}

bool CProtoParseBase::FillFieldMembers(PROTO_FIELD &field, BOOL bOptional, EProtoFildType eFieldType, const std::string &strKey, void *pValue)
{
	CHECK_RET(pValue != NULL, false);
	field.bOptional = bOptional;
	field.eFieldType = eFieldType;
	field.strFieldKey = strKey;
	switch (field.eFieldType)
	{
	case ProtoFild_Int32:
		field.pInt32 = (int*)pValue;
		break;
	case ProtoFild_Int64:
		field.pInt64 = (INT64*)pValue;
		break;
	case ProtoFild_StringA:
		field.pStrA = (std::string*)pValue;
		break;
	case ProtoFild_StringW:
		field.pStrW = (std::wstring*)pValue;
		break;
	case ProtoFild_Struct:
		field.pStruct = pValue;
		break;
	case ProtoFild_Vector:
		field.pVector = pValue;
		break;
	default:
		CHECK_RET(FALSE, false);
		break;
	}
	return true;
}