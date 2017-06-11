#pragma once
#include "FTPluginQuoteDefine.h"
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
*	行情操纵接口 IFTQuoteOperation，插件宿主实现，通过查询IFTPluginCore::QueryFTInterface得到
*/
static const GUID IID_IFTQuoteOperation =
{ 0x9c65990c, 0x903, 0x4185, { 0x97, 0x12, 0x3e, 0xa7, 0xab, 0x34, 0xd, 0xc5 } };

interface IFTQuoteOperation
{
	//行情定阅，返回错误码
	virtual StockSubErrCode Subscribe_PriceBase(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb, SOCKET sock) = 0;
	virtual StockSubErrCode Subscribe_OrderQueue(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb, SOCKET sock) = 0;
	virtual StockSubErrCode Subscribe_Ticker(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb, SOCKET sock) = 0;
	virtual StockSubErrCode Subscribe_RTData(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb, SOCKET sock) = 0;
	virtual StockSubErrCode Subscribe_KLData(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb, StockSubType eStockSubType, SOCKET sock) = 0;
	virtual StockSubErrCode Subscribe_BrokerQueue(const GUID &guidPlugin, LPCWSTR wstrStockCode, StockMktType eType, bool bSubb, SOCKET sock) = 0;

	virtual QueryDataErrCode QueryStockRTData(const GUID &guidPlugin, DWORD* pCookie, LPCWSTR wstrStockCode, StockMktType eType) = 0;
	virtual QueryDataErrCode QueryStockKLData(const GUID &guidPlugin, DWORD* pCookie, LPCWSTR wstrStockCode, StockMktType eType, int nKLType) = 0;

	//请求股票快照，最多一次200个,通过 IQuoteInfoCallback::OnReqStockSnapshot返回
	virtual QueryDataErrCode QueryStockSnapshot(const GUID &guidPlugin, INT64 *arStockHash, int nStockNum, DWORD &dwReqCookie) = 0;
	virtual void  CancelQuerySnapshot(DWORD dwReqCookie) = 0;

	//通知连接关闭
	virtual void  NotifyFTPluginSocketClosed(const GUID &guidPlugin, SOCKET sock) = 0;

	//请求板块及板块集合的列表
	virtual QueryDataErrCode QueryPlatesetSubIDList(const GUID &guidPlugin, INT64 nPlatesetID, DWORD& dwCookie) = 0;
	virtual QueryDataErrCode QueryPlateStockIDList(const GUID &guidPlugin, INT64 nPlateID, DWORD& dwCookie) = 0;
};


/**
*	行情数据接口 IFTQuoteData，插件宿主实现，通过查询IFTPluginCore::QueryFTInterface得到
*/
static const GUID IID_IFTQuoteData =
{ 0xb75073e3, 0xaa3a, 0x4717, { 0xac, 0xa2, 0x11, 0x94, 0xa1, 0x3, 0x78, 0xc7 } };

interface IFTQuoteData
{
	/**
	* 当前是否订阅某只股票某个订阅位(针对所有连接)
	*/
	virtual bool   IsSubStockOneType(INT64 ddwStockHash, StockSubType eStockSubType) = 0;

	/**
	* 当前是否是实时行情
	*/
	virtual bool   IsRealTimeQuotes(INT64 ddwStockHash) = 0;

	/**
	* stock 的hash值, 回调接口方便
	*/
	virtual INT64  GetStockHashVal(LPCWSTR pstrStockCode, StockMktType eMkt) = 0;

	/**
	* stock 的hash值, 回调接口方便
	*/
	virtual bool  GetStockInfoByHashVal(INT64 ddwStockID, StockMktType& eMkt,
		wchar_t szStockCode[16], wchar_t szStockName[128], int* pLotSize = NULL,
		int* pSecurityType = NULL, int* pSubType = NULL, INT64* pnOwnerStockID = NULL) = 0;

