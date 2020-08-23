/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   defines the load version string
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   03/27/2006   Yushu Shi      Initial file creation.
1、增加了复位DSp 5的话，同时复位DSP 1;
2、修改了boot任务中定时器的问题
3、文件系统还是有问题的,如果有问题的话，
4、修改如果L2复位的下载数据等待时间为1s，以前为10s
5、增加ts配置目前L2层只支持4:4和5:3 ,6:2,3:5,2:6  
6、增加对timeoffset的支持SetOffset()函数
7、将取版本的时间长度修改为5s,以前30s太长，如果取不到版本到复位的话需要90s，现在修改为15s
比较合理
8、由于将取版本时间缩短为5s,对wrru FPGA代码加载有影响，可能是消息被丢失。特修改如下代码，如果
处于代码加载时，不响应取版本消息，同时开始计数，如果超过100s，WRRU没有复位的话，则将相关标志清零，
修改WRRU文件和l3taskdiag.cpp文件 

 9、本版本基于大基站目前的版本为"McWill 1.6.0.22"
 10、不支持WCPE的相关功能，语音模块也同步到最新版本
 11、修改sendtowan函数，不能往网络发送数据的bug;
 12、增加校准成功的话，则将数据同时保存在WRRU的NVRAM数据结构中；
 13、增加对单个DSP的复位。
 14、修改终端代码不能升级的bug，由于DealCpeUpdateReqNew这个函数没有返回值2010\5\19
 15、增加对新EMS的修改；
 16、修改了不能给core0发消息的bug
 17、增加了发送温度给aux;
 18、增加了校准告警；
 19、增加了负载均衡的相关功能20100628
 20、将DMShow 和SNShow 基站号显示修改16进制20100630
 21、将EMS加载文件命名为BBU_RRU.X.X.X.X.BIN格式。20100702
 22、将WBBU_TOP.bin修改为wbbu_fpga.bin
 23、将增加对在线用户列表的支持
 24、修改风扇告警的显示20100705
 25、增加对复位WRRU和WRRU远程代码加载的支持
 26、fiber delay为2个字节
 27、电流值*2
 28、增加对CPU面板上灯的控制
 29、修改WRRU的告警显示包括温度和电流容易理解
 30、增加对WCPE和Rcpe的支持0715 版本号修改为2.7.0.29,对应BSP也增加
 31、增加了arpResolve网关的功能替换成ping的功能
 32、操作系统增加对arpAdd等函数的支持。
 33、修改了WRRU温度负值显示不对的问题20100910
 34、修改了从EMS做同步配置时集群配置越界的问题
 35、修改了WBBU温度的bug 20100901
 36、修改周胜提出当RRU与BBU失连时，基站状态显示为RF打开状态,现在修改为显示关闭，当连接上时，打开
 37、增加告警，当不是GPS做同步方式时，产生告警
 38、增加对RTC I2C的支持20110421
 *---------------------------------------------------------------------------*/
#ifndef LOAD_VERSION_H

#define LOAD_VERSION_H


#define VERSION "McWill 2.6.0.2"

#endif

