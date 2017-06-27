# coding:utf-8
import sys
import path
folder = path.path(__file__).abspath()
openft_folder = folder.parent.parent
sys.path.append(openft_folder)
from openft.open_quant_context import *
import numpy as np
import matplotlib.pyplot as plt
from math import floor

'''
    跟踪止损:跟踪止损是一种更高级的条件单，需要指定如下参数，以便制造出移动止损价
        跟踪止损的详细介绍：https://www.futu5.com/faq/topic214
    api_svr_ip: (string)ip
    api_svr_port: (int)port
    unlock_password: (string)交易解锁密码, 必需修改！！！，模拟交易设为一个非空字符串即可
    code: (string)股票
    trade_env:
        0: 真实交易
        1: 模拟交易 （美股暂不支持模拟交易）
    method:
        0:股票下跌drop价格就会止损
        1:股票下跌drop的百分比就会止损
    drop:
        method==0,股票下跌的价格
        method==1，股票下跌的百分比，0.01表示下跌1%则止损
    volume: 需要卖掉的股票数量
    how_to_sell：以何种方式卖出股票，默认值为0
        0: 以(市价-diff)的价格卖出
        1: 以smart_sell方式卖出
    diff： 默认为0，当how_to_sell为0时，以(市价-diff)的价格卖出
'''

# 全局参数配置
api_svr_ip = '127.0.0.1'
api_svr_port = 11111
unlock_password = "a"
code = 'HK.00005'  # 'US.BABA' #'HK.00700'
trade_env = 1
method = 0
drop = 0.05
volume = 800
how_to_sell = 1
diff = 0
#

# 程序中的一些全局变量，不需要修改
RET_OK = 0
RET_ERROR = -1
#


class StockSeller:
    @staticmethod
    def simple_sell(
            quote_ctx,
            trade_ctx,
            passwd,
            stock_code,
            trade_price,
            trade_env):
        lot_size = 0
        while True:
            if trade_env == 0:
                ret, data = trade_ctx.unlock_trade(passwd)
                if ret != RET_OK:
                    print('trying to unlock trade {}'.format(data))
                    continue
            if lot_size == 0:
                ret, data = quote_ctx.get_market_snapshot([stock_code])
                lot_size = data.loc[0, 'lot_size'] if ret == 0 else 0
                if ret != 0:
                    print("can't get lot_size, retrying")
                    continue
                elif lot_size <= 0:
                    raise BaseException('lot_size error {}'.format(stock_code))
            qty = floor(volume / lot_size) * lot_size
            trade_ctx.place_order(
                price=trade_price,
                qty=qty,
                strcode=stock_code,
                orderside=1,
                envtype=trade_env)
            return

    @staticmethod
    def smart_sell(quote_ctx, trade_ctx, passwd, stock_code, trade_env):
        lot_size = 0
        while True:
            if trade_env == 0:
                ret, data = trade_ctx.unlock_trade(passwd)
                if ret != RET_OK:
                    print('trying to unlock trade {}'.format(data))
                    continue
            if lot_size == 0:
                ret, data = quote_ctx.get_market_snapshot([stock_code])
                lot_size = data.loc[0, 'lot_size'] if ret == 0 else 0
                if ret != 0:
                    print("can't get lot_size, retrying")
                    continue
                elif lot_size <= 0:
                    raise BaseException('lot_size error {}'.format(stock_code))
            qty = floor(volume / lot_size) * lot_size
            ret, data = quote_ctx.subscribe(stock_code, 'ORDER_BOOK')
            if ret != 0:
                print("retrying to subscribe orderbook")
                continue
            ret, data = quote_ctx.get_order_book(stock_code)
            if ret != RET_OK:
                print('can not get orderbook,retrying')
                continue
            price = data['Bid'][0][0]
            print('bid price is {}'.format(price))
            trade_ctx.place_order(
                price=price,
                qty=qty,
                strcode=stock_code,
                orderside=1,
                envtype=trade_env)
            return


