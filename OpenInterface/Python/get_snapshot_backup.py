from openft.open_quant_context import *
from host import *
import time
import pandas as pd
import sys
import prefix

quote_context = OpenQuoteContext(host=host, async_port=11111)
name=sys.argv[1]
data=pd.read_csv(name)


hk_codes=list(pd.read_csv(name)['code'].values)

while True:
    try:
        prefix_name=prefix.get_prefix()
        snap=[]
        for i in range(1,len(hk_codes),200):
            print(i)
            ret=1
            while ret>0:
                ret,y=quote_context.get_market_snapshot(hk_codes[i:i+200])
                time.sleep(1)
            snap.append(y)
            time.sleep(0.5)
        z=pd.concat(snap)
        z=z.reset_index(drop=True)
        z.to_csv('snap/'+name+'_'+prefix_name+'.csv')
        time.sleep(1)
    except:
        time.sleep(1)



