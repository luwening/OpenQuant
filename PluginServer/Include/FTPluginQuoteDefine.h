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

//协议请求超时
#define REQ_TIMEOUT_MILLISECOND   10000

/**
*股票的市场类型
*/
enum StockMktType
{
	StockMkt_None = 0,
	StockMkt_HK = 1,  //港股 
	StockMkt_US = 2,  //美股
	StockMkt_SH = 3,  //沪股
	StockMkt_SZ = 4,  //深股
	StockMkt_Feature_Old = 5,  //旧的期货 code: 999000, 999001 （旧期货分时数据在一天连续）
	StockMkt_Feature_New = 6,  //新期货 code: 999010, 999011 （新期货分时数据会跨天，与传统软件保持一致）
};
#define  IsValidMktID(mkt)  ((int)mkt >= (int)StockMkt_HK && (int)mkt <= (int)StockMkt_Feature_New)

enum StockSubErrCode
{
	StockSub_Suc = 0,	//订阅成功
	StockSub_FailUnknown = 1,	//未知的失败
	StockSub_FailMaxSubNum = 2,	//到达最大订阅数
	StockSub_FailCodeNoFind = 3,	//代码没找到(也有可能是市场类型错了)
	StockSub_FailGuidNoFind = 4,	//插件GUID传错
	StockSub_FailNoImplInf = 5,		//行情接口未完成
	StockSub_UnSubTimeError = 6,	//未满足反订阅要求时间-1分钟
};

enum QueryDataErrCode
{
	QueryData_Suc = 0,	//查询成功
	QueryData_FailUnknown = 1,	//未知的失败
	QueryData_FailMaxSubNum = 2,	//到达最大查询数
	QueryData_FailCodeNoFind = 3,	//代码没找到(也有可能是市场类型错了)
	QueryData_FailGuidNoFind = 4,	//插件GUID传错
	QueryData_FailNoImplInf = 5,		//行情接口未完成
	
	QueryData_FailFreqLimit = 7,	//查询频率限制导致失败
	QueryData_FailNetwork = 8,		//网络异常，发送失败
	QueryData_FailErrParam = 9,		//参数错误
};

enum StockSubType
{
	StockSubType_None = 0,
	StockSubType_Simple = 1,
	StockSubType_Gear = 2,
	StockSubType_Ticker = 4,
	StockSubType_RT = 5,
	StockSubType_KL_DAY = 6,
	StockSubType_KL_MIN5 = 7,
	StockSubType_KL_MIN15 = 8,
	StockSubType_KL_MIN30 = 9,
	StockSubType_KL_MIN60 = 10,
	StockSubType_KL_MIN1 = 11,
	StockSubType_KL_WEEK = 12,
	StockSubType_KL_MONTH = 13,
	StockSubType_Broker = 14, //定阅经纪队列

	StockSubType_Max = StockSubType_Broker + 1,
};

#define  IsStockSubType_RTKL(eType)  (StockSubType_RT == eType || StockSubType_KL_DAY == eType || StockSubType_KL_MIN5 == eType ||  \
									  StockSubType_KL_MIN15 == eType || StockSubType_KL_MIN30 == eType || StockSubType_KL_MIN60 == eType ||  \
									  StockSubType_KL_MIN1 == eType || StockSubType_KL_WEEK == eType || StockSubType_KL_MONTH == eType)


//IFTQuoteOperation::QueryStockKLData的nKLType 类型 
enum
{
	FT_KL_CLASS_MIN_1 = 1,
	FT_KL_CLASS_DAY = 2,
	FT_KL_CLASS_WEEK = 3,
	FT_KL_CLASS_MONTH = 4,
	FT_KL_CLASS_YEAR = 5,
	FT_KL_CLASS_MIN_5 = 6,
	FT_KL_CLASS_MIN_15 = 7,
	FT_KL_CLASS_MIN_30 = 8,
	FT_KL_CLASS_MIN_60 = 9,
};

