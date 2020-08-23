#coding=utf-8

# useful data for test case ,other module should import

testValue = [('风速控制', 'BBHIIII', [2, 1, 0x0101, 1, 0, 0, 2]),
             ('风扇板复位', 'BBHIIII', [2, 1, 0x0100, 1, 0, 0, 0]),
             ('风扇速率读取', 'BBHIIII', [2, 1, 0x0102, 1, 0, 0, 1]),
             # ('风扇板ARM在线升级','BBHIIII',[2,2,0x0103,1,0,0,0]),
             ('风扇板EEPROM测试', 'BBHIIII', [2, 1, 0x0104, 1, 0, 0, 0]),
             ('风扇板ARM版本查询', 'BBHIIII', [2, 1, 0x0105, 1, 0, 0, 0]),
             ('基带板SWITH路由配置', 'BBHIIII', [2, 2, 0x0205, 1, 0, 0, 1]),
             ('基带板SRIO路由配置', 'BBHIIII', [2, 2, 0x0206, 1, 0, 0, 1]),
             ('基带板SRIO数据', 'BBHIIII', [2, 2, 0xD401, 1, 0, 0, 1]),
             ('基带板AIF测试', 'BBHIIII', [2, 2, 0xD501, 1, 0, 0, 1]),
             ('基带板SDRAM读写', 'BBHIIII', [2, 2, 0x0209, 1, 0, 0, 1]),
             ('基带板温度查询', 'BBHIIII', [2, 2, 0x020A, 1, 0, 0, 1]),
             ('基带板电压电流查询', 'BBHIIII', [2, 2, 0x020B, 1, 0, 0, 1]),
             ('基带板EEPROM读写', 'BBHIIII', [2, 2, 0x020C, 1, 0, 0, 1]),
             ('基带板DSP_PPC链路', 'BBHIIII', [2, 2, 0xD303, 1, 0, 0, 1]),
             ('基带板DSP_DDR', 'BBHIIII', [2, 2, 0xD601, 1, 0, 0, 1]),
             ('基带板启动引导', 'BBHIIII', [2, 2, 0x0200, 1, 0, 0, 1]),
             ('基带板FPGA加载', 'BBHIIII', [2, 2, 0x0201, 1, 0, 0, 1]),
             ('基带板DSP加载', 'BBHIIII', [2, 2, 0x0202, 1, 0, 0, 1]),
             ('基带板SRIO效率', 'BBHIIII', [2, 2, 0xD402, 1, 0, 0, 1]),
             ('基带板ARM版本查询', 'BBHIIII', [2, 2, 0x0207, 1, 0, 0, 1]),
             ('基带板CPLD读写', 'BBHIIII', [2, 2, 0x0208, 1, 0, 0, 1]),
             ('主控板PHY测试', 'BBHIIII', [2, 4, 0x030C, 1, 0, 0, 1]),
             ('主控板CPLD读写', 'BBHIIII', [2, 4, 0x0301, 1, 0, 0, 1]),
             ('主控板SWITCH配置', 'BBHIIII', [2, 4, 0x0302, 1, 0, 0, 1]),
             ('主控板DDR读写', 'BBHIIII', [2, 4, 0x0303, 1, 0, 0, 1]),
             ('主控板Nandflash读写', 'BBHIIII', [2, 4, 0x0304, 1, 0, 0, 1]),
             ('主控板Norflash读写', 'BBHIIII', [2, 4, 0x0305, 1, 0, 0, 1]),
             ('主控板USB读写', 'BBHIIII', [2, 4, 0x0306, 1, 0, 0, 1]),
             ('主控板光模块读写', 'BBHIIII', [2, 4, 0x0307, 1, 0, 0, 1]),
             ('主控板温度查询', 'BBHIIII', [2, 4, 0x0308, 1, 0, 0, 1]),
             ('主控板电压电流查询', 'BBHIIII', [2, 4, 0x0309, 1, 0, 0, 1]),
             ('主控板GPS信息查询', 'BBHIIII', [2, 4, 0x030A, 1, 0, 0, 1]),
             ('主控板EEPROM', 'BBHIIII', [2, 4, 0x030B, 1, 0, 0, 1]),
             ('主控板PPC版本查询', 'BBHIIII', [2, 4, 0x030D, 1, 0, 0, 1]),
             ('下行链路', 'BBHIIII', [2, 0, 0xD503, 1, 0, 0, 2]),
             ('上行链路', 'BBHIIII', [2, 0, 0xD502, 1, 0, 0, 2]),
             ('主控板HMI读写基带板', 'BBHIIII', [2, 0, 0x0401, 1, 0, 0, 2]),
             ('主控板HMI读写风扇板', 'BBHIIII', [2, 0, 0x0402, 1, 0, 0, 2]),
             ('主控板HMI读写环境监控板', 'BBHIIII', [2, 0, 0x0403, 1, 0, 0, 2]),
             ('AFC锁定状态', 'BBHIIII', [2, 0, 0x0404, 1, 0, 0, 2]),
             ('整机启动状态', 'BBHIIII', [2, 0, 0x040B, 1, 0, 0, 2]),
             ('系统帧号同步开始', 'BBHIIII', [2, 0, 0xD40E, 1, 0, 0, 2]),
             ('系统帧号同步停止', 'BBHIIII', [2, 0, 0xD40F, 1, 0, 0, 2]),
             ('光时延值测量', 'BBHIIII', [2, 0, 0x0405, 1, 0, 0, 2]),
             ('环境监控板复位', 'BBHIIII', [2, 3, 0x0500, 1, 0, 0, 1]),
             ('整机掉电', 'BBHIIII', [2, 3, 0x0501, 1, 0, 0, 1]),
             ('环境监控板干结点', 'BBHIIII', [2, 3, 0x0502, 1, 0, 0, 1]),
             ('环境温度', 'BBHIIII', [2, 3, 0x0503, 1, 0, 0, 1]),
             ('环境监控板ARM版本查询', 'BBHIIII', [2, 3, 0x0504, 1, 0, 0, 1]), ]

