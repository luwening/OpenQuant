#pragma once

#ifndef interface
#define interface struct
#endif

 
/************************************************* 
Copyright: FUTU
Author: ysq
Date: 2015-03-18
Description: 行情API和回调接口定义
**************************************************/  

/**
*股票的市场类型 
*/ 
enum StockMktType 
{	
	StockMkt_HK = 1,  //港股 
	StockMkt_US = 2,  //美股
	StockMkt_SH = 3,  //沪股
	StockMkt_SZ = 4,  //深股
	StockMkt_Feature_Old = 5,  //旧的期货 code: 999000, 999001 （旧期货分时数据在一天连续）
	StockMkt_Feature_New = 6,  //新期货 code: 999010, 999011 （新期货分时数据会跨天，与传统软件保持一致）
}; 

enum StockSubErrCode
{
	StockSub_Suc = 0,	//订阅成功
	StockSub_FailUnknown	= 1,	//未知的失败
	StockSub_FailMaxSubNum	= 2,	//到达最大订阅数
	StockSub_FailCodeNoFind = 3,	//代码没找到(也有可能是市场类型错了)
	StockSub_FailGuidNoFind = 4,	//插件GUID传错
	StockSub_FailNoImplInf = 5,		//行情接口未完成
};

enum QueryDataErrCode
{
	QueryData_Suc = 0,	//查询成功
	QueryData_FailUnknown	= 1,	//未知的失败
	QueryData_FailMaxSubNum	= 2,	//到达最大查询数
	QueryData_FailCodeNoFind = 3,	//代码没找到(也有可能是市场类型错了)
	QueryData_FailGuidNoFind = 4,	//插件GUID传错
	QueryData_FailNoImplInf = 5,		//行情接口未完成
	QueryData_IsExisted = 6,
};

/**
* 股票基础报价信息：
* 价格精度是3位小数， 如报价8.888存储值 88888
*/
typedef struct tagQuotePriceBase
{ 
	DWORD dwOpen;		//开盘价
	DWORD dwLastClose;  //昨收价
	DWORD dwCur;		//当前价
	DWORD dwHigh;		//最高价
	DWORD dwLow;		//最低价
	INT64 ddwVolume;	//成交量
	INT64 ddwTrunover;	//成交额
	DWORD dwTime;		//报价时间
	DWORD ddwLotSize;	//每手数量
}Quote_PriceBase, *LPQuote_PriceBase;


/**
* 股票十档数据
* IFTQuoteData::FillOrderQueue 的接口参数  
*/
typedef struct tagQuoteOrderQueue
{
	DWORD	dwBuyPrice, dwSellPrice;  //买价 卖价
	INT64	ddwBuyVol, ddwSellVol;    //买量 卖量
	int		nBuyOrders, nSellOrders;  //档位 
}Quote_OrderQueue, *LPQuote_OrderQueue;  


/**
* 分时数据
*/
typedef struct tagQuoteStockRTData
{
	int   nDataStatus; 
	DWORD dwTime; 
	DWORD dwOpenedMins;  //开盘第多少分钟  

	int   nCurPrice;
	DWORD nLastClosePrice; //昨天的收盘价 

	int   nAvgPrice;

	INT64 ddwTDVolume;
	INT64 ddwTDValue;  
}Quote_StockRTData, *LPQuote_StockRTData;  

/**
* K线数据
*/
typedef struct tagQueryStockKLData
{
	int   nDataStatus; 
	DWORD dwTime;

	int   nOpenPrice; 
	int   nClosePrice; 

	int   nHighestPrice; 
	int   nLowestPrice;  

	int   nPERatio; //市盈率(三位小数)
	int   nTurnoverRate;//换手率(正股及指数的日/周/月K线)

	INT64 ddwTDVol; 
	INT64 ddwTDVal;
}Quote_StockKLData, *LPQuote_StockKLData; 

//nKLType:
//1 = 1分K;  
//2 = 日K; 
//3 = 周K;   
//4 = 月K;
//6 = 5分K; 
//7 = 15分K;  
//8 = 30分K;  
//9 = 60分K;
//
//nRehabType:
//0 = 不复权；
//1 = 前复权；
//2 = 后复权；

