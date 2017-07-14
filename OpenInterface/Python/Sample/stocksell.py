import os, sys
sys.path.append(os.path.abspath('../'))
from openft.open_quant_context import *
from math import floor

def simple_sell(quote_ctx,trade_ctx,stock_code,trade_price,volume,trade_env):
    lot_size=0
    while True:
        if lot_size==0:
            ret,data=quote_ctx.get_market_snapshot([stock_code])
            lot_size=data.iloc[0]['lot_size'] if ret==0 else 0
            if ret!=RET_OK:
                print("can't get lot size,retrying")
                continue
            elif lot_size<=0:
                raise Exception('lot size error {}:{}'.format(lot_size,stock_code))
        qty=floor(volume/lot_size)*lot_size
        ret,data=trade_ctx.place_order(price=trade_price,qty=qty,strcode=stock_code,orderside=1,envtype=trade_env)
        if ret!=RET_OK:
            print('下单失败{}'.format(data))
            return None
        else:
            print('下单成功')
            return data
        return None

def smart_sell(quote_ctx,trade_ctx,stock_code,volume,trade_env):
    lot_size=0
    while True:
        if lot_size==0:
            ret,data=quote_ctx.get_market_snapshot([stock_code])
            lot_size=data.iloc[0]['lot_size'] if ret==0 else 0
            if ret!=RET_OK:
                print("can't get lot size,retrying")
                continue
            elif lot_size<=0:
                raise Exception('lot size error {}:{}'.format(lot_size,stock_code))
        qty=floor(volume/lot_size)*lot_size
        ret,data=quote_ctx.get_order_book(stock_code)
        if ret!=RET_OK:
            print("can't get orderbook,retrying")
            continue
        price=data['Bid'][0][0]
        print('bid price is {}'.format(price))
        ret,data=trade_ctx.place_order(price=price,qty=qty,strcode=stock_code,orderside=1,envtype=trade_env)
        if ret!=RET_OK:
            print('下单失败{}'.format(data))
            return None
        else:
            return data
        return None