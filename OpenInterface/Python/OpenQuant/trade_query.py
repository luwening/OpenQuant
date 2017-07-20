# -*- coding: utf-8 -*-

import sys
import json
from datetime import datetime
from datetime import timedelta


'''
 parameter relation Mappings between PLS and Python programs
'''
mkt_map = {"HK": 1,
           "US": 2,
           "SH": 3,
           "SZ": 4,
           "HK_FUTURE": 6
           }
rev_mkt_map = {mkt_map[x]: x for x in mkt_map}


sec_type_map = {"STOCK": 3,
                "IDX": 6,
                "ETF": 4,
                "WARRANT": 5,
                "BOND": 1
                }
rev_sec_type_map = {sec_type_map[x]: x for x in sec_type_map}


subtype_map = {"TICKER": 4,
               "QUOTE":  1,
               "ORDER_BOOK": 2,
               "K_1M":    11,
               "K_5M":     7,
               "K_15M":    8,
               "K_30M":    9,
               "K_60M":   10,
               "K_DAY":    6,
               "K_WEEK":  12,
               "K_MON":   13
               }
rev_subtype_map = {subtype_map[x]: x for x in subtype_map}


ktype_map = {"K_1M":     1,
             "K_5M":     6,
             "K_15M":    7,
             "K_30M":    8,
             "K_60M":    9,
             "K_DAY":    2,
             "K_WEEK":   3,
             "K_MON":    4
             }

rev_ktype_map = {ktype_map[x]: x for x in ktype_map}

autype_map = {None: 0,
              "qfq": 1,
              "hfq": 2
              }

rev_autype_map = {autype_map[x]: x for x in autype_map}


ticker_direction = {"TT_BUY": 1,
                    "TT_SELL": 2,
                    "TT_NEUTRAL": 3
                    }

rev_ticker_direction = {ticker_direction[x]: x for x in ticker_direction}

order_status = {"CANCEL": 0,
                "INVALID": 1,
                "VALID": 2,
                "DELETE": 3}

rev_order_status = {order_status[x]: x for x in order_status}

envtype_map = {"TRUE": 0,
               "SIMULATE": 1}

rev_envtype_map = {envtype_map[x]: x for x in envtype_map}


RET_OK = 0
RET_ERROR = -1

ERROR_STR_PREFIX = 'ERROR. '


def check_date_str_format(s):
    try:
        _ = datetime.strptime(s, "%Y-%m-%d")
        return RET_OK, None
    except ValueError:
        err = sys.exc_info()[1]
        error_str = ERROR_STR_PREFIX + str(err)
        return RET_ERROR, error_str


def extract_pls_rsp(rsp_str):
    try:
        rsp = json.loads(rsp_str)
    except ValueError:
        err = sys.exc_info()[1]
        err_str = ERROR_STR_PREFIX + str(err)
        return RET_ERROR, err_str, None

    error_code = int(rsp['ErrCode'])

    if error_code != 0:
        error_str = ERROR_STR_PREFIX + rsp['ErrDesc']
        return RET_ERROR, error_str, None

    if 'RetData' not in rsp:
        error_str = ERROR_STR_PREFIX + 'No ret data found in client rsp. Response: %s' % rsp
        return RET_ERROR, error_str, None

    return RET_OK, "", rsp


def normalize_date_format(date_str):
    date_obj = datetime.strptime(date_str, "%Y-%m-%d")
    ret = date_obj.strftime("%Y-%m-%d")
    return ret


def split_stock_str(stock_str):

    if isinstance(stock_str, str) is False:
        error_str = ERROR_STR_PREFIX + "value of stock_str is %s of type %s, and type %s is expected" \
                                       % (stock_str, type(stock_str), str(str))
        return RET_ERROR, error_str

    split_loc = stock_str.find(".")
    '''do not use the built-in split function in python.
    The built-in function cannot handle some stock strings correctly.
    for instance, US..DJI, where the dot . itself is a part of original code'''
    if 0 <= split_loc < len(stock_str) - 1 and stock_str[0:split_loc] in mkt_map:
        market_str = stock_str[0:split_loc]
        market_code = mkt_map[market_str]
        partial_stock_str = stock_str[split_loc+1:]
        return RET_OK, (market_code, partial_stock_str)

    else:

        error_str = ERROR_STR_PREFIX + "format of %s is wrong. (US.AAPL, HK.00700, SZ.000001)" % stock_str
        return RET_ERROR, error_str


