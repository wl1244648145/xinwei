
# -*- coding: utf-8 -*-
# 20150516 for BTS Platform test

import logging
import socket
import struct
from time import sleep, ctime
import threading
from configparser import ConfigParser
import emsServer
from tkinter import mainloop
import sqlite
import serial
import queue, os

config_para = ConfigParser()
config_para.read('para.ini')

platform_type = config_para.get('eNB', 'TYPE')
if platform_type == 'CZZ':
    import myGUI_grid_CZZ as myGUI_grid
if platform_type == 'eBBU':
    import myGUI_grid_eBBU as myGUI_grid

logger = logging.getLogger('test')
logger.setLevel(logging.DEBUG)

# fh=logging.FileHandler('log/%s'%(logName()))
# fh.setLevel(logging.DEBUG)

ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)

formatter = logging.Formatter('%(asctime)s - %(lineno)d - %(message)s')
# fh.setFormatter(formatter)
ch.setFormatter(formatter)

# logger.addHandler(fh)
logger.addHandler(ch)

LOCAL_IP = config_para.get('LOCAL', 'IP')
LOCAL_PORT = config_para.getint('LOCAL', 'PORT')
LOCAL_ADDR = (LOCAL_IP, LOCAL_PORT)

TRX_IP = config_para.get('eNB', 'TRX')
TRX_PORT = config_para.getint('eNB', 'TRXPORT')
TRX_ADDR = (TRX_IP, TRX_PORT)
DSTDIR = config_para.get('eNB', 'DSTDIR')

TIMEOUT = config_para.getint('SLEEP', 'TIMEOUT')
SUB_GAP = config_para.getint('SLEEP', 'SUB_GAP')

# ADD stop test when fail or timeout   20160115
DEBUG = config_para.getint('DEBUG', 'VALUE')

stop_flag_page1 = 0
stop_flag_page2 = 0
stop_flag_page3 = 0
stop_flag_page4 = 0
stop_flag_page5 = 0
stop_flag_page6 = 0
stop_flag_page7 = 0

COM = serial.Serial()


def current_page():
    tab_select = myGUI_grid.GUI.notebook.select()
    tab = tab_select.split('.')[-1]
    return tab


def thread_control_flag(switch, stop_flag):
    # switch: 0 is stop, 1 is start, 2 is return current flag
    global stop_flag_page1, stop_flag_page2, stop_flag_page3, stop_flag_page4, stop_flag_page5, stop_flag_page6, stop_flag_page7
    if switch == 0:
        if stop_flag == 1:
            stop_flag_page1 = 1
        elif stop_flag == 2:
            stop_flag_page2 = 1
        elif stop_flag == 3:
            stop_flag_page3 = 1
        elif stop_flag == 4:
            stop_flag_page4 = 1
        elif stop_flag == 5:
            stop_flag_page5 = 1
        elif stop_flag == 6:
            stop_flag_page6 = 1
        elif stop_flag == 7:
            stop_flag_page7 = 1
        else:
            logger.info('stop flag is not define!!\n')

    elif switch == 1:
        if stop_flag == 1:
            stop_flag_page1 = 0
        elif stop_flag == 2:
            stop_flag_page2 = 0
        elif stop_flag == 3:
            stop_flag_page3 = 0
        elif stop_flag == 4:
            stop_flag_page4 = 0
        elif stop_flag == 5:
            stop_flag_page5 = 0
        elif stop_flag == 6:
            stop_flag_page6 = 0
        elif stop_flag == 7:
            stop_flag_page7 = 0
        else:
            logger.info('start flag is not define!!\n')

    elif switch == 2:
        if stop_flag == 1:
            return stop_flag_page1
        elif stop_flag == 2:
            return stop_flag_page2
        elif stop_flag == 3:
            return stop_flag_page3
        elif stop_flag == 4:
            return stop_flag_page4
        elif stop_flag == 5:
            return stop_flag_page5
        elif stop_flag == 6:
            return stop_flag_page6
        elif stop_flag == 7:
            return stop_flag_page7
        else:
            logger.info('return value flag is not define!!\n')

    else:
        logger.info('switch value is not define!!\n')


def com_config():
    com_port = myGUI_grid.GUI.combobox_IT_com.get()
    COM.port = int(com_port[-1])-1
    COM.baudrate = 9600
    try:
        COM.open()
        myGUI_grid.inert_message_queue.put('%s连接成功\n' % com_port)
    except serial.serialutil.SerialException:
        myGUI_grid.inert_message_queue.put('%s连接失败\n' % com_port)


