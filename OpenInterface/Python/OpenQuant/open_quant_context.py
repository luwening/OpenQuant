# -*- coding: utf-8 -*-

from .quote_query import *
from .trade_query import *
from multiprocessing import Queue
from threading import RLock, Thread
import socket
import select
import sys
import pandas as pd
import asyncore
import socket as sock
import time
from time import sleep
from abc import ABCMeta, abstractmethod
from struct import pack

class RspHandlerBase(object):
    def __init__(self):
        pass

    def on_recv_rsp(self, rsp_content):
        return 0, None

    def on_error(self, error_str):
        pass


class StockQuoteHandlerBase(RspHandlerBase):

    def on_recv_rsp(self, rsp_str):
        ret_code, msg, quote_list = StockQuoteQuery.unpack_rsp(rsp_str)
        if ret_code == RET_ERROR:
            return ret_code, msg
        else:
            col_list = ['code', 'data_date', 'data_time', 'last_price', 'open_price',
                        'high_price', 'low_price', 'prev_close_price',
                        'volume', 'turnover', 'turnover_rate', 'amplitude', 'suspension', 'listing_date'
                        ]

            quote_frame_table = pd.DataFrame(quote_list, columns=col_list)

            return RET_OK, quote_frame_table

    def on_error(self, error_str):
        return error_str


class OrderBookHandlerBase(RspHandlerBase):

    def on_recv_rsp(self, rsp_str):
        ret_code, msg, order_book = OrderBookQuery.unpack_rsp(rsp_str)
        if ret_code == RET_ERROR:
            return ret_code, msg
        else:
            return ret_code, order_book

    def on_error(self, error_str):
        return error_str


class CurKlineHandlerBase(RspHandlerBase):

    def on_recv_rsp(self, rsp_str):
        ret_code, msg, kline_list = CurKlineQuery.unpack_rsp(rsp_str)
        if ret_code == RET_ERROR:
            return ret_code, msg
        else:
            col_list = ['code', 'time_key', 'open', 'close', 'high', 'low', 'volume', 'turnover', 'k_type']
            kline_frame_table = pd.DataFrame(kline_list, columns=col_list)

            return RET_OK, kline_frame_table

    def on_error(self, error_str):
        return error_str


class TickerHandlerBase(RspHandlerBase):

    def on_recv_rsp(self, rsp_str):
        ret_code, msg, ticker_list = TickerQuery.unpack_rsp(rsp_str)
        if ret_code == RET_ERROR:
            return ret_code, msg
        else:

            col_list = ['code', 'time', 'price', 'volume', 'turnover', "ticker_direction", 'sequence']
            ticker_frame_table = pd.DataFrame(ticker_list, columns=col_list)

            return RET_OK, ticker_frame_table

    def on_error(self, error_str):
        return error_str


class RTDataHandlerBase(RspHandlerBase):

    def on_recv_rsp(self, rsp_str):
        ret_code, msg, rt_data_list = RtDataQuery.unpack_rsp(rsp_str)
        if ret_code == RET_ERROR:
            return ret_code, msg
        else:

            col_list = ['code', 'time', 'data_status', 'opened_mins', 'cur_price', "last_close", 'avg_price',
                        'turnover', 'volume']
            rt_data_table = pd.DataFrame(rt_data_list, columns=col_list)

            return RET_OK, rt_data_table

    def on_error(self, error_str):
        return error_str


class BrokerHandlerBase(RspHandlerBase):
    def on_recv_rsp(self, rsp_str):
        ret_code, bid_content, ask_content = BrokerQueueQuery.unpack_rsp(rsp_str)
        if ret_code == RET_ERROR:
            return ret_code, bid_content, ask_content
        else:
            bid_list = ['bid_broker_id', 'bid_broker_name', 'bid_broker_pos']
            ask_list = ['ask_broker_id', 'ask_broker_name', 'ask_broker_pos']
            bid_frame_table = pd.DataFrame(bid_content, columns=bid_list)
            ask_frame_table = pd.DataFrame(ask_content, columns=ask_list)

            return RET_OK, [bid_frame_table, ask_frame_table]

    def on_error(self, error_str):
        return error_str


class HandlerContext:
    def __init__(self):
        self._default_handler = RspHandlerBase()
        self._handler_table = {"1030": {"type": StockQuoteHandlerBase, "obj": StockQuoteHandlerBase()},
                               "1031": {"type": OrderBookHandlerBase,  "obj": OrderBookHandlerBase()},
                               "1032": {"type": CurKlineHandlerBase,  "obj": CurKlineHandlerBase()},
                               "1033": {"type": TickerHandlerBase, "obj": TickerHandlerBase()},
                               "1034": {"type": RTDataHandlerBase, "obj": RTDataHandlerBase()},
                               "1035": {"type": BrokerHandlerBase, "obj": BrokerHandlerBase()},
                               }

    def set_handler(self, handler):
        """
        set the callback processing object to be used by the receiving thread after receiving the data.User should set
        their own callback object setting in order to achieve event driven.
        :param handler:the object in callback handler base
        :return: ret_error or ret_ok
        """
        set_flag = False
        for protoc in self._handler_table:
            if isinstance(handler, self._handler_table[protoc]["type"]):
                self._handler_table[protoc]["obj"] = handler
                return RET_OK

        if set_flag is False:
            return RET_ERROR

    def recv_func(self, rsp_str):
        ret, msg, rsp = extract_pls_rsp(rsp_str)
        if ret != RET_OK:
            error_str = msg + rsp_str
            print(error_str)
            return
        else:
            protoc_num = rsp["Protocol"]
            if protoc_num not in self._handler_table:
                handler = self._default_handler
            else:
                handler = self._handler_table[protoc_num]['obj']

        ret, result = handler.on_recv_rsp(rsp_str)
        if ret != RET_OK:
            error_str = result
            handler.on_error(error_str)

    def error_func(self, err_str):
        print(err_str)