enum PluginSecurityType
{
	PluginSecurity_All = 0,
	PluginSecurity_Bond = 1, //债券	
	PluginSecurity_Stock = 3, //正股	
	PluginSecurity_ETF = 4,
	PluginSecurity_Warrant = 5, //涡轮牛熊		
	PluginSecurity_Index = 6,
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
typedef struct tagQuoteOrderItem
{
	DWORD	dwBuyPrice, dwSellPrice;  //买价 卖价
	INT64	ddwBuyVol, ddwSellVol;    //买量 卖量
	int		nBuyOrders, nSellOrders;  //档位 
}Quote_OrderItem, *LPQuote_OrderItem;

typedef struct tagPluginTickItem
{
	DWORD dwPrice;
	DWORD dwTime;
	int nDealType;
	INT64 nSequence;
	INT64 nVolume;
	INT64 nTurnover; //成交额
}PluginTickItem, *LPPluginTickItem;

enum RTKL_DATA_STATUS
{
	RTKL_DATA_STATUS_NULL = 0,
	RTKL_DATA_STATUS_OVER = 1,		//完成
	RTKL_DATA_STATUS_HALT = 2,    //停牌 
	RTKL_DATA_STATUS_RUNNING = 3,	   //交易中 
	RTKL_DATA_STATUS_FAKED = 4,		//伪造假数据 
	RTKL_DATA_STATUS_NO_OCCUR = 5,     //未发生的	
	RTKL_DATA_STATUS_CLIENT_FILL = 6,	//客户端自动补的点，除开补点时逻辑不同外其它等同FG_RT_DATA_NO_OCCUR，参考BugID:5073
	RTKL_DATA_STATUS_FACK_FUTURE = 7, //未来点，也是fake状态的一种
};

//定义可以对外push有效点
#define  IS_RTKL_VALID_PUSH_DATA_STATUS(nStatus) (RTKL_DATA_STATUS_NULL != nStatus && RTKL_DATA_STATUS_FAKED != nStatus && RTKL_DATA_STATUS_NO_OCCUR != nStatus && RTKL_DATA_STATUS_FACK_FUTURE != nStatus)

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

	INT64   nOpenPrice;
	INT64   nClosePrice;

	INT64   nHighestPrice;
	INT64   nLowestPrice;

	int   nPERatio; //市盈率(三位小数)
	int   nTurnoverRate;//换手率(正股及指数的日/周/月K线)

	INT64 ddwTDVol;
	INT64 ddwTDVal;
}Quote_StockKLData, *LPQuote_StockKLData;

typedef struct tagSubInfo
{
	INT64 ddwStockHash;
	StockSubType eStockSubType;
	bool operator==(const tagSubInfo &Item) const
	{
		return (this->ddwStockHash == Item.ddwStockHash && this->eStockSubType == Item.eStockSubType);
	};

}Quote_SubInfo, *LPQuote_SubInfo;

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

//nStockSubType:
//1 = 报价
//2 = 摆盘
//4 = 逐笔
//5 = 分时//未做
//6 = 日K
//7 =  5分K
//8 =  15分K
//9 =  30分K
//10 =  60分K
//11 =  1分K
//12 = 周K
//13 = 月K

typedef struct tagPluginStockInfo
{
	INT64 nStockID;
	int  nLotSize;
	PluginSecurityType nSecType;
	WCHAR chSimpName[64];
	WCHAR chCodeSig[16];
	int nSubType; 
	INT64 nOwnerStockID;
	WCHAR chListDate[12];
}PluginStockInfo, *LPPluginStockInfo;

typedef struct tagBatchBasic
{
	DWORD dwOpen;		//开盘价
	DWORD dwLastClose;  //昨收价
	DWORD dwCur;		//当前价
	DWORD dwHigh;		//最高价
	DWORD dwLow;		//最低价
	DWORD dwAmpli;
	INT64 ddwVolume;	//成交量
	INT64 ddwTurnover;	//成交额
	int   nSuspension;
	int   nTurnoverRate;
	DWORD dwTime;		//报价时间
	DWORD dwListTime;	//上市时间
}Quote_BatchBasic, *LPQuote_BatchBasic;

enum PlugErtFlag
{
	Ert_NONE = 0x00,
	Ert_SPLIT = 0x01,    //拆股
	Ert_JOIN = 0x02,	 //合股
	Ert_BONUS_STK = 0x04,//送股
	Ert_INTOSHARES = 0x08,//转增股
	Ert_ALLOT = 0x10,    //配股
	Ert_ADD = 0x20,		//增发股
	Ert_DIVIDEND = 0x40,//有现金分红
	Ert_SPECIALDIVIDEND = 0x80, //有现金分红
};