def merge_stock_str(market, partial_stock_str):
    """
    :param market: market code
    :param partial_stock_str: original stock code string. i.e. "AAPL","00700", "000001"
    :return: unified representation of a stock code. i.e. "US.AAPL", "HK.00700", "SZ.000001"

    """

    market_str = rev_mkt_map[market]
    stock_str = '.'.join([market_str, partial_stock_str])
    return stock_str


def str2binary(s):
    """
    :param s: string content to be transformed to binary
    :return: binary
    """
    return s.encode('utf-8')


def binary2str(b):
    """

    :param b: binary content to be transformed to string
    :return: string
    """
    return b.decode('utf-8')


class UnlockTrade:
    def __init__(self):
        pass

    @classmethod
    def pack_req(cls, cookie, password):

        req = {"Protocol": "6006",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "Password": password,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'SvrResult' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find SvrResult in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None
        elif int(rsp_data['SvrResult']) != 0:
            error_str = ERROR_STR_PREFIX + rsp['ErrDesc']
            return RET_ERROR, error_str, None

        unlock_list = [{"svr_result": rsp_data["SvrResult"]}]

        return RET_OK, "", unlock_list


class PlaceOrder:
    def __init__(self):
        pass

    @classmethod
    def hk_pack_req(cls, cookie, envtype, orderside, ordertype, price, qty, strcode):

        if int(orderside) < 0 or int(orderside) > 1:
            error_str = ERROR_STR_PREFIX + "parameter orderside is wrong"
            return RET_ERROR, error_str, None

        if int(ordertype) is not 0 and int(ordertype) is not 1 and int(ordertype) is not 3:
            error_str = ERROR_STR_PREFIX + "parameter ordertype is wrong"
            return RET_ERROR, error_str, None

        if int(envtype) < 0 or int(envtype) > 1:
            error_str = ERROR_STR_PREFIX + "parameter envtype is wrong"
            return RET_ERROR, error_str, None

        req = {"Protocol": "6003",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            "OrderSide": orderside,
                            "OrderType": ordertype,
                            "Price": str(float(price)*1000),
                            "Qty": qty,
                            "StockCode": strcode
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def hk_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'SvrResult' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find SvrResult in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None
        elif int(rsp_data['SvrResult']) != 0:
            error_str = ERROR_STR_PREFIX + rsp['ErrDesc']
            return RET_ERROR, error_str, None

        if 'EnvType' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find EnvType in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        place_order_list = [{'envtype': rsp_data['EnvType'],
                             'orderid': rsp_data['OrderID']
                             }]

        return RET_OK, "", place_order_list

    @classmethod
    def us_pack_req(cls, cookie, envtype, orderside, ordertype, price, qty, strcode):

        if int(orderside) < 0 or int(orderside) > 1:
            error_str = ERROR_STR_PREFIX + "parameter orderside is wrong"
            return RET_ERROR, error_str, None

        if int(ordertype) is not 1 and int(ordertype) is not 2 and int(ordertype) is not 51 and int(ordertype) is not 52:
            error_str = ERROR_STR_PREFIX + "parameter ordertype is wrong"
            return RET_ERROR, error_str, None

        req = {"Protocol": "7003",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            "OrderSide": orderside,
                            "OrderType": ordertype,
                            "Price": str(float(price)*1000),
                            "Qty": qty,
                            "StockCode": strcode
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def us_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'SvrResult' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find SvrResult in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None
        elif int(rsp_data['SvrResult']) != 0:
            error_str = ERROR_STR_PREFIX + rsp['ErrDesc']
            return RET_ERROR, error_str, None

        if 'EnvType' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find EnvType in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        place_order_list = [{'envtype': rsp_data['EnvType'],
                             'orderid': rsp_data['OrderID']
                             }]

        return RET_OK, "", place_order_list


class SetOrderStatus:
    def __init__(self):
        pass

    @classmethod
    def hk_pack_req(cls, cookie, envtype, localid, orderid, status):

        if int(envtype) < 0 or int(envtype) > 1:
            error_str = ERROR_STR_PREFIX + "parameter envtype is wrong"
            return RET_ERROR, error_str, None

        if int(status) < 0 or int(status) > 3:
            error_str = ERROR_STR_PREFIX + "parameter status is wrong"
            return RET_ERROR, error_str, None

        req = {"Protocol": "6004",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            "LocalID": localid,
                            "OrderID": orderid,
                            "SetOrderStatus": status,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def hk_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'SvrResult' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find SvrResult in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None
        elif int(rsp_data['SvrResult']) != 0:
            error_str = ERROR_STR_PREFIX + rsp['ErrDesc']
            return RET_ERROR, error_str, None

        if 'EnvType' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find EnvType in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        if 'OrderID' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find OrderID in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        set_order_list = [{'envtype': rsp_data['EnvType'],
                           'orderID': rsp_data['OrderID']
                           }]

        return RET_OK, "", set_order_list

    @classmethod
    def us_pack_req(cls, cookie, envtype, localid, orderid, status):

        req = {"Protocol": "7004",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            "LocalID": localid,
                            "OrderID": orderid,
                            "SetOrderStatus": status,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def us_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'SvrResult' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find SvrResult in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None
        elif int(rsp_data['SvrResult']) != 0:
            error_str = ERROR_STR_PREFIX + rsp['ErrDesc']
            return RET_ERROR, error_str, None

        if 'EnvType' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find EnvType in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        if 'OrderID' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find OrderID in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        set_order_list = [{'envtype': rsp_data['EnvType'],
                           'orderID': rsp_data['OrderID']
                           }]

        return RET_OK, "", set_order_list


class ChangeOrder:
    def __init__(self):
        pass

    @classmethod
    def hk_pack_req(cls, cookie, envtype, localid, orderid, price, qty):

        if int(envtype) < 0 or int(envtype) > 1:
            error_str = ERROR_STR_PREFIX + "parameter envtype is wrong"
            return RET_ERROR, error_str, None

        req = {"Protocol": "6005",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            "LocalID": localid,
                            "OrderID": orderid,
                            "Price": str(float(price)*1000),
                            "Qty": qty,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def hk_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'SvrResult' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find SvrResult in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None
        elif int(rsp_data['SvrResult']) != 0:
            error_str = ERROR_STR_PREFIX + rsp['ErrDesc']
            return RET_ERROR, error_str, None

        if 'EnvType' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find EnvType in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        if 'OrderID' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find OrderID in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        change_order_list = [{'envtype': rsp_data['EnvType'],
                              'orderID': rsp_data['OrderID']
                              }]

        return RET_OK, "", change_order_list

    @classmethod
    def us_pack_req(cls, cookie, envtype, localid, orderid, price, qty):

        req = {"Protocol": "7005",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            "LocalID": localid,
                            "OrderID": orderid,
                            "Price": str(float(price)*1000),
                            "Qty": qty,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def us_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'SvrResult' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find SvrResult in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None
        elif int(rsp_data['SvrResult']) != 0:
            error_str = ERROR_STR_PREFIX + rsp['ErrDesc']
            return RET_ERROR, error_str, None

        if 'EnvType' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find EnvType in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        if 'OrderID' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find OrderID in client rsp: %s" % rsp_str
            return RET_ERROR, error_str, None

        change_order_list = [{'envtype': rsp_data['EnvType'],
                              'orderID': rsp_data['OrderID']
                              }]

        return RET_OK, "", change_order_list


class AccInfoQuery:
    def __init__(self):
        pass

    @classmethod
    def hk_pack_req(cls, cookie, envtype):

        if int(envtype) < 0 or int(envtype) > 1:
            error_str = ERROR_STR_PREFIX + "parameter envtype is wrong"
            return RET_ERROR, error_str, None

        req = {"Protocol": "6007",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def hk_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'Cookie' not in rsp_data or 'EnvType' not in rsp_data:
            return RET_ERROR, msg, None

        if 'Power' not in rsp_data or 'ZCJZ' not in rsp_data or 'ZQSZ' not in rsp_data or 'XJJY' not in rsp_data:
            return RET_ERROR, msg, None

        if 'KQXJ' not in rsp_data or 'DJZJ' not in rsp_data or 'ZSJE' not in rsp_data or 'ZGJDE' not in rsp_data:
            return RET_ERROR, msg, None

        if 'YYJDE' not in rsp_data or 'GPBZJ' not in rsp_data:
            return RET_ERROR, msg, None

        accinfo_list = [{'Power': float(rsp_data['Power'])/1000, 'ZCJZ': float(rsp_data['ZCJZ'])/1000,
                         'ZQSZ': float(rsp_data['ZQSZ']) / 1000, 'XJJY': float(rsp_data['XJJY']) / 1000,
                         'KQXJ': float(rsp_data['KQXJ']) / 1000, 'DJZJ': float(rsp_data['DJZJ']) / 1000,
                         'ZSJE': float(rsp_data['ZSJE']) / 1000, 'ZGJDE': float(rsp_data['ZGJDE']) / 1000,
                         'YYJDE': float(rsp_data['YYJDE']) / 1000, 'GPBZJ': float(rsp_data['GPBZJ']) / 1000
                         }]

        return RET_OK, "", accinfo_list

    @classmethod
    def us_pack_req(cls, cookie, envtype):

        req = {"Protocol": "7007",
               "Version": "1",
               "ReqParam": {"Cookie": str(cookie),
                            "EnvType": str(envtype),
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def us_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'EnvType' not in rsp_data:
            return RET_ERROR, msg, None

        if 'Power' not in rsp_data or 'ZCJZ' not in rsp_data or 'ZQSZ' not in rsp_data or 'XJJY' not in rsp_data:
            return RET_ERROR, msg, None

        if 'KQXJ' not in rsp_data or 'DJZJ' not in rsp_data or 'ZSJE' not in rsp_data or 'ZGJDE' not in rsp_data:
            return RET_ERROR, msg, None

        if 'YYJDE' not in rsp_data or 'GPBZJ' not in rsp_data:
            return RET_ERROR, msg, None

        accinfo_list = [{'Power': float(rsp_data['Power'])/1000, 'ZCJZ': float(rsp_data['ZCJZ'])/1000,
                         'ZQSZ': float(rsp_data['ZQSZ']) / 1000, 'XJJY': float(rsp_data['XJJY']) / 1000,
                         'KQXJ': float(rsp_data['KQXJ']) / 1000, 'DJZJ': float(rsp_data['DJZJ']) / 1000,
                         'ZSJE': float(rsp_data['ZSJE']) / 1000, 'ZGJDE': float(rsp_data['ZGJDE']) / 1000,
                         'YYJDE': float(rsp_data['YYJDE']) / 1000, 'GPBZJ': float(rsp_data['GPBZJ']) / 1000
                         }]

        return RET_OK, "", accinfo_list


class OrderListQuery:
    def __init__(self):
        pass

    @classmethod
    def hk_pack_req(cls, cookie, envtype, statusfilter):

        if int(envtype) < 0 or int(envtype) > 1:
            error_str = ERROR_STR_PREFIX + "parameter envtype is wrong"
            return RET_ERROR, error_str, None

        req = {"Protocol": "6008",
               "Version": "1",
               "ReqParam": {"Cookie": str(cookie),
                            "EnvType": str(envtype),
                            "StatusFilterStr": str(statusfilter)
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def hk_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'EnvType' not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find EnvType in client rsp. Response: %s" % rsp_str
            return RET_ERROR, error_str, None

        if "HKOrderArr" not in rsp_data :
            error_str = ERROR_STR_PREFIX + "cannot find HKOrderArr in client rsp. Response: %s" % rsp_str
            return RET_ERROR, error_str, None

        raw_order_list = rsp_data["HKOrderArr"]
        if raw_order_list is None or len(raw_order_list) == 0:
            return RET_OK, "", []

        order_list = [{"code": merge_stock_str(1, order['StockCode']),
                       "stock_name": order["StockName"],
                       "dealt_avg_price": float(order['DealtAvgPrice'])/1000,
                       "dealt_qty": order['DealtQty'],
                       "qty": order['Qty'],
                       "orderid": order['OrderID'],
                       "order_type": order['OrderType'],
                       "order_side": order['OrderSide'],
                       "price": float(order['Price'])/1000,
                       "status": order['Status'],
                       "submited_time": order['SubmitedTime'],
                       "updated_time": order['UpdatedTime']
                       }
                      for order in raw_order_list]
        return RET_OK, "", order_list

    @classmethod
    def us_pack_req(cls, cookie, envtype, statusfilter):

        req = {"Protocol": "7008",
               "Version": "1",
               "ReqParam": {"Cookie": str(cookie),
                            "EnvType": str(envtype),
                            "StatusFilterStr": str(statusfilter)
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def us_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if "USOrderArr" not in rsp_data :
            error_str = ERROR_STR_PREFIX + "cannot find USOrderArr in client rsp. Response: %s" % rsp_str
            return RET_ERROR, error_str, None

        raw_order_list = rsp_data["USOrderArr"]
        if raw_order_list is None or len(raw_order_list) == 0:
            return RET_OK, "", []

        order_list = [{"code": merge_stock_str(2, order['StockCode']),
                       "stock_name": order["StockName"],
                       "dealt_avg_price": float(order['DealtAvgPrice'])/1000,
                       "dealt_qty": order['DealtQty'],
                       "qty": order['Qty'],
                       "orderid": order['OrderID'],
                       "order_type": order['OrderType'],
                       "order_side": order['OrderSide'],
                       "price": float(order['Price'])/1000,
                       "status": order['Status'],
                       "submited_time": order['SubmitedTime'],
                       "updated_time": order['UpdatedTime']
                       }
                      for order in raw_order_list]
        return RET_OK, "", order_list


class PositionListQuery:
    def __init__(self):
        pass

    @classmethod
    def hk_pack_req(cls, cookie, envtype):

        if int(envtype) < 0 or int(envtype) > 1:
            error_str = ERROR_STR_PREFIX + "parameter envtype is wrong"
            return RET_ERROR, error_str, None

        req = {"Protocol": "6009",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def hk_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'Cookie' not in rsp_data or 'EnvType' not in rsp_data:
            return RET_ERROR, msg, None

        if "HKPositionArr" not in rsp_data :
            error_str = ERROR_STR_PREFIX + "cannot find HKPositionArr in client rsp. Response: %s" % rsp_str
            return RET_ERROR, error_str, None

        raw_position_list = rsp_data["HKPositionArr"]
        if raw_position_list is None or len(raw_position_list) == 0:
            return RET_OK, "", []

        position_list = [{"code": merge_stock_str(1, position['StockCode']),
                          "stock_name": position["StockName"],
                          "qty": position['Qty'],
                          "can_sell_qty": position['CanSellQty'],
                          "cost_price": float(position['CostPrice'])/1000,
                          "cost_price_valid": position['CostPriceValid'],
                          "market_val": float(position['MarketVal'])/1000,
                          "nominal_price": float(position['NominalPrice'])/1000,
                          "pl_ratio": float(position['PLRatio'])/1000,
                          "pl_ratio_valid": position['PLRatioValid'],
                          "pl_val": float(position['PLVal'])/1000,
                          "pl_val_valid": position['PLValValid'],
                          "today_buy_qty": position['Today_BuyQty'],
                          "today_buy_val": float(position['Today_BuyVal'])/1000,
                          "today_pl_val": float(position['Today_PLVal'])/1000,
                          "today_sell_qty": position['Today_SellQty'],
                          "today_sell_val": float(position['Today_SellVal'])/1000
                          }
                         for position in raw_position_list]
        return RET_OK, "", position_list

    @classmethod
    def us_pack_req(cls, cookie, envtype):

        req = {"Protocol": "7009",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def us_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if "USPositionArr" not in rsp_data :
            error_str = ERROR_STR_PREFIX + "cannot find USPositionArr in client rsp. Response: %s" % rsp_str
            return RET_ERROR, error_str, None

        raw_position_list = rsp_data["USPositionArr"]
        if raw_position_list is None or len(raw_position_list) == 0:
            return RET_OK, "", []

        position_list = [{"code": merge_stock_str(2, position['StockCode']),
                          "stock_name": position["StockName"],
                          "qty": position['Qty'],
                          "can_sell_qty": position['CanSellQty'],
                          "cost_price": float(position['CostPrice'])/1000,
                          "cost_price_valid": position['CostPriceValid'],
                          "market_val": float(position['MarketVal'])/1000,
                          "nominal_price": float(position['NominalPrice'])/1000,
                          "pl_ratio": float(position['PLRatio'])/1000,
                          "pl_ratio_valid": position['PLRatioValid'],
                          "pl_val": float(position['PLVal'])/1000,
                          "pl_val_valid": position['PLValValid'],
                          "today_buy_qty": position['Today_BuyQty'],
                          "today_buy_val": float(position['Today_BuyVal'])/1000,
                          "today_pl_val": float(position['Today_PLVal'])/1000,
                          "today_sell_qty": position['Today_SellQty'],
                          "today_sell_val": float(position['Today_SellVal'])/1000
                          }
                         for position in raw_position_list]
        return RET_OK, "", position_list


class DealListQuery:
    def __init__(self):
        pass

    @classmethod
    def hk_pack_req(cls, cookie, envtype):

        if int(envtype) < 0 or int(envtype) > 1:
            error_str = ERROR_STR_PREFIX + "parameter envtype is wrong"
            return RET_ERROR, error_str, None

        req = {"Protocol": "6010",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def hk_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if 'Cookie' not in rsp_data or 'EnvType' not in rsp_data:
            return RET_ERROR, msg, None

        if "HKDealArr" not in rsp_data :
            error_str = ERROR_STR_PREFIX + "cannot find HKDealArr in client rsp. Response: %s" % rsp_str
            return RET_ERROR, error_str, None

        raw_deal_list = rsp_data["HKDealArr"]
        if raw_deal_list is None or len(raw_deal_list) == 0:
            return RET_OK, "", []

        deal_list = [{"code": merge_stock_str(1, deal['StockCode']),
                      "stock_name": deal["StockName"],
                      "dealid": deal['DealID'],
                      "orderid": deal['OrderID'],
                      "qty": deal['Qty'],
                      "price": float(deal['Price'])/1000,
                      "orderside": deal['OrderSide'],
                      "time": deal['Time']
                      }
                     for deal in raw_deal_list]
        return RET_OK, "", deal_list

    @classmethod
    def us_pack_req(cls, cookie, envtype):

        req = {"Protocol": "7010",
               "Version": "1",
               "ReqParam": {"Cookie": cookie,
                            "EnvType": envtype,
                            }
               }
        req_str = json.dumps(req) + '\r\n'
        return RET_OK, "", req_str

    @classmethod
    def us_unpack_rsp(cls, rsp_str):

        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None

        rsp_data = rsp['RetData']

        if "USDealArr" not in rsp_data:
            error_str = ERROR_STR_PREFIX + "cannot find USDealArr in client rsp. Response: %s" % rsp_str
            return RET_ERROR, error_str, None

        raw_deal_list = rsp_data["USDealArr"]
        if raw_deal_list is None or len(raw_deal_list) == 0:
            return RET_OK, "", []

        deal_list = [{"code": merge_stock_str(2, deal['StockCode']),
                      "stock_name": deal["StockName"],
                      "dealid": deal['DealID'],
                      "orderid": deal['OrderID'],
                      "qty": deal['Qty'],
                      "price": float(deal['Price'])/1000,
                      "orderside": deal['OrderSide'],
                      "time": deal['Time']
                      }
                     for deal in raw_deal_list]
        return RET_OK, "", deal_list

