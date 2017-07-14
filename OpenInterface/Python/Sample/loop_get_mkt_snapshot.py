# -*- coding: utf-8 -*-

#指定加载的openft api目录
import os, sys
sys.path.append(os.path.abspath('../'))
from openft.open_quant_context import *

from time import  sleep
import threading

'''
  验证接口：获取某个市场的全部快照数据 get_mkt_snapshot
  api_svr_ip: (string)ip
  api_svr_port: (int)port
'''

#全局参数配置
api_svr_ip = '127.0.0.1'
api_svr_port = 11111
market = 'US' #'US', 'SH', 'SZ'
#

def loop_get_mkt_snapshot(api_svr_ip, api_svr_port , market):
    # 创建行情api
    quote_ctx = OpenQuoteContext(host=api_svr_ip, port=api_svr_port)
    stock_type = ['STOCK', 'IDX', 'ETF', 'WARRANT', 'BOND']

    while True:
        stock_codes = []
        #枚举所有的股票类型，获取股票codes
        for sub_type in stock_type:
            ret_code, ret_data = quote_ctx.get_stock_basicinfo(market, sub_type)
            if ret_code == 0:
                for ix, row in ret_data.iterrows():
                    stock_codes.append(row['code'])

        if len(stock_codes) == 0:
            quote_ctx.close()
            print("Error market:'{}' can not get stock info".format(market))
            return

        #按频率限制获取股票快照: 每5秒200支股票
        for i in range(1,len(stock_codes),200):
            print("from {}, total {}".format(i, len(stock_codes)))
            ret_code, ret_data = quote_ctx.get_market_snapshot(stock_codes[i:i+200])
            if ret_code != 0:
                print(ret_code, ret_data)
            time.sleep(5)
        sleep(10)
    #detroy obj
    quote_ctx.close()

if __name__ == "__main__":
    loop_get_mkt_snapshot(api_svr_ip, api_svr_port, market)






