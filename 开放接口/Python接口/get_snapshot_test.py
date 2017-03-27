from openft.open_quant_context import *
from host import *
import time
import pandas as pd


quote_context = OpenQuoteContext(host=host, async_port=11111)
hk_codes=pd.read_csv('HK.csv')['code'].values

while True:
    snap=[]
    for i in range(1,len(hk_codes),200):
        x,y=quote_context.get_market_snapshot(list(hk_codes[i:i+200]))
        if x==0:
            snap.append(y)
        else:
            print(x)
            print(y)
    z=pd.concat(snap)
    z=z.reset_index(drop=True)
    z.to_csv('snap/nap_hk_'+str(time.time())+'.csv')
    time.sleep(1)



