#pragma once
#include <vector>
#include "ProtoDataStruct.h"


//////////////////////////////////////////////////////////////////////////
//拉取基本报价协议, PROTO_ID_GET_BASIC_PRICE

struct	BasicPriceReqBody
{
	int nStockMarket;
	std::string strStockCode;
	
	BasicPriceReqBody()
	{
		nStockMarket = 0;
	}
};

struct BasicPriceAckBody
{
	int nHigh;
	int nOpen;
	int nClose;
	int nLastClose;
	int nLow;
	int nCur;
	INT64 nVolume;
	INT64 nTurnover;
	INT64 nLotSize;

	int nStockMarket;
	std::string strStockCode;
	DWORD dwTime;

	BasicPriceAckBody()
	{
		nHigh = 0;
		nOpen = 0;
		nClose = 0;
		nLastClose = 0;
		nLow = 0;
		nCur = 0;
		nVolume = 0;
		nTurnover = 0;
		nLotSize = 0;

		nStockMarket = 0;
		dwTime = 0;
	}
};

struct	BasicPrice_Req
{
	ProtoHead			head;
	BasicPriceReqBody	body;
};

struct	BasicPrice_Ack
{
	ProtoHead				head;
	BasicPriceAckBody		body;
};


//////////////////////////////////////////////////////////////////////////
//拉取摆盘信息协议, PROTO_ID_GET_GEAR_PRICE

struct	GearPriceReqBody
{
	int nNum;
	int nStockMarket;	
	std::string strStockCode;	

	GearPriceReqBody()
	{
		nNum = 0;
		nStockMarket = 0;
	}
};

struct GearPriceAckItem
{
	int nBuyOrder;
	int nSellOrder;
	int nBuyPrice;
	int nSellPrice;
	INT64 nBuyVolume;
	INT64 nSellVolume;

	GearPriceAckItem()
	{
		nBuyOrder = 0;
		nSellOrder = 0;
		nBuyPrice = 0;
		nSellPrice = 0;
		nBuyVolume = 0;
		nSellVolume = 0;
	}
};

typedef std::vector<GearPriceAckItem>	VT_GEAR_PRICE;

struct GearPriceAckBody 
{
	int nStockMarket;
	std::string strStockCode;
	VT_GEAR_PRICE vtGear;

	GearPriceAckBody()
	{
		nStockMarket = 0;
	}
};

struct	GearPrice_Req
{
	ProtoHead			head;
	GearPriceReqBody	body;
};

struct	GearPrice_Ack
{
	ProtoHead			head;
	GearPriceAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//拉取分时数据协议, PROTO_ID_QT_GET_RT_DATA

struct	RTDataReqBody
{
	int nStockMarket;	
	std::string strStockCode;

	RTDataReqBody()
	{
		nStockMarket = 0;
	}
};

struct RTDataAckItem
{
	int   nDataStatus; 
	std::wstring strTime; 
	DWORD dwOpenedMins;  //开盘第多少分钟  

	int   nCurPrice;
	DWORD nLastClosePrice; //昨天的收盘价 

	int   nAvgPrice;

	INT64 ddwTDVolume;
	INT64 ddwTDValue;  

	RTDataAckItem()
	{
		nDataStatus = 0;
		dwOpenedMins = 0;

		nCurPrice = 0;
		nLastClosePrice = 0;

		nAvgPrice = 0;

		ddwTDVolume = 0;
		ddwTDValue = 0;
	}
};

typedef std::vector<RTDataAckItem>	VT_RT_DATA;

struct RTDataAckBody 
{
	int nNum;
	int nStockMarket;
	std::string strStockCode;
	VT_RT_DATA vtRTData;

	RTDataAckBody()
	{
		nNum = 0;
		nStockMarket = 0;
	}
};

struct RTData_Req
{
	ProtoHead			head;
	RTDataReqBody		body;
};

struct RTData_Ack
{
	ProtoHead			head;
	RTDataAckBody		body;
};

//////////////////////////////////////////////////////////////////////////
//拉取当前K线数据协议

struct	KLDataReqBody
{
	int nRehabType;
	int nKLType;
	int nStockMarket;
	int nNum;
	std::string strStockCode;