class _SyncNetworkQueryCtx:
    """
    Network query context manages connection between python program and FUTU client program.

    Short (non-persistent) connection can be created by setting long_conn prarameter False, which suggests that
    TCP connection is closed once a query session finished

    Long (persistent) connection can be created by setting long_conn prarameter True,  which suggests that TCP
    connection is persisted after a query session finished, waiting for next query.

    """
    def __init__(self, host, port, long_conn=True, connected_handler = None):
        self.s = None
        self.__host = host
        self.__port = port
        self.long_conn = long_conn
        self._socket_lock = RLock()
        self._connected_handler = connected_handler
        self._is_loop_connecting = False

    def close_socket(self):
        self._socket_lock.acquire()
        self._force_close_session()
        self._socket_lock.release()

    def is_sock_ok(self, timeout_select):
        self._socket_lock.acquire()
        try:
            ret = self._is_socket_ok(timeout_select)
        finally:
            self._socket_lock.release()
        return ret

    def _is_socket_ok(self, timeout_select):
        if self.s == None:
            return False
        _, _, sel_except = select.select([self.s], [], [], timeout_select)
        if self.s in sel_except:
            return False
        return  True

    def reconnect(self):
        self._socket_create_and_loop_connect()

    def network_query(self, req_str):
        """
        the function sends req_str to FUTU client and try to get response from the client.
        :param req_str
        :return: rsp_str
        """
        try:
            ret, msg = self._create_session()
            self._socket_lock.acquire()
            if ret != RET_OK:
                return ret, msg, None

            rsp_str = ''
            s_buf = str2binary(req_str)
            s_cnt = self.s.send(s_buf)

            rsp_buf = b''
            while rsp_buf.find(b'\r\n\r\n') < 0:

                try:
                    recv_buf = self.s.recv(5 * 1024 * 1024)
                    rsp_buf += recv_buf
                    if recv_buf == b'':
                        raise Exception("_SyncNetworkQueryCtx : remote server close")
                except Exception:
                    err = sys.exc_info()[1]
                    error_str = ERROR_STR_PREFIX + str(
                        err) + ' when recving after sending %s bytes. For req: ' % s_cnt + req_str
                    self._force_close_session()
                    return RET_ERROR, error_str, None

            rsp_str = binary2str(rsp_buf)
            self._close_session()
        except Exception:
            err = sys.exc_info()[1]
            error_str = ERROR_STR_PREFIX + str(err) + ' when sending. For req: ' + req_str

            self._force_close_session()
            return RET_ERROR, error_str, None
        finally:
            self._socket_lock.release()

        return RET_OK, "", rsp_str

    def _socket_create_and_loop_connect(self):
        '''
        :return: (err_code, err_msg)
        '''
        self._socket_lock.acquire()
        is_socket_lock = True

        if self._is_loop_connecting:
            return RET_ERROR, "is loop connecting, can't create_session"
        self._is_loop_connecting = True

        if self.s is not None:
            self._force_close_session()

        while True:
            try:
                if not is_socket_lock:
                    is_socket_lock = True
                    self._socket_lock.acquire()
                s = sock.socket(sock.AF_INET, sock.SOCK_STREAM)
                s.setsockopt(sock.SOL_SOCKET, sock.SO_REUSEADDR, 0)
                s.setsockopt(sock.SOL_SOCKET, sock.SO_LINGER, pack("ii", 0, 0))
                s.settimeout(10)
                self.s = s
                self.s.connect((self.__host, self.__port))
            except Exception:
                err = sys.exc_info()[1]
                err_msg = ERROR_STR_PREFIX + str(err)
                print("socket connect err:{}".format(err_msg))
                sleep(1)
                continue

            if self._connected_handler is not None:
                is_socket_lock = False
                self._socket_lock.release()

                sock_ok, is_retry = self._connected_handler._notify_sync_socket_connected(self)
                if not sock_ok:
                    self._force_close_session()
                    if is_retry:
                        print("wait to connect futunn plugin server")
                        sleep(1)
                        continue
                    else:
                        return RET_ERROR, "obj is closed"
                else:
                    break
        self._is_loop_connecting = False
        if is_socket_lock:
            is_socket_lock = False
            self._socket_lock.release()

        return RET_OK, ''

    def _create_session(self):
        if self.long_conn is True and self.s is not None:
            return RET_OK, ""
        ret, msg = self._socket_create_and_loop_connect()
        if ret != RET_OK:
            return ret, msg
        return RET_OK, ""

    def _force_close_session(self):
        if self.s is None:
            return
        self.s.close()
        del self.s
        self.s = None

    def _close_session(self):
        if self.s is None or self.long_conn is True:
            return
        self.s.close()
        self.s = None

    def __del__(self):
        if self.s is not None:
            self.s.close()
            self.s = None


class _AsyncNetworkManager(asyncore.dispatcher_with_send):

    def __init__(self, host, port, handler_ctx, close_handler = None):
        self.__host = host
        self.__port = port
        self.__close_handler = close_handler

        asyncore.dispatcher_with_send.__init__(self)
        self._socket_create_and_connect()

        time.sleep(0.1)
        self.rsp_buf = b''
        self.handler_ctx = handler_ctx

    def reconnect(self):
        self._socket_create_and_connect()

    def close_socket(self):
        self.close()

    def handle_read(self):
        """
        deal with Json package
        :return: err
        """
        delimiter = b'\r\n\r\n'
        try:
            recv_buf = self.recv(5 * 1024 * 1024)
            if recv_buf == b'':
                raise Exception("_AsyncNetworkManager : remote server close")
            self.rsp_buf += recv_buf
            loc = self.rsp_buf.find(delimiter)
            while loc >= 0:
                loc += len(delimiter)
                rsp_binary = self.rsp_buf[0:loc]
                self.rsp_buf = self.rsp_buf[loc:]

                rsp_str = binary2str(rsp_binary)

                self.handler_ctx.recv_func(rsp_str)
                loc = self.rsp_buf.find(delimiter)
        except Exception:
            err = sys.exc_info()[1]
            self.handler_ctx.error_func(str(err))
            return

    def network_query(self, req_str):

        s_buf = str2binary(req_str)
        self.send(s_buf)

    def __del__(self):
        self.close()

    def handle_close(self):
      if self.__close_handler is not None:
          self.__close_handler._notify_async_socket_close(self)

    def _socket_create_and_connect(self):
        if self.socket is not None:
            self.close()
        if self.__host is not None and self.__port is not None:
            self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
            self.connect((self.__host, self.__port))