	/**
	* 填充基础报价
	*/
	virtual bool   FillPriceBase(INT64 ddwStockHash, Quote_PriceBase* pInfo) = 0;

	/**
	* 填充十档数据
	*/
	virtual bool   FillOrderQueue(INT64 ddwStockHash, Quote_OrderItem* parOrder, int nCount) = 0;

	/**
	*	填充内存逐笔数据，不会去server拉新的数据，返回实际fill的个数
	*   当nLastSequence为0时，得到的是最近的逐笔数据、nLastSequence不为0时为大于nLastSequence的Ticker数据
	*/
	virtual int    FillTickArr(INT64 ddwStockHash, PluginTickItem *parTickBuf, int nTickBufCount, INT64 nLastSequence = 0) = 0;

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
	* 得到交易日列表
	* @pszDateFrom: "YYYYMMDD"格式，为NULL则默认为pszDateTo往前推一年
	* @pszDateTo: "YYYYMMDD"格式，为NULL则默认为当前日历时间
	* @nDateArr:  返回YYYYMMDD格式的整数日期数组，接收方必须将返回的数据copy一份保存起来
	* @nDateLen:  nDateArr数组长度
	* @return:    返回true或false表示成功或失败，注意即使成功nDateLen也有可能为0
	*/
	virtual bool GetTradeDateList(StockMktType mkt, LPCWSTR pszDateFrom, LPCWSTR pszDateTo, int* &narDateArr, int &nDateArrLen) = 0;

	//得到股票列表
	virtual bool GetStocksInfo(StockMktType mkt, PluginSecurityType eSecurityType, LPPluginStockInfo *&parInfo, int &nInfoArrLen) = 0;

	//得到除权除息信息
	//返回值：完全成功返回true, 部分成功或全部失败都返回false
	virtual bool  GetExRightInfo(INT64 *arStockHash, int nStockNum, PluginExRightItem* &arExRightInfo, int &nExRightInfoNum) = 0;

	//得到历史K线 
	//返回值： 如果参数错误或者数据未下载，则返回false；返回的数量满足或不足都返回true
	//pszDateTimeFrom,pszDateTimeTo: 不能为null, 日期字符串格式为YYYY-MM-DD HH:MM:SS
	virtual bool  GetHistoryKLineTimeStr(StockMktType mkt, INT64 ddwStockHash, int nKLType, int nRehabType, LPCWSTR pszDateTimeFrom, LPCWSTR pszDateTimeTo, Quote_StockKLData* &arKLData, int &nKLNum) = 0;
	virtual bool  GetHistoryKLineTimestamp(StockMktType mkt, INT64 ddwStockHash, int nKLType, int nRehabType, INT64 nTimestampFrom, INT64 nTimestampTo, Quote_StockKLData *&arKLData, int &nKLNum) = 0;

	/**
	* dwTime转成wstring 日期+时间
	*/
	virtual void TimeStampToStr(INT64 ddwStockHash, DWORD dwTime, wchar_t szTime[64]) = 0;

	/**
	* dwTime转成wstring 日期
	*/
	virtual void TimeStampToStrDate(INT64 ddwStockHash, DWORD dwTime, wchar_t szData[64]) = 0;

	/**
	* dwTime转成wstring 时间
	*/
	virtual void TimeStampToStrTime(INT64 ddwStockHash, DWORD dwTime, wchar_t szTime[64]) = 0;

	/**
	* 得到股票订阅情况
	* sock为0时得到的结果是所有连接的订阅信息，当sock不为0时，返回的是当前sock连接的订阅信息
	*/
	virtual bool GetStockSubInfoList(Quote_SubInfo* &pSubInfoArr, int &nSubInfoLen, SOCKET sock = 0) = 0;

	/**
	* 填充批量报价数据
	*/
	virtual bool FillBatchBasic(INT64 ddwStockHash, Quote_BatchBasic* pInfo) = 0;

