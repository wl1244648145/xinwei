
# -*- coding: utf-8 -*-
# 20170711 for RRU Interface test

import socket
import struct
from time import sleep
import threading
from configparser import ConfigParser
from ctypes import *
import telnetlib

sem_heartbeat = threading.Event()

config_para = ConfigParser()
config_para.read('para.ini')

platform_type = config_para.get('eNB', 'TYPE')
if platform_type == 'CZZ':
    import myGUI_grid_CZZ as myGUI_grid
elif platform_type == 'eBBU':
    import myGUI_grid_eBBU as myGUI_grid
elif platform_type == 'RRU':
    import myGUI_grid_rru as myGUI_grid
else:
    print('测试平台类型是：%s' % platform_type)

send_headType = '>I'
send_headData = [0xaabbbeef, ]
# add ebbu slot id, default 2
send_bodyType = 'BBHIIIIB'
send_bodyData = [0x2, 0xa, 0, 1, 0, 0, 2, 2]
send_tailType = 'I'
send_tailData = [0xaa5555aa, ]
body_data_lock = threading.Lock()

protocol_dic = {0xA001: ('版本查询', ''),
                0xA002: ('版本下载', ''),
                0xA003: ('版本激活', ''),
                0xA004: ('天线状态配置', 'BB'),
                0xA005: ('射频状态查询', 'B'),
                0xA006: ('RRU运行状态查询', ''),
                0xA007: ('光口状态查询', 'B'),
                0xA008: ('系统时间配置', ''),
                0xA009: ('告警查询', ''),
                0xA00A: ('参数查询', ''),
                0xA00B: ('校准', 'HI'),
                0xA00C: ('小区配置', 'BIHBBBBBIIBIIBBH'),
                0xA00D: ('时延配置', ''),
                0xA00E: ('RRU复位', 'B'),
                0xA00F: ('telnet串口关闭', ''),
                0xA010: ('硬件参数查询', ''),
                0xAAAA: ('', 'B'),
                }


def calc_byte_size(fmt):
    size = 0
    for i in fmt:
        if i == 'B':
            size += 1
        elif i == 'H':
            size += 2
        elif i == 'I':
            size += 4
        else:
            size += 0
    return size


def protocol_test(cmd, value_list=[], extra=[]):

    if cmd == 0xA00C:
        tn = serial_rru()
        tn.write('txrx 1,1'.encode('ascii') + b"\n")
        tn.write('sig 0,7'.encode('ascii') + b"\n")
        sleep(0.2)
        tn.write('fpgaw 1,3,0x9a69'.encode('ascii') + b"\n")
        tn.close()
        print('xiaoquqqq')

    socket_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    parameter = []
    if value_list:
        parameter_entry = value_list
        for value in parameter_entry:
            para = value.get()
            if (para == '软复位') or (para == '下行'):
                para = 0
            elif (para == '掉电复位') or (para == '上行'):
                para = 1
            elif para == '上下行':
                para = 2
            elif isinstance(para, type('a')):
                para = int(para, 16)
            parameter.append(para)
    else:
        parameter = extra
    body_data_lock.acquire()
    send_bodyData[2] = cmd
    send_bodyData[-2] = calc_byte_size(protocol_dic[cmd][1])+1
    print('len=%d' % send_bodyData[-2])
    body_data_lock.release()

    send_type = send_headType + send_bodyType + protocol_dic[cmd][1] + send_tailType
    send_data = send_headData + send_bodyData + parameter + send_tailData
    print(send_data)
    test = struct.pack(send_type, *send_data)
    try:
        socket_send.sendto(test, eNB_ADDR)
        if protocol_dic[cmd][0]:
            message = '%s测试开始...\n' % protocol_dic[cmd][0]
            print(message)
            myGUI_grid.inert_message_queue.put(message)

    except NameError:
        message = '消息发送失败,请先设置目的地址...\n'
        myGUI_grid.inert_message_queue.put(message)
    socket_send.close()


def address_set():
    eNB_IP = myGUI_grid.GUI.value_dstIP.get()
    eNB_PORT = 9000
    global eNB_ADDR
    eNB_ADDR = (eNB_IP, eNB_PORT)
    message = 'The BTS ADDR is %s @ %d\n' % (eNB_IP, eNB_PORT)
    myGUI_grid.inert_message_queue.put(message)
    myGUI_grid.GUI.heartbeat()


def heartbeat_to_rru():
    address_set()
    i = c_ubyte()
    i.value = 1
    while 1:
        extra = [i.value, ]
        protocol_test(0xAAAA, extra=extra)
        if sem_heartbeat.wait(30):
            i.value += 1
        else:
            message = 'BBU未连接[%d]\n' % i.value
            myGUI_grid.inert_message_queue.put(message)
        sleep(20)
        sem_heartbeat.clear()


def serial_rru():
    bbu_ip = myGUI_grid.GUI.value_dstIP.get().strip()
    rru_id = myGUI_grid.GUI.value_eeprom[0].get()
    rru_ip = '10.0.0.%s' % rru_id
    try:
        tn = telnetlib.Telnet(bbu_ip, '23', timeout=30)
    except TimeoutError:
        myGUI_grid.inert_message_queue.put('BBU Telnet连接失败!\n')
        return
    # tn.debuglevel = 1
    tn.read_until(b"login: ")
    tn.write('root'.encode('ascii') + b"\n")

    tn.read_until(b"Password: ")
    tn.write('12345678'.encode('ascii') + b"\n")
    sleep(0.2)
    tn.write(('telnet %s 9999\n' % rru_ip).encode())
    tn.read_until(b"Login")
    tn.write('rru'.encode('ascii') + b"\n")
    tn.read_until(b"Passwd")
    tn.write('12345678'.encode('ascii') + b"\n")
    tn.read_until(b"->")
    tn.write('cmdOpen'.encode('ascii') + b"\n")
    sleep(0.2)
    return tn


def list_to_string(data_list):
        a = '0x'
        for item in data_list:
            item_hex = hex(item)
            item_str = str(item_hex)
            item_hex_split = item_str.split('x')[-1]
            if len(item_hex_split) == 1:
                item_hex_split = '0' + item_hex_split

            a += item_hex_split
        return a


def list_crc_sum(data_list):
    content = ''
    check_sum = 0x0000
    for item in data_list:
        item = item.split('x')[-1]
        content += item

    while content:
        check_sum ^= int(content[:4], 16)
        content = content[4:]
    return hex(check_sum)


def test():
    pass

if __name__ == "__main__":
    test()