	KLDataReqBody()
	{
		nRehabType = 0;
		nKLType = 0;
		nStockMarket = 0;
		nNum = 0;
	}
};

struct KLDataAckItem
{
	int   nDataStatus; 
	std::wstring strTime; 
	INT64   nOpenPrice;
	INT64   nClosePrice;
	INT64   nHighestPrice;
	INT64   nLowestPrice;
	int   nPERatio; //市盈率(三位小数)
	int   nTurnoverRate;//换手率(正股及指数的日/周/月K线)
	INT64 ddwTDVol; 
	INT64 ddwTDVal;

	KLDataAckItem()
	{
		nDataStatus = 0;
		nOpenPrice = 0;
		nClosePrice = 0;
		nHighestPrice = 0;
		nLowestPrice = 0;
		nPERatio = 0;
		nTurnoverRate = 0;
		ddwTDVol = 0;
		ddwTDVal = 0;
	}
};

typedef std::vector<KLDataAckItem>	VT_KL_DATA;

struct KLDataAckBody 
{
	int nRehabType;
	int nKLType;
	int nStockMarket;
	std::string strStockCode;
	VT_KL_DATA vtKLData;

	KLDataAckBody()
	{
		nRehabType = 0;
		nKLType = 0;
		nStockMarket = 0;
	}
};

struct KLData_Req
{
	ProtoHead			head;
	KLDataReqBody		body;
};

struct KLData_Ack
{
	ProtoHead			head;
	KLDataAckBody		body;
};

/////////////////////////////////////////
//股票订阅协议, PROTO_ID_QT_SUBSTOCK

struct StockSubReqBody
{
	int nStockSubType;
	int nStockMarket;
	std::string strStockCode;

	StockSubReqBody()
	{
		nStockSubType = 0;
		nStockMarket = 0;
	}
};

struct StockSubAckBody
{
	int nStockSubType;
	int nStockMarket;
	std::string strStockCode;

	StockSubAckBody()
	{
		nStockSubType = 0;
		nStockMarket = 0;
	}
};

struct StockSub_Req
{
	ProtoHead		head;
	StockSubReqBody	body;
};

struct StockSub_Ack
{
	ProtoHead			head;
	StockSubAckBody		body;
};

/////////////////////////////////////////
//股票反订阅协议, PROTO_ID_QT_UNSUBSTOCK

struct StockUnSubReqBody
{
	int nStockSubType;
	int nStockMarket;
	std::string strStockCode;

	StockUnSubReqBody()
	{
		nStockSubType = 0;
		nStockMarket = 0;
	}
};

struct StockUnSubAckBody
{
	int nStockSubType;
	int nStockMarket;
	std::string strStockCode;

	StockUnSubAckBody()
	{
		nStockSubType = 0;
		nStockMarket = 0;
	}
};

struct StockUnSub_Req
{
	ProtoHead			head;
	StockUnSubReqBody	body;
};

struct StockUnSub_Ack
{
	ProtoHead				head;
	StockUnSubAckBody		body;
};

/////////////////////
////查询订阅接口PROTO_ID_QT_QueryStockSub
struct QueryStockSubReqBody
{
	QueryStockSubReqBody()
	{
		nQueryAllSocket = 0;
	}
	int nQueryAllSocket;//是否请求所有连接的定阅信息 0：1
};

struct SubInfoAckItem
{
	int nStockSubType;
	int nStockMarket;
	std::wstring strStockCode;

	SubInfoAckItem()
	{
		nStockSubType = 0;
		nStockMarket = 0;
	}
};

typedef std::vector<SubInfoAckItem>	VT_SUB_INFO;

struct QueryStockSubAckBody
{
	VT_SUB_INFO vtSubInfo;
};

struct QueryStockSub_Req
{
	ProtoHead					head;
	QueryStockSubReqBody		body;
};

struct QueryStockSub_Ack
{
	ProtoHead					head;
	QueryStockSubAckBody		body;
};


//////////////////////////////////////////////////////////////////////////
//获得逐笔协议, PROTO_ID_QT_GET_TICKER

struct	TickerReqBody
{
	int nGetTickNum;
	int nStockMarket;	// enum StockMktType
	INT64 nSequence;	// 目前没有起作用
	std::string strStockCode;	

