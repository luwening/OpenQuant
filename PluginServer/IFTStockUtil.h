#pragma once
#include "Include/FTPluginQuoteInterface.h"

struct StockMktCodeEx
{
	StockMktType eMarketType;
	char strCode[16];
	StockMktCodeEx()
	{
		memset(this, 0, sizeof(*this));
	}
	StockMktCodeEx(StockMktType eVal, const char* pstrCode)
	{
		eMarketType = eVal;
		memset(strCode, 0, sizeof(strCode));
		if (pstrCode)
		{
			strncpy(strCode, pstrCode, _countof(strCode) - 1);
		}
	}
	StockMktCodeEx(StockMktType eVal, const wchar_t* pwstrCode)
	{
		eMarketType = eVal;
		memset(strCode, 0, sizeof(strCode));
		if (pwstrCode)
		{
			strncpy(strCode, CW2A(pwstrCode), _countof(strCode) - 1);
		}
	}
	bool operator < (const StockMktCodeEx& stVal) const
	{
		return  strncmp(strCode, stVal.strCode, _countof(strCode)) < 0;
	}
};

class IFTStockUtilImpl;
class IFTStockUtil
{
public:
	IFTStockUtil();
	~IFTStockUtil();

	static void Init(IFTQuoteData* pQuoteData);
	static bool GetStockMktCode(INT64 nStockID, StockMktCodeEx& stMktCode);
	static int GetStockLotsize(INT64 nStockID, bool* pRet);

	static INT64 GetStockHashVal(const char* pstrCode, StockMktType eMktType);
	static INT64 GetStockHashVal(const wchar_t* pwstrCode, StockMktType eMktType);

	static void Uninit();

private:
	static IFTStockUtilImpl* m_pImpl;
 
};