class PlatFixTest:

    testTimes = 1

    send_times_update = sqlite.send_times_update
    send_times_case = sqlite.send_times_statistics
    send_times_eeprom = sqlite.send_times_eeprom
    send_headType = '>I'
    send_headData = [0xaabbbeef, ]
    send_bodyType = 'BBHIIII'
    send_tailType = 'I'
    send_tailData = [0xaa5555aa, ]

    def __init__(self, sql_case='', sql_e2prom=''):

        self.tab_dic = {}
        self.sql_case = sql_case
        self.sql_e2prom = sql_e2prom
        self.public_config()
        self.private_config()

    def public_config(self):
        myGUI_grid.GUI.button_IT_1['command'] = self.setENB
        myGUI_grid.GUI.button_control_1['command'] = self.setTimes
        myGUI_grid.GUI.button_IT_2['command'] = lambda: self.eth3_config(sqlite.config_value[0])
        myGUI_grid.GUI.button_control_2['command'] = self.testResultShow
        try:
            myGUI_grid.GUI.button_IT_com['command'] = com_config
        except AttributeError:
            pass

    def private_config(self):
        pass

    def test_case_setup(self, test_item, test_id_list):
        for test_id in test_id_list:
                self.send_times_case[test_id] = 0
                emsServer.recValue[test_id] = [0, 0, 0]
        numList = []
        testIDList = []

        for Item in test_item:
                if Item.get() == 1:
                       num = test_item.index(Item)
                       testID = test_id_list[num]
                       numList.append(num)
                       testIDList.append(testID)
        sleep(0.1)
        tab_select = myGUI_grid.GUI.notebook.select()
        # print('----', tab_select)
        tab = tab_select.split('.')[-1]

        item_ui = self.tab_dic[tab][1]
        for item in item_ui:
                try:
                    item['foreground']='black'
                except:
                    continue
        return numList, testIDList

    def fix_test_case(self, test_item, test_id_list):

            pass

    def eeprom_data_generator(self, data_text, i):
            data_list = []
            if i in [0, 1, 4, 5, 7]:
                    data_byte = data_text.encode('ascii')
                    dataToList_type = '>'+'B'*len(data_byte)
                    data = struct.unpack(dataToList_type,data_byte)
                    data_list = list(data)

            elif i in [2, 3]:
                    while data_text:
                            c = data_text[0:2]
                            try:
                                    c = int(c,16)
                                    data_list.append(c)
                                    data_text = data_text[2:]
                            except ValueError:
                                    insert_message = '写入字符格式错误！\n'
                                    myGUI_grid.inert_message_queue.put(insert_message)
                                    break

            elif i == 6:
                    c = ctime()
                    c = c.split()
                    moth = {'Oct': 10, 'Jan': 1, 'Feb': 2, 'Mar': 3, 'Apr': 4, 'May': 5, 'Jun': 6, 'Jul': 7, 'Aug': 8,
                            'Sep': 9, 'Nov': 11, 'Dec': 12}
                    if c[1] in moth:
                            mothValue = moth[c[1]]
                            data_list.append(mothValue)
                            dayValue = c[2]
                            data_list.append(int(dayValue))
                            yearValue = c[-1]
                            yearValue_byteH = struct.pack('>H',int(yearValue))
                            yearValue_dataB = struct.unpack('>BB',yearValue_byteH)
                            yearValue_dataB = list(yearValue_dataB)
                            data_list.extend(yearValue_dataB)
                            # print(data_list)
                    else:
                            insert_message = '日期格式错误！\n'
                            myGUI_grid.inert_message_queue.put(insert_message)

            elif i == 8:
                    fan_speed_initialize = data_text.split('|')
                    for speed in fan_speed_initialize:
                        speed = int(speed.strip())
                        if speed > 65535:
                            myGUI_grid.inert_message_queue.put('风速值超出范围，风扇速率写入失败！\n')
                            return
                        speed_byteH = struct.pack('>H', speed)
                        speed_byteB = struct.unpack('>BB', speed_byteH)
                        data_list.extend(list(speed_byteB))

            elif i == 9:
                    tempRange = data_text.split('~')
                    for value in tempRange:
                            while ' ' in value:
                                    value = value.strip()
                            data_list.insert(0, int(value))

            return data_list

    def send_message_struct(self, data_struc_data, null_len=0, data_type='', data_list=[]):
            send_type = self.send_headType + self.send_bodyType + data_type + self.send_tailType
            send_data = self.send_headData + data_struc_data + data_list + [0]*null_len + self.send_tailData
            return send_type, send_data

    def fix_test_eeprom_write(self, option, entry, index):
            eepromContent = ['0x'+'ff'*16, '0x'+'ff'*32, '0x'+'ff'*6, '0x'+'ff'*6, '0x'+'ff'*32, '0x'+'ff'*12,
                             '0x'+'ff'*4, '0x'+'ff'*12, '0x'+'ff'*2, '0x'+'ff'*2]

            for test_id in sqlite.test_id_eeprom_total:
                    self.send_times_eeprom[test_id] = 0
                    emsServer.eepromRec[test_id] = [0, 0, 0]

            socket_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            for i in range(10):
                    if option[i]['state'] == 'normal':
                            testID = sqlite.test_id_eeprom_total[i+2]
                            line_no = i + 2 + index
                            test_case_info = sqlite.query_single_NO(line_no, self.sql_e2prom)
                            msg = test_case_info[0][1]
                            data_struc_data = list(test_case_info[0][2:])
                            data_len = test_case_info[0][-1]
                            data_text = entry[i].get()
                            data_list = self.eeprom_data_generator(data_text, i)

                            null_len = data_len-len(data_list)

                            # todo eBBU's datalen = data_lisit + slot
                            if len(data_list) > data_len:
                                    insert_message = '写EEPROM失败，输入字节长度太长！\n'
                                    myGUI_grid.inert_message_queue.put(insert_message)
                                    break
                            else:
                                    if i == 9:
                                        data_type = 'b'*data_len
                                    else:
                                        data_type = 'B'*data_len

                                    send_type, send_data = self.send_message_struct(data_struc_data, null_len,
                                                                                    data_type, data_list)

                                    test = struct.pack(send_type, *send_data)
                                    socket_send.sendto(test, eNB_ADDR)

                                    insert_message = '%s...！\n' % msg
                                    myGUI_grid.inert_message_queue.put(insert_message)
                                    self.send_times_eeprom[testID] += 1
                                    eepromContentValue = data_list + [0]*null_len
                                    eepromContent[i] = listTOstr(eepromContentValue)

                            sleep(0.1)
                            timeout_gap = 0
                            timeOut(testID, self.send_times_eeprom, emsServer.eepromRec, timeout_gap, 6)

            checkSum = int(listCRCsum(eepromContent), 16)
            data_list_crc = []
            data_list_crc.append(checkSum)
            test_id_crc = sqlite.test_id_eeprom_total[1]
            line_no = index + 1
            data_structure_crc = sqlite.query_single_NO(line_no, self.sql_e2prom)
            msg_crc = data_structure_crc[0][1]
            data_struc_data_crc = list(data_structure_crc[0][2:])
            data_len = data_structure_crc[0][-1]
            if data_len == 2:
                data_type_crc = 'H'
            elif data_len == 3:
                data_type_crc = 'BH'

            send_type_crc, send_data_crc = self.send_message_struct(data_struc_data_crc, 0, data_type_crc, data_list_crc)
            test_crc = struct.pack(send_type_crc, *send_data_crc)
            socket_send.sendto(test_crc, eNB_ADDR)

            myGUI_grid.GUI.msg.insert('end', '%s:%s！\n' % (msg_crc, hex(checkSum)))
            myGUI_grid.GUI.msg.see('end')

            socket_send.close()
            insert_message = 'eeprom数据写入完毕！\n'
            myGUI_grid.inert_message_queue.put(insert_message)
        
    def fix_test_eeprom_read(self, option, index):
            for test_id in sqlite.test_id_eeprom_total:
                    self.send_times_eeprom[test_id] = 0
                    emsServer.eepromRec[test_id] = [0, 0, 0]

            socket_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            for i in range(10):
                    if option[i]['state'] == 'normal':
                            testID = sqlite.test_id_eeprom_total[i+14]
                            line_no = i + 14 + index
                            data_structure = sqlite.query_single_NO(line_no, self.sql_e2prom)
                            msg = data_structure[0][1]
                            data_struc_data = list(data_structure[0][2:])
                            data_len = data_structure[0][-1]
                            data_type = data_len * 'B'

                            send_type, send_data = self.send_message_struct(data_struc_data, data_type=data_type)
                            test = struct.pack(send_type, *send_data)
                            socket_send.sendto(test, eNB_ADDR)

                            myGUI_grid.GUI.msg.insert('end', '%s...！\n' % msg)
                            myGUI_grid.GUI.msg.see('end')
                            self.send_times_eeprom[testID] += 1

                            sleep(0.1)
                            timeout_gap = 0
                            timeOut(testID, self.send_times_eeprom, emsServer.eepromRec, timeout_gap, 6)

            insert_message = 'eeprom读取完毕！\n'
            myGUI_grid.inert_message_queue.put(insert_message)
            socket_send.close()

    def testResultShow(self):
        testTimes_show = []
        tab = current_page()
        i = 0
        wrongInfo = ''
        for item in emsServer.recValue:
                if item in self.tab_dic[tab][0]:
                        index_item = self.tab_dic[tab][0].index(item)
                        checkButton = self.tab_dic[tab][1][index_item]

                        totaltimes = self.send_times_case[item]
                        # print(totaltimes)
                        testTimes_show.append(totaltimes)

                        succtimes = emsServer.recValue[item][0]

                        if totaltimes-succtimes > 0.1:
                                itemName = sqlite.query_value(item, self.sql_case)[0][0]
                                a='%s失败%d次、  '%(itemName, totaltimes-succtimes)
                                wrongInfo += a
                                checkButton['foreground'] = 'red'
                                i += 1
                        elif (totaltimes-succtimes<0.1) and totaltimes != 0:
                                checkButton['foreground'] = 'green'
        testTimesShow = max(testTimes_show)
        result = '-------------------------老化%d次，错误信息---------------------------\n' % testTimesShow
        result += wrongInfo

        if i == 0 and sum(self.send_times_case.values()) != 0:
                result = '测试%d次，测试通过！' % self.testTimes
                myGUI_grid.GUI.value_testResult.set(result)
        elif sum(self.send_times_case.values()) == 0:
                myGUI_grid.GUI.value_testResult.set('正在测试....')
        else:
                myGUI_grid.GUI.value_testResult.set(result)

    def setENB(self):
       eNB_IP = myGUI_grid.GUI.value_dstIP.get()
       eNB_PORT = 9000
       global eNB_ADDR
       eNB_ADDR = (eNB_IP, eNB_PORT)

       logger.info('The dst ADDR is %s@%d'%(eNB_IP,eNB_PORT))
       insert_message = 'The BTS ADDR is %s @ %d\n' % (eNB_IP, eNB_PORT)
       myGUI_grid.inert_message_queue.put(insert_message)
       myGUI_grid.GUI.heartbeat()

    def setTimes(self):
        self.testTimes = myGUI_grid.GUI.value_testTime.get()
        insert_message = '试验次数：%d\n' % (self.testTimes)
        myGUI_grid.inert_message_queue.put(insert_message)

    def updata(self, checkbutton, checkbutton_value, updataValue, testID, stop_flag):
        checkbutton['foreground'] = 'black'
        thread_control_flag(1, stop_flag)
        self.send_times_update[testID] = 0
        emsServer.rec_times_update[testID] = [0, 0, 0]
        if checkbutton_value.get() == 1:
                board_slot = self.board_slot_get()
                socket_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                send_type = self.send_headType + updataValue[1] + len(board_slot)*'B' + self.send_tailType
                send_data = self.send_headData + updataValue[2] + board_slot + self.send_tailData
                test = struct.pack(send_type, *send_data)
                socket_send.sendto(test, eNB_ADDR)
                insert_message = '%s 测试开始...\n' % updataValue[0]
                self.send_times_update[testID] += 1
                myGUI_grid.inert_message_queue.put(insert_message)
                if testID in [0x030E, 0x030F]:
                    test_sequence = '0123456789abcdef'.encode('ascii')
                    try:
                        COM.write(test_sequence)
                    except ValueError:
                        myGUI_grid.inert_message_queue.put('测试序列发送失败：串口未连接！\n')
                        checkbutton['foreground'] = 'red'
                        return

                timeOut(testID, self.send_times_update, emsServer.rec_times_update, 0, stop_flag)
                socket_send.close()
                if self.send_times_update[testID] == emsServer.rec_times_update[testID][0]:
                    checkbutton['foreground'] = 'green'
                else:
                    checkbutton['foreground'] = 'red'


    def eth3_config(self, config_value):
        socket_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        parameter_eth3 = myGUI_grid.GUI.entry_IT_2.get().split('.')
        ip_eth3 = []
        # MCT_slot = []
        for item in parameter_eth3:
            if item:
                ip_eth3.append(int(item))
        # if myGUI_grid.GUI.MCTslot_IT_combobox:
        #     slot = myGUI_grid.GUI.MCTslot_IT_combobox.get()
        #     MCT_slot.append(int(slot))

        send_type = self.send_headType + config_value[1] + 'B'*4 + self.send_tailType
        send_data = self.send_headData + config_value[2] + ip_eth3 + self.send_tailData
        # print(send_data)
        if len(ip_eth3) == 4:
            test = struct.pack(send_type,*send_data)
            socket_send.sendto(test, eNB_ADDR)
            insert_message = '%s ...\n' % config_value[0]
            myGUI_grid.inert_message_queue.put(insert_message)
            socket_send.close()
        else:
            myGUI_grid.inert_message_queue.put('IP地址输入错误!\n')

    def start_test_thread(self, function, args, name=''):
        thread_list = threading.enumerate()
        for item in thread_list:
            if name in str(item):
                return
        th_case_test = threading.Thread(target=function, args=args, name=name)
        th_case_test.setDaemon(True)
        th_case_test.start()

    def board_slot_get(self):
        return []


