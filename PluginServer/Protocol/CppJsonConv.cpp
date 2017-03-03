#include "stdafx.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json_op.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CppJsonConv::CppJsonConv()
{
	m_pJsonVal = NULL;
}

CppJsonConv::~CppJsonConv()
{

}

void  CppJsonConv::SetJsonValue(const Json::Value &jsnValue)
{
	m_pJsonVal = const_cast<Json::Value*>(&jsnValue);
}

bool CppJsonConv::GetInt64Value(const std::string &strUtfKey, INT64 &nRet)
{
	nRet = 0;
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);
	std::string strTmp;	
	jop.read(strUtfKey, strTmp);

	if ( !jop.good() )
		return false;

	std::wstring strUnicode;
	CA::UTF2Unicode(strTmp.c_str(), strUnicode);
	nRet = _wtoi64(strUnicode.c_str());
	return true;
}

bool CppJsonConv::GetInt32Value(const std::string &strUtfKey, int  &nRet)
{
	nRet = 0;
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);
	std::string strTmp;	
	jop.read(strUtfKey, strTmp);

	if ( !jop.good() )
		return false;

	std::wstring strUnicode;
	CA::UTF2Unicode(strTmp.c_str(), strUnicode);
	nRet = _wtoi(strUnicode.c_str());
	return true;
}

bool CppJsonConv::GetStringValueA(const std::string &strUtfKey,std::string &strRetUtf)
{
	strRetUtf.clear();
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);	
	jop.read(strUtfKey, strRetUtf);

	return jop.good();
}

bool CppJsonConv::GetStringValueW(const std::string &strUtfKey,std::wstring &strRet)
{
	strRet.clear();
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);
	std::string strTmp;	
	jop.read(strUtfKey, strTmp);

	if ( !jop.good() )
		return false;

	CA::UTF2Unicode(strTmp.c_str(), strRet);	
	return true;
}

bool CppJsonConv::GetJsonValue(const std::string &strUtfKey, Json::Value &jsnRet)
{
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);
	jop.read(strUtfKey, jsnRet);
	return true;
}


bool CppJsonConv::GetArrItemInt64Value(int nIndex, INT64 &nRet)
{
	nRet = 0;
	CHECK_RET(m_pJsonVal, false);

	const Json::Value &jsnItem = (*m_pJsonVal)[nIndex];
	if ( !jsnItem.isInt() )
	{
		return false;
	}
	
	nRet = jsnItem.asLargestInt();
	return true;
}

bool CppJsonConv::GetArrItemInt32Value(int nIndex, int  &nRet)
{
	nRet = 0;
	CHECK_RET(m_pJsonVal, false);

	const Json::Value &jsnItem = (*m_pJsonVal)[nIndex];
	if ( !jsnItem.isInt() )
	{
		return false;
	}

	nRet = jsnItem.asInt();
	return true;
}

bool CppJsonConv::GetArrItemStringValueA(int nIndex, std::string &strRetUtf)
{
	strRetUtf.clear();
	CHECK_RET(m_pJsonVal, false);

	const Json::Value &jsnItem = (*m_pJsonVal)[nIndex];
	if ( !jsnItem.isString() )
	{
		return false;
	}

	strRetUtf = jsnItem.asString();
	return true;
}

bool CppJsonConv::GetArrItemStringValueW(int nIndex, std::wstring &strRet)
{
	std::string strTmp;
	if ( GetArrItemStringValueA(nIndex, strTmp) )
	{
		CA::UTF2Unicode(strTmp.c_str(), strRet);
		return true;
	}	
	return false;
}

bool CppJsonConv::GetArrItemJsonValue(int nIndex, Json::Value &jsnRet)
{
	CHECK_RET(m_pJsonVal, false);

	const Json::Value &jsnItem = (*m_pJsonVal)[nIndex];
	if ( !jsnItem.isObject() )
	{
		return false;
	}

	jsnRet = jsnItem;
	return true;
}

bool CppJsonConv::SetInt64Value(const std::string &strUtfKey, INT64 nVal)
{	
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);
	wchar_t buf[64];
	_i64tow(nVal, buf, 10);

	std::string strUtfVal;
	CA::Unicode2UTF(buf, strUtfVal);
	jop.write(strUtfKey, strUtfVal);
	return jop.good();
}

bool CppJsonConv::SetInt32Value(const std::string &strUtfKey, int nVal)
{
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);
	wchar_t buf[64];
	_i64tow(nVal, buf, 10);

	std::string strUtfVal;
	CA::Unicode2UTF(buf, strUtfVal);
	jop.write(strUtfKey, strUtfVal);
	return jop.good();
}

bool CppJsonConv::SetStringValueA(const std::string &strUtfKey, const std::string &strValUtf)
{
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);
	jop.write(strUtfKey, strValUtf);
	return jop.good();
}

bool CppJsonConv::SetStringValueW(const std::string &strUtfKey, const std::wstring &strVal)
{
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);

	std::string strUtfVal;
	CA::Unicode2UTF(strVal.c_str(), strUtfVal);
	jop.write(strUtfKey, strUtfVal);
	return jop.good();
}


bool CppJsonConv::SetJsonValue(const std::string &strUtfKey, Json::Value &jsnValue)
{
	CHECK_RET(m_pJsonVal && !strUtfKey.empty(), false);

	json_op jop(*m_pJsonVal);

	std::string strUtfVal;	
	jop.write(strUtfKey, jsnValue);
	return jop.good();
}

bool CppJsonConv::SetArrItemInt64Value(int nIndex, INT64 nVal)
{
	CHECK_RET(m_pJsonVal && m_pJsonVal->isArray() && nIndex >= 0, false);
	(*m_pJsonVal)[nIndex] = nVal;
	return true;
}

bool CppJsonConv::SetArrItemInt32Value(int nIndex, int nVal)
{
	CHECK_RET(m_pJsonVal && m_pJsonVal->isArray() && nIndex >= 0, false);
	(*m_pJsonVal)[nIndex] = nVal;
	return true;
}

bool CppJsonConv::SetArrItemStringValueA(int nIndex, const std::string &strValUtf)
{
	CHECK_RET(m_pJsonVal && m_pJsonVal->isArray() && nIndex >= 0, false);
	(*m_pJsonVal)[nIndex] = strValUtf;
	return true;
}

bool CppJsonConv::SetArrItemStringValueW(int nIndex, const std::wstring &strVal)
{
	CHECK_RET(m_pJsonVal && m_pJsonVal->isArray() && nIndex >= 0, false);
	std::string strTmp;
	CA::Unicode2UTF(strVal.c_str(), strTmp);	
	(*m_pJsonVal)[nIndex] = strTmp;
	return true;
}

bool CppJsonConv::SetArrItemJsonValue(int nIndex, Json::Value &jsnValue)
{	
	CHECK_RET(m_pJsonVal && m_pJsonVal->isArray() && nIndex >= 0, false);
	(*m_pJsonVal)[nIndex] = jsnValue;
	return true;
}