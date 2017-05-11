#pragma once
#include "FormatTime.h"
#include "Include/FTPluginQuoteDefine.h"
#include "Protocol/ProtoDataStruct.h"

class UtilPlugin
{
public:
	static  int  GetMarketTimezone(StockMktType eMkt, int nTimestamp);
	static  int  GetMarketTimezone2(StockMktType eMkt, int nYear, int nMonth, int nDay);
	static  std::string FormatMktTimestamp(int nTimestamp, StockMktType eMkt, FormatTimeType eFmtType);

	static  std::string GetErrStrByCode(QueryDataErrCode eCode);
	static ProtoErrCode ConvertErrCode(QueryDataErrCode eCode);
};