	TickerReqBody()
	{
		nGetTickNum = 0;
		nStockMarket = 0;
		nSequence = 0;
	}
};

struct TickerAckItem
{
	int nPrice;	
	int nDealType;	
	INT64 nSequence;
	INT64 nVolume;
	INT64 nTurnover;
	std::string strTickTime;

	TickerAckItem()
	{
		nPrice = 0;
		nDealType = 0;
		nSequence = 0;
		nVolume = 0;
		nTurnover = 0;
	}

};

typedef std::vector<TickerAckItem>	VT_TICKER_DATA;

struct TickerAckBody 
{
	int nStockMarket;	// enum StockMktType
	std::string strStockCode;
	INT64 nNextSequence;// 目前没有起作用
	VT_TICKER_DATA vtTicker;

	TickerAckBody()
	{
		nStockMarket = 0;
		nNextSequence = 0;
	}
};

struct	Ticker_Req
{
	ProtoHead		head;
	TickerReqBody	body;
};

struct	Ticker_Ack
{
	ProtoHead		head;
	TickerAckBody	body;
};


//////////////////////////////////////////////////////////////////////////
//获得指定时间段的交易日列表, PROTO_ID_QT_GET_TRADE_DATE

struct	TradeDateReqBody
{
	int nStockMarket;	// enum StockMktType	
	std::string strStartDate;//"YYYY-MM-DD"，空字符串表示End往前一年
	std::string strEndDate;//"YYYY-MM-DD"，空字符串表示最新

	TradeDateReqBody()
	{
		nStockMarket = 0;
	}
};

//"YYYY-MM-DD"
typedef std::vector<std::string>	VT_TRADE_DATE; 

struct TradeDateAckBody 
{
	int nStockMarket;	// enum StockMktType	
	std::string strStartDate;
	std::string strEndDate;
	VT_TRADE_DATE vtTradeDate;

	TradeDateAckBody()
	{
		nStockMarket = 0;
	}
};

struct	TradeDate_Req
{
	ProtoHead			head;
	TradeDateReqBody	body;
};

struct	TradeDate_Ack
{
	ProtoHead			head;
	TradeDateAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//获得指定类型的股票信息, PROTO_ID_QT_GET_STOCK_LIST
struct	StockListReqBody
{
	int nStockMarket;	// enum StockMktType
	int nSecurityType;  // enum PluginSecurityType	

	StockListReqBody()
	{
		nStockMarket = 0;
		nSecurityType = 0;
	}
};

struct StockListAckItem
{
	INT64 nStockID;
	int nLotSize;	
	int nSecurityType;  // enum PluginSecurityType	
	int nStockMarket;
	std::string strStockCode;
	std::string strSimpName;

	int nSubType; //如果是涡轮
	INT64 nOwnerStockID; //涡轮正股
	std::string strOwnerStockCode;
	int nOwnerMarketType;
	
	std::string strListDate; //上市时间

	StockListAckItem()
	{
		nStockID = 0;
		nLotSize = 0;
		nSecurityType = 0;
		nStockMarket = 0;

		nSubType = 0;
		nOwnerStockID = 0;
		nOwnerMarketType = 0;
	}
};

typedef std::vector<StockListAckItem>	VT_STOCK_INFO;

struct StockListAckBody 
{
	int nStockMarket;	// enum StockMktType	
	VT_STOCK_INFO vtStockList;

	StockListAckBody()
	{
		nStockMarket = 0;
	}
};

struct	StockList_Req
{
	ProtoHead			head;
	StockListReqBody	body;
};

struct	StockList_Ack
{
	ProtoHead			head;
	StockListAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//获得指定类型的股票信息, PROTO_ID_QT_GET_SNAPSHOT
struct SnapshotReqItem
{
	int nStockMarket; // enum StockMktType
	std::string strStockCode;	

	SnapshotReqItem()
	{
		nStockMarket = 0;
	}
};

typedef std::vector<SnapshotReqItem> VT_REQ_SNAPSHOT;

struct	SnapshotReqBody
{
	VT_REQ_SNAPSHOT vtReqSnapshot;
};

struct SnapshotAckItem
{
	INT64 nStockID;
	std::string strStockCode;
	int nStockMarket; // enum StockMktType
	int instrument_type;// enum PluginSecurityType	

	INT64 last_close_price;
	INT64 nominal_price;
	INT64 open_price;
	INT64  update_time;