class PlatFixTestCzz(PlatFixTest):

    def __init__(self):
        PlatFixTest.__init__(self, sql_case='CZZ_case', sql_e2prom='CZZ_eeprom')
        self.tab_dic = {'it': (sqlite.test_id_it, myGUI_grid.GUI.cbName_IT),
                            'mct': (sqlite.test_id_mct, myGUI_grid.MCT.cbName),
                            'bbp': (sqlite.test_id_bbp, myGUI_grid.BBP.cbName),
                            'ges': (sqlite.test_id_ges, myGUI_grid.GES.cbName),
                            'fm': (sqlite.test_id_fm, myGUI_grid.FM.cbName)}

    def fix_test_case(self, test_item, test_id_list):
            num_list, id_list = self.test_case_setup(test_item, test_id_list)
            socket_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            j = 0
            while j < self.testTimes:
                    i = 0
                    while i < len(num_list):
                            num = num_list[i]
                            testID0 = id_list[i]
                            test_case_info = sqlite.query_single_TestID(testID0, self.sql_case)
                            test_case_title = test_case_info[0][1]
                            send_bodyData = list(test_case_info[0][2:])
                            send_type = self.send_headType + self.send_bodyType + self.send_tailType
                            send_data = self.send_headData + send_bodyData + self.send_tailData

                            if testID0 == 2:
                                    try:
                                            dic_workMode = {'自适应':0,'十兆':1,'百兆':2,'千兆':3}
                                            index = myGUI_grid.GES.null_4.get()
                                            workMode = dic_workMode[index]

                                            port = myGUI_grid.GES.null_7.get()
                                            port = int(port)
                                            send_type_workMode = self.send_headType + 'BBHIIIIBB' + self.send_tailType
                                            send_data_workMode = self.send_headData+[1, 1, 0x0002, 1, 0, 0, 2, port, workMode] \
                                                                 + self.send_tailData
                                            test_workMode = struct.pack(send_type_workMode, *send_data_workMode)
                                            socket_send.sendto(test_workMode, eNB_ADDR)
                                            self.send_times_case[testID0] += 1