class TrailingStopHandler(StockQuoteHandlerBase):
    def __init__(self,
                 quote_ctx, trade_ctx,
                 unlock_password="",
                 code='HK.00700',
                 trade_env=1,
                 method=0,
                 drop=1,
                 volume=1000,
                 how_to_sell=1,
                 diff=0):
        self.quote_ctx = quote_ctx
        self.trade_ctx = trade_ctx
        self.unlock_password = unlock_password
        self.code = code
        self.trade_env = trade_env
        self.method = method
        self.drop = drop
        self.volume = volume
        self.how_to_sell = how_to_sell
        self.diff = diff
        # 是否完成交易
        self.finished = False
        # 截止线的价格
        self.stop = None
        self.pricelist = []
        self.stoplist = []
        self.timelist = []

    def on_recv_rsp(self, rsp_str):
        ret_code, content = super(
            TrailingStopHandler, self).on_recv_rsp(rsp_str)
        if self.finished:
            return ret_code,content
        if ret_code != RET_OK:
            print("StockQuoteTest: error, msg: %s" % content)
            return RET_ERROR, content
        last_price = content.loc[0, 'last_price']
        print('last_price={}'.format(last_price))
        if self.stop is None:
            self.stop = last_price - \
                self.drop if self.method == 0 else last_price * (1 - self.drop)
        elif (self.stop + self.drop < last_price) if self.method == 0 else (self.stop < last_price * (1 - self.drop)):
            self.stop = last_price - \
                self.drop if self.method == 0 else last_price * (1 - self.drop)
        elif self.stop >= last_price:
            # 交易股票
            print('sell the stock, stop={}'.format(self.stop))
            if how_to_sell == 0:
                StockSeller.simple_sell(
                    quote_ctx=self.quote_ctx,
                    trade_ctx=self.trade_ctx,
                    passwd=self.unlock_password,
                    stock_code=self.code,
                    trade_price=self.stop - self.diff,
                    trade_env=self.trade_env)
            else:
                StockSeller.smart_sell(
                    quote_ctx=self.quote_ctx,
                    trade_ctx=self.trade_ctx,
                    passwd=self.unlock_password,
                    stock_code=self.code,
                    trade_env=self.trade_env)
            self.finished = True
        self.pricelist.append(last_price)
        self.stoplist.append(self.stop)
        return RET_OK, content


def trailingstop(
        api_svr_ip='127.0.0.1',
        api_svr_port=11111,
        unlock_password="",
        code='HK.00700',
        trade_env=1,
        method=0,
        drop=1,
        volume=1000,
        how_to_sell=1,
        diff=0):
    if unlock_password == "":
        raise "请先配置交易解锁密码!"
    quote_ctx = OpenQuoteContext(host=api_svr_ip, port=api_svr_port)
    trade_ctx = None
    is_hk_trade = 'HK.' in (code)
    if is_hk_trade:
        trade_ctx = OpenHKTradeContext(host=api_svr_ip, port=api_svr_port)
    else:
        if trade_env != 0:
            raise "美股交易接口不支持仿真环境"
        trade_ctx = OpenUSTradeContext(host=api_svr_ip, port=api_svr_port)
    stop = None
    ret, data = quote_ctx.subscribe(code, 'QUOTE', push=True)
    if ret != RET_OK:
        print('error {0}:{1}'.format(ret, data))
        exit(ret)
    trailing_stop_handler = TrailingStopHandler(
        quote_ctx,
        trade_ctx,
        unlock_password,
        code,
        trade_env,
        method,
        drop,
        volume,
        how_to_sell,
        diff)
    quote_ctx.set_handler(trailing_stop_handler)
    quote_ctx.start()
    while True:
        if trailing_stop_handler.finished:
            pricelist = trailing_stop_handler.pricelist
            plt.plot(np.arange(len(pricelist)), pricelist, color='blue')
            stoplist = trailing_stop_handler.stoplist
            plt.plot(np.arange(len(stoplist)), stoplist)
            plt.show()
            break
    quote_ctx.stop()
    trade_ctx.stop()


def demo():
    quote_ctx = OpenQuoteContext(host='127.0.0.1', port=11111)
    code = 'HK.00700'
    drop = 0.8
    diff = 0.2
    count = 100
    stop = None
    stop_list = []
    quote_ctx.subscribe(code, 'RT_DATA')
    ret, data = quote_ctx.get_rt_data(code)
    if ret != RET_OK:
        print('error {0}:{1}'.format(ret, data))
        exit(ret)
    price_list = data['cur_price']
    time = data['time']
    for i in range(len(price_list)):
        price = price_list[i]
        if stop is None:
            stop = price - drop
        elif price > stop + drop:
            stop = price - drop
        elif price <= stop:
            # sell count stocks at price-diff
            print('sell the stock {0} at price {1},{2}'.format(
                count, price - diff, i))
            break
        stop_list.append(stop)
    stop_list = np.array(stop_list)
    # stop_list = np.concatenate(
    #     (stop_list, stop_list[-1]+np.zeros(len(price_list)-len(stop_list))))
    x = np.arange(len(price_list))
    plt.plot(x, price_list, color='b')
    x = np.arange(len(stop_list))
    plt.plot(x, stop_list, color='r')
    plt.xlabel('time(m)')
    plt.ylabel('price')
    plt.show()


if __name__ == '__main__':
    trailingstop(
        api_svr_ip,
        api_svr_port,
        unlock_password,
        code,
        trade_env,
        method,
        drop,
        volume,
        how_to_sell,
        diff)