	INT64  suspend_flag;
	INT64  listing_status;
	INT64  listing_date;
	INT64 shares_traded;

	INT64 turnover;
	INT64 highest_price;
	INT64 lowest_price;
	int turnover_ratio;
	int ret_err;//0为成功

	INT64 nTatalMarketVal; //市值
	INT64 nCircularMarketVal; //流通市值
	UINT32 nLostSize; //每手
	std::string strUpdateTime; //增加字符串时间

	struct tagWarrantData
	{
		BOOL bDataValid;  //如果非涡轮 == 0 
		int  nWarrantType;  //涡轮类型 Quote_WarrantType
		UINT32 nConversionRatio; //换股比率
		INT64  nStrikePrice; //行使价

		INT64  nMaturityDate; //到期日
		std::string strMaturityData;
		INT64 nEndtradeDate;  //最后交易日
		std::string strEndtradeDate;

		std::string strOwnerStockCode;
		int nOwnerStockMarket; 

		INT64 nRecoveryPrice; //回收价
		UINT64 nStreetVol;  //街货量
		UINT64 nIssueVol;  //发行量
		INT64 nOwnerStockPrice;  //正股价格

		int nStreetRatio; //街货占比
		int nDelta;	 //对冲值
		int nImpliedVolatility; //引伸波幅
		int nPremium; //溢价

		tagWarrantData()
		{
			bDataValid = 0; nWarrantType = 0; nConversionRatio = 0; nStrikePrice = 0;
			nMaturityDate = 0; nEndtradeDate = 0;
			nOwnerStockMarket = 0;
			nRecoveryPrice = 0; nStreetVol = 0; nIssueVol = 0; nOwnerStockPrice = 0;
			nStreetRatio = 0; nDelta = 0; nImpliedVolatility = 0; nPremium = 0;
		}
	}stWrtData;

	SnapshotAckItem()
	{
		nStockID = 0;
		nStockMarket = 0;
		instrument_type = 0;

		last_close_price = 0;
		nominal_price = 0;
		open_price = 0;
		update_time = 0;

		suspend_flag = 0;
		listing_status = 0;
		listing_date = 0;
		shares_traded = 0;

		turnover = 0;
		highest_price = 0;
		lowest_price = 0;
		turnover_ratio = 0;
		ret_err = 0;

		nTatalMarketVal = 0;
		nCircularMarketVal = 0;
		nLostSize = 0;
	}
};

typedef std::vector<SnapshotAckItem>	VT_ACK_SNAPSHOT;

struct SnapshotAckBody 
{	
	VT_ACK_SNAPSHOT vtSnapshot;
};

struct	Snapshot_Req
{
	ProtoHead		head;
	SnapshotReqBody	body;
};

struct	Snapshot_Ack
{
	ProtoHead		head;
	SnapshotAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//批量报价, PROTO_ID_QT_GET_STOCK_INFO

struct BatchBasicReqItem
{
	int nStockMarket;
	std::string strStockCode;	

	BatchBasicReqItem()
	{
		nStockMarket = 0;
	}
};

typedef std::vector<BatchBasicReqItem>	VT_REQ_BATCHBASIC;

struct BatchBasicReqBody
{
	VT_REQ_BATCHBASIC vtReqBatchBasic;
};

struct BatchBasicAckItem
{
	int nStockMarket;
	std::string strStockCode;	

	int nHigh;
	int nOpen;
	int nLastClose;
	int nLow;

	int nCur;
	int nSuspension;
	int nTurnoverRate;

	INT64 nVolume;
	INT64 nValue;
	INT64 nAmpli;
	std::wstring strDate; 
	std::wstring strTime;
	std::wstring strListTime; 

	BatchBasicAckItem()
	{
		nStockMarket = 0;
		nHigh = 0;
		nOpen = 0;
		nLastClose = 0;
		nLow = 0;

		nCur = 0;
		nSuspension = 0;
		nTurnoverRate = 0;

		nVolume = 0;
		nValue = 0;
		nAmpli = 0;
	}
};

typedef std::vector<BatchBasicAckItem>	VT_ACK_BATCHBASIC;

struct BatchBasicAckBody 
{	
	VT_ACK_BATCHBASIC vtAckBatchBasic;
};

struct BatchBasic_Req
{
	ProtoHead			head;
	BatchBasicReqBody	body;
};

struct BatchBasic_Ack
{
	ProtoHead			head;
	BatchBasicAckBody	body;
};

//获得历史K线, PROTO_ID_QT_GET_HISTORYKL

struct	HistoryKLReqBody
{
	int nRehabType;
	int nKLType;
	int nStockMarket;
	std::string strStockCode;
	std::string strStartDate;
	std::string strEndDate;

