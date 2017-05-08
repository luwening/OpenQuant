# monitor the 15M realtime 

import sys
from openft.open_quant_context import *
from host import *
import time
import numpy as np
import talib as tl
import matplotlib.pylab as plt
import time

from IPython import display

np.set_printoptions(suppress=True)

quote_context = OpenQuoteContext(host=host, async_port=11111)

codes=['SZ.002030','HK.02328','SZ.002557','SZ.002517','SZ.002283','HK.800000','HK.00700']

for code in codes:
    quote_context.subscribe(code,'K_15M')


while True:
    for code in codes:
        x,y1=quote_context.get_cur_kline(code, 150, ktype='K_15M', autype='qfq')
        #do some other things
        if code=='HK.80000':
            x,y1=quote_context.get_cur_kline(code, 50, ktype='K_5M', autype='qfq')
            #do some other things
    time.sleep(60)