##                                            logger.info('###### %s ######'%test_case_title)
                                            insert_message = '%s 测试开始...\n'%test_case_title
                                            myGUI_grid.inert_message_queue.put(insert_message)

                                    except:
                                            logger.info('测试交换板工作模式错误！')
                                            insert_message = '测试交换板工作模式错误！\n'
                                            myGUI_grid.inert_message_queue.put(insert_message)

                            elif testID0 == 257:
                                    try:
                                            dic_fanPort = {'ALL':3,'0':0,'1':1,'2':2}
                                            index = myGUI_grid.FM.null_7.get()
                                            fanPort = dic_fanPort[index]

                                            fanRate=myGUI_grid.FM.null_4.entry.get()
                                            fanRate=int(fanRate)

                                            send_type_fanRate = self.send_headType + 'BBHIIIIBB'+ self.send_tailType
                                            send_data_fanRate = self.send_headData + [1, 1, 0x0101, 1, 0, 0, 2, fanPort, fanRate] \
                                                                + self.send_tailData
                                            test_fanRate = struct.pack(send_type_fanRate, *send_data_fanRate)
                                            socket_send.sendto(test_fanRate, eNB_ADDR)
                                            self.send_times_case[testID0] += 1

##                                            logger.info('###### %s ######'%test_case_title)
                                            insert_message = '%s 测试开始...\n'%test_case_title
                                            myGUI_grid.inert_message_queue.put(insert_message)


                                    except:
                                            logger.info('测试风扇板风速错误！')
                                            insert_message = '测试风扇板风速错误！\n'
                                            myGUI_grid.inert_message_queue.put(insert_message)

                            else:
                                    test=struct.pack(send_type,*send_data)

                                    try:
                                            socket_send.sendto(test,eNB_ADDR)

                                            self.send_times_case[testID0] += 1