	HistoryKLReqBody()
	{
		nRehabType = 0;
		nKLType = 0;
		nStockMarket = 0;
	}
};

struct HistoryKLAckItem
{
	std::wstring strTime; 
	INT64   nOpenPrice;
	INT64   nClosePrice;
	INT64   nHighestPrice;
	INT64   nLowestPrice;

	int   nPERatio; //市盈率(三位小数)
	int   nTurnoverRate;//换手率(正股及指数的日/周/月K线)
	INT64 ddwTDVol; 
	INT64 ddwTDVal;

	HistoryKLAckItem()
	{
		nOpenPrice = 0;
		nClosePrice = 0;
		nHighestPrice = 0;
		nLowestPrice = 0;

		nPERatio = 0;
		nTurnoverRate = 0;
		ddwTDVol = 0;
		ddwTDVal = 0;
	}
};

typedef std::vector<HistoryKLAckItem>	VT_HISTORY_KL;

struct HistoryKLAckBody 
{
	int nRehabType;
	int nKLType;
	int nStockMarket;
	std::string strStockCode;
	std::string strStartDate;
	std::string strEndDate;
	VT_HISTORY_KL vtHistoryKL;

	HistoryKLAckBody()
	{
		nRehabType = 0;
		nKLType = 0;
		nStockMarket = 0;
	}
};

struct HistoryKL_Req
{
	ProtoHead				head;
	HistoryKLReqBody		body;
};

struct HistoryKL_Ack
{
	ProtoHead				head;
	HistoryKLAckBody		body;
};

//////////////////////////////////////////////////////////////////////////
//除权, PROTO_ID_QT_GET_EXRIGHTINFO
struct ExRightInfoReqItem
{
	int nStockMarket; // enum StockMktType
	std::string strStockCode;	

	ExRightInfoReqItem()
	{
		nStockMarket = 0;
	}
};

typedef std::vector<ExRightInfoReqItem> VT_REQ_EXRIGHTINFO;

struct ExRightInfoReqBody
{
	VT_REQ_EXRIGHTINFO vtReqExRightInfo;
};

struct ExRightInfoAckItem
{
	int nStockMarket; // enum StockMktType
	std::string strStockCode;	
	std::string str_ex_date;    // 除权除息日期, 例如20160615
	
	INT32 split_ratio;//拆合股比例
	INT64 per_cash_div;//现金派现
	INT32 per_share_ratio;//送股比例
	INT32 per_share_trans_ratio;//转增股比例

	INT32 allotment_ratio;//配股比例
	INT64 allotment_price;//配股价
	INT32 stk_spo_ratio;//增发比例
	INT64 stk_spo_price;//增发价格

	// result_self
	INT64 fwd_factor_a;
	INT64 fwd_factor_b;
	INT64 bwd_factor_a;
	INT64 bwd_factor_b;

	// 简体中文文本描述
	//std::wstring str_sc_txt;
	// 繁体中文文本描述
	//std::wstring str_tc_txt;

	ExRightInfoAckItem()
	{
		nStockMarket = 0;
		split_ratio = 0;
		per_cash_div = 0;
		per_share_ratio = 0;
		per_share_trans_ratio = 0;

		allotment_ratio = 0;
		allotment_price = 0;
		stk_spo_ratio = 0;
		stk_spo_price = 0;

		fwd_factor_a = 0;
		fwd_factor_b = 0;
		bwd_factor_a = 0;
		bwd_factor_b = 0;
	}
};

typedef std::vector<ExRightInfoAckItem>	VT_ACK_EXRIGHTINFO;

struct ExRightInfoAckBody 
{	
	VT_ACK_EXRIGHTINFO vtAckExRightInfo;
};

struct	ExRightInfo_Req
{
	ProtoHead		head;
	ExRightInfoReqBody	body;
};

struct ExRightInfo_Ack
{
	ProtoHead		head;
	ExRightInfoAckBody	body;
};

/////////////////////////////////////////
//推送协议, PROTO_ID_QT_PushStockData

struct PushStockDataReqBody
{
	int nStockPushType;
	int nStockMarket;
	int nUnPush;
	std::string strStockCode;

