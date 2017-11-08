from OpenQuant.open_quant_context import *
from host import *
import time
import pandas as pd
import sys
import prefix

#quote_context = OpenQuoteContext(host=host )
#host=futu_host
quote_context = OpenQuoteContext(host=host,port=11111)# async_port=11111 )
name=sys.argv[1]
data=pd.read_csv(name)


hk_codes=list(pd.read_csv(name)['code'].values)

count=0
while True:
    snap=[]
    prefix_name=prefix.get_prefix()
    count+=1
    print(count)
    for i in range(1,len(hk_codes),200):
        print(i)
        #print(hk_codes[i:i+200])
        ret=-1
        while ret==-1:
            ret,y=quote_context.get_market_snapshot(hk_codes[i:i+200])
            if ret==-1:
                print(ret,y)
            time.sleep(5)
        #if x==0:
        snap.append(y)
        time.sleep(0.5)
    z=pd.concat(snap)
    z=z.reset_index(drop=True)
    z.to_csv('daily_hk_warrant_snap2/'+name+'_'+prefix_name+'.csv')
    break
    #time.sleep(5)
