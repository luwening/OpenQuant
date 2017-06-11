#pragma once
#include <vector>

//参考"需求.txt"

//协议ID
enum
{
	PROTO_ID_QUOTE_MIN				= 1001,
	PROTO_ID_QT_GET_BASIC_PRICE		= 1001,  //报价
	PROTO_ID_QT_GET_GEAR_PRICE		= 1002,	 //十档
	PROTO_ID_QT_SUBSTOCK			= 1005,
	PROTO_ID_QT_UNSUBSTOCK			= 1006,
	PROTO_ID_QT_QueryStockSub		= 1007,
	PROTO_ID_QT_PushStockData		= 1008,
	PROTO_ID_QT_GET_RTDATA			= 1010,	 //分时
	PROTO_ID_QT_GET_KLDATA			= 1011,  //K线
	PROTO_ID_QT_GET_TICKER			= 1012,  //逐笔
	PROTO_ID_QT_GET_TRADE_DATE		= 1013,  //交易日
	PROTO_ID_QT_GET_STOCK_LIST		= 1014,  //股票信息
	PROTO_ID_QT_GET_SNAPSHOT		= 1015,  //市场快照
	PROTO_ID_QT_GET_BATCHBASIC		= 1023,  //批量报价
	PROTO_ID_QT_GET_HISTORYKL		= 1024,  //历史K线
	PROTO_ID_QT_GET_EXRIGHTINFO		= 1025,  //复权因子
	PROTO_ID_QT_GET_PLATESETIDS		= 1026,  //板块集合的列表
	PROTO_ID_QT_GET_PLATESUBIDS		= 1027,  //板块下的股票列表
	PROTO_ID_QT_GET_BROKER_QUEUE	= 1028,  //经纪队列
	PROTO_ID_QT_GET_GLOBAL_STATE	= 1029,  //全局状态 

	PROTO_ID_PUSH_BATCHPRICE        = 1030,  //推送报价
	PROTO_ID_PUSH_GEARPRICE			= 1031,  //推送摆盘
	PROTO_ID_PUSH_KLDATA			= 1032,  //推送K线
	PROTO_ID_PUSH_TICKER			= 1033,  //推送逐笔
	PROTO_ID_PUSH_RTDATA			= 1034,  //推送分时
	PROTO_ID_PUSH_BROKER_QUEUE		= 1035,	 //推送经纪队列

	PROTO_ID_QUOTE_MAX				= 1999,  

	PROTO_ID_TRADE_HK_MIN			= 6003,	  
	PROTO_ID_TDHK_PLACE_ORDER		= 6003,   //下单
	PROTO_ID_TDHK_SET_ORDER_STATUS	= 6004,   //订单状态更改
	PROTO_ID_TDHK_CHANGE_ORDER		= 6005,   //改单
	PROTO_ID_TDHK_UNLOCK_TRADE		= 6006,	  //解锁交易
	PROTO_ID_TDHK_QUERY_ACC_INFO	= 6007,	  //查询帐户信息
	PROTO_ID_TDHK_QUERY_ORDER		= 6008,	  //查询港股订单列表
	PROTO_ID_TDHK_QUERY_POSITION	= 6009,	  //查询港股持仓
	PROTO_ID_TDHK_QUERY_DEAL		= 6010,	  //查询港股成交记录
	PROTO_ID_TRADE_HK_MAX			= 6999,    

	PROTO_ID_TRADE_US_MIN           = 7003,
	PROTO_ID_TDUS_PLACE_ORDER		= 7003,   //下单
	PROTO_ID_TDUS_SET_ORDER_STATUS	= 7004,   //订单状态更改
	PROTO_ID_TDUS_CHANGE_ORDER		= 7005,   //改单

	PROTO_ID_TDUS_QUERY_ACC_INFO	= 7007,	  //查询帐户信息
	PROTO_ID_TDUS_QUERY_ORDER		= 7008,	  //查询美股订单列表
	PROTO_ID_TDUS_QUERY_POSITION	= 7009,	  //查询美股持仓
	PROTO_ID_TDUS_QUERY_DEAL		= 7010,	  //查询美股成交列表
	PROTO_ID_TRADE_US_MAX			= 7999,    

};

#define KEY_REQ_PARAM	"ReqParam"
#define KEY_ACK_DATA	"RetData"
#define FIELD_KEY_HEAD  "head"
#define FIELD_KEY_BODY  "body"

enum ProtoErrCode
{
	PROTO_ERR_NO_ERROR	= 0,

	PROTO_ERR_UNKNOWN_ERROR = 400,   //未知错误
	PROTO_ERR_VER_NOT_SUPPORT = 401,  //版本号不支持
	PROTO_ERR_STOCK_NOT_FIND = 402,   //未知股票
	PROTO_ERR_COMMAND_NOT_SUPPORT = 403,
	PROTO_ERR_PARAM_ERR = 404,
	PROTO_ERR_FREQUENCY_ERR = 405,
	PROTO_ERR_MAXSUB_ERR = 406,
	PROTO_ERR_UNSUB_ERR = 407,
	PROTO_ERR_UNSUB_TIME_ERR = 408,

	PROTO_ERR_SERVER_BUSY	= 501,
	PROTO_ERR_SERVER_TIMEROUT = 502,
	PROTO_ERR_NETWORK = 503,
};

//////////////////////////////////////////////////////////////////////////
//通用协议头部
#define  ProtoHead_Version  1 

struct ProtoHead
{
	int   nProtoVer;
	int   nProtoID;
	INT64 ddwErrCode;
	std::string strErrDesc;

	ProtoHead()
	{
		nProtoVer = ProtoHead_Version;
		nProtoID = 0;
		ddwErrCode = 0;
	}

};
