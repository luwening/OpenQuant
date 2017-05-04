### 历史K线下载指引

历史K线提供了两种下载方式，可自行任选其一即可

**程序自动下载**

1. 下载安装。在QQ群108534288群文件中下载API专用版本，安装。 
  ![image](https://github.com/FutunnOpen/OpenQuant/raw/master/Resources/Download.png)

2. 历史数据存放。有默认路径存放和自定义存放两种方式。选择一种即可。 

   如果选择默认历史数据的存放目录，不需要做任何更改，直接到第3步。（默认存放在%UserProfile%\AppData\Roaming\FTNN\1.0\Common\HistData）

   如果选择自定义目录，那么需要修改配置文件。修改配置文件方法如下：打开富途牛牛的安装目录（这里以默认安装路径为例），C:\Program Files (x86)\FTNN\plugin，以记事本方式打开这个文件夹下的ftplugin.ini，如果默认的下载市场，类型符合个人需求，就不需要改动。如果还有额外的需求，市场类型，K线类型都可以根据自己需要修改。其中分K提供最近三个月的，日K等提供2000年以后的。
   ![image](https://github.com/FutunnOpen/OpenQuant/raw/master/Resources/HistDataConfig.png) 
   设置自定义历史数据存放位置，将hist_data_dir=D:\ft_hist_data这句话的句首分号去掉即可。新路径在重启富途牛牛之后才会生效。

3. 历史数据下载。先开启富途牛牛，再打开富途牛牛的安装目录（这里以默认安装路径为例）C:\Program Files (x86)\FTNN\FTHistData.exe，运行FTHistData.exe。

   如果在第2步中选择的是默认存放目录，那么下载的历史数据存放在%UserProfile%\AppData\Roaming\FTNN\1.0\Common\HistData。 

   如果在第2步中选择的是自定义目录，那么下载的历史数据存放在自定义目录D:\ft_hist_data。无论哪个存放目录都会有个更新历史数据的界面提示更新进度。更新过程中最小化界面即可。 

4. 历史数据更新。支持增量更新，每天运行FTHistData.exe，就会自动下载尚未更新的数据了。 


---

**历史数据导入**

1. 在http://pan.baidu.com/s/1gf7SvJX中下载历史数据导入包，提供了三大市场的历史数据，可以全部下载，也可以根据个人需求相应下载。 
   ![image](https://github.com/FutunnOpen/OpenQuant/raw/master/Resources/HistDataDownload.png)

2. 将上述下载好的数据导入包放在默认目录或者自定义目录下。 

   如果放在默认路径下，不需要修改配置文件，找到目录%UserProfile%\AppData\Roaming\FTNN\1.0\Common\HistData，新建个文件夹ImportDataDir，把数据放入即可。 

   如果放在自定义目录下，需要修改配置文件，修改配置文件方式如下：打开富途牛牛安装目录（这里以默认安装路径为例），C:\Program Files (x86)\FTNN\plugin，以记事本方式打开这个文件夹下的ftplugin.ini，将hist_data_dir=D:\ft_hist_data这句话的句首分号去掉，设置自定义历史数据存放位置。 
   ![image](https://github.com/FutunnOpen/OpenQuant/raw/master/Resources/HistDataImport.png)

   把下载好的历史数据导入包，放入自定义历史数据存放目录D:\ft_hist_data\下，新建个文件夹ImportDataDir（有这个文件夹的话就不用新建了），把数据放入即可。 

3. 先重启富途牛牛，在打开富途牛牛的安装目录C:\Program Files (x86)\FTNN，找到FTHistData.exe并运行，即可实现历史数据的导入。在更新历史数据界面会提示导入数据进度，提示更新完成表示导入数据已成功。 

4. 新数据在上述链接http://pan.baidu.com/s/1gf7SvJX中获得，重复步骤1，2，3即可（只有第一次导入在自定义目录中才需要修改配置文件，以后导入数据不需要重复修改配置文件）。 