	PushStockDataReqBody()
	{
		nStockPushType = 0;
		nStockMarket = 0;
		nUnPush = 0;
	}
};

struct PushStockDataAckBody
{
	int nStockPushType;
	int nStockMarket;
	std::string strStockCode;

	PushStockDataAckBody()
	{
		nStockPushType = 0;
		nStockMarket = 0;
	}
};

struct PushStockData_Req
{
	ProtoHead		head;
	PushStockDataReqBody	body;
};

struct PushStockData_Ack
{
	ProtoHead					head;
	PushStockDataAckBody		body;
};


//推送报价, PROTO_PUSH_BATCHPRICE

struct PushBatchBasicReqItem
{
	int nStockMarket;
	std::string strStockCode;

	PushBatchBasicReqItem()
	{
		nStockMarket = 0;
	}
};

typedef std::vector<PushBatchBasicReqItem>	VT_REQ_PUSHBATCHBASIC;

struct PushBatchBasicReqBody
{
	VT_REQ_PUSHBATCHBASIC vtReqBatchBasic;
};

struct PushBatchBasicAckItem
{
	int nStockMarket;
	std::string strStockCode;

	int nHigh;
	int nOpen;
	int nLastClose;
	int nLow;

	int nCur;
	int nSuspension;
	int nTurnoverRate;
	INT64 nVolume;
	INT64 nValue;
	INT64 nAmpli;
	std::wstring strDate;
	std::wstring strTime;
	std::wstring strListTime;

	PushBatchBasicAckItem()
	{
		nStockMarket = 0;
		nHigh = 0;
		nOpen = 0;
		nLastClose = 0;
		nLow = 0;

		nCur = 0;
		nSuspension = 0;
		nTurnoverRate = 0;
		nVolume = 0;
		nValue = 0;
		nAmpli = 0;
	}
};

typedef std::vector<PushBatchBasicAckItem>	VT_ACK_PUSHBATCHBASIC;

struct PushBatchBasicAckBody
{
	VT_ACK_PUSHBATCHBASIC vtAckBatchBasic;
};

struct PushBatchBasic_Req
{
	ProtoHead			head;
	PushBatchBasicReqBody	body;
};

struct PushBatchBasic_Ack
{
	ProtoHead			head;
	PushBatchBasicAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//推送摆盘, PROTO_ID_PUSH_GERA

struct	PushGearPriceReqBody
{
	int nNum;
	int nStockMarket;
	std::string strStockCode;

	PushGearPriceReqBody()
	{
		nNum = 0;
		nStockMarket = 0;
	}
};

struct PushGearPriceAckItem
{
	int nBuyOrder;
	int nSellOrder;
	int nBuyPrice;
	int nSellPrice;
	INT64 nBuyVolume;
	INT64 nSellVolume;

	PushGearPriceAckItem()
	{
		nBuyOrder = 0;
		nSellOrder = 0;
		nBuyPrice = 0;
		nSellPrice = 0;
		nBuyVolume = 0;
		nSellVolume = 0;
	}
};

typedef std::vector<PushGearPriceAckItem>	VT_GEAR_PRICE_PUSH;

struct PushGearPriceAckBody
{
	int nStockMarket;
	std::string strStockCode;
	VT_GEAR_PRICE_PUSH vtGear;
};

struct	PushGearPrice_Req
{
	ProtoHead			head;
	PushGearPriceReqBody	body;
};

struct	PushGearPrice_Ack
{
	ProtoHead			head;
	PushGearPriceAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//推送逐笔协议, PROTO_ID_PUSH_TICKER

struct	PushTickerReqBody
{
	int nGetTickNum;
	int nStockMarket;	// enum StockMktType
	INT64 nSequence;	// 目前没有起作用
	std::string strStockCode;

