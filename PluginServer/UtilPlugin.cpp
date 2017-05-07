#include "stdafx.h"
#include "UtilPlugin.h"
#include "UsTimezone.h"

int  UtilPlugin::GetMarketTimezone(StockMktType eMkt, int nTimestamp)
{
	switch (eMkt)
	{
	case StockMkt_HK:
	case StockMkt_Feature_Old:
	case StockMkt_Feature_New:
	case StockMkt_SH:
	case StockMkt_SZ:
		return 8;
		break;
	case StockMkt_US:
		return UsTimezone::GetTimestampTimezone(nTimestamp);
		break;
	default:
		assert(0);
		return 0;
		break;
	}
}

int  UtilPlugin::GetMarketTimezone2(StockMktType eMkt, int nYear, int nMonth, int nDay)
{
	switch (eMkt)
	{
	case StockMkt_HK:
	case StockMkt_Feature_Old:
	case StockMkt_Feature_New:
	case StockMkt_SH:
	case StockMkt_SZ:
		return 8;
		break;
	case StockMkt_US:
		return UsTimezone::GetTMStructTimezone(nYear, nMonth, nDay);
			break;
	default:
		assert(0);
		return 0;
		break;
	}
}

std::string UtilPlugin::FormatMktTimestamp(int nTimestamp, StockMktType eMkt, FormatTimeType eFmtType)
{
	int nTimezone = GetMarketTimezone(eMkt, nTimestamp);
	std::string strFmt = FormatTime::FormatTimestamp(nTimestamp, nTimezone, eFmtType);
	return strFmt;
}

std::string UtilPlugin::GetErrStrByCode(QueryDataErrCode eCode)
{
	CString strErr;
	switch (eCode)
	{
	case QueryData_Suc:
		strErr = "Success!";
		break;
	case QueryData_FailUnknown:
		strErr = L"请求遇到未知错误";
		break;
	case QueryData_FailMaxSubNum:
		strErr = L"请求到达最大查询数";
		break;
	case QueryData_FailCodeNoFind:
		strErr = L"请求代码没找到";
		break;
	case QueryData_FailGuidNoFind:
		strErr = L"请求插件GUID传错";
		break;
	case QueryData_FailNoImplInf:
		strErr = L"请求行情接口未完成";
		break;
	case QueryData_FailFreqLimit:
		strErr = L"请求查询频率限制导致失败";
		break;
	case QueryData_FailNetwork:
		strErr = L"请求网络异常，发送失败";
		break;
	case QueryData_FailErrParam:
		strErr = L"参数错误";
		break;
	default:
		strErr.Format(_T("请求未知错误:%d"), (int)eCode);
		break;
	}

	std::string strTmp;
	CA::Unicode2UTF(CT2CW(strErr), strTmp);

	return strTmp;
}