updataValue = [('基带板MCU在线升级', 'BBHIIII', [1, 3, 0x0203, 1, 0, 0, 1]),
               ('基带板CPLD在线升级', 'BBHIIII', [1, 3, 0x0204, 1, 0, 0, 1]),
               ('主控板CPLD在线升级', 'BBHIIII', [1, 4, 0x0300, 1, 0, 0, 1]),
               ('环境监控板MCU在线升级', 'BBHIIII', [1, 1, 0x0505, 1, 0, 0, 1]),
               ('风扇板MCU在线升级', 'BBHIIII', [1, 1, 0x0103, 1, 0, 0, 0]),]

eepromValue = [('主控板保留信息写入', 'BBHIIII', [2, 4, 0xE000, 1, 0, 0, 256]),
               ('主控板校验数据写入', 'BBHIIII', [2, 4, 0xE001, 1, 0, 0, 2]),
               ('主控板设备代号写入', 'BBHIIII', [2, 4, 0xE002, 1, 0, 0, 16]),
               ('主控板类型及硬件版本写入', 'BBHIIII', [2, 4, 0xE003, 1, 0, 0, 32]),
               ('主控板MAC地址1写入', 'BBHIIII', [2, 4, 0xE004, 1, 0, 0, 6]),
               ('主控板MAC地址2写入', 'BBHIIII', [2, 4, 0xE005, 1, 0, 0, 6]),
               ('主控板生产序列号写入', 'BBHIIII', [2, 4, 0xE006, 1, 0, 0, 32]),
               ('主控板制造商写入', 'BBHIIII', [2, 4, 0xE007, 1, 0, 0, 12]),
               ('主控板生产日期写入', 'BBHIIII', [2, 4, 0xE008, 1, 0, 0, 4]),
               ('主控板卫星接收机型号写入', 'BBHIIII', [2, 4, 0xE009, 1, 0, 0, 12]),
               ('主控板风扇出厂转速写入', 'BBHIIII', [2, 4, 0xE00A, 1, 0, 0, 2]),
               ('主控板工作温度范围写入', 'BBHIIII', [2, 4, 0xE00B, 1, 0, 0, 2]),
               ('主控板保留信息读取', 'BBHIIII', [2, 4, 0xE100, 1, 0, 0, 0]),
               ('主控板校验数据读取', 'BBHIIII', [2, 4, 0xE101, 1, 0, 0, 0]),
               ('主控板设备代号读取', 'BBHIIII', [2, 4, 0xE102, 1, 0, 0, 0]),
               ('主控板类型及硬件版本读取', 'BBHIIII', [2, 4, 0xE103, 1, 0, 0, 0]),
               ('主控板MAC地址1读取', 'BBHIIII', [2, 4, 0xE104, 1, 0, 0, 0]),
               ('主控板MAC地址2读取', 'BBHIIII', [2, 4, 0xE105, 1, 0, 0, 0]),
               ('主控板生产序列号读取', 'BBHIIII', [2, 4, 0xE106, 1, 0, 0, 0]),
               ('主控板制造商读取', 'BBHIIII', [2, 4, 0xE107, 1, 0, 0, 0]),
               ('主控板生产日期读取', 'BBHIIII', [2, 4, 0xE108, 1, 0, 0, 0]),
               ('主控板卫星接收机型号读取', 'BBHIIII', [2, 4, 0xE109, 1, 0, 0, 0]),
               ('主控板风扇出厂转速读取', 'BBHIIII', [2, 4, 0xE10A, 1, 0, 0, 0]),
               ('主控板工作温度范围读取', 'BBHIIII', [2, 4, 0xE10B, 1, 0, 0, 0]),
               ('基带板保留信息写入', 'BBHIIII', [2, 2, 0xE000, 1, 0, 0, 256]),
               ('基带板校验数据写入', 'BBHIIII', [2, 2, 0xE001, 1, 0, 0, 2]),
               ('基带板设备代号写入', 'BBHIIII', [2, 2, 0xE002, 1, 0, 0, 16]),
               ('基带板类型及硬件版本写入', 'BBHIIII', [2, 2, 0xE003, 1, 0, 0, 32]),
               ('基带板MAC地址1写入', 'BBHIIII', [2, 2, 0xE004, 1, 0, 0, 6]),
               ('基带板MAC地址2写入', 'BBHIIII', [2, 2, 0xE005, 1, 0, 0, 6]),
               ('基带板生产序列号写入', 'BBHIIII', [2, 2, 0xE006, 1, 0, 0, 32]),
               ('基带板制造商写入', 'BBHIIII', [2, 2, 0xE007, 1, 0, 0, 12]),
               ('基带板生产日期写入', 'BBHIIII', [2, 2, 0xE008, 1, 0, 0, 4]),
               ('基带板卫星接收机型号写入', 'BBHIIII', [2, 2, 0xE009, 1, 0, 0, 12]),
               ('基带板风扇出厂转速写入', 'BBHIIII', [2, 2, 0xE00A, 1, 0, 0, 2]),
               ('基带板工作温度范围写入', 'BBHIIII', [2, 2, 0xE00B, 1, 0, 0, 2]),
               ('基带板保留信息读取', 'BBHIIII', [2, 2, 0xE100, 1, 0, 0, 0]),
               ('基带板校验数据读取', 'BBHIIII', [2, 2, 0xE101, 1, 0, 0, 0]),
               ('基带板设备代号读取', 'BBHIIII', [2, 2, 0xE102, 1, 0, 0, 0]),
               ('基带板类型及硬件版本读取', 'BBHIIII', [2, 2, 0xE103, 1, 0, 0, 0]),
               ('基带板MAC地址1读取', 'BBHIIII', [2, 2, 0xE104, 1, 0, 0, 0]),
               ('基带板MAC地址2读取', 'BBHIIII', [2, 2, 0xE105, 1, 0, 0, 0]),
               ('基带板生产序列号读取', 'BBHIIII', [2, 2, 0xE106, 1, 0, 0, 0]),
               ('基带板制造商读取', 'BBHIIII', [2, 2, 0xE107, 1, 0, 0, 0]),
               ('基带板生产日期读取', 'BBHIIII', [2, 2, 0xE108, 1, 0, 0, 0]),
               ('基带板卫星接收机型号读取', 'BBHIIII', [2, 2, 0xE109, 1, 0, 0, 0]),
               ('基带板风扇出厂转速读取', 'BBHIIII', [2, 2, 0xE10A, 1, 0, 0, 0]),
               ('基带板工作温度范围读取', 'BBHIIII', [2, 2, 0xE10B, 1, 0, 0, 0]),
               ('风扇板保留信息写入', 'BBHIIII', [2, 1, 0xE000, 1, 0, 0, 256]),
               ('风扇板校验数据写入', 'BBHIIII', [2, 1, 0xE001, 1, 0, 0, 2]),
               ('风扇板设备代号写入', 'BBHIIII', [2, 1, 0xE002, 1, 0, 0, 16]),
               ('风扇板类型及硬件版本写入', 'BBHIIII', [2, 1, 0xE003, 1, 0, 0, 32]),
               ('风扇板MAC地址1写入', 'BBHIIII', [2, 1, 0xE004, 1, 0, 0, 6]),
               ('风扇板MAC地址2写入', 'BBHIIII', [2, 1, 0xE005, 1, 0, 0, 6]),
               ('风扇板生产序列号写入', 'BBHIIII', [2, 1, 0xE006, 1, 0, 0, 32]),
               ('风扇板制造商写入', 'BBHIIII', [2, 1, 0xE007, 1, 0, 0, 12]),
               ('风扇板生产日期写入', 'BBHIIII', [2, 1, 0xE008, 1, 0, 0, 4]),
               ('风扇板卫星接收机型号写入', 'BBHIIII', [2, 1, 0xE009, 1, 0, 0, 12]),
               ('风扇板风扇出厂转速写入', 'BBHIIII', [2, 1, 0xE00A, 1, 0, 0, 2]),
               ('风扇板工作温度范围写入', 'BBHIIII', [2, 1, 0xE00B, 1, 0, 0, 2]),
               ('风扇板保留信息读取', 'BBHIIII', [2, 1, 0xE100, 1, 0, 0, 0]),
               ('风扇板校验数据读取', 'BBHIIII', [2, 1, 0xE101, 1, 0, 0, 0]),
               ('风扇板设备代号读取', 'BBHIIII', [2, 1, 0xE102, 1, 0, 0, 0]),
               ('风扇板类型及硬件版本读取', 'BBHIIII', [2, 1, 0xE103, 1, 0, 0, 0]),
               ('风扇板MAC地址1读取', 'BBHIIII', [2, 1, 0xE104, 1, 0, 0, 0]),
               ('风扇板MAC地址2读取', 'BBHIIII', [2, 1, 0xE105, 1, 0, 0, 0]),
               ('风扇板生产序列号读取', 'BBHIIII', [2, 1, 0xE106, 1, 0, 0, 0]),
               ('风扇板制造商读取', 'BBHIIII', [2, 1, 0xE107, 1, 0, 0, 0]),
               ('风扇板生产日期读取', 'BBHIIII', [2, 1, 0xE108, 1, 0, 0, 0]),
               ('风扇板卫星接收机型号读取', 'BBHIIII', [2, 1, 0xE109, 1, 0, 0, 0]),
               ('风扇板风扇出厂转速读取', 'BBHIIII', [2, 1, 0xE10A, 1, 0, 0, 0]),
               ('风扇板工作温度范围读取', 'BBHIIII', [2, 1, 0xE10B, 1, 0, 0, 0]), ]