	PushTickerReqBody()
	{
		nGetTickNum = 0;
		nStockMarket = 0;
		nSequence = 0;
	}
};

struct PushTickerAckItem
{
	int nPrice;
	int nDealType;
	INT64 nSequence;
	INT64 nVolume;
	INT64 nTurnover;
	std::string strTickTime;

	PushTickerAckItem()
	{
		nPrice = 0;
		nDealType = 0;
		nSequence = 0;
		nVolume = 0;
		nTurnover = 0;
	}
};

typedef std::vector<PushTickerAckItem>	VT_TICKER_DATA_PUSH;

struct PushTickerAckBody
{
	int nStockMarket;	// enum StockMktType
	std::string strStockCode;
	INT64 nNextSequence;// 目前没有起作用
	VT_TICKER_DATA_PUSH vtTicker;

	PushTickerAckBody()
	{
		nStockMarket = 0;
		nNextSequence = 0;
	}
};

struct	PushTicker_Req
{
	ProtoHead		head;
	PushTickerReqBody	body;
};

struct	PushTicker_Ack
{
	ProtoHead		head;
	PushTickerAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//推送K线数据协议

struct	PushKLDataReqBody
{
	int nRehabType;
	int nKLType;
	int nStockMarket;
	int nNum;
	std::string strStockCode;

	PushKLDataReqBody()
	{
		nRehabType = 0;
		nKLType = 0;
		nStockMarket = 0;
		nNum = 0;
	}
};

struct PushKLDataAckItem
{
	std::wstring strTime;
	INT64   nOpenPrice;
	INT64   nClosePrice;
	INT64   nHighestPrice;
	INT64   nLowestPrice;

	int   nPERatio; //市盈率(三位小数)
	int   nTurnoverRate;//换手率(正股及指数的日/周/月K线)
	INT64 ddwTDVol;
	INT64 ddwTDVal;

	PushKLDataAckItem()
	{
		nOpenPrice = 0;
		nClosePrice = 0;
		nHighestPrice = 0;
		nLowestPrice = 0;

		nPERatio = 0;
		nTurnoverRate = 0;
		ddwTDVol = 0;
		ddwTDVal = 0;
	}
};

typedef std::vector<PushKLDataAckItem>	VT_KL_DATA_PUSH;

struct PushKLDataAckBody
{
	int nRehabType;
	int nKLType;
	int nStockMarket;
	std::string strStockCode;
	VT_KL_DATA_PUSH vtKLData;

	PushKLDataAckBody()
	{
		nRehabType = 0;
		nKLType = 0;
		nStockMarket = 0;
	}
};

struct PushKLData_Req
{
	ProtoHead			head;
	PushKLDataReqBody		body;
};

struct PushKLData_Ack
{
	ProtoHead			head;
	PushKLDataAckBody		body;
};

//////////////////////////////////////////////////////////////////////////
//推送分时数据协议

struct	PushRTDataReqBody
{
	int nStockMarket;
	std::string strStockCode;

	PushRTDataReqBody()
	{
		nStockMarket = 0;
	}
};

struct PushRTDataAckItem
{
	int   nDataStatus;
	std::wstring strTime;
	DWORD dwOpenedMins;  //开盘第多少分钟  

	int   nCurPrice;
	DWORD nLastClosePrice; //昨天的收盘价 

	int   nAvgPrice;

	INT64 ddwTDVolume;
	INT64 ddwTDValue;

	PushRTDataAckItem()
	{
		nDataStatus = 0;
		dwOpenedMins = 0;

		nCurPrice = 0;
		nLastClosePrice = 0;

		nAvgPrice = 0;

		ddwTDVolume = 0;
		ddwTDValue = 0;
	}
};

typedef std::vector<PushRTDataAckItem>	VT_RT_DATA_PUSH;

struct PushRTDataAckBody
{
	int nNum;
	int nStockMarket;
	std::string strStockCode;
	VT_RT_DATA_PUSH vtRTData;

	PushRTDataAckBody()
	{
		nNum = 0;
		nStockMarket = 0;
	}
};

struct PushRTData_Req
{
	ProtoHead			head;
	PushRTDataReqBody		body;
};

struct PushRTData_Ack
{
	ProtoHead			head;
	PushRTDataAckBody		body;
};

//指定板块集合下的板块ID列表
//
enum PlateClass
{
	PlateClass_All = 0,
	PlateClass_Industry = 1,  //板块行业分类
	PlateClass_Region = 2,    //板块地域分类
	PlateClass_Concept = 3,	  //板块概念分类
};

struct	PlatesetIDsReqBody
{
	int nPlateClassType;
	int nStockMarket;

