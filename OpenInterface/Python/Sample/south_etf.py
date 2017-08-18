# -*- coding: utf-8 -*-
"""
本策略基于日线, 由于交易时间的限制，不考虑同时交易港股和每股的可能性
"""
import os
from OpenInterface.Python.OpenQuant.open_quant_context import *
import pickle
import talib as ta
import numpy as np
from time import sleep
import datetime

'''
  使用请先配置正确参数:
  API_SVR_IP: (string) ip
  API_SVR_PORT: (int) port
  UNLOCK_PASSWORD: (string) 交易解锁密码, 必需修改！
  TRADE_ENV: (int) 0：真实交易 1：仿真交易（美股暂不支持仿真）
'''


class SouthETF(object):
    """
    南方东英杠反ETF策略
    详细参考 https://act.futunn.com/south-etf
    以及富途相应课程信息 https://live.futunn.com/course/1012
    """
    # stock code setting
    HSI_CALL_CODE = 'HK.07200'
    HSI_PUT_CODE = 'HK.07300'

    HSCEI_CALL_CODE = 'HK.07288'
    HSCEI_PUT_CODE = 'HK.07388'

    order_type = 0  # 港股增强限价单(普通交易)

    # parameter setting
    index_type = 'HSI'      # HSI/HSCEI
    indicator_type = 'MA'  # CHG/MA

    # CHG: trading signal setting
    up_chg_signal = True
    up_chg_rate = 0.005
    up_buy_type = 'CALL'
    up_hold_sell = 5

    down_chg_signal = True
    down_chg_rate = -0.005
    down_buy_type = 'PUT'
    down_hold_sell = 5

    # MA: trading signal setting
    gold_ma_signal = True
    gold_ma_short = 5
    gold_ma_long = 10
    gold_hold_sell = 5

    dead_ma_signal = True
    dead_ma_short = 5
    dead_ma_long = 10
    dead_hold_sell = 5

    def __init__(self, api_svr_ip, api_svr_port, unlock_password, trade_env, holding):
        """Constructor"""
        self.api_svr_ip = api_svr_ip
        self.api_svr_port = api_svr_port
        self.unlock_password = unlock_password
        self.trade_env = trade_env
        self.holding = holding

        if self.index_type.lower() == 'hsi':
            self.code = 'HK.800000'

            if self.up_buy_type.lower() == 'call':
                self.up_buy_code = self.HSI_CALL_CODE
            elif self.up_buy_type.lower() == 'put':
                self.up_buy_code = self.HSI_PUT_CODE
            else:
                raise 'up_buy_type设置错误: {}'.format(self.up_buy_type)

            if self.down_buy_type.lower() == 'call':
                self.down_buy_code = self.HSI_CALL_CODE
            elif self.down_buy_type.lower() == 'put':
                self.down_buy_code = self.HSI_PUT_CODE
            else:
                raise 'down_buy_type设置错误: {}'.format(self.down_buy_type)

            self.gold_buy_code = self.HSI_CALL_CODE
            self.dead_buy_code = self.HSI_PUT_CODE

        elif self.index_type.lower() == 'hscei':
            self.code = 'HK.800100'

            if self.up_buy_type.lower() == 'call':
                self.up_buy_code = self.HSCEI_CALL_CODE
            elif self.up_buy_type.lower() == 'put':
                self.up_buy_code = self.HSCEI_PUT_CODE
            else:
                raise 'up_buy_type设置错误: {}'.format(self.up_buy_type)

            if self.down_buy_type.lower() == 'call':
                self.down_buy_code = self.HSCEI_CALL_CODE
            elif self.down_buy_type.lower() == 'put':
                self.down_buy_code = self.HSCEI_PUT_CODE
            else:
                raise 'down_buy_type设置错误:{}'.format(self.down_buy_type)

            self.gold_buy_code = self.HSCEI_CALL_CODE
            self.dead_buy_code = self.HSCEI_PUT_CODE
        else:
            raise 'index_type设置错误: {}'.format(self.index_type)

        self.quote_ctx, self.trade_ctx = self.context_setting()

    def context_setting(self):
        """根据策略参数设置报价和交易上下文"""
        if self.unlock_password == "":
            raise Exception("请先配置交易解锁密码! password: {}".format(self.unlock_password))

        quote_ctx = OpenQuoteContext(host=self.api_svr_ip, port=self.api_svr_port)
        is_hk_trade = 'HK.' in (self.HSI_CALL_CODE+self.HSCEI_PUT_CODE+self.HSCEI_CALL_CODE+self.HSCEI_PUT_CODE)
        if is_hk_trade:
            trade_ctx = OpenHKTradeContext(host=self.api_svr_ip, port=self.api_svr_port)
        else:
            if self.trade_env != 0:
                raise "美股交易接口不支持仿真环境 trade_env: {}".format(self.trade_env)
            self.order_type = 2  # 美股限价单
            trade_ctx = OpenUSTradeContext(host=self.api_svr_ip, port=self.api_svr_port)

        return quote_ctx, trade_ctx

    def handle_bar(self):
        """根据数据判断是否有交易信号产生，并根据信号下单(日线级)"""
        is_unlock_trade = False
        is_fire_trade = False
        trade_sum = 0

        while not is_fire_trade:
            if not self.trade_env and not is_unlock_trade:
                ret_code, ret_data = self.trade_ctx.unlock_trade(self.unlock_password)
                is_unlock_trade = (ret_code == 0)
                if not is_unlock_trade:
                    print("请求交易解锁失败: {} 重试中...".format(ret_data))
                    sleep(1)
                    continue

            ret_code, ret_data = self.quote_ctx.get_history_kline(self.code, start='2017-01-01')
            if ret_code == 0:
                close = np.array(ret_data['close'])
            else:
                print('k线数据获取异常, 重试中: {}'.format(ret_data))
                sleep(1)
                continue

            if self.indicator_type.lower() == 'chg':
                # calculate change%
                chg_rate = (close[-1] - close[-2]) / close[-2]

                # open position
                if self.up_chg_signal and not self.holding.get(self.up_buy_code, 0):
                    if chg_rate > self.up_chg_rate:
                        ret, data = self.quote_ctx.get_market_snapshot([self.up_buy_code])
                        up_lst_price = data.iloc[0]['last_price'] if ret == 0 else 0
                        up_lot_size = data.iloc[0]['lot_size'] if ret == 0 else 0
                        if up_lot_size == 0 or up_lst_price == 0.0:
                            continue
                        ret_code, ret_data = self.trade_ctx.place_order(price=up_lst_price, qty=up_lot_size,
                                                                        strcode=self.up_buy_code,
                                                                        orderside=0, ordertype=self.order_type,
                                                                        envtype=self.trade_env)
                        if not ret_code:
                            trade_sum += 1
                            print('up_ma_signal MAKE BUY ORDER\n\tcode = {} price = {} quantity = {}'
                                  .format(self.up_buy_code, up_lst_price, up_lot_size))
                            up_sell_date = datetime.datetime.now() + datetime.timedelta(days=self.up_hold_sell)
                            self.holding[self.up_buy_code] = up_sell_date.strftime('%Y-%m-%d')
                        else:
                            print('up_ma_signal: MAKE BUY ORDER FAILURE!')

                if self.down_chg_signal and not self.holding.get(self.down_buy_code, 0):
                    if chg_rate < self.down_chg_rate:
                        ret, data = self.quote_ctx.get_market_snapshot([self.down_buy_code])
                        down_lst_price = data.iloc[0]['last_price'] if ret == 0 else 0
                        down_lot_size = data.iloc[0]['lot_size'] if ret == 0 else 0
                        if down_lot_size == 0 or down_lst_price == 0.0:
                            continue
                        ret_code, ret_data = self.trade_ctx.place_order(price=down_lst_price, qty=down_lot_size,
                                                                        strcode=self.down_buy_code,
                                                                        orderside=0, ordertype=self.order_type,
                                                                        envtype=self.trade_env)
                        if not ret_code:
                            trade_sum += 1
                            print('down_ma_signal MAKE BUY ORDER\n\tcode = {} price = {} quantity = {}'
                                  .format(self.down_buy_code, down_lst_price, down_lot_size))
                            down_sell_date = datetime.datetime.now() + datetime.timedelta(days=self.down_hold_sell)
                            self.holding[self.down_buy_code] = down_sell_date.strftime('%Y-%m-%d')
                        else:
                            print('down_ma_signal: MAKE BUY ORDER FAILURE!')

            elif self.indicator_type.lower() == 'ma':
                # calculate MA
                ma_short1 = ta.MA(close, timeperiod=self.gold_ma_short, matype=0)
                ma_long1 = ta.MA(close, timeperiod=self.gold_ma_long, matype=0)
                ma_short2 = ta.MA(close, timeperiod=self.dead_ma_short, matype=0)
                ma_long2 = ta.MA(close, timeperiod=self.dead_ma_long, matype=0)
                # open position
                if self.gold_ma_signal and not self.holding.get(self.gold_buy_code, 0):
                    if ma_short1[-1] > ma_long1[-1]:
                        ret, data = self.quote_ctx.get_market_snapshot([self.gold_buy_code])
                        gold_lst_price = data.iloc[0]['last_price'] if ret == 0 else 0
                        gold_lot_size = data.iloc[0]['lot_size'] if ret == 0 else 0
                        if gold_lst_price == 0.0 or gold_lot_size == 0:
                            continue
                        ret_code, ret_data = self.trade_ctx.place_order(price=gold_lst_price, qty=gold_lot_size,
                                                                        strcode=self.gold_buy_code,
                                                                        orderside=0, ordertype=self.order_type,
                                                                        envtype=self.trade_env)
                        if not ret_code:
                            trade_sum += 1
                            print('gold_ma_signal: MAKE BUY ORDER\n\tcode = {} price = {} quantity = {}'
                                  .format(self.gold_buy_code, gold_lst_price, gold_lot_size))
                            gold_sell_date = datetime.datetime.now() + datetime.timedelta(days=self.gold_hold_sell)
                            self.holding[self.gold_buy_code] = gold_sell_date.strftime('%Y-%m-%d')
                        else:
                            print('gold_ma_signal: MAKE BUY ORDER FAILURE!')

                if self.dead_ma_signal and not self.holding.get(self.dead_buy_code, 0):
                    if ma_short2[-1] < ma_long2[-1]:
                        ret, data = self.quote_ctx.get_market_snapshot([self.dead_buy_code])
                        dead_lst_price = data.iloc[0]['last_price'] if ret == 0 else 0
                        dead_lot_size = data.iloc[0]['lot_size'] if ret == 0 else 0
                        if dead_lst_price == 0.0 or dead_lot_size == 0:
                            continue
                        ret_code, ret_data = self.trade_ctx.place_order(price=dead_lst_price, qty=dead_lot_size,
                                                                        strcode=self.dead_buy_code,
                                                                        orderside=0, ordertype=self.order_type,
                                                                        envtype=self.trade_env)
                        if not ret_code:
                            trade_sum += 1
                            print('dead_ma_signal: MAKE BUY ORDER\n\tcode = {} price = {} quantity = {}'
                                  .format(self.dead_buy_code, dead_lst_price, dead_lot_size))
                            dead_sell_date = datetime.datetime.now() + datetime.timedelta(days=self.dead_hold_sell)
                            self.holding[self.dead_buy_code] = dead_sell_date.strftime('%Y-%m-%d')
                        else:
                            print('dead_ma_signal: MAKE BUY ORDER FAILURE!')
            else:
                raise 'indicator_type设置错误: {}'.format(self.indicator_type)

            # close position
            temp_hold = self.holding.copy()
            if len(temp_hold):
                for key, item in temp_hold.items():
                    if datetime.datetime.now().strftime('%Y-%m-%d') == item:
                        ret, data = self.quote_ctx.get_market_snapshot([key])
                        lst_price = data.iloc[0]['last_price'] if ret == 0 else 0
                        lot_size = data.iloc[0]['lot_size'] if ret == 0 else 0
                        if lst_price == 0.0 or lot_size == 0:
                            continue
                        ret_code, ret_data = self.trade_ctx.place_order(price=lst_price, qty=lot_size,
                                                                        strcode=key,
                                                                        orderside=1, ordertype=self.order_type,
                                                                        envtype=self.trade_env)
                        if not ret_code:
                            print('close position: MAKE SELL ORDER\n\tcode = {} price = {} quantity = {}'
                                  .format(key, lst_price, lot_size))
                            del self.holding[key]
                        else:
                            print('close position MAKE SELL ORDER FAILURE!')

            if trade_sum > 0:
                is_fire_trade = True
                # destroy obj
                self.quote_ctx.close()
                self.trade_ctx.close()

        return self.holding


if __name__ == "__main__":
    API_SVR_IP = '119.29.141.202'
    API_SVR_PORT = 11111
    UNLOCK_PASSWORD = "123"
    TRADE_ENV = 1

    if os.path.exists('south_etf.pkl'):
        lst_holding = pickle.load(open('south_etf.pkl', 'rb'))
        print(lst_holding)
        strategy_test = SouthETF(API_SVR_IP, API_SVR_PORT, UNLOCK_PASSWORD, TRADE_ENV, lst_holding)
        etf_holding = strategy_test.handle_bar()
        pickle.dump(etf_holding, open('south_etf.pkl', 'wb'))    # 持久化
    else:
        strategy_test = SouthETF(API_SVR_IP, API_SVR_PORT, UNLOCK_PASSWORD, TRADE_ENV, {})
        etf_holding = strategy_test.handle_bar()
        pickle.dump(etf_holding, open('south_etf.pkl', 'wb'))    # 持久化