//除权记录：
struct PluginExRightItem
{
	INT64 stock_id;
	UINT ex_date;    // 除权除息日期, 例如20160615
	UINT ert_flag;    // 公司行动类型组合，ErtFlag

	//拆股(eg. 1拆5，Base为1，ERT为5)
	UINT split_base;
	UINT split_ert;

	//合股(eg. 50合1，Base为50，ERT为1)
	UINT join_base;
	UINT join_ert;

	//送股(eg. 10送3, Base为10,ERT为3)
	UINT bonus_stk_base;
	UINT bonus_stk_ert;

	//配股(eg. 10送2, 配股价为6.3元, Base为10, ERT为2, Price为6300)
	UINT allot_base;
	UINT allot_ert;
	double allot_price;

	//转增股(跟送股类似)
	UINT into_shr_base;
	UINT into_ert;

	//增发(跟配股类似)
	UINT stk_add_base;
	UINT stk_add_ert;
	double stk_add_price;

	// 现金分红(eg. 每10股派现0.5元，Base为10, Amount为500)
	UINT dividend_base;
	double dividend_amount;

	// 特别股息
	UINT dividend_special_base;
	double dividend_special_amount;

	// result_self
	double fwd_factor_a;
	double fwd_factor_b;
	double bwd_factor_a;
	double bwd_factor_b;

	// 简体中文文本描述
	wchar_t sc_txt[160];

	// 繁体中文文本描述
	wchar_t tc_txt[160];
};

enum Quote_WarrantType
{
	Quote_WarrantType_Buy = 1,  //认购
	Quote_WarrantType_Sell = 2, //认沽
	Quote_WarrantType_Bull = 3, //牛
	Quote_WarrantType_Bear = 4, //熊
};

// 股票的行情快照数据
struct PluginStockSnapshot
{
	PluginStockSnapshot()
	{
		memset(this, 0, sizeof(*this));
	}
	INT64 stock_id;    // 股票id
	int  ret;    // 是否找到快照记录，0为成功找到，snapshot有数据。其他值，snapshot无数据（可能是找不到股票）
	//char   stock_code[16];
	UINT instrument_type;
	UINT market_code;

	// 价格相关
	double nominal_price;
	double last_close_price;
	double open_price;
	INT64 update_time;

	INT64 suspend_flag;
	INT64 listing_status;
	INT64 listing_date;

	// 成交统计信息
	INT64 shares_traded;
	double turnover;
	double highest_price;
	double lowest_price;
	float  turnover_ratio;

	//每手
	UINT32  nLotSize;

	//正股类型数据
	struct tagEquitiesData
	{
		bool bDataValid; //数据是否有效
		UINT64 nIssuedShares; //发行股本,即总股本
		double dbNetAssetValue; //资产净值
		double dbNetProfit; //盈利（亏损）
		double dbEarningPerShare; //每股盈利
		UINT64 nOutStandingShares; //流通股本
		double dbNetAssetPerShare; //每股净资产
		double dbEYRatio; //收益率
		double dbPERatio; //市盈率
		double dbPBRatio; //市净率 
	}stEquitiesData;

	//涡轮相关数据
	struct tagWarrantsData
	{
		bool bDataValid;  //如果非涡轮 == 0
		UINT32 nConversionRatio; //换股比率
		int  nWarrantType;  //涡轮类型 Quote_WarrantType
		double dbStrikePrice; //行使价
		INT64  nMaturityDate; //到期日
		INT64 nEndtradeDate;  //最后交易日
		INT64 nWarrantOwnerID; //正股ID

		UINT32 nIssuerCode; //发行商id
		char  strIssuerName[64]; //发行商名字
		double dbRecoveryPrice; //回收价
		UINT64 nStreetVol;  //街货量
		UINT64 nIssueVol;  //发行量
		double dbOwnerStockPrice;  //正股价格
		double dbStreetRatio; //街货占比
		double dbDelta;	 //对冲值
		double dbImpliedVolatility; //引伸波幅
		double dbPremiun; //溢价		
	}stWrtData;
};