	PlatesetIDsReqBody()
	{
		nPlateClassType = 0;
		nStockMarket = 0;
	}
};

struct PlatesetIDsAckItem
{
	int nStockMarket;
	std::string strStockCode;
	std::string strStockName;
	UINT64  nStockID;

	PlatesetIDsAckItem()
	{
		nStockMarket = 0;
		nStockID = 0;
	}
};
typedef std::vector<PlatesetIDsAckItem>	VT_PlatesetID;

struct PlatesetIDsAckBody
{
	int nPlateClassType;
	int nStockMarket;

	VT_PlatesetID vtPlatesetIDs;

	PlatesetIDsAckBody()
	{
		nPlateClassType = 0;
		nStockMarket = 0;
	}
};

struct	PlatesetIDs_Req
{
	ProtoHead			head;
	PlatesetIDsReqBody	body;
};

struct	PlatesetIDs_Ack
{
	ProtoHead				head;
	PlatesetIDsAckBody		body;
};

//指定板块集合下的股票ID列表
//
struct	PlateSubIDsReqBody
{
	int nStockMarket;
	std::string strStockCode;

	PlateSubIDsReqBody()
	{
		nStockMarket = 0;
	}
};

typedef StockListAckItem PlateSubIDsAckItem;
typedef std::vector<PlateSubIDsAckItem>	VT_PlateSubID;

struct PlateSubIDsAckBody
{
	int nStockMarket;
	std::string strStockCode;

	VT_PlateSubID vtPlateSubIDs;

	PlateSubIDsAckBody()
	{
		nStockMarket = 0;
	}
};

struct	PlateSubIDs_Req
{
	ProtoHead			head;
	PlateSubIDsReqBody	body;
};

struct	PlateSubIDs_Ack
{
	ProtoHead				head;
	PlateSubIDsAckBody		body;
};

//////////////////////////////////////////////////////////////////////////
//经纪队列协议, PROTO_ID_GET_BROKER_QUEUE

struct	BrokerQueueReqBody
{
	int nStockMarket;
	std::string strStockCode;

	BrokerQueueReqBody()
	{
		nStockMarket = 0;
	}
};

struct BrokerQueueAckItem
{
	int nBrokerID;	//经纪id 
	std::string strBrokerName;  
	int nBrokerPos;  //经纪摆盘档位, 取值 0 , 1, 2, 3 ...

	BrokerQueueAckItem()
	{
		nBrokerID = 0;
		nBrokerPos = 0;
	}
};

typedef std::vector<BrokerQueueAckItem>	VT_BROKER_QUEUE;

struct BrokerQueueAckBody
{
	int nStockMarket;
	std::string strStockCode;
	VT_BROKER_QUEUE vtBrokerAsk; //卖盘经纪
	VT_BROKER_QUEUE vtBrokerBid; //买盘经纪

	BrokerQueueAckBody()
	{
		nStockMarket = 0;
	}
};

struct	BrokerQueue_Req
{
	ProtoHead			head;
	BrokerQueueReqBody	body;
};

struct	BrokerQueue_Ack
{
	ProtoHead			head;
	BrokerQueueAckBody	body;
};



//////////////////////////////////////////////////////////////////////////
//获取全局状态 , PROTO_ID_GET_GLOBAL_STATE

struct	GlobalStateReqBody
{
	int nStateType;  //no use now 
	
	GlobalStateReqBody()
	{
		nStateType = 0;
	}
};

struct GlobalStateAckBody
{
	int nMarketStateHK;
	int nMarketStateUS;
	int nMarketStateSH;
	int nMarketStateSZ;
	int nMarketStateHKFuture;
	int nQuoteLogined;
	int nTradeLogined;

	GlobalStateAckBody()
	{
		memset(this, 0, sizeof(*this));
	}
};

struct	GlobalState_Req
{
	ProtoHead			head;
	GlobalStateReqBody	body;
};

struct	GlobalState_Ack
{
	ProtoHead			head;
	GlobalStateAckBody	body;
};
