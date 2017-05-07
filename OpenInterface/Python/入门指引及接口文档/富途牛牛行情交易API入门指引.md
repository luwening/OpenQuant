### 富途牛牛行情交易API入门指引

**准备工作**

1.      准备一台Windows 32bit 或者64bit 的电脑（win7/8/10）  
2.      下载富途牛牛客户端，需要在QQ群108534288群文件中下载专用版本![image](https://github.com/FutunnOpen/OpenQuant/raw/master/Resources/Download.png)  
3.      下载Anaconda python 3和 Pycharm 2016.3.2，Anaconda python 是Python科学技术包的合集，提供了很多用于科学计算的模块，Pycharm是一款Python的IDE，用于程序开发，可以在QQ群108534288群文件中下载python接入开发相关工具，可选择32位或者64位下载。如果已经安装了Anaconda python2，也可以使用。  
4.      在GITHUB上下载FutunnOpen项目代码并解压<https://github.com/FutunnOpen/OpenQuant>


---

**安装部署**

1.      安装富途牛牛客户端，注册并登录，可以在客户端直接注册或者在网站注册，网站注册链接<https://passport.futu5.com/?target=http%3A%2F%2Fwww.futunn.com%2F#reg>

2.      安装Anaconda python 3, Pycharm 2016.3.2，请尽量先安装Anaconda python再安装Pycharm，如果不是这个顺序那么还需要在Pycharm中手动添加Anaconda python的路径

3.      在操作系统的命令行模式中输入netstat -nat | findstr 11111，查看网络连接，看状态是否为LISTEN，不是的话请检查是否开启了富途牛牛客户端

4.      在不安装富途牛牛客户端的情况下，还可以选择无需客户端的仅体验版本，在Pycharm中运行项目代码中OpenQuant\OpenInterface\Python下的sample.py，把sample.py中host='127.0.0.1'的值改为119.29.141.202，即可利用云端进行测试，运行。

---

**使用**

1. 在开启了富途牛牛客户端或者利用云端获取数据的情况下，在Pycharm中运行项目代码中OpenQuant\OpenInterface\Python下的sample.py，即可进行调试，运行。

2. 注册并登录富途牛牛即可免费获取港股实时行情，有效期至10月底

3. 行情和交易接口文档可在如下地址获取，

   <https://github.com/FutunnOpen/OpenQuant/tree/master/OpenInterface/Python>

​      