typedef struct tagStockPushInfo
{
	INT64 ddwStockID;
	INT64 nSequence;
	DWORD dwKLTime_1MIN;
	DWORD dwKLTime_5MIN;
	DWORD dwKLTime_15MIN;
	DWORD dwKLTime_30MIN;
	DWORD dwKLTime_60MIN;
	DWORD dwKLTime_DAY;
	DWORD dwKLTime_WEEK;
	DWORD dwKLTime_MONTH;
	DWORD dwRTTime;
	tagStockPushInfo()
	{
		ddwStockID = 0;
		nSequence = 0;
		dwKLTime_1MIN = 0;
		dwKLTime_5MIN = 0;
		dwKLTime_15MIN = 0;
		dwKLTime_30MIN = 0;
		dwKLTime_60MIN = 0;
		dwKLTime_DAY = 0;
		dwKLTime_WEEK = 0;
		dwKLTime_MONTH = 0;
		dwRTTime = 0;
	}
}Stock_PushInfo, *LPStock_PushInfo;

//经纪队列
typedef struct tagQuoteBrokerItem
{
	bool bAskOrBid;  //ask=卖  bid=买 
	int nBrokerID;
	int nBrokerPos;
	char strBrokerName[32];
}Quote_BrokerItem, *LPQuote_BrokerItem;


enum FT_MARKET_STATUS
{
	FT_MARKET_STATUS_NONE = 0, //无交易，			美股的未开盘
	FT_MARKET_STATUS_JJJY = 1, //竞价交易
	FT_MARKET_STATUS_WAITOPEN = 2,   //早盘前等待开盘
	FT_MARKET_STATUS_MORNING = 3,     //早盘,		美股的盘中
	FT_MARKET_STATUS_NOON_REST = 4,   //午休
	FT_MARKET_STATUS_NOON_TRADE = 5,  //午盘
	FT_MARKET_STATUS_TRADE_OVER = 6, //交易日结束,	美股的已收盘
	FT_MARKET_STATUS_BEFORE_BEGIN = 8, //美股的盘前开始
	FT_MARKET_STATUS_BEFORE_END = 9,	//美股的盘前结束
	FT_MARKET_STATUS_AFTER_BEGIN = 10,	//美股的盘后开始
	FT_MARKET_STATUS_AFTER_END = 11,	//美股的盘后结束
	FT_MARKET_STATUS_FUTU_SWITCH_DATE = 12, //富途的切换交易状态，专门用来只是切一下交易日

	FT_MARKET_STATUS_FUTURE_NIGHT_TRADE = 13,//夜市交易中
	FT_MARKET_STATUS_FUTURE_NIGHT_END = 14,//夜市收盘
	FT_MARKET_STATUS_FUTURE_DAY_TRADE = 15,//日市交易中
	FT_MARKET_STATUS_FUTURE_DAY_BREAK = 16, //日市午休
	FT_MARKET_STATUS_FUTURE_DAY_CLOSE = 17, //日市收盘
	FT_MARKET_STATUS_FUTURE_DAY_WAIT_OPEN = 18, //日市等待开盘

	FT_MARKET_STATUS_HK_CLOSING_AUCTION = 19, //港股盘后竞价
};

//全局状态 
typedef struct tagNNGlobalState
{
	bool bQuoteSvrLogined;
	bool bTradeSvrLogined;

	FT_MARKET_STATUS eMktHK;
	FT_MARKET_STATUS eMktHKFuture;
	FT_MARKET_STATUS eMktUS;
	FT_MARKET_STATUS eMktSH;
	FT_MARKET_STATUS eMktSZ;

	tagNNGlobalState()
	{
		bQuoteSvrLogined = false;
		bTradeSvrLogined = false;

		eMktHK = FT_MARKET_STATUS_NONE;
		eMktHKFuture = FT_MARKET_STATUS_NONE;
		eMktUS = FT_MARKET_STATUS_NONE;
		eMktSH = FT_MARKET_STATUS_NONE;
		eMktSZ = FT_MARKET_STATUS_NONE;
	}
}NNGlobalState;