def _net_proc(async_ctx, req_queue):
    """
    processing request queue
    :param async_ctx:
    :param req_queue: request queue
    :return:
    """
    while True:
        if req_queue.empty() is False:
            ctl_flag, req_str = req_queue.get(timeout=0.001)
            if ctl_flag is False:
                break
            async_ctx.network_query(req_str)

        asyncore.loop(timeout=0.001, count=5)

class OpenContextBase(object):
    metaclass__ = ABCMeta

    def __init__(self, host, port, sync_enable, async_enable):
        self.__host = host
        self.__port = port
        self.__sync_socket_enable = sync_enable
        self.__async_socket_enable = async_enable
        self._async_ctx = None
        self._sync_net_ctx = None
        self._thread_check_sync_sock = None
        self._thread_is_exit = False
        self._check_last_req_time = None
        self._is_socket_reconnecting = False
        self._is_obj_closed = False

        self._req_queue = None
        self._handlers_ctx = None
        self._proc_run = False
        self._net_proc = None
        self._sync_query_lock = RLock()

        if not self.__sync_socket_enable and not self.__async_socket_enable:
            raise 'you should sepcify at least one socket type to create !'

        self._socket_reconnect_and_wait_ready()

    def __del__(self):
       self._close()

    @abstractmethod
    def close(self):
        '''
        to call close old obj before loop create new, otherwise socket will encounter erro 10053 or more!
        '''
        self._close()

    @abstractmethod
    def on_api_socket_reconnected(self):
        '''
        # callback after reconnect ok
        '''
        print("on_api_socket_reconnected obj ID={}".format(id(self)))
        pass

    def _close(self):

        self._is_obj_closed = True
        self.stop()

        if self._thread_check_sync_sock is not None:
            self._thread_check_sync_sock.join(timeout=10)
            self._thread_check_sync_sock = None
            assert  self._thread_is_exit

        if self._sync_net_ctx is not None:
            self._sync_net_ctx.close_socket()
            self._sync_net_ctx = None

        if self._async_ctx is not None:
            self._async_ctx.close_socket()
            self._async_ctx = None

        if self._sync_query_lock is not None:
            self._sync_query_lock = None

        self._req_queue = None
        self._handlers_ctx = None

    def start(self):
        """
        start the receiving thread,asynchronously receive the data pushed by the client
        """
        if self._proc_run is True or self._net_proc is None:
            return

        self._net_proc.start()
        self._proc_run = True

    def stop(self):
        """
        stop the receiving thread, no longer receive the data pushed by the client
        """
        if self._proc_run:
            self._stop_net_proc()
            self._net_proc.join(timeout=5)
            self._net_proc = None
            self._proc_run = False

    def set_handler(self, handler):
        '''
        set async push hander obj
        :param handler: RspHandlerBase deviced obj
        :return: ret_error or ret_ok
        '''
        if self._handlers_ctx is not None:
            return self._handlers_ctx.set_handler(handler)
        return RET_ERROR

    def get_global_state(self):
        '''
        get api server(exe) global state
        :return: RET_OK, state_dict | err_code, msg
        '''
        query_processor = self._get_sync_query_processor(GlobalStateQuery.pack_req,
                                                         GlobalStateQuery.unpack_rsp)
        kargs = {"state_type": 0}
        ret_code, msg, state_dict = query_processor(**kargs)
        if ret_code != RET_OK:
            return ret_code, msg
        return RET_OK, state_dict

    def _send_sync_req(self, req_str):
        """
        send a synchronous request
        """
        ret, msg, content = self._sync_net_ctx.network_query(req_str)
        if ret != RET_OK:
            return RET_ERROR, msg, None
        return RET_OK, msg, content

    def _send_async_req(self, req_str):
        """
        send a asynchronous request
        """
        if self._req_queue.full() is False:
            try:
                self._req_queue.put((True, req_str), timeout=1)
                return RET_OK, ''
            except Exception as e:
                _ = e
                err = sys.exc_info()[1]
                error_str = ERROR_STR_PREFIX + str(err)
                return RET_ERROR, error_str
        else:
            error_str = ERROR_STR_PREFIX + "Request queue is full. The size: %s" % self._req_queue.qsize()
            return RET_ERROR, error_str

    def _get_sync_query_processor(self, pack_func, unpack_func):
        """
        synchronize the query processor
        :param pack_func: back
        :param unpack_func: unpack
        :return: sync_query_processor
        """
        send_req = self._send_sync_req

        def sync_query_processor(**kargs):

            msg_obj_del = "the object may have been deleted!"
            if self._is_obj_closed or self._sync_query_lock is None:
                return RET_ERROR, msg_obj_del, None
            try:
                self._sync_query_lock.acquire()
                if self._is_obj_closed:
                    return RET_ERROR, msg_obj_del, None

                ret_code, msg, req_str = pack_func(**kargs)
                if ret_code == RET_ERROR:
                    return ret_code, msg, None

                ret_code, msg, rsp_str = send_req(req_str)
                if ret_code == RET_ERROR:
                    return ret_code, msg, None

                ret_code, msg, content = unpack_func(rsp_str)
                if ret_code == RET_ERROR:
                    return ret_code, msg, None
                return RET_OK, msg, content
            finally:
                try:
                    if self._sync_query_lock:
                        self._sync_query_lock.release()
                except Exception:
                    err = sys.exc_info()[1]
                    print(err)
        return sync_query_processor

    def _stop_net_proc(self):
        """
        stop the request of network
        :return: (ret_error,error_str)
        """
        if self._req_queue.full() is False:
            try:
                self._req_queue.put((False, None), timeout=1)
                return RET_OK, ''
            except Exception as e:
                _ = e
                err = sys.exc_info()[1]
                error_str = ERROR_STR_PREFIX + str(err)
                return RET_ERROR, error_str
        else:
            error_str = ERROR_STR_PREFIX + "Cannot send stop request. queue is full. The size: %s" \
                                           % self._req_queue.qsize()
            return RET_ERROR, error_str

    def _socket_reconnect_and_wait_ready(self):
        '''
        sync_socket & async_socket recreate
        :return: None
        '''
        if self._is_socket_reconnecting or self._is_obj_closed or self._sync_query_lock is None:
            return

        try:
            self._is_socket_reconnecting = True
            self._sync_query_lock.acquire()

            #create async socket (for push data)
            if self.__async_socket_enable:
                if self._async_ctx is None:
                    self._handlers_ctx = HandlerContext()
                    self._req_queue = Queue()
                    self._async_ctx = _AsyncNetworkManager(self.__host, self.__port, self._handlers_ctx, self)
                    if self._net_proc is None:
                        self._net_proc = Thread(target=_net_proc, args=(self._async_ctx, self._req_queue,))
                else:
                    self._async_ctx.reconnect()

            # create sync socket and loop wait to connect api server
            if self.__sync_socket_enable:
                self._thread_check_sync_sock = None
                if self._sync_net_ctx is None:
                    self._sync_net_ctx = _SyncNetworkQueryCtx(self.__host, self.__port, long_conn=True, connected_handler=self)
                self._sync_net_ctx.reconnect()

            # notify reconnected
            self.on_api_socket_reconnected()

            #run thread to check sync socket state
            if self.__sync_socket_enable:
                self._thread_check_sync_sock = Thread(target=self._thread_check_sync_sock_fun)
                self._thread_check_sync_sock.setDaemon(True)
                self._thread_check_sync_sock.start()
        finally:
            try:
                self._is_socket_reconnecting = False
                if self._sync_query_lock:
                    self._sync_query_lock.release()
            except Exception:
                err = sys.exc_info()[1]
                print(err)

    def _notify_sync_socket_connected(self, sync_ctxt):
        '''
        :param sync_ctxt:
        :return: (is_socket_ok[bool], is_to_retry_connect[bool])
        '''
        if self._is_obj_closed or self._sync_net_ctx is None or self._sync_net_ctx is not sync_ctxt:
            return False, False

        is_ready = False
        ret_code, state_dict = self.get_global_state()
        if ret_code == 0:
            is_ready = int(state_dict['Quote_Logined']) != 0 and int(state_dict['Trade_Logined']) != 0
        return is_ready, True

    def _notify_async_socket_close(self, async_ctx):
        '''
         AsyncNetworkManager onclose callback
        '''
        if self._is_obj_closed or self._async_ctx is None or async_ctx is not self._async_ctx:
            return
        # auto reconnect
        self._socket_reconnect_and_wait_ready()

    def _thread_check_sync_sock_fun(self):
        '''
        thread fun : timer to check socket state
        '''
        thread_handle = self._thread_check_sync_sock
        while True:
            if self._is_obj_closed or self._thread_check_sync_sock is not thread_handle:
                self._thread_is_exit = True
                return
            sync_net_ctx = self._sync_net_ctx
            if sync_net_ctx is None:
                self._thread_is_exit = True
                return
            # select sock to get err state
            if not sync_net_ctx.is_sock_ok(0.01):
                if self._thread_check_sync_sock is thread_handle and not self._is_obj_closed:
                    print("thread check socket error")
                    self._socket_reconnect_and_wait_ready()
                self._thread_is_exit = True
                return
            else:
                sleep(0.1)
            #send req loop per 10 seconds
            cur_time = datetime.now().timestamp()
            if (self._check_last_req_time is None) or (cur_time - self._check_last_req_time > 10):
                self._check_last_req_time = cur_time
                if self._thread_check_sync_sock is thread_handle:
                    self.get_global_state()

