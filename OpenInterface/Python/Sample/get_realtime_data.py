# -*- coding: utf-8 -*-

#指定加载的openft api目录
import os, sys
sys.path.append(os.path.join(os.path.abspath(__file__),'../../'))
from openft.open_quant_context import *

# Examples for use the python functions


def _example_stock_quote(quote_ctx):
    """
    获取批量报价，输出 股票名称，时间，当前价，开盘价，最高价，最低价，昨天收盘价，成交量，成交额，换手率，振幅，股票状态
    """
    stock_code_list = ["US.AAPL", "HK.00700"]

    # subscribe "QUOTE"
    for stk_code in stock_code_list:
        ret_status, ret_data = quote_ctx.subscribe(stk_code, "QUOTE")
        if ret_status != RET_OK:
            print("%s %s: %s" % (stk_code, "QUOTE", ret_data))
            exit()

    ret_status, ret_data = quote_ctx.query_subscription()

    if ret_status == RET_ERROR:
        print(ret_status)
        exit()

    print(ret_data)

    ret_status, ret_data = quote_ctx.get_stock_quote(stock_code_list)
    if ret_status == RET_ERROR:
        print(ret_data)
        exit()
    quote_table = ret_data

    print("QUOTE_TABLE")
    print(quote_table)


def _example_cur_kline(quote_ctx):
    """
    获取当前K线，输出 股票代码，时间，开盘价，收盘价，最高价，最低价，成交量，成交额
    """
    # subscribe Kline
    stock_code_list = ["US.AAPL", "HK.00700"]
    sub_type_list = ["K_1M", "K_5M", "K_15M", "K_30M", "K_60M", "K_DAY", "K_WEEK", "K_MON"]

    for code in stock_code_list:
        for sub_type in sub_type_list:
            ret_status, ret_data = quote_ctx.subscribe(code, sub_type)
            if ret_status != RET_OK:
                print("%s %s: %s" % (code, sub_type, ret_data))
                exit()

    ret_status, ret_data = quote_ctx.query_subscription()

    if ret_status == RET_ERROR:
        print(ret_data)
        exit()

    print(ret_data)

    for code in stock_code_list:
        for ktype in ["K_DAY", "K_1M", "K_5M"]:
            ret_code, ret_data = quote_ctx.get_cur_kline(code, 5, ktype)
            if ret_code == RET_ERROR:
                print(code, ktype, ret_data)
                exit()
            kline_table = ret_data
            print("%s KLINE %s" % (code, ktype))
            print(kline_table)
            print("\n\n")


def _example_rt_ticker(quote_ctx):
    """
    获取逐笔，输出 股票代码，时间，价格，成交量，成交金额，暂时没什么意义的序列号
    """
    stock_code_list = ["HK.00700", "US.AAPL"]

    # subscribe "TICKER"
    for stk_code in stock_code_list:
        ret_status, ret_data = quote_ctx.subscribe(stk_code, "TICKER")
        if ret_status != RET_OK:
            print("%s %s: %s" % (stk_code, "TICKER", ret_data))
            exit()

    for stk_code in stock_code_list:
        ret_status, ret_data = quote_ctx.get_rt_ticker(stk_code, 3)
        if ret_status == RET_ERROR:
            print(stk_code, ret_data)
            exit()
        print("%s TICKER" % stk_code)
        print(ret_data)
        print("\n\n")


def _example_order_book(quote_ctx):
    """
    获取摆盘数据，输出 买价，买量，买盘经纪个数，卖价，卖量，卖盘经纪个数
    """
    stock_code_list = ["US.AAPL", "HK.00700"]

    # subscribe "ORDER_BOOK"
    for stk_code in stock_code_list:
        ret_status, ret_data = quote_ctx.subscribe(stk_code, "ORDER_BOOK")
        if ret_status != RET_OK:
            print("%s %s: %s" % (stk_code, "ORDER_BOOK", ret_data))
            exit()

    for stk_code in stock_code_list:
        ret_status, ret_data = quote_ctx.get_order_book(stk_code)
        if ret_status == RET_ERROR:
            print(stk_code, ret_data)
            exit()
        print("%s ORDER_BOOK" % stk_code)
        print(ret_data)
        print("\n\n")


def _example_get_trade_days(quote_ctx):
    """
    获取交易日列表，输出 交易日列表
    """
    ret_status, ret_data = quote_ctx.get_trading_days("US", "2017-06-19", "2017-07-20")
    if ret_status == RET_ERROR:
        print(ret_data)
        exit()
    print("TRADING DAYS")
    for x in ret_data:
        print(x)