##                                            logger.info('###### %s ######'%test_case_title)
                                            insert_message = '%s 测试开始...\n' % test_case_title
                                            myGUI_grid.inert_message_queue.put(insert_message)

                                    except (NameError,OSError):
                                            logger.info('请设置正确的目的地址！')
                                            myGUI_grid.GUI.msg.insert('end','请设置正确的目的地址！\n','fail')
                                            myGUI_grid.GUI.msg.see('end')
                                            insert_message = '请设置正确的目的地址！\n'
                                            myGUI_grid.inert_message_queue.put(insert_message)

                            sleep(SUB_GAP*0.1)
                            i += 1

                    for testID in id_list:
                            timeout_gap = 0
                            timeOut(testID, self.send_times_case, emsServer.recValue, timeout_gap, stop_flag = 0)

                    j += 1
                    sleep(SUB_GAP*0.4)
            insert_message = '测试项测试完毕！\n'
            myGUI_grid.inert_message_queue.put(insert_message)
            socket_send.close()

    def private_config(self):
        myGUI_grid.GUI.button_IT_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.GUI.value, sqlite.test_id_it), 'IT_TEST')
        myGUI_grid.MCT.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.MCT.value, sqlite.test_id_mct), 'MCT_TEST')
        myGUI_grid.BBP.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.BBP.value, sqlite.test_id_bbp), 'BBP_TEST')
        myGUI_grid.GES.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.GES.value, sqlite.test_id_ges), 'GES_TEST')
        myGUI_grid.FM.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.FM.value, sqlite.test_id_fm), 'FM_TEST')
        ##firmware update
        myGUI_grid.MCT.null_3['command'] = lambda: self.updata(myGUI_grid.MCT.value_var, sqlite.update_value[2])
        myGUI_grid.BBP.null_3['command'] = lambda: self.updata(myGUI_grid.BBP.value_var, sqlite.update_value[1])
        myGUI_grid.BBP.null_4['command'] = lambda: self.updata(myGUI_grid.BBP.value_var1, sqlite.update_value[0])
        myGUI_grid.GES.mcu_update['command'] = lambda: self.updata(myGUI_grid.GES.value_var1, sqlite.update_value[3])

        #bind button for eeprom test
        myGUI_grid.MCT.button_2['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_write,
                                                    (myGUI_grid.MCT.option, myGUI_grid.MCT.entry,
                                                      1), 'E2PROM_W')

        myGUI_grid.MCT.button_1['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_read,
                                                    (myGUI_grid.MCT.option, 1), 'E2PROM_R')

        myGUI_grid.BBP.button_2['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_write,
                                                    (myGUI_grid.BBP.option, myGUI_grid.BBP.entry,
                                                     25), 'E2PROM_W')
        myGUI_grid.BBP.button_1['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_read,
                                                    (myGUI_grid.BBP.option, 25), 'E2PROM_R')

        myGUI_grid.GES.button_2['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_write,
                                                    (myGUI_grid.GES.option, myGUI_grid.GES.entry,
                                                     49), 'E2PROM_W')
        myGUI_grid.GES.button_1['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_read,
                                                    (myGUI_grid.GES.option, 49), 'E2PROM_R')


class PlatFixTestEBBU(PlatFixTest):

    def __init__(self):
        PlatFixTest.__init__(self, sql_case='eBBU_case', sql_e2prom='eBBU_eeprom')
        self.tab_dic = {'it': (sqlite.test_id_it, myGUI_grid.GUI.cbName_IT),
                        'mct': (sqlite.test_id_mct, myGUI_grid.MCT.cbName),
                        'bbp': (sqlite.test_id_bbp, myGUI_grid.BBP.cbName),
                        'em': (sqlite.test_id_em, myGUI_grid.EM.cbName),
                        'fm': (sqlite.test_id_fm, myGUI_grid.FM.cbName),
                        'fs': (sqlite.test_id_fs, myGUI_grid.FS.cbName),
                        'es': (sqlite.test_id_es, myGUI_grid.ES.cbName)
                        }

    def fix_test_case(self, test_item, test_id_list, stop_flag):
        thread_control_flag(1, stop_flag)
        num_list, id_list = self.test_case_setup(test_item, test_id_list)
        socket_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        slot = self.board_slot_get()

        j = 0
        while j < self.testTimes:
            i = 0
            while i < len(num_list):
                if thread_control_flag(2, stop_flag) == 1:
                    break
                num = num_list[i]
                testID0 = id_list[i]
                test_case_info = sqlite.query_single_TestID(testID0, self.sql_case)
                test_case_title = test_case_info[0][1]
                send_bodyData = list(test_case_info[0][2:])

                if testID0 == 257:
                    try:
                        dic_fanPort = {'ALL': 3, '0': 0, '1': 1, '2': 2}
                        index = myGUI_grid.FM.null_7.get()
                        fanPort = dic_fanPort[index]

                        fanRate = myGUI_grid.FM.null_4.entry.get()
                        fanRate = int(fanRate)
                        fan_value = [fanPort, fanRate]

                        send_type = self.send_headType + self.send_bodyType + 'B'*len(slot) + 'BB' + self.send_tailType
                        send_data = self.send_headData + send_bodyData + slot + fan_value + self.send_tailData

                    except:
                        logger.info('测试风扇板风速错误！')
                        insert_message = '测试风扇板风速错误！\n'
                        myGUI_grid.inert_message_queue.put(insert_message)

                elif testID0 == 258:
                    try:
                        dic_fanPort = {'ALL': 3, '0': 0, '1': 1, '2': 2}
                        index = myGUI_grid.FM.null_7.get()
                        fanPort = dic_fanPort[index]
                        fanPort = [fanPort, ]
                        send_type = self.send_headType + self.send_bodyType + 'B'*len(slot) + 'B'+ self.send_tailType
                        send_data = self.send_headData + send_bodyData + slot + fanPort + self.send_tailData

                    except:
                        logger.info('测试风扇板风速错误！')
                        insert_message = '测试风扇板风速错误！\n'
                        myGUI_grid.inert_message_queue.put(insert_message)

                else:
                    send_type = self.send_headType + self.send_bodyType + 'B'*len(slot) + self.send_tailType
                    send_data = self.send_headData + send_bodyData + slot + self.send_tailData


                test = struct.pack(send_type, *send_data)
                # print(test)

                try:
##                    logger.info('###### %s ######' % test_case_title)
                    insert_message = '%s 测试开始...\n' % test_case_title
                    myGUI_grid.inert_message_queue.put(insert_message)
                    socket_send.sendto(test, eNB_ADDR)
                    self.send_times_case[testID0] += 1
                    

                except (NameError, OSError):
                    logger.info('请设置正确的目的地址！')
                    insert_message = '请设置正确的目的地址！\n'
                    myGUI_grid.inert_message_queue.put(insert_message)

                sleep(SUB_GAP * 0.1)
                i += 1

            for testID in test_id_list:
                timeout_gap = 0
                timeOut(testID, self.send_times_case, emsServer.recValue, timeout_gap, stop_flag)
            j += 1
            sleep(SUB_GAP * 0.4)

        insert_message = '测试项测试完毕！\n'
        myGUI_grid.inert_message_queue.put(insert_message)
        socket_send.close()

    def private_config(self):
        myGUI_grid.GUI.button_IT_3['command'] = lambda: self.start_test_thread(self.fix_test_case_it,
                                                        (myGUI_grid.GUI.value, sqlite.test_id_it, 1), 'IT_TEST')
        myGUI_grid.MCT.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.MCT.value, sqlite.test_id_mct, 2), 'MCT_TEST')
        myGUI_grid.BBP.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.BBP.value, sqlite.test_id_bbp, 3), 'BBP_TEST')
        myGUI_grid.EM.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.EM.value, sqlite.test_id_em, 5), 'EM_TEST')
        myGUI_grid.FM.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.FM.value, sqlite.test_id_fm, 4), 'FM_TEST')
        myGUI_grid.FS.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.FS.value, sqlite.test_id_fs, 6), 'FS_TEST')
        myGUI_grid.ES.button_3['command'] = lambda: self.start_test_thread(self.fix_test_case,
                                                        (myGUI_grid.ES.value, sqlite.test_id_es, 7), 'ES_TEST')


        myGUI_grid.GUI.button_IT_stop['command'] = lambda: thread_control_flag(0, 1)
        myGUI_grid.MCT.button_stop['command'] = lambda: thread_control_flag(0, 2)
        myGUI_grid.BBP.button_stop['command'] = lambda: thread_control_flag(0, 3)
        myGUI_grid.EM.button_stop['command'] = lambda: thread_control_flag(0, 5)
        myGUI_grid.FM.button_stop['command'] = lambda: thread_control_flag(0, 4)
        myGUI_grid.FS.button_stop['command'] = lambda: thread_control_flag(0, 6)
        myGUI_grid.ES.button_stop['command'] = lambda: thread_control_flag(0, 7)

        ##firmware update
        myGUI_grid.MCT.null_3['command'] = lambda: self.start_test_thread(self.updata,
                                                    (myGUI_grid.MCT.null_3, myGUI_grid.MCT.value_var, sqlite.update_value[2], 0x0300, 2),
                                                    'mctcpldupdate')

        myGUI_grid.MCT.syn_485['command'] = lambda: self.start_test_thread(self.updata,
                                                    (myGUI_grid.MCT.syn_485, myGUI_grid.MCT.value_reset, sqlite.update_value[11], 0x030E, 2),
                                                    'mct485sync')

        myGUI_grid.MCT.syn_tod['command'] = lambda: self.start_test_thread(self.updata,
                                                    (myGUI_grid.MCT.syn_tod, myGUI_grid.MCT.value_reset1, sqlite.update_value[12], 0x030F, 2),
                                                    'mct485tod')

        myGUI_grid.BBP.null_3['command'] = lambda: self.start_test_thread(self.updata,
                                                     (myGUI_grid.BBP.null_3, myGUI_grid.BBP.value_var, sqlite.update_value[1], 0x0204, 3),
                                                     'bbpcpldupdate')

        myGUI_grid.BBP.null_4['command'] = lambda: self.start_test_thread(self.updata,
                                                        (myGUI_grid.BBP.null_4, myGUI_grid.BBP.value_var1, sqlite.update_value[0], 0x0203, 3),
                                                        'bbpmcuupdate')

        myGUI_grid.EM.update_mcu_check['command'] = lambda: self.start_test_thread(self.updata,
                                                                (myGUI_grid.EM.update_mcu_check, myGUI_grid.EM.value_var, sqlite.update_value[3], 0x0505, 5),
                                                                'emmcuupdate')

        myGUI_grid.FM.update_mcu_check['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.FM.update_mcu_check, myGUI_grid.FM.value_var1, sqlite.update_value[4], 0x0103, 4),
                                                            'fmmcuupdate')

        myGUI_grid.BBP.reset_checkbutton['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.BBP.reset_checkbutton, myGUI_grid.BBP.value_reset, sqlite.update_value[5], 0x0200, 3),
                                                            'bbpreset')

        myGUI_grid.BBP.reset_dsp_checkbutton['command'] = lambda: self.start_test_thread(self.updata,
                                                                 (myGUI_grid.BBP.reset_dsp_checkbutton, myGUI_grid.BBP.value_reset1, sqlite.update_value[6], 0x0202, 3),
                                                                 'dspreset')

        myGUI_grid.BBP.reset_fpga_checkbutton['command'] = lambda: self.start_test_thread(self.updata,
                                                                    (myGUI_grid.BBP.reset_fpga_checkbutton, myGUI_grid.BBP.value_reset2, sqlite.update_value[7], 0x0201, 3),
                                                                    'fpgareset')
        myGUI_grid.EM.reset_checkbutton['command'] = lambda: self.start_test_thread(self.updata,
                                                             (myGUI_grid.EM.reset_checkbutton, myGUI_grid.EM.value_reset, sqlite.update_value[8], 0x0500, 5),
                                                             'emreset')

        myGUI_grid.EM.reset_power_checkbutton['command'] = lambda: self.start_test_thread(self.updata,
                                                                    (myGUI_grid.EM.reset_power_checkbutton, myGUI_grid.EM.value_reset1, sqlite.update_value[9], 0x0501, 5),
                                                                    'empowerreset')

        myGUI_grid.FM.reset_checkbutton['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.FM.reset_checkbutton, myGUI_grid.FM.value_reset, sqlite.update_value[10], 0x0100, 4),
                                                            'fmreset')
        myGUI_grid.FS.null_3['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.FS.null_3, myGUI_grid.FS.value_var, sqlite.update_value[13], 0x0603, 6),
                                                            'fs160load')
        myGUI_grid.FS.null_4['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.FS.null_4, myGUI_grid.FS.value_var1, sqlite.update_value[14], 0x0604, 6),
                                                            'fs325load')
        myGUI_grid.FS.update_mcu_check['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.FS.update_mcu_check, myGUI_grid.FS.value_reset, sqlite.update_value[15], 0x0606, 6),
                                                            'fsarmupdate')
        myGUI_grid.FS.reset_check['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.FS.reset_check, myGUI_grid.FS.value_reset1, sqlite.update_value[16], 0x060C, 6),
                                                            'fsreset')
        myGUI_grid.ES.null_3['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.ES.null_3, myGUI_grid.ES.value_var, sqlite.update_value[17], 0x0702, 7),
                                                            'esfpgaload')
        myGUI_grid.ES.null_4['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.ES.null_4, myGUI_grid.ES.value_var1, sqlite.update_value[18], 0x0704, 7),
                                                            'esarmupdate')
        myGUI_grid.ES.reset_check['command'] = lambda: self.start_test_thread(self.updata,
                                                            (myGUI_grid.ES.reset_check, myGUI_grid.ES.value_reset, sqlite.update_value[19], 0x0709, 7),
                                                            'esreset')

        #bind button for eeprom test
        myGUI_grid.MCT.button_2['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_write,
                                                    (myGUI_grid.MCT.option, myGUI_grid.MCT.entry,
                                                      1), 'E2PROM_W')

        myGUI_grid.MCT.button_1['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_read,
                                                    (myGUI_grid.MCT.option, 1), 'E2PROM_R')

        myGUI_grid.BBP.button_2['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_write,
                                                    (myGUI_grid.BBP.option, myGUI_grid.BBP.entry,
                                                     25), 'E2PROM_W')
        myGUI_grid.BBP.button_1['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_read,
                                                    (myGUI_grid.BBP.option, 25), 'E2PROM_R')

        myGUI_grid.FM.button_2['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_write,
                                                    (myGUI_grid.FM.option, myGUI_grid.FM.entry, 49), 'E2PROM_W')
        myGUI_grid.FM.button_1['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_read,
                                                    (myGUI_grid.FM.option, 49), 'E2PROM_R')
        myGUI_grid.FS.button_2['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_write,
                                                    (myGUI_grid.FS.option, myGUI_grid.FS.entry, 73), 'E2PROM_W')
        myGUI_grid.FS.button_1['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_read,
                                                    (myGUI_grid.FS.option, 73), 'E2PROM_R')
        myGUI_grid.ES.button_2['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_write,
                                                    (myGUI_grid.ES.option, myGUI_grid.ES.entry, 97), 'E2PROM_W')
        myGUI_grid.ES.button_1['command'] = lambda: self.start_test_thread(self.fix_test_eeprom_read,
                                                    (myGUI_grid.ES.option, 97), 'E2PROM_R')

    def board_slot_get(self):
        tab_select = myGUI_grid.GUI.notebook.select()
        tab = current_page()
        if tab == 'mct':
            slot = myGUI_grid.MCT.null_6.get()
            slot_value = [int(slot), ]
        elif tab == 'bbp':
            slot = myGUI_grid.BBP.null_6.get()
            slot_value = [int(slot), ]
        elif tab == 'em':
            slot = myGUI_grid.EM.null_6.get()
            slot_value = [int(slot), ]
        elif tab == 'fs':
            slot = myGUI_grid.FS.null_6.get()
            slot_value = [int(slot), ]
        elif tab == 'es':
            slot = myGUI_grid.ES.null_6.get()
            slot_value = [int(slot), ]
        elif tab == 'it':
            slot_mct_value = myGUI_grid.GUI.MCTslot_IT_combobox.get()
            slot_value = [int(slot_mct_value), ]
            for item in myGUI_grid.GUI.sub_slot_id_value:
                if item.get() == 1:
                    index = myGUI_grid.GUI.sub_slot_id_value.index(item)
                    slotID = myGUI_grid.GUI.sub_slot_id[index]
                    slot_value.append(slotID)
            # print(slot_value)
        elif tab == 'fm':
            slot_value = [10, ]
        else:
            slot_value = []

        return slot_value

    def send_message_struct(self, data_struc_data, null_len=0, data_type='', data_list=[]):
        slot = self.board_slot_get()
        if len(slot) != 0:
            send_data = self.send_headData + data_struc_data + slot + data_list + [0]*(null_len-1) + self.send_tailData
        else:
            send_data = self.send_headData + data_struc_data + data_list + [0]*null_len + self.send_tailData

        send_type = self.send_headType + self.send_bodyType + data_type + self.send_tailType
        return send_type, send_data

    def fix_test_case_it(self, test_item, test_id_list, stop_flag):
        thread_control_flag(1, stop_flag)
        num_list, id_list = self.test_case_setup(test_item, test_id_list)
        socket_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        slot = self.board_slot_get()
        slot_mct = slot[:1]
        slot_bbp = []
        slot_peu = []
        for item in slot[1:]:
            if item in [2, 3, 4, 5, 6, 7]:
                slot_bbp.append(item)
            elif item in [8, 9]:
                slot_peu.append(item)

        j = 0
        while j < self.testTimes:
            i = 0
            while i < len(num_list):
                if thread_control_flag(2, stop_flag) == 1:
                    break
                num = num_list[i]
                testID0 = id_list[i]
                test_case_info = sqlite.query_single_TestID(testID0, self.sql_case)
                test_case_title = test_case_info[0][1]
                send_bodyData = list(test_case_info[0][2:])

                if testID0 == 1026:
                    slot = slot_mct + [10, ]
                    send_type = self.send_headType + self.send_bodyType + 'B'*len(slot) + self.send_tailType
                    send_data = self.send_headData + send_bodyData + slot + self.send_tailData
                    test = struct.pack(send_type, *send_data)
                    self.socket_send_message(socket_send, test, testID0, test_case_title)

                elif testID0 in [1028, 1035]:
                    slot = slot_mct
                    send_type = self.send_headType + self.send_bodyType + 'B'*len(slot) + self.send_tailType
                    send_data = self.send_headData + send_bodyData + slot + self.send_tailData
                    test = struct.pack(send_type, *send_data)
                    self.socket_send_message(socket_send, test, testID0, test_case_title)

                elif testID0 in [1025, 54286, 54287]:
                    if len(slot_bbp) == 0:
                        continue
                    for item in slot_bbp:
                        slot = slot_mct + [item, ]
                        # print(slot)
                        send_type = self.send_headType + self.send_bodyType + 'B'*len(slot) + self.send_tailType
                        send_data = self.send_headData + send_bodyData + slot + self.send_tailData
                        test = struct.pack(send_type, *send_data)
                        self.socket_send_message(socket_send, test, testID0, test_case_title)

                elif testID0 == 1027:
                    if len(slot_peu) == 0:
                        continue
                    for item in slot_peu:
                        slot = slot_mct + [item, ]
                        # print(slot)
                        send_type = self.send_headType + self.send_bodyType + 'B'*len(slot) + self.send_tailType
                        send_data = self.send_headData + send_bodyData + slot + self.send_tailData
                        test = struct.pack(send_type, *send_data)
                        self.socket_send_message(socket_send, test, testID0, test_case_title)

                elif testID0 == 54275:
                    slot = slot_mct
                    srio_send_slot = myGUI_grid.GUI.combobox_srio_send.get()
                    srio_receive_slot = myGUI_grid.GUI.combobox_srio_receive.get()
                    srio_bbp = [int(srio_send_slot), int(srio_receive_slot)]

                    send_type = self.send_headType + self.send_bodyType + 'B'*len(slot) + 'BB'+ self.send_tailType
                    send_data = self.send_headData + send_bodyData + slot + srio_bbp + self.send_tailData
                    test = struct.pack(send_type, *send_data)
                    self.socket_send_message(socket_send, test, testID0, test_case_title)


                else:
                    logger.info('not define it test case !!\n')

                i += 1

            for testID in test_id_list:
                timeout_gap = 0
                timeOut(testID, self.send_times_case, emsServer.recValue, timeout_gap, stop_flag)
            j += 1
            sleep(SUB_GAP * 0.4)

        insert_message = '测试项测试完毕！\n'
        myGUI_grid.inert_message_queue.put(insert_message)
        socket_send.close()

    def socket_send_message(self, socket_send, test, testID0, test_case_title):
            try:
                socket_send.sendto(test, eNB_ADDR)

                self.send_times_case[testID0] += 1