/**
* 行情操作接口 
*/
interface IFTQuoteOperation 
{
	//行情定阅，返回错误码
	virtual StockSubErrCode Subscribe_PriceBase(const GUID &guidPlugin, LPCWSTR wstrStockCode,  StockMktType eType, bool bSubb) = 0;  
	virtual StockSubErrCode Subscribe_OrderQueue(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb) = 0; 
	virtual StockSubErrCode Subscribe_RTData(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb) = 0; 
	virtual StockSubErrCode Subscribe_KLData(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb, int nKLType) = 0; 
	virtual QueryDataErrCode QueryStockRTData(const GUID &guidPlugin, DWORD* pCookie, LPCWSTR wstrStockCode, StockMktType eType) = 0;
	virtual QueryDataErrCode QueryStockKLData(const GUID &guidPlugin, DWORD* pCookie, LPCWSTR wstrStockCode, StockMktType eType, int nKLType) = 0;
};

/**
* 行情数据的接口
*/
interface IFTQuoteData
{ 
	/**
	* 当前是否是实时行情
	*/
	virtual bool   IsRealTimeQuotes() = 0; 

	/**
	* stock 的hash值, 回调接口方便 
	*/ 
	virtual INT64  GetStockHashVal(LPCWSTR pstrStockCode, StockMktType eMkt) = 0; 

	/**
	* 填充基础报价 
	*/ 
	virtual bool   FillPriceBase(INT64 ddwStockHash,  Quote_PriceBase* pInfo) = 0; 

	/**
	* 填充十档数据
	*/ 
	virtual bool   FillOrderQueue(INT64 ddwStockHash, Quote_OrderQueue* parQuote, int nCount) = 0; 

	/**
	* 填充分时数据
	*/ 
	virtual bool   FillRTData(INT64 ddwStockHash, Quote_StockRTData* &pQuoteRT, int& nCount) = 0;

	virtual BOOL   IsRTDataExist(INT64 ddwStockHash) = 0;

	virtual void   DeleteRTDataPointer(Quote_StockRTData* pRTData) = 0; 

	/**
	* 填充K线数据
	*/ 
	virtual BOOL   FillKLData(INT64 ddwStockHash, Quote_StockKLData* &pQuoteKL, int& nCount, int nKLType, int nRehabType) = 0;

	virtual BOOL   IsKLDataExist(INT64 ddwStockHash, int nKLType) = 0;

	virtual void   DeleteKLDataPointer(Quote_StockKLData* pRTData) = 0; 

	/**
	* 
	*/ 
	virtual void   CheckRemoveQuoteRT(INT64 ddwStockID) = 0;

	virtual void   CheckRemoveQuoteKL(INT64 ddwStockID, int nKLType) = 0;

	/**
	* dwTime转成wstring
	*/ 
	virtual void   TimeStampToStr(INT64 ddwStockHash, DWORD dwTime, wchar_t szTime[64]) = 0;
}; 

/**
* 行情数据回调
*/
interface IQuoteInfoCallback
{ 
	/**
	* 基础报价信息变化 
	*/ 
	virtual void  OnChanged_PriceBase(INT64  ddwStockHash) = 0; 

	/**
	* 十档数据变化
	*/ 
	virtual void  OnChanged_OrderQueue(INT64 ddwStockHash) = 0; 

	/**
	* 分时数据变化
	*/ 
	virtual void  OnChanged_RTData(INT64 ddwStockHash) = 0; 

		/**
	* 分时数据变化
	*/ 
	virtual void  OnChanged_KLData(INT64 ddwStockHash, int nKLType) = 0; 
};

interface IQuoteKLCallback
{
	/**
	* 请求分时回调
	*/ 
	virtual void OnQueryStockRTData(DWORD dwCookie, int nCSResult) = 0; 

	/**
	* 请求K线回调
	*/ 
	virtual void OnQueryStockKLData(DWORD dwCookie, int nCSResult) = 0; 
};