def _example_stock_basic(quote_ctx):
    """
    获取股票信息，输出 股票代码，股票名，每手数量，股票类型，子类型所属正股
    """
    ret_status, ret_data = quote_ctx.get_stock_basicinfo("HK", "STOCK")
    if ret_status == RET_ERROR:
        print(ret_data)
        exit()
    print("stock_basic")
    print(ret_data)


def _example_get_market_snapshot(quote_ctx):
    """
    获取市场快照，输出 股票代码，更新时间，按盘价，开盘价，最高价，最低价，昨天收盘价，成交量，成交额，换手率，
    停牌状态，上市日期，流通市值，总市值，是否涡轮，换股比例，窝轮类型，行使价格，格式化窝轮到期时间，
    格式化窝轮最后到期时间，窝轮对应的正股，窝轮回收价，窝轮街货量，窝轮发行量，窝轮街货占比，窝轮对冲值，窝轮引伸波幅，
    窝轮溢价
    """
    ret_status, ret_data = quote_ctx.get_market_snapshot(["US.AAPL", "HK.00700"])
    if ret_status == RET_ERROR:
        print(ret_data)
        exit()
    print("market_snapshot")
    print(ret_data)


def _example_rt_data(quote_ctx):
    """
    获取分时数据，输出 时间，数据状态，开盘多少分钟，目前价，昨收价，平均价，成交量，成交额
    """
    stock_code_list = ["HK.00700"]

    for stk_code in stock_code_list:
        ret_status, ret_data = quote_ctx.subscribe(stk_code, "RT_DATA")
        if ret_status != RET_OK:
            print("%s %s: %s" % (stk_code, "RT_DATA", ret_data))
            exit()

    for stk_code in stock_code_list:
        ret_status, ret_data = quote_ctx.get_rt_data(stk_code)
        if ret_status == RET_ERROR:
            print(stk_code, ret_data)
            exit()
        print("%s RT_DATA" % stk_code)
        print(ret_data)
        print("\n\n")


def _example_plate_subplate(quote_ctx):
    """
    获取板块集合下的子板块列表，输出 市场，板块分类,板块代码，名称，ID
    """
    ret_status, ret_data = quote_ctx.get_plate_list("SZ", "ALL")
    if ret_status == RET_ERROR:
        print(ret_data)
        exit()
    print("plate_subplate")
    print(ret_data)


def _example_plate_stock(quote_ctx):
    """
    获取板块下的股票列表，输出 市场，股票每手，股票名称，所属市场，子类型，股票类型
    """
    ret_status, ret_data = quote_ctx.get_plate_stock("SH.BK0531")
    if ret_status == RET_ERROR:
        print(ret_data)
        exit()
    print("plate_stock")
    print(ret_data)


def _example_broker_queue(quote_ctx):
    """
    获取经纪队列，输出 买盘卖盘的经纪ID，经纪名称，经纪档位
    """
    stock_code_list = ["HK.00700"]

    for stk_code in stock_code_list:
        ret_status, ret_data = quote_ctx.subscribe(stk_code, "BROKER")
        if ret_status != RET_OK:
            print("%s %s: %s" % (stk_code, "BROKER", ret_data))
            exit()

    for stk_code in stock_code_list:
        ret_status, bid_data, ask_data = quote_ctx.get_broker_queue(stk_code)
        if ret_status == RET_ERROR:
            print(ret_data)
            exit()
        print("%s BROKER" % stk_code)
        print(ask_data)
        print("\n\n")
        print(bid_data)
        print("\n\n")


def _example_global_state(quote_ctx):
    ret_status, ret_data = quote_ctx.get_global_state()
    if ret_status != RET_OK:
        print("get global state: error, msg: %s" % ret_data)
        exit()
    print("get global state")
    print(ret_data)


if __name__ == "__main__":

    quote_context = OpenQuoteContext(host='127.0.0.1', port=11111)

    # 获取实时数据
    _example_stock_quote(quote_context)
    _example_get_market_snapshot(quote_context)
    _example_cur_kline(quote_context)
    _example_rt_ticker(quote_context)
    _example_order_book(quote_context)
    _example_get_trade_days(quote_context)
    _example_stock_basic(quote_context)
    _example_rt_data(quote_context)
    _example_plate_subplate(quote_context)
    _example_plate_stock(quote_context)
    _example_broker_queue(quote_context)
    _example_global_state(quote_context)


