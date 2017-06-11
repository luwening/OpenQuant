#include "stdafx.h"
#include "IFTStockUtil.h"
#include "IFTStockUtilImpl.h"

IFTStockUtilImpl* IFTStockUtil::m_pImpl = NULL;

//////////////////////////////////////////////////////////////////////////
//

IFTStockUtil::IFTStockUtil()
{
}

IFTStockUtil::~IFTStockUtil()
{

}

void IFTStockUtil::Init(IFTQuoteData* pQuoteData)
{
	if (!m_pImpl)
	{
		m_pImpl = new IFTStockUtilImpl;
		CHECK_RET(m_pImpl, NORET);
		m_pImpl->Init(pQuoteData);
	}
}

bool IFTStockUtil::GetStockMktCode(INT64 nStockID, StockMktCodeEx& stMktCode)
{
	if (m_pImpl)
	{
		return m_pImpl->GetStockMktCode(nStockID, stMktCode);
	}
	return false;
}

int IFTStockUtil::GetStockLotsize(INT64 nStockID, bool* pRet)
{
	if (m_pImpl)
	{
		return m_pImpl->GetStockLotsize(nStockID, pRet);
	}
	return false;
}

INT64 IFTStockUtil::GetStockHashVal(const char* pstrCode, StockMktType eMktType)
{
	if (m_pImpl)
	{
		return m_pImpl->GetStockHashVal(pstrCode, eMktType);
	}
	return 0;
}

INT64 IFTStockUtil::GetStockHashVal(const wchar_t* pwstrCode, StockMktType eMktType)
{
	return GetStockHashVal((const char*)CW2A(pwstrCode), eMktType);
}

void IFTStockUtil::Uninit()
{
	if (m_pImpl)
	{
		m_pImpl->Uninit();
		SAFE_DELETE(m_pImpl);
	}
}