##                logger.info('###### %s ######' % test_case_title)
                insert_message = '%s 测试开始...\n' % test_case_title
                myGUI_grid.inert_message_queue.put(insert_message)

            except (NameError, OSError):
                logger.info('请设置正确的目的地址！')
                insert_message = '请设置正确的目的地址！\n'
                myGUI_grid.inert_message_queue.put(insert_message)
            sleep(SUB_GAP * 0.1)
            # sleep(2)


def timeOut(testID, dic_send, dic_rec, timeout_gap, stop_flag, timeout=TIMEOUT):
        if dic_send[testID] == sum(dic_rec[testID]):
                timeout_gap = 0
                return
        elif dic_send[testID] > sum(dic_rec[testID]):
                if emsServer.error_flag == 1:
                    dic_rec[testID][1] += 1
                    emsServer.error_flag = 0

                while timeout_gap<timeout:
                        if thread_control_flag(2, stop_flag) == 1:
                            tab = current_page()
                            myGUI_grid.inert_message_queue.put('%s测试项测试停止！\n'% tab.upper() )
                            return
                        succ = dic_rec[testID][0]
                        fail = dic_rec[testID][1]

                        sleep(5)

                        if succ < dic_rec[testID][0] or fail < dic_rec[testID][1]:

                                break
                        timeout_gap += 5
