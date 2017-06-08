#include "stdafx.h"
#include "IFTStockUtilImpl.h"

#define  Max_Cache_TickCount  (3600*1000)

 
//////////////////////////////////////////////////////////////////////////
//
IFTStockUtilImpl::IFTStockUtilImpl()
{
}

IFTStockUtilImpl::~IFTStockUtilImpl()
{
	DoClearAllData();
}

void IFTStockUtilImpl::Init(IFTQuoteData* pQuoteData)
{
	CA::CAutoLock<CA::CCriticalSection> lock(&m_safe);

	m_pFTQuoteData = pQuoteData;
}

bool IFTStockUtilImpl::GetStockMktCode(INT64 nStockID, StockMktCodeEx& stMktCode)
{
	CA::CAutoLock<CA::CCriticalSection> lock(&m_safe);

	StockData_Cache* pItem = DoGetStockCacheData(nStockID, NULL);
	if (!pItem)
	{
		return false;
	}
	stMktCode = pItem->stStockMktCode;
	return true;
}

INT64 IFTStockUtilImpl::GetStockHashVal(const char* pstrCode, StockMktType eMktType)
{
	CA::CAutoLock<CA::CCriticalSection> lock(&m_safe);

	StockMktCodeEx  stStock(eMktType, pstrCode);
	StockData_Cache* pItem = DoGetStockCacheData(0, &stStock);
	
	if (pItem)
	{
		return pItem->nStockID;
	}
	return 0;
}

int IFTStockUtilImpl::GetStockLotsize(INT64 nStockID, bool* pRet)
{
	CA::CAutoLock<CA::CCriticalSection> lock(&m_safe);

	bool bRet = false;
	int nLotsize = 0;
	StockData_Cache* pItem = DoGetStockCacheData(nStockID, NULL);

	if (pItem)
	{
		bRet = true;
		nLotsize = pItem->nLotSize;
	}
	if (pRet)
	{
		*pRet = bRet;
	}
	return nLotsize;
}

void IFTStockUtilImpl::Uninit()
{
	CA::CAutoLock<CA::CCriticalSection> lock(&m_safe);

	m_pFTQuoteData = NULL;
	DoClearAllData();
}

StockData_Cache* IFTStockUtilImpl::DoGetStockCacheData(INT64 nStockID, const StockMktCodeEx* pStockMkt)
{
	CHECK_RET(m_pFTQuoteData, NULL);
	StockData_Cache* pItem = NULL;
	bool bCreateNew = false;
	DWORD dwCurTick = GetTickCount();
	if (nStockID  != 0)
	{
		std::map<INT64, StockData_Cache* >::iterator itFind = m_mapStockID.find(nStockID);
		if (itFind != m_mapStockID.end())
		{
			pItem = itFind->second;
			if (pItem && dwCurTick - pItem->dwLastTick > Max_Cache_TickCount)
			{
				DoClearCacheItem(pItem);
				pItem = NULL;
			}
		}
		if (!pItem)
		{ 
			StockMktType eMarketType = StockMkt_None;
			wchar_t szStockCode[16] = {};
			wchar_t szStockName[128] = {};
			int nLotSize = 0;
			if (!m_pFTQuoteData->GetStockInfoByHashVal(nStockID, eMarketType, szStockCode, szStockName, &nLotSize))
			{
				CHECK_RET(false, NULL);
			}
			bCreateNew = true;
			pItem = new StockData_Cache;
			CHECK_RET(pItem, NULL);
			std::string strTmp = CT2A(szStockCode);
			pItem->stStockMktCode = StockMktCodeEx(eMarketType, strTmp.c_str());
			pItem->dwLastTick = dwCurTick;
			pItem->nStockID = nStockID;
			pItem->nLotSize = nLotSize;
		}
	}
	else if (pStockMkt)
	{
		CHECK_RET(pStockMkt->strCode && pStockMkt->eMarketType != StockMkt_None, NULL);
		std::map<StockMktCodeEx, StockData_Cache*>::iterator itFind = m_mapStockMktCode.find(*pStockMkt);
		if (itFind != m_mapStockMktCode.end())
		{
			pItem = itFind->second;
			if (pItem && dwCurTick - pItem->dwLastTick > Max_Cache_TickCount)
			{
				DoClearCacheItem(pItem);
				pItem = NULL;
			}
		}
		if (!pItem)
		{
			INT64 nStockID = m_pFTQuoteData->GetStockHashVal(CA2W(pStockMkt->strCode), pStockMkt->eMarketType);
			CHECK_RET(nStockID != 0, NULL);

			StockMktType eMarketType = StockMkt_None;
			wchar_t szStockCode[16] = {};
			wchar_t szStockName[128] = {};
			int nLotSize = 0;
			if (!m_pFTQuoteData->GetStockInfoByHashVal(nStockID, eMarketType, szStockCode, szStockName, &nLotSize))
			{
				CHECK_RET(false, NULL);
			}
			CHECK_OP(pStockMkt->eMarketType == eMarketType, NOOP);

			bCreateNew = true;
			pItem = new StockData_Cache;
			pItem->stStockMktCode = *pStockMkt;
			pItem->dwLastTick = dwCurTick;
			pItem->nStockID = nStockID;
			pItem->nLotSize = nLotSize;
		}
	}

	if (bCreateNew && pItem)
	{
		m_vtCacheData.push_back(pItem);
		m_mapStockID[pItem->nStockID] = pItem;
		m_mapStockMktCode[pItem->stStockMktCode] = pItem;
	}
	return pItem;
}

StockData_Cache::StockData_Cache()
{
	dwLastTick = 0;
	nStockID = 0;
	nLotSize = 0;
}

void IFTStockUtilImpl::DoClearCacheItem(StockData_Cache* pItem)
{
	CHECK_RET(pItem, NORET);

	std::map<INT64, StockData_Cache* >::iterator  itMapStockID = m_mapStockID.begin();
	while (itMapStockID != m_mapStockID.end())
	{
		if (itMapStockID->second == pItem)
		{
			itMapStockID = m_mapStockID.erase(itMapStockID);
			break;
		}
		++itMapStockID;
	}

	std::map<StockMktCodeEx, StockData_Cache*>::iterator  itMapMkt = m_mapStockMktCode.begin();
	while (itMapMkt != m_mapStockMktCode.end())
	{

		if (itMapMkt->second == pItem)
		{
			itMapMkt = m_mapStockMktCode.erase(itMapMkt);
			break;
		}
		++itMapMkt;
	}

	std::vector<StockData_Cache*>::iterator it = std::find(m_vtCacheData.begin(), m_vtCacheData.end(), pItem);
	if (it != m_vtCacheData.end())
	{
		m_vtCacheData.erase(it);
		SAFE_DELETE(pItem);
	}
}

void IFTStockUtilImpl::DoClearAllData()
{
	std::vector<StockData_Cache*>::iterator it = m_vtCacheData.begin();
	while (it != m_vtCacheData.end())
	{
		SAFE_DELETE(*it);
		++it;
	}
	m_vtCacheData.clear();
	m_mapStockMktCode.clear();
	m_mapStockID.clear();
}
