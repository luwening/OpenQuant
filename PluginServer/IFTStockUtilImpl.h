#pragma once
#include "Include/FTPluginQuoteInterface.h"
#include "IFTStockUtil.h"
#include "CM/CA_Lock.h"
#include <map>
#include <vector>
#include <string>  


struct StockData_Cache
{
	StockMktCodeEx stStockMktCode;
	INT64 nStockID;
	int nLotSize;
	DWORD dwLastTick;
	StockData_Cache();
};

class IFTStockUtilImpl
{
public:
	IFTStockUtilImpl();
	~IFTStockUtilImpl();

	void Init(IFTQuoteData* m_pQuoteData);
	bool GetStockMktCode(INT64 nStockID, StockMktCodeEx& stMktCode);
	INT64 GetStockHashVal(const char* pstrCode, StockMktType eMktType);
	int GetStockLotsize(INT64 nStockID, bool* pRet);
	void Uninit();

private:
	StockData_Cache* DoGetStockCacheData(INT64 nStockID, const StockMktCodeEx* pStockMkt);
	void DoClearCacheItem(StockData_Cache* pItem);
	void DoClearAllData();

private:
	IFTQuoteData* m_pFTQuoteData;
	std::vector<StockData_Cache*> m_vtCacheData;
	std::map<INT64, StockData_Cache* > m_mapStockID;
	std::map<StockMktCodeEx, StockData_Cache*> m_mapStockMktCode;
	CA::CCriticalSection m_safe;
};