##                        logger.info(timeout_gap)
                        insert_message = '测试中...\n'
                        myGUI_grid.inert_message_queue.put(insert_message)
                        # print('send:', dic_send, '\nreceive:', dic_rec)
                        break
                if timeout_gap >= timeout:
                       emsServer.test_times_statistics_lock.acquire()
                       dic_rec[testID][2] += 1
                       emsServer.test_times_statistics_lock.release()
                       timeout_gap = 0

        elif dic_send[testID]<sum(dic_rec[testID]):
                logger.info('#####发送小于接收#####')
                # print(testID, dic_send[testID], dic_rec[testID])
                timeout_gap=0
                return

        timeOut(testID, dic_send,dic_rec, timeout_gap, stop_flag, timeout=TIMEOUT)


def listTOstr(dataList):
        a = '0x'
        for item in dataList:
            item_hex = hex(item)
            item_str = str(item_hex)
            item_hex_split = item_str.split('x')[-1]
            if len(item_hex_split) == 1:
                item_hex_split = '0' + item_hex_split

            a += item_hex_split
        return a


def listCRCsum(listdata):
    content = ''
    checkSum = 0x0000
    for item in listdata:
        item = item.split('x')[-1]
        content += item

    while content:
        checkSum ^= int(content[:4],16)
        content = content[4:]
    return hex(checkSum)


def insert_message():
    try:
        message = myGUI_grid.inert_message_queue.get(block=False)
    except queue.Empty:
        pass
    else:
        if ('失败' in message) or ('失步' in message):
            myGUI_grid.GUI.msg.insert('end', message, 'fail')
            myGUI_grid.GUI.msg.see('end')
        else:
            myGUI_grid.GUI.msg.insert('end', message)
            myGUI_grid.GUI.msg.see('end')
    myGUI_grid.GUI.top.after(300, insert_message)


def start_emsUdpServer_thread():
        global emsUdpServer
        emsUdpServer = emsServer.emsUdpServer(LOCAL_ADDR, emsServer.messageHandler)
        th_emsUdpServer = threading.Thread(target=emsUdpServer.serve_forever, daemon=1)
        th_emsUdpServer.start()


def start_socket_handle_thread():
        th_socket_message_handle = threading.Thread(target = emsServer.socket_message_handle)
        th_socket_message_handle.start()



def main():
        if platform_type == 'CZZ':
            PlatFixTestCzz()
        elif platform_type == 'eBBU':
            PlatFixTestEBBU()
        start_emsUdpServer_thread()
        start_socket_handle_thread()
        insert_message()
        mainloop()
        os._exit(0)
        

if __name__ == "__main__":
        main()