class OpenQuoteContext(OpenContextBase):
    def __init__(self, host = '119.29.141.202', port = 11111):
        self._ctx_subscribe = set()
        super(OpenQuoteContext, self).__init__(host, port, True, True)

    def close(self):
        '''
        to call close old obj before loop create new, otherwise socket will encounter erro 10053 or more!
        '''
        super(OpenQuoteContext, self).close()

    def on_api_socket_reconnected(self):
        # auto subscribe
        set_sub = self._ctx_subscribe.copy()
        for (stock_code, data_type, push) in set_sub:
            for i in range(3):
                ret, _= self.subscribe(stock_code, data_type, push)
                if ret == 0:
                    break
                else:
                    sleep(1)

    def get_trading_days(self, market, start_date=None, end_date=None):

        if market is None or isinstance(market, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of market param is wrong"
            return RET_ERROR, error_str

        if start_date is not None and isinstance(start_date, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of start_date param is wrong"
            return RET_ERROR, error_str

        if end_date is not None and isinstance(end_date, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of end_date param is wrong"
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(TradeDayQuery.pack_req,
                                                         TradeDayQuery.unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'market': market, 'start_date': start_date, "end_date": end_date}
        ret_code, msg, trade_day_list = query_processor(**kargs)

        if ret_code != RET_OK:
            return RET_ERROR, msg

        return RET_OK, trade_day_list

    def get_stock_basicinfo(self, market, stock_type='STOCK'):
        param_table = {'market': market, 'stock_type': stock_type}
        for x in param_table:
            param = param_table[x]
            if param is None or isinstance(param, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of %s param is wrong" % x
                return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(StockBasicInfoQuery.pack_req,
                                                         StockBasicInfoQuery.unpack_rsp)
        kargs = {"market": market, 'stock_type': stock_type}

        ret_code, msg, basic_info_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'name', 'lot_size', 'stock_type', 'stock_child_type', "owner_stock_code", "listing_date"]

        basic_info_table = pd.DataFrame(basic_info_list, columns=col_list)

        return RET_OK, basic_info_table

    def get_history_kline(self, code, start=None, end=None, ktype='K_DAY', autype='qfq'):

        if start is not None and isinstance(start, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of start param is wrong"
            return RET_ERROR, error_str

        if end is not None and isinstance(end, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of end param is wrong"
            return RET_ERROR, error_str

        if autype is None:
            autype = 'None'

        param_table = {'code': code, 'ktype': ktype, 'autype': autype}
        for x in param_table:
            param = param_table[x]
            if param is None or isinstance(param, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of %s param is wrong" % x
                return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(HistoryKlineQuery.pack_req,
                                                         HistoryKlineQuery.unpack_rsp)
        kargs = {"stock_str": code, "start_date": start, "end_date": end, "ktype": ktype, "autype": autype}

        ret_code, msg, kline_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'time_key', 'open', 'close', 'high', 'low', 'volume', 'turnover']
        kline_frame_table = pd.DataFrame(kline_list, columns=col_list)

        return RET_OK, kline_frame_table

    def get_autype_list(self, code_list):

        if code_list is None or isinstance(code_list, list) is False:
            error_str = ERROR_STR_PREFIX + "the type of code_list param is wrong"
            return RET_ERROR, error_str

        for code in code_list:
            if code is None or isinstance(code, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of param in code_list is wrong"
                return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(ExrightQuery.pack_req,
                                                         ExrightQuery.unpack_rsp)
        kargs = {"stock_list": code_list}
        ret_code, msg, exr_record = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code',
                    'ex_div_date',
                    'split_ratio',
                    'per_cash_div',
                    'per_share_div_ratio',
                    'per_share_trans_ratio',
                    'allotment_ratio',
                    'allotment_price',
                    'stk_spo_ratio',
                    'stk_spo_price',
                    'forward_adj_factorA',
                    'forward_adj_factorB',
                    'backward_adj_factorA',
                    'backward_adj_factorB']

        exr_frame_table = pd.DataFrame(exr_record, columns=col_list)

        return RET_OK, exr_frame_table

    def get_market_snapshot(self, code_list):
        if code_list is None or isinstance(code_list, list) is False:
            error_str = ERROR_STR_PREFIX + "the type of code_list param is wrong"
            return RET_ERROR, error_str

        for code in code_list:
            if code is None or isinstance(code, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of param in code_list is wrong"
                return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(MarketSnapshotQuery.pack_req,
                                                         MarketSnapshotQuery.unpack_rsp)
        kargs = {"stock_list": code_list}

        ret_code, msg, snapshot_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'update_time', 'last_price', 'open_price',
                    'high_price', 'low_price', 'prev_close_price',
                    'volume', 'turnover', 'turnover_rate', 'suspension', 'listing_date',
                    'circular_market_val', 'total_market_val', 'wrt_valid',
                    'wrt_conversion_ratio', 'wrt_type', 'wrt_strike_price',
                    'wrt_maturity_date', 'wrt_end_trade', 'wrt_code',
                    'wrt_recovery_price', 'wrt_street_vol', 'wrt_issue_vol',
                    'wrt_street_ratio', 'wrt_delta', 'wrt_implied_volatility', 'wrt_premium','lot_size'
                    ]

        snapshot_frame_table = pd.DataFrame(snapshot_list, columns=col_list)

        return RET_OK, snapshot_frame_table

    def get_rt_data(self, code):
        if code is None or isinstance(code, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of param in code_list is wrong"
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(RtDataQuery.pack_req,
                                                         RtDataQuery.unpack_rsp)
        kargs = {"stock_str": code}

        ret_code, msg, rt_data_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'time', 'data_status', 'opened_mins', 'cur_price', 'last_close',
                    'avg_price', 'volume', 'turnover']

        rt_data_table = pd.DataFrame(rt_data_list, columns=col_list)

        return RET_OK, rt_data_table

    def get_plate_list(self, market, plate_class):
        param_table = {'market': market, 'plate_class': plate_class}
        for x in param_table:
            param = param_table[x]
            if param is None or isinstance(market, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of market param is wrong"
                return RET_ERROR, error_str

        if market not in mkt_map:
            error_str = ERROR_STR_PREFIX + "the value of market param is wrong "
            return RET_ERROR, error_str

        if plate_class not in plate_class_map:
            error_str = ERROR_STR_PREFIX + "the class of plate is wrong"
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(SubplateQuery.pack_req,
                                                         SubplateQuery.unpack_rsp)
        kargs = {'market': market, 'plate_class': plate_class}

        ret_code, msg, subplate_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'plate_name', 'plate_id']

        subplate_frame_table = pd.DataFrame(subplate_list, columns=col_list)

        return RET_OK, subplate_frame_table

    def get_plate_stock(self, plate_code):
        if plate_code is None or isinstance(plate_code, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of stock_code is wrong"
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(PlateStockQuery.pack_req,
                                                         PlateStockQuery.unpack_rsp)
        kargs = {"plate_code": plate_code}

        ret_code, msg, plate_stock_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'lot_size', 'stock_name', 'owner_market', 'stock_child_type', 'stock_type']

        plate_stock_table = pd.DataFrame(plate_stock_list, columns=col_list)

        return RET_OK, plate_stock_table

    def get_broker_queue(self, code):
        if code is None or isinstance(code, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of param in code_list is wrong"
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(BrokerQueueQuery.pack_req,
                                                         BrokerQueueQuery.unpack_rsp)
        kargs = {"stock_str": code}

        ret_code, bid_list, ask_list = query_processor(**kargs)

        if ret_code == RET_ERROR:
            return ret_code, ERROR_STR_PREFIX

        col_bid_list = ['bid_broker_id', 'bid_broker_name', 'bid_broker_pos']
        col_ask_list = ['ask_broker_id', 'ask_broker_name', 'ask_broker_pos']

        bid_frame_table = pd.DataFrame(bid_list, columns=col_bid_list)
        sak_frame_table = pd.DataFrame(ask_list, columns=col_ask_list)
        return RET_OK, bid_frame_table, sak_frame_table

    def subscribe(self, stock_code, data_type, push=False):
        """
        subcribe a sort of data for a stock
        :param stock_code: string stock_code . For instance, "HK.00700", "US.AAPL"
        :param data_type: string  data type. For instance, "K_1M", "K_MON"
        :param push: push option
        :return: (ret_code, ret_data). ret_code: RET_OK or RET_ERROR.
        """
        param_table = {'stock_code': stock_code, 'data_type': data_type}
        for x in param_table:
            param = param_table[x]
            if param is None or isinstance(param, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of %s param is wrong" % x
                return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(SubscriptionQuery.pack_subscribe_req,
                                                         SubscriptionQuery.unpack_subscribe_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'stock_str': stock_code, 'data_type': data_type}
        ret_code, msg, _ = query_processor(**kargs)

        # update subscribe context info
        self._ctx_subscribe.add((stock_code, data_type, push))

        if ret_code != RET_OK:
            return RET_ERROR, msg

        if push:
            ret_code, msg, push_req_str = SubscriptionQuery.pack_push_req(stock_code, data_type)

            if ret_code != RET_OK:
                return RET_ERROR, msg

            ret_code, msg = self._send_async_req(push_req_str)
            if ret_code != RET_OK:
                return RET_ERROR, msg

        return RET_OK, None

    def unsubscribe(self, stock_code, data_type, unpush=True):
        """
        unsubcribe a sort of data for a stock
        :param stock_code: string stock_code . For instance, "HK.00700", "US.AAPL"
        :param data_type: string  data type. For instance, "K_1M", "K_MON"
        :return: (ret_code, ret_data). ret_code: RET_OK or RET_ERROR.
        """

        param_table = {'stock_code': stock_code, 'data_type': data_type}
        for x in param_table:
            param = param_table[x]
            if param is None or isinstance(param, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of %s param is wrong" % x
                return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(SubscriptionQuery.pack_unsubscribe_req,
                                                         SubscriptionQuery.unpack_unsubscribe_rsp)
        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'stock_str': stock_code, 'data_type': data_type}

        # update subscribe context info
        self._ctx_subscribe.remove((stock_code, data_type, unpush))

        ret_code, msg, _ = query_processor(**kargs)

        if ret_code != RET_OK:
            return RET_ERROR, msg

        if unpush:
            ret_code, msg, unpush_req_str = SubscriptionQuery.pack_unpush_req(stock_code, data_type)

            if ret_code != RET_OK:
                return RET_ERROR, msg

            ret_code, msg = self._send_async_req(unpush_req_str)
            if ret_code != RET_OK:
                return RET_ERROR, msg

        return RET_OK, None

    def query_subscription(self, query=0):
        """
        get the current subscription table
        :return:
        """
        query_processor = self._get_sync_query_processor(SubscriptionQuery.pack_subscription_query_req,
                                                         SubscriptionQuery.unpack_subscription_query_rsp)
        kargs = {"query": query}

        ret_code, msg, subscription_table = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        return RET_OK, subscription_table

    def get_stock_quote(self, code_list):
        """
        :param code_list:
        :return: DataFrame of quote data

        Usage:

        After subcribe "QUOTE" type for given stock codes, invoke

        get_stock_quote to obtain the data

        """
        if code_list is None or isinstance(code_list, list) is False:
            error_str = ERROR_STR_PREFIX + "the type of code_list param is wrong"
            return RET_ERROR, error_str

        for code in code_list:
            if code is None or isinstance(code, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of param in code_list is wrong"
                return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(StockQuoteQuery.pack_req,
                                                         StockQuoteQuery.unpack_rsp,
                                                         )
        kargs = {"stock_list": code_list}

        ret_code, msg, quote_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'data_date', 'data_time', 'last_price', 'open_price',
                    'high_price', 'low_price', 'prev_close_price',
                    'volume', 'turnover', 'turnover_rate', 'amplitude', 'suspension', 'listing_date'
                    ]

        quote_frame_table = pd.DataFrame(quote_list, columns=col_list)

        return RET_OK, quote_frame_table

    def get_rt_ticker(self, code, num=500):
        """
        get transaction information
        :param code: stock code
        :param num: the default is 500
        :return: (ret_ok, ticker_frame_table)
        """

        if code is None or isinstance(code, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of code param is wrong"
            return RET_ERROR, error_str

        if num is None or isinstance(num, int) is False:
            error_str = ERROR_STR_PREFIX + "the type of num param is wrong"
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(TickerQuery.pack_req,
                                                         TickerQuery.unpack_rsp,
                                                         )
        kargs = {"stock_str": code, "num": num}
        ret_code, msg, ticker_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'time', 'price', 'volume', 'turnover', "ticker_direction", 'sequence']
        ticker_frame_table = pd.DataFrame(ticker_list, columns=col_list)

        return RET_OK, ticker_frame_table

    def get_cur_kline(self, code, num, ktype='K_DAY', autype='qfq'):
        """
        get current kline
        :param code: stock code
        :param num:
        :param ktype: the type of kline
        :param autype:
        :return:
        """
        param_table = {'code': code, 'ktype': ktype}
        for x in param_table:
            param = param_table[x]
            if param is None or isinstance(param, str) is False:
                error_str = ERROR_STR_PREFIX + "the type of %s param is wrong" % x
                return RET_ERROR, error_str

        if num is None or isinstance(num, int) is False:
            error_str = ERROR_STR_PREFIX + "the type of num param is wrong"
            return RET_ERROR, error_str

        if autype is not None and isinstance(autype, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of autype param is wrong"
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(CurKlineQuery.pack_req,
                                                         CurKlineQuery.unpack_rsp,
                                                         )

        kargs = {"stock_str": code, "num": num, "ktype": ktype, "autype": autype}
        ret_code, msg, kline_list = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        col_list = ['code', 'time_key', 'open', 'close', 'high', 'low', 'volume', 'turnover']
        kline_frame_table = pd.DataFrame(kline_list, columns=col_list)

        return RET_OK, kline_frame_table

    def get_order_book(self, code):
        if code is None or isinstance(code, str) is False:
            error_str = ERROR_STR_PREFIX + "the type of code param is wrong"
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(OrderBookQuery.pack_req,
                                                         OrderBookQuery.unpack_rsp,
                                                         )

        kargs = {"stock_str": code}
        ret_code, msg, orderbook = query_processor(**kargs)
        if ret_code == RET_ERROR:
            return ret_code, msg

        return RET_OK, orderbook


class OpenHKTradeContext(OpenContextBase):
    cookie = 100000

    def __init__(self, host="119.29.141.202", port=11111):
        self._ctx_unlock = None
        super(OpenHKTradeContext, self).__init__(host, port, True, False)

    def close(self):
        '''
        to call close old obj before loop create new, otherwise socket will encounter erro 10053 or more!
        '''
        super(OpenHKTradeContext, self).close()

    def on_api_socket_reconnected(self):
        # auto unlock
        if self._ctx_unlock is not None:
            for i in range(3):
                ret, data = self.unlock_trade(self._ctx_unlock)
                if ret == RET_OK:
                    break
                sleep(1)

    def unlock_trade(self, password):
        query_processor = self._get_sync_query_processor(UnlockTrade.pack_req,
                                                         UnlockTrade.unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'password': str(password)}

        ret_code, msg, unlock_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        # reconnected to auto unlock
        if RET_OK == ret_code:
            self._ctx_unlock = password
        return RET_OK, None

    def place_order(self, price, qty, strcode, orderside, ordertype=0, envtype=0):
        if int(envtype) not in rev_envtype_map:
            error_str = ERROR_STR_PREFIX + "the type of environment param is wrong "
            return RET_ERROR, error_str

        ret_code, content = split_stock_str(strcode)
        if ret_code == RET_ERROR:
            error_str = content
            return RET_ERROR, error_str, None

        market_code, stock_code = content
        if int(market_code) != 1:
            error_str = ERROR_STR_PREFIX + "the type of stocks is wrong "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(PlaceOrder.hk_pack_req,
                                                         PlaceOrder.hk_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': str(envtype), 'orderside': str(orderside),
                 'ordertype': str(ordertype), 'price': str(price), 'qty': str(qty), 'strcode': str(stock_code)}

        ret_code, msg, place_order_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ['envtype', 'orderid']
        place_order_table = pd.DataFrame(place_order_list, columns=col_list)

        return RET_OK, place_order_table

    def set_order_status(self, status, orderid=0, envtype=0):
        if int(status) not in rev_order_status:
            error_str = ERROR_STR_PREFIX + "the type of status is wrong "
            return RET_ERROR, error_str

        if int(envtype) not in rev_envtype_map:
            error_str = ERROR_STR_PREFIX + "the type of environment param is wrong "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(SetOrderStatus.hk_pack_req,
                                                         SetOrderStatus.hk_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': str(envtype), 'localid': str(0),
                 'orderid': str(orderid), 'status': str(status)}

        ret_code, msg, set_order_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ['envtype', 'orderID']
        set_order_table = pd.DataFrame(set_order_list, columns=col_list)

        return RET_OK, set_order_table

    def change_order(self, price, qty, orderid=0, envtype=0):
        if int(envtype) not in rev_envtype_map:
            error_str = ERROR_STR_PREFIX + "the type of environment param is wrong "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(ChangeOrder.hk_pack_req,
                                                         ChangeOrder.hk_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': str(envtype), 'localid': str(0),
                 'orderid': str(orderid), 'price': str(price), 'qty': str(qty)}

        ret_code, msg, change_order_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ['envtype', 'orderID']
        change_order_table = pd.DataFrame(change_order_list, columns=col_list)

        return RET_OK, change_order_table

    def accinfo_query(self, envtype=0):
        """
        query account information
        :param cookie: request operation flag
        :param envtype: trading environment parameters,0 means real transaction and 1 means simulation trading
        :return:error return RET_ERROR,msg and ok return RET_OK,ret
        """
        if int(envtype) not in rev_envtype_map:
            error_str = ERROR_STR_PREFIX + "the type of environment param is wrong "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(AccInfoQuery.hk_pack_req,
                                                         AccInfoQuery.hk_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': str(envtype)}

        ret_code, msg, accinfo_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ['Power', 'ZCJZ', 'ZQSZ', 'XJJY', 'KQXJ', 'DJZJ', 'ZSJE', 'ZGJDE', 'YYJDE', 'GPBZJ']
        accinfo_frame_table = pd.DataFrame(accinfo_list, columns=col_list)

        return RET_OK, accinfo_frame_table

    def order_list_query(self, statusfilter="", envtype=0):
        if int(envtype) not in rev_envtype_map:
            error_str = ERROR_STR_PREFIX + "the type of environment param is wrong "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(OrderListQuery.hk_pack_req,
                                                         OrderListQuery.hk_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': str(envtype), 'statusfilter': str(statusfilter)}
        ret_code, msg, order_list = query_processor(**kargs)

        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ["code", "stock_name", "dealt_avg_price", "dealt_qty", "qty",
                    "orderid", "order_type", "order_side", "price",
                    "status", "submited_time", "updated_time"]

        order_list_table = pd.DataFrame(order_list, columns=col_list)

        return RET_OK, order_list_table

    def position_list_query(self, envtype=0):
        if int(envtype) not in rev_envtype_map:
            error_str = ERROR_STR_PREFIX + "the type of environment param is wrong "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(PositionListQuery.hk_pack_req,
                                                         PositionListQuery.hk_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': str(envtype)}
        ret_code, msg, position_list = query_processor(**kargs)

        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ["code", "stock_name", "qty", "can_sell_qty", "cost_price",
                    "cost_price_valid", "market_val", "nominal_price", "pl_ratio",
                    "pl_ratio_valid", "pl_val", "pl_val_valid", "today_buy_qty",
                    "today_buy_val", "today_pl_val", "today_sell_qty", "today_sell_val"]

        position_list_table = pd.DataFrame(position_list, columns=col_list)

        return RET_OK, position_list_table

    def deal_list_query(self, envtype=0):
        if int(envtype) not in rev_envtype_map:
            error_str = ERROR_STR_PREFIX + "the type of environment param is wrong "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(DealListQuery.hk_pack_req,
                                                         DealListQuery.hk_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': str(envtype)}
        ret_code, msg, deal_list = query_processor(**kargs)

        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ["code", "stock_name", "dealid", "orderid",
                    "qty", "price", "orderside", "time"]

        deal_list_table = pd.DataFrame(deal_list, columns=col_list)

        return RET_OK, deal_list_table


class OpenUSTradeContext(OpenContextBase):
    cookie = 100000

    def __init__(self, host="119.29.141.202", port=11111):
        self._ctx_unlock = None
        super(OpenUSTradeContext, self).__init__(host, port, True, False)

    def close(self):
        '''
        to call close old obj before loop create new, otherwise socket will encounter erro 10053 or more!
        '''
        super(OpenUSTradeContext, self).close()

    def on_api_socket_reconnected(self):
        #auto unlock
        if self._ctx_unlock is not None:
            for i in range(3):
                ret, data = self.unlock_trade(self._ctx_unlock)
                if ret == RET_OK:
                    break

    def unlock_trade(self, password):
        query_processor = self._get_sync_query_processor(UnlockTrade.pack_req,
                                                         UnlockTrade.unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'password': str(password)}
        ret_code, msg, unlock_list = query_processor(**kargs)

        if ret_code != RET_OK:
            return RET_ERROR, msg

        #reconnected to auto unlock
        if RET_OK == ret_code:
            self._ctx_unlock = password

        return RET_OK, None

    def place_order(self, price, qty, strcode, orderside, ordertype=2, envtype=0):
        if int(envtype) != 0:
            error_str = ERROR_STR_PREFIX + "us stocks temporarily only support real trading "
            return RET_ERROR, error_str

        ret_code, content = split_stock_str(strcode)
        if ret_code == RET_ERROR:
            error_str = content
            return RET_ERROR, error_str, None

        market_code, stock_code = content
        if int(market_code) != 2:
            error_str = ERROR_STR_PREFIX + "the type of stocks is wrong "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(PlaceOrder.us_pack_req,
                                                         PlaceOrder.us_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': '0', 'orderside': str(orderside),
                 'ordertype': str(ordertype), 'price': str(price), 'qty': str(qty), 'strcode': str(stock_code)}

        ret_code, msg, place_order_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ['envtype', 'orderid']
        place_order_table = pd.DataFrame(place_order_list, columns=col_list)

        return RET_OK, place_order_table

    def set_order_status(self, status=0, orderid=0, envtype=0):
        if int(envtype) != 0:
            error_str = ERROR_STR_PREFIX + "us stocks temporarily only support real trading "
            return RET_ERROR, error_str

        if int(status) != 0:
            error_str = ERROR_STR_PREFIX + "us stocks temporarily only support cancel order "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(SetOrderStatus.us_pack_req,
                                                         SetOrderStatus.us_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': '0', 'localid': str(0),
                 'orderid': str(orderid), 'status': '0'}

        ret_code, msg, set_order_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ['envtype', 'orderID']
        set_order_table = pd.DataFrame(set_order_list, columns=col_list)

        return RET_OK, set_order_table

    def change_order(self, price, qty, orderid=0, envtype=0):
        if int(envtype) != 0:
            error_str = ERROR_STR_PREFIX + "us stocks temporarily only support real trading "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(ChangeOrder.us_pack_req,
                                                         ChangeOrder.us_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': '0', 'localid': str(0),
                 'orderid': str(orderid), 'price': str(price), 'qty': str(qty)}

        ret_code, msg, change_order_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ['envtype', 'orderID']
        change_order_table = pd.DataFrame(change_order_list, columns=col_list)

        return RET_OK, change_order_table

    def accinfo_query(self, envtype=0):
        if int(envtype) != 0:
            error_str = ERROR_STR_PREFIX + "us stocks temporarily only support real trading "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(AccInfoQuery.us_pack_req,
                                                         AccInfoQuery.us_unpack_rsp)

         # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': '0'}

        ret_code, msg, accinfo_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ['Power', 'ZCJZ', 'ZQSZ', 'XJJY', 'KQXJ', 'DJZJ', 'ZSJE', 'ZGJDE', 'YYJDE', 'GPBZJ']
        accinfo_frame_table = pd.DataFrame(accinfo_list, columns=col_list)

        return RET_OK, accinfo_frame_table

    def order_list_query(self, statusfilter="", envtype=0):
        if int(envtype) != 0:
            error_str = ERROR_STR_PREFIX + "us stocks temporarily only support real trading "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(OrderListQuery.us_pack_req,
                                                         OrderListQuery.us_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': '0', 'statusfilter': str(statusfilter)}

        ret_code, msg, order_list = query_processor(**kargs)
        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ["code", "stock_name", "dealt_avg_price", "dealt_qty", "qty",
                    "orderid", "order_type", "order_side", "price",
                    "status", "submited_time", "updated_time"]

        order_list_table = pd.DataFrame(order_list, columns=col_list)

        return RET_OK, order_list_table

    def position_list_query(self, envtype=0):
        if int(envtype) != 0:
            error_str = ERROR_STR_PREFIX + "us stocks temporarily only support real trading "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(PositionListQuery.us_pack_req,
                                                         PositionListQuery.us_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': '0'}
        ret_code, msg, position_list = query_processor(**kargs)

        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ["code", "stock_name", "qty", "can_sell_qty", "cost_price",
                    "cost_price_valid", "market_val", "nominal_price", "pl_ratio",
                    "pl_ratio_valid", "pl_val", "pl_val_valid", "today_buy_qty",
                    "today_buy_val", "today_pl_val", "today_sell_qty", "today_sell_val"]

        position_list_table = pd.DataFrame(position_list, columns=col_list)

        return RET_OK, position_list_table

    def deal_list_query(self, envtype=0):
        if int(envtype) != 0:
            error_str = ERROR_STR_PREFIX + "us stocks temporarily only support real trading "
            return RET_ERROR, error_str

        query_processor = self._get_sync_query_processor(DealListQuery.us_pack_req,
                                                         DealListQuery.us_unpack_rsp)

        # the keys of kargs should be corresponding to the actual function arguments
        kargs = {'cookie': str(self.cookie), 'envtype': '0'}
        ret_code, msg, deal_list = query_processor(**kargs)

        if ret_code != RET_OK:
            return RET_ERROR, msg

        col_list = ["code", "stock_name", "dealid", "orderid",
                    "qty", "price", "orderside", "time"]

        deal_list_table = pd.DataFrame(deal_list, columns=col_list)

        return RET_OK, deal_list_table




