testValue_FM = testValue[:5]
testValue_BBP = testValue[5:21]
testValue_MCT = testValue[21:34]
testValue_IT = testValue[34:44]
testValue_EM = testValue[44:]

eepromValue_MCT = eepromValue[:24]
eepromValue_BBP = eepromValue[24:48]
eepromValue_GES = eepromValue[48:72]
eepromValue_FM = eepromValue[72:]

testItem_MCT = ['PHY', 'CPLD', 'SWITCH', 'DDR', 'NandFlash', 'NorFlash', 'USB',
                '光模块', '温度', '功耗', 'GPS', 'EEPROM', '版本', 'NULL2', 'NULL3']

testItem_BBP = ['SWITCH', 'SRIO路由', 'SRIO数据', 'AIF', 'SDRAM', '温度', '功耗', 'EEPROM', 'DSP链路',
                'DSP_DDR', '复位', 'FPGA加载', 'DSP加载', 'SRIO效率', '版本', 'CPLD', 'NULL1', 'NULL2', 'NULL3', 'NULL4']

testItem_GES = ['SWITCH', '模式', '温度', 'EEPROM', '复位']

testItem_FM = ['风速控制', '复位', '风速', 'EEPROM', '版本']

testItem_IT = ['下行', '上行', 'HMI_MB', 'HMI_MF', 'HMI_ME', 'AFC', '启动状态', '同步开',
               '同步停', '光时延']

testItem_EM = ['复位', '整机掉电', '干结点', '温度', '版本']


def sendValue():
    dic = {}
    for item in testValue:
        a = item[2][2]
        dic[a] = 0

    return dic


def recValue():
    dic = {}
    for item in testValue:
        a = item[2][2]
        dic[a] = [0,0,0] #[succ,fail,timeout]

    return dic


def testID_TAB(testValue):
    testID = []
    for item in testValue:
        ID = item[2][2]
        testID.append(ID)
    return testID


def eepromSend():
    dic = {}
    for item in eepromValue:
        a = item[2][2]
        dic[a] = 0
    return dic


def eepromRec():
    dic = {}
    for item in eepromValue:
        a = item[2][2]
        dic[a] = [0, 0, 0]
    return dic

testID_MCT = testID_TAB(testValue_MCT)
testID_IT = testID_TAB(testValue_IT)
testID_BBP = testID_TAB(testValue_BBP)
# testID_GES=testID_TAB(testValue_GES)
testID_FM = testID_TAB(testValue_FM)
testID_EM = testID_TAB(testValue_EM)


if __name__=='__main__':
#    print(sendValue())
     print(sendValue().values())