	/**
	* 股票数据推送
	* bUnPush = true 不再需要推送
	*/
	virtual bool RecordPushRequest(SOCKET sock, INT64 ddwStockHash, StockSubType nStockPushType, bool bUnPush = true) = 0;

	/**
	* 板块集合的列表
	@GetPlatesetIDList 板块集合ID
	@parID ID列表返回，由调用方分配空间, 先传NULL得到nCount
	@nCount ID个数
	@返回值：flase表示数据不存在， 需要请求
	*/
	virtual bool GetPlatesetIDList(INT64 nPlatesetID, INT64* parID, int& nCount) = 0;

	/**
	* 二级板块的股票列表
	@GetPlateStockIDList 板块的股票列表ID
	@parID ID列表返回，由调用方分配空间, 先传NULL得到nCount
	@nCount ID个数
	@返回值：flase表示数据不存在， 需要请求
	*/
	virtual bool GetPlateStockIDList(INT64 nPlateID, INT64* parID, int& nCount) = 0;

	/**
	* 经纪队列
	@GetBrokerQueueList  经纪队列列表
	@parID ID列表返回，由调用方分配空间, 先传NULL得到nCount
	@nCount ID个数
	@返回值：flase表示数据不存在， 需要定阅等待数据push
	*/
	virtual bool GetBrokerQueueList(INT64 nStockID, Quote_BrokerItem* parID, int& nCount) = 0;

	/**
	* 获取全局状态
	@GetNNGlobalState   
	@NNGlobalState  返回对象
	@返回值：void 
	*/
	virtual void GetNNGlobalState(NNGlobalState* pState) = 0;
};

/**
*  插件宿主通知插件行情数据变化接口
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

	/**
	* 经纪队列变更
	*/
	virtual void  OnChanged_BrokerQueue(INT64 ddwStockHash) = 0;

	//请求股票快照返回
	virtual void OnReqStockSnapshot(DWORD dwCookie, PluginStockSnapshot *arSnapshot, int nSnapshotNum) = 0;

	/**
	* 请求板块集合的子项返回
	*/
	virtual void  OnReqPlatesetIDs(int nCSResult, DWORD dwCookie) = 0;

	/**
	* 请求板块的子项返回
	*/
	virtual void  OnReqPlateStockIDs(int nCSResult, DWORD dwCookie) = 0;

	/**
	* 推送基础报价
	*/
	virtual void  OnPushPriceBase(INT64  ddwStockHash, SOCKET sock) = 0;

	/**
	* 推送摆盘
	*/
	virtual void  OnPushGear(INT64  ddwStockHash, SOCKET sock) = 0;

	/**
	* 推送逐笔
	*/
	virtual void  OnPushTicker(INT64  ddwStockHash, SOCKET sock) = 0;

	/**
	* 推送K线
	*/
	virtual void  OnPushKL(INT64  ddwStockHash, SOCKET sock, StockSubType eStockSubType) = 0;

	/**
	* 推送分时
	*/
	virtual void  OnPushRT(INT64  ddwStockHash, SOCKET sock) = 0;

	/**
	* 推送经纪队列
	*/
	virtual void  OnPushBrokerQueue(INT64  ddwStockHash, SOCKET sock) = 0;

	/**
	* 新交易日推送
	*/
	virtual void  OnPushMarketNewTrade(StockMktType eMkt, INT64 ddwLastTradeStamp, INT64 ddwNewTradeStamp) = 0;

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

/**
*  数据上报接口
*/
static const GUID IID_IFTDataReport =
{ 0xdf2cba3e, 0x98c4, 0x4391, { 0x88, 0x23, 0x32, 0xc1, 0x7d, 0xea, 0x93, 0x1e } };

interface IFTDataReport
{
	/**
	* PLS数据上报-CmdID
	*/
	virtual void PLSCmdIDReport(const GUID &guidPlugin, int nCmdID) = 0;
};

