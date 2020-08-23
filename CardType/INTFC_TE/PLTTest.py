
# -*- coding: utf-8 -*-
# 20170711 for RRU Interface test

import logging
import threading
from configparser import ConfigParser
from tkinter import mainloop
import queue
import os

import emsServer
import protocal

config_para = ConfigParser()
config_para.read('para.ini')

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

platform_type = config_para.get('eNB', 'TYPE')
if platform_type == 'CZZ':
    import myGUI_grid_CZZ as myGUI_grid
elif platform_type == 'eBBU':
    import myGUI_grid_eBBU as myGUI_grid
elif platform_type == 'RRU':
    import myGUI_grid_rru as myGUI_grid
else:
    print('测试平台类型是：%s' % platform_type)

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


def start_heartbeat_thread():
        thread_list = threading.enumerate()
        for item in thread_list:
            if 'heartbeat' in str(item):
                return
        th_heartbeat = threading.Thread(target=protocal.heartbeat_to_rru, daemon=1, name='heartbeat')
        th_heartbeat.start()


def start_server_thread():
        server = emsServer.emsUdpServer(LOCAL_ADDR, emsServer.messageHandler)
        th_server = threading.Thread(target=server.serve_forever, daemon=1)
        th_server.start()


def start_socket_handle_therad():
        th_socket_message_handle = threading.Thread(target=emsServer.socket_message_handle)
        th_socket_message_handle.start()


def button_bind():
    myGUI_grid.GUI.button_IT_1['command'] = start_heartbeat_thread


def insert_message():
    try:
        message = myGUI_grid.inert_message_queue.get(block=False)
    except queue.Empty:
        pass
    else:
        if ('失败' in message) or ('失步' in message) or ('未连接' in message) or ('ALARM' in message):
            myGUI_grid.GUI.msg.insert('end', message, 'fail')
            myGUI_grid.GUI.msg.see('end')
        elif ('heartbeat' in message) or ('心跳' in message):
            myGUI_grid.GUI.msg.insert('end', message, 'INT')
            myGUI_grid.GUI.msg.see('end')
        else:
            myGUI_grid.GUI.msg.insert('end', message)
            myGUI_grid.GUI.msg.see('end')
    myGUI_grid.GUI.top.after(100, insert_message)


def main():
    button_bind()
    start_server_thread()
    start_socket_handle_therad()
    insert_message()
    mainloop()
    os._exit(0)

if __name__ == "__main__":
    main()
