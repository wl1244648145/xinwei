
# coding=utf-8

__author__ = 'x'

from tkinter import *
from functools import partial
from tkinter.scrolledtext import *
import time
from socket import create_connection
from configparser import ConfigParser
from tkinter import tix
from tkinter import ttk
from tkinter import font
import queue

##import protocal

config_para = ConfigParser()
config_para.read('para.ini')
inert_message_queue = queue.Queue()
MCT_IP = config_para.get('eNB', 'MCT')

myLabel = partial(Label, width=10, height=2)


class MsgGui:

    eeprom = ('RRU ID：', '通道建立原因：', 'RRU复位原因：', '软件包号：', 'rruapp版本号：', 'FPGA版本号：', '主板卡版本号：', '从板卡版本号：',
              '主功放版本号：', '从功放版本号：', '主板卡序列号：', '从板卡序列号：', '主功放序列号：', '从功放序列号：',
              '最大发射功率：', '最大接收功率：', '上行定标值：', '下行定标值：', 'RRU频段：', '天线收发状态：', 'RRU产品类型：')

    protocol = ('小区建立', '版本管理', '射频状态', '运行状态', '光模块', '功率校准', '告警查询', '硬件参数', 'RRU复位', '时延配置',
                '串口关闭', '参数查询', '时间配置', '掩码配置')

    def __init__(self):

        self.top = tix.Tk(className='接口测试_RRU')
        self.top.geometry('=640x670+450+100')
        self.top.maxsize(640, 670)
        self.top.iconbitmap(default='myicon.ico')
        f = font.nametofont('TkDefaultFont')
        f.configure(family='宋体')

        '''
        variables in self
        '''
        self.value_dstIP = StringVar()
        self.value_dstIP.set(MCT_IP)
        self.value_eth3 = StringVar()
        self.value_date = StringVar()
        self.value_heartBeat = StringVar()
        self.value_heartBeat.set('板卡未连接')
        self.value_eeprom = []

        self.value_cell_para = [0]*16
        self.value_cell_response = [StringVar(), StringVar()]

        self.value_version_test = StringVar()
        self.value_version_select = IntVar()
        self.value_version_select.set(1)

        self.value_rfstatus_channel = [IntVar(value=8), ]
        self.value_rfstatus_result = StringVar()

        self.value_rrustatus_response = StringVar()
        self.value_rrustatus_para = [0]*9

        self.value_fiberstatus_channel = [IntVar(value=2), ]
        self.value_fiberstatus_response = StringVar()

        self.value_calibration_para = [StringVar(value='上下行'), IntVar(value=415000)]
        self.value_calibration_result = [0]*4

        self.value_alarm_response = StringVar()

        self.value_hardware_result = [0]*9

        self.value_reboot_type = [StringVar(value='软复位'), ]
        self.value_reboot_response = StringVar()

        self.value_delay_test = StringVar()
        self.value_delay_response = StringVar()

        self.value_parameter_response = StringVar()
        self.value_parameter_result = [0]*5

        self.value_time_config_response = StringVar()
        self.value_time_config_result = StringVar()

        self.value_mask_config = [StringVar(value='0xf'), StringVar(value='0xf')]
        self.value_mask_config_result = StringVar()
        self.value_mask_config_response = StringVar()
        '''
        Sub widget declare
        '''
        self.sub_cell = ''
        self.sub_version = ''
        self.sub_rfstatus = ''
        self.sub_rrustatus = ''
        self.sub_fiberstatus = ''
        self.sub_calibration = ''
        self.sub_alarm = ''
        self.sub_hardware = ''
        self.sub_reboot = ''
        self.sub_delay = ''
        self.sub_parameter = ''
        self.sub_time_config = ''
        self.sub_mask_config = ''

        j = 0
        while j < 21:
            a = StringVar()
            self.value_eeprom.append(a)
            j += 1

        '''
          main UI
        '''

        self.frame_IT_1 = LabelFrame(self.top, rel=GROOVE, borderwidth=2, text='配置信息', foreground='blue')
        self.frame_IT_1.grid(pady=10, padx=10, sticky=W)

        self.label_IT_1 = myLabel(self.frame_IT_1, text='基站IP：')
        self.label_IT_1.grid(column=0, row=0, sticky=W)
        self.button_IT_1 = ttk.Button(self.frame_IT_1, text='连接', width=5)
        self.button_IT_1.grid(column=2, row=0, padx=5)
        self.entry_IT_1 = Entry(self.frame_IT_1, width=22, textvariable=self.value_dstIP, font=('Arial', 9))
        self.entry_IT_1.grid(column=1, row=0, pady=5)

        self.null_IT_1 = Label(self.frame_IT_1)
        self.null_IT_1.grid(column=3, row=0, padx=32)

        self.label_IT_2 = myLabel(self.frame_IT_1, text='仪器IP：')
        self.label_IT_2.grid(column=4, row=0, sticky=W)
        self.button_IT_2 = ttk.Button(self.frame_IT_1, text='连接', width=5)
        self.button_IT_2.grid(column=6, row=0, padx=5)
        self.entry_IT_2 = Entry(self.frame_IT_1, width=22, font=('Arial', 9))
        self.entry_IT_2.grid(column=5, row=0, pady=5)

        self.frame_IT_2 = LabelFrame(self.top, rel=GROOVE, borderwidth=2, text='设备信息', foreground='blue')
        self.frame_IT_2.grid(pady=1, padx=10, ipadx=0)

        self.canvas = Canvas(self.frame_IT_2, selectbackground='green', insertbackground='blue',
                             highlightcolor='#ECE9D8', state='disabled', insertborderwidth=200)
        self.canvas.grid(padx=4)
        self.canvas.create_line(0, 58, 650, 58, 650, 145, 0, 145, fill='#C4C4C4', width=1)
        setup_msg_count = self.testList(7, 3)
        i = 0
        while i < 21:
            if setup_msg_count[i][1] == 0:
                label_column = 0
                entry_column = 1
            elif setup_msg_count[i][1] == 1:
                label_column = 2
                entry_column = 3
            elif setup_msg_count[i][1] == 2:
                label_column = 4
                entry_column = 5

            label = Label(self.canvas, text=self.eeprom[i], anchor='center', width=14)
            label.grid(pady=5, column=label_column, row=setup_msg_count[i][0], padx=2)

            entry = Entry(self.canvas, width=15, textvariable=self.value_eeprom[i], font=('Arial', 9))
            entry.grid(pady=5, column=entry_column, row=setup_msg_count[i][0])
            i += 1

        self.frame_IT_3 = LabelFrame(self.top, rel=GROOVE, borderwidth=2, text='协议测试', foreground='blue')
        self.frame_IT_3.grid(pady=1, padx=10)

        self.button_name = ['button%s' % (x) for x in range(14)]
        i = 0
        protocol_count = self.testList(4, 4)
        while i < 14:
            if (protocol_count[i][1] == 0) or (protocol_count[i][1]) == 3:
                padx = 5
            else:
                padx = 66
            self.button_name[i] = Button(self.frame_IT_3, text=self.protocol[i], relief='ridge', width=12)
            self.button_name[i].grid(column=protocol_count[i][1], row=protocol_count[i][0], padx=padx, pady=5)
            i += 1
        self.button_name_config()
        self.frame_IT_4 = LabelFrame(self.top, rel=GROOVE, borderwidth=2, text='日志', foreground='blue')
        self.frame_IT_4.grid(pady=1, padx=10)

        self.msg = ScrolledText(self.frame_IT_4, width=83, height=10, font=('Arial', 9))
        self.msg.grid(padx=5, pady=5, sticky=W, column=0, columnspan=2)

        button_log_1 = ttk.Button(self.frame_IT_4, text='保存', width=5, command=self.save_log)
        button_log_1.grid(row=1, column=0, padx=10, sticky=W)

        button_log_2 = ttk.Button(self.frame_IT_4, text='清除', width=5, command=self.clear_log)
        button_log_2.grid(row=1, column=1, padx=3, sticky=E)

        self.frame_default = Frame(self.top, relief=FLAT)
        self.frame_default.grid(sticky=W)

        self.label_default_1 = myLabel(self.frame_default, text='  时间：')
        self.label_default_1.grid(column=0, row=0, sticky=W)
        self.label_default_3 = Label(self.frame_default, textvariable=self.value_date, justify='left')
        self.data_show()
        # self.insert_message()
        self.label_default_3.grid(column=1, row=0, pady=5, sticky=W)

        self.null_default_1 = Label(self.frame_default)
        self.null_default_1.grid(column=2, row=0, padx=185, pady=10)

        self.label_default_2 = myLabel(self.frame_default, textvariable=self.value_heartBeat, foreground='blue')
        self.label_default_2.grid(column=3, row=0, stick=E)

    def button_name_config(self):
        self.button_name[0]['command'] = self.cell_generate
        self.button_name[1]['command'] = self.version_ui
        self.button_name[2]['command'] = self.rfstatus_ui
        self.button_name[3]['command'] = self.rrustatus_ui
        self.button_name[4]['command'] = self.fiberstatus_ui
        self.button_name[5]['command'] = self.calibration_ui
        self.button_name[6]['command'] = self.alarm_ui
        self.button_name[7]['command'] = self.hardwara_ui
        self.button_name[8]['command'] = self.reboot_ui
        self.button_name[9]['command'] = self.delay_ui
        self.button_name[10]['command'] = lambda: protocal.protocol_test(0xA00F)
        self.button_name[10]['background'] = '#FF9968'
        self.button_name[10]['activebackground'] = '#FF9968'
        self.button_name[11]['command'] = self.parameter_ui
        self.button_name[12]['command'] = self.time_config_ui
        self.button_name[13]['command'] = self.mask_config_ui

    def cell_generate(self):
        self.sub_cell = Dialog(self.top, 450, 180, title='小区配置')
        config_item = ('配置标识', '小区标识', '小区功率', '天线组号', '下行掩码', '上行掩码', '  频点数', '  载波号', '中心频点',
                       '频点主辅', '子帧配比', '生效子帧', '载波带宽', '特殊子帧', '循环前缀', 'SCG Mask')
        value_default = (0, 0, 100, 4, 0xf, 0xf, 1, 0, 415000, 0, 0, 0, 5, 7, 0, 0x1f)
        for num in range(16):
            value = IntVar(value=value_default[num])
            self.value_cell_para[num] = value
        for entry in self.value_cell_response:
            entry.__init__()
        setup_msg_count = self.testList(4, 4)
        for num in setup_msg_count:
            index = setup_msg_count.index(num)
            parameter = tix.LabelEntry(self.sub_cell.canvas)
            parameter.grid(column=num[1], row=num[0], padx=5, pady=5)
            parameter.subwidget_list['label']['text'] = config_item[index]
            parameter.subwidget_list['entry']['textvariable'] = self.value_cell_para[index]
        self.sub_cell.canvas.create_line(0, 103, 440, 103, fill='#C4C4C4', width=1)

        parameter = tix.LabelEntry(self.sub_cell.canvas)
        parameter.grid(column=2, row=4, padx=10, pady=5)
        parameter.subwidget_list['label']['text'] = '小区标识'
        parameter.subwidget_list['entry']['textvariable'] = self.value_cell_response[0]

        parameter = tix.LabelEntry(self.sub_cell.canvas)
        parameter.grid(column=3, row=4, padx=10, pady=5)
        parameter.subwidget_list['label']['text'] = '结果返回'
        parameter.subwidget_list['entry']['textvariable'] = self.value_cell_response[1]

        self.sub_cell.button_test['command'] = lambda: protocal.protocol_test(0xA00C, value_list=self.value_cell_para)

    def version_ui(self):
        self.value_version_test.__init__()
        self.sub_version = Dialog(self.top, 300, 110, title='版本管理')
        button1 = Radiobutton(self.sub_version.canvas, text='版本查询', variable=self.value_version_select, value=1)
        button1.grid(column=0, row=0, padx=10, pady=5)

        button1 = Radiobutton(self.sub_version.canvas, text='版本下载', variable=self.value_version_select, value=2)
        button1.grid(column=1, row=0, padx=10, pady=5)

        button1 = Radiobutton(self.sub_version.canvas, text='版本激活', variable=self.value_version_select, value=3)
        button1.grid(column=2, row=0, padx=10, pady=5)

        self.sub_version.canvas.create_line(0, 30, 300, 30, fill='#C4C4C4', width=1)

        parameter = tix.LabelEntry(self.sub_version.canvas)
        parameter.grid(columnspan=2, column=1, row=1, padx=15, pady=5, stick='E')
        parameter.subwidget_list['label']['text'] = '结果返回'
        parameter.subwidget_list['entry']['textvariable'] = self.value_version_test

        def radiobutton_test():
            # print('radiobutton value is:', self.value_version_select.get())
            if self.value_version_select.get() == 1:
                protocal.protocol_test(0xA001)
            elif self.value_version_select.get() == 2:
                protocal.protocol_test(0xA002)
            elif self.value_version_select.get() == 3:
                protocal.protocol_test(0xA003)
            else:
                inert_message_queue.put('ALARM: version manage radiobutton variable is wrong!\n')

        self.sub_version.button_test['command'] = radiobutton_test

    def rfstatus_ui(self):
        self.value_rfstatus_result.__init__()
        self.sub_rfstatus = Dialog(self.top, 620, 300, title='射频状态查询', daltx=5)

        parameter = tix.LabelEntry(self.sub_rfstatus.canvas)
        parameter.grid(column=0, row=0, padx=5, pady=5, stick='w')
        parameter.subwidget_list['label']['text'] = '射频通道数'
        parameter.subwidget_list['entry']['textvariable'] = self.value_rfstatus_channel[0]
        parameter.subwidget_list['entry']['justify'] = 'center'

        self.sub_rfstatus.canvas.create_line(0, 25, 620, 25, fill='#C4C4C4', width=1)

        parameter = tix.LabelEntry(self.sub_rfstatus.canvas)
        parameter.grid(column=0, row=1, padx=5, pady=5, stick='w')
        parameter.subwidget_list['label']['text'] = '  结果返回'
        parameter.subwidget_list['entry']['textvariable'] = self.value_rfstatus_result
        parameter.subwidget_list['entry']['justify'] = 'center'

        column_name =['通道号', '通道温度', '发射功率', '发射增益', '接收功率', '接收增益', '驻波比', '下行结果', '上行结果', '驻波结果']
        self.sub_rfstatus.table = ttk.Treeview(self.sub_rfstatus.canvas, column=[1, 2, 3, 4, 5, 6, 7, 8, 9, 10], height=8, show='headings')
        for i in range(10):
            if (i == 0) or (i == 6):
                self.sub_rfstatus.table.column(column='#%d' % (i+1), width=50, minwidth=50, anchor='center')
            elif (i == 2) or (i == 4):
                self.sub_rfstatus.table.column(column='#%d' % (i+1), width=70, minwidth=70, anchor='center')
            else:
                self.sub_rfstatus.table.column(column='#%d' % (i+1), width=60, minwidth=60, anchor='center')
            self.sub_rfstatus.table.heading(column='#%d' % (i+1), text=column_name[i])

        self.sub_rfstatus.table.grid(column=0, row=2, padx=5, pady=5)
        for i in range(8):
            self.sub_rfstatus.table.insert('', 'end', value=(['%s' % (i+1), ] + ['-', ]*9), iid=i+1)
        self.sub_rfstatus.button_test['command'] = \
            lambda: protocal.protocol_test(0xA005, value_list=self.value_rfstatus_channel)

    def rrustatus_ui(self):
        self.value_rrustatus_response.__init__()
        self.sub_rrustatus = Dialog(self.top, 550, 160, title='RRU状态查询')

        for num in range(9):
            value = StringVar()
            self.value_rrustatus_para[num] = value

        parameter = tix.LabelEntry(self.sub_rrustatus.canvas)
        parameter.grid(column=0, row=0, padx=5, pady=5, stick='w')
        parameter.subwidget_list['label']['text'] = '结果返回'
        parameter.subwidget_list['entry']['textvariable'] = self.value_rrustatus_response
        parameter.subwidget_list['entry']['justify'] = 'center'

        para = ['本振频率', '本振状态', '时钟状态', '  IR接口', '运行状态', '主板温度', '从板温度', '运行时间', '系统时间']
        setup_msg_count = self.testList(2, 4)
        for num in setup_msg_count:
            index = setup_msg_count.index(num)
            parameter = tix.LabelEntry(self.sub_rrustatus.canvas)
            parameter.grid(column=num[1], row=(num[0]+1), padx=5, pady=5)
            parameter.subwidget_list['label']['text'] = para[index]
            parameter.subwidget_list['entry']['textvariable'] = self.value_rrustatus_para[index]
            parameter.subwidget_list['entry']['width'] = 12
            parameter.subwidget_list['entry']['justify'] = 'center'

        parameter = tix.LabelEntry(self.sub_rrustatus.canvas)
        parameter.grid(columnspan=2, column=0, row=3, padx=5, pady=5, stick='w')
        parameter.subwidget_list['label']['text'] = para[-1]
        parameter.subwidget_list['entry']['textvariable'] = self.value_rrustatus_para[-1]
        parameter.subwidget_list['entry']['width'] = 20
        parameter.subwidget_list['entry']['justify'] = 'center'

        self.sub_rrustatus.button_test['command'] = lambda: protocal.protocol_test(0xA006)

    def fiberstatus_ui(self):
        self.value_fiberstatus_response.__init__()
        self.sub_fiberstatus = Dialog(self.top, 560, 180, title='光模块状态查询')

        parameter = tix.LabelEntry(self.sub_fiberstatus.canvas)
        parameter.grid(column=0, row=0, padx=5, pady=5, stick='w')
        parameter.subwidget_list['label']['text'] = '光模块数'
        parameter.subwidget_list['entry']['textvariable'] = self.value_fiberstatus_channel[0]
        parameter.subwidget_list['entry']['justify'] = 'center'

        self.sub_fiberstatus.canvas.create_line(0, 25, 620, 25, fill='#C4C4C4', width=1)

        parameter = tix.LabelEntry(self.sub_fiberstatus.canvas)
        parameter.grid(column=0, row=1, padx=5, pady=5, stick='w')
        parameter.subwidget_list['label']['text'] = '结果返回'
        parameter.subwidget_list['entry']['textvariable'] = self.value_fiberstatus_response
        parameter.subwidget_list['entry']['justify'] = 'center'

        column_name =['光口号', '接收功率', '发射功率', '在位信息', '厂商', '传输速率', '温度', '电压', '电流']
        self.sub_fiberstatus.table = ttk.Treeview(self.sub_fiberstatus.canvas, column=[1, 2, 3, 4, 5, 6, 7, 8, 9], height=2, show='headings')
        for i in range(9):
            self.sub_fiberstatus.table.column(column='#%d' % (i+1), width=60, minwidth=60, anchor='center')
            self.sub_fiberstatus.table.heading(column='#%d' % (i+1), text=column_name[i])
        self.sub_fiberstatus.table.grid(column=0, row=2, padx=5, pady=5)
        for i in range(2):
            self.sub_fiberstatus.table.insert('', 'end', value=(['%s' % (i+1), ] + ['-', ]*8), iid=i+1)
        self.sub_fiberstatus.button_test['command'] = \
            lambda: protocal.protocol_test(0xA007, value_list=self.value_fiberstatus_channel)

    def calibration_ui(self):
        self.sub_calibration = Dialog(self.top, 450, 295, title='校准')

        label = Label(self.sub_calibration.canvas, text='校准类型')
        label.grid(pady=5, column=0, row=0, stick='w')
        combobox = ttk.Combobox(self.sub_calibration.canvas, value=['上行', '下行', '上下行'], \
                                textvariable=self.value_calibration_para[0], width=9, justify='center')
        combobox.grid(column=1, row=0)

        center_freq = tix.LabelEntry(self.sub_calibration.canvas)
        center_freq.grid(column=2, row=0, padx=5, pady=5, stick='w')
        center_freq.subwidget_list['label']['text'] = '中心频点'
        center_freq.subwidget_list['entry']['textvariable'] = self.value_calibration_para[1]
        center_freq.subwidget_list['entry']['justify'] = 'center'

        self.sub_calibration.canvas.create_line(0, 25, 620, 25, fill='#C4C4C4', width=1)

        for num in range(4):
            value = StringVar()
            self.value_calibration_result[num] = value

        result = ['返回结果', '校准结果', 'RMS结果', 'DPD结果']
        setup_msg_count = self.testList(1, 4)
        for num in setup_msg_count:
            index = setup_msg_count.index(num)
            parameter = tix.LabelEntry(self.sub_calibration.canvas)
            if index == 0:
                parameter.grid(column=num[1], row=(num[0]+1), padx=5, pady=5, columnspan=2, stick='w')
                parameter.subwidget_list['entry']['width'] = 12
            else:
                parameter.grid(column=num[1]+1, row=(num[0]+1), padx=5, pady=5)
            parameter.subwidget_list['label']['text'] = result[index]
            parameter.subwidget_list['entry']['textvariable'] = self.value_calibration_result[index]
            parameter.subwidget_list['entry']['justify'] = 'center'

        column_name =['通道号', '发射功率', '校准偏差', '发射增益', '接收增益', '接收功率']
        self.sub_calibration.table = ttk.Treeview(self.sub_calibration.canvas, column=[1, 2, 3, 4, 5, 6], height=8, show='headings')
        for i in range(6):
            self.sub_calibration.table.column(column='#%d' % (i+1), width=72, minwidth=72, anchor='center')
            self.sub_calibration.table.heading(column='#%d' % (i+1), text=column_name[i])
        self.sub_calibration.table.grid(columnspan=5, column=0, row=2, padx=5, pady=5)
        for i in range(8):
            self.sub_calibration.table.insert('', 'end', value=(['%s' % (i+1), ] + ['-', ]*5), iid=i+1)
        self.sub_calibration.button_test['command'] = \
            lambda: protocal.protocol_test(0xA00B, value_list=self.value_calibration_para)

    def alarm_ui(self):
        self.value_alarm_response.__init__()
        self.sub_alarm = Dialog(self.top, 120, 80, title='告警查询')

        alarm_query = tix.LabelEntry(self.sub_alarm.canvas)
        alarm_query.grid(column=0, row=0, padx=5, pady=5, stick='w')
        alarm_query.subwidget_list['label']['text'] = '有无告警'
        alarm_query.subwidget_list['entry']['textvariable'] = self.value_alarm_response
        alarm_query.subwidget_list['entry']['justify'] = 'center'

        self.sub_alarm.button_test['command'] = lambda: protocal.protocol_test(0xA009)

    def hardwara_ui(self):
        self.sub_hardware = Dialog(self.top, 500, 130, title='硬件参数查询')

        for num in range(9):
            value = StringVar()
            self.value_hardware_result[num] = value

        result = ['本振类型', '带宽', '最小频点', '最大频点', '通道数', '配置字增益', '分频比系数', '产品型号', '产品类型']
        setup_msg_count = self.testList(3, 3)
        for num in setup_msg_count:
            index = setup_msg_count.index(num)
            parameter = tix.LabelEntry(self.sub_hardware.canvas)
            parameter.grid(column=num[1], row=num[0], padx=5, pady=5, stick='E')
            parameter.subwidget_list['label']['text'] = result[index]
            parameter.subwidget_list['entry']['textvariable'] = self.value_hardware_result[index]
            parameter.subwidget_list['entry']['width'] = 15

        self.sub_hardware.button_test['command'] = lambda: protocal.protocol_test(0xA010)

    def reboot_ui(self):
        self.value_reboot_response.__init__()
        self.sub_reboot = Dialog(self.top, 150, 110, title='RRU复位')

        label = Label(self.sub_reboot.canvas, text='复位类型', anchor='w')
        label.grid(pady=5, column=0, row=0, stick='w')
        combobox = ttk.Combobox(self.sub_reboot.canvas, value=['软复位', '掉电复位'], textvariable=self.value_reboot_type[0], width=10)
        combobox.grid(column=1, row=0, stick='w')

        self.sub_reboot.canvas.create_line(0, 26, 160, 26, fill='#C4C4C4', width=1)

        parameter = tix.LabelEntry(self.sub_reboot.canvas)
        parameter.grid(columnspan=2, column=0, row=1, padx=5, pady=5, stick='w')
        parameter.subwidget_list['label']['text'] = '结果返回'
        parameter.subwidget_list['entry']['textvariable'] = self.value_reboot_response
        parameter.subwidget_list['entry']['width'] = 12

        self.sub_reboot.button_test['command'] = \
            lambda: protocal.protocol_test(0xA00E, value_list=self.value_reboot_type)

    def delay_ui(self):
        self.value_delay_test.__init__()
        self.value_delay_response.__init__()

        self.sub_delay = Dialog(self.top, 240, 80, title='时延配置')

        parameter1 = tix.LabelEntry(self.sub_delay.canvas)
        parameter1.grid(column=0, row=0, padx=5, pady=5, stick='w')
        parameter1.subwidget_list['label']['text'] = '光纤时延'
        parameter1.subwidget_list['entry']['textvariable'] = self.value_delay_test
        parameter1.subwidget_list['entry']['width'] = 10

        parameter2 = tix.LabelEntry(self.sub_delay.canvas)
        parameter2.grid(column=1, row=0, padx=5, pady=5, stick='w')
        parameter2.subwidget_list['label']['text'] = '结果返回'
        parameter2.subwidget_list['entry']['textvariable'] = self.value_delay_response

        self.sub_delay.button_test['command'] = lambda: protocal.protocol_test(0xA00D)

    def parameter_ui(self):
        self.value_parameter_response.__init__()
        self.sub_parameter = Dialog(self.top, 420, 130, title='参数查询')

        parameter2 = tix.LabelEntry(self.sub_parameter.canvas)
        parameter2.grid(column=0, row=0, padx=5, pady=5, stick='e')
        parameter2.subwidget_list['label']['text'] = '结果返回'
        parameter2.subwidget_list['entry']['textvariable'] = self.value_parameter_response

        for num in range(5):
            value = StringVar()
            self.value_parameter_result[num] = value

        result = ['驻波比门限', '板卡温度门限', '射频通道温度门限', '总时隙个数', '下行时隙比']
        setup_msg_count = self.testList(2, 3)
        for i in range(5):
            num = setup_msg_count[i]
            parameter = tix.LabelEntry(self.sub_parameter.canvas)
            parameter.grid(column=num[1], row=num[0]+1, padx=5, pady=5, stick='E')
            parameter.subwidget_list['label']['text'] = result[i]
            parameter.subwidget_list['entry']['textvariable'] = self.value_parameter_result[i]
        self.sub_parameter.button_test['command'] = lambda: protocal.protocol_test(0xA00A)

    def time_config_ui(self):
        self.value_time_config_result.__init__()
        self.value_time_config_response.__init__()

        self.sub_time_config = Dialog(self.top, 220, 80, title='系统时间配置')

        parameter1 = tix.LabelEntry(self.sub_time_config.canvas)
        parameter1.grid(column=0, row=0, padx=5, pady=5, stick='w')
        parameter1.subwidget_list['label']['text'] = 'Result'
        parameter1.subwidget_list['entry']['textvariable'] = self.value_time_config_result

        parameter2 = tix.LabelEntry(self.sub_time_config.canvas)
        parameter2.grid(column=1, row=0, padx=5, pady=5, stick='w')
        parameter2.subwidget_list['label']['text'] = '结果返回'
        parameter2.subwidget_list['entry']['textvariable'] = self.value_time_config_response

        self.sub_time_config.button_test['command'] = lambda: protocal.protocol_test(0xA008)

    def mask_config_ui(self):
        self.value_mask_config_result.__init__()
        self.value_mask_config_response.__init__()

        self.sub_mask_config = Dialog(self.top, 220, 105, title='天线掩码配置')

        parameter1 = tix.LabelEntry(self.sub_mask_config.canvas)
        parameter1.grid(column=0, row=0, padx=5, pady=5, stick='w')
        parameter1.subwidget_list['label']['text'] = '下行掩码'
        parameter1.subwidget_list['entry']['textvariable'] = self.value_mask_config[0]

        parameter2 = tix.LabelEntry(self.sub_mask_config.canvas)
        parameter2.grid(column=1, row=0, padx=5, pady=5, stick='w')
        parameter2.subwidget_list['label']['text'] = '上行掩码'
        parameter2.subwidget_list['entry']['textvariable'] = self.value_mask_config[1]

        self.sub_mask_config.canvas.create_line(0, 26, 220, 26, fill='#C4C4C4', width=1)

        parameter3 = tix.LabelEntry(self.sub_mask_config.canvas)
        parameter3.grid(column=0, row=1, padx=5, pady=5, stick='e')
        parameter3.subwidget_list['label']['text'] = 'Result'
        parameter3.subwidget_list['entry']['textvariable'] = self.value_mask_config_result

        parameter4 = tix.LabelEntry(self.sub_mask_config.canvas)
        parameter4.grid(column=1, row=1, padx=5, pady=5, stick='w')
        parameter4.subwidget_list['label']['text'] = '结果返回'
        parameter4.subwidget_list['entry']['textvariable'] = self.value_mask_config_response

        self.sub_mask_config.button_test['command'] = \
            lambda: protocal.protocol_test(0xA004, value_list=self.value_mask_config)

    def data_show(self):
        data = time.ctime()

        moth = {'Oct': 10, 'Jan': 1, 'Feb': 2, 'Mar': 3, 'Apr': 4, 'May': 5, 'Jun': 6, 'Jul': 7, 'Aug': 8, 'Sep': 9,
                'Nov': 11, 'Dec': 12}
        # week = {'Sat': '六', 'Tue': '二', 'Mon': '一', 'Wed': '三', 'Thu': '四', 'Fri': '五', 'Sun': '日'}

        b = data.split()
        # if b[0] in week:
        #     weekShow=week[b[0]]
        if b[1] in moth:
            moth_show = moth[b[1]]

        data1 = str(b[-1]) + '-' +str(moth_show) + '-' + str(b[2]) + ' '+str(b[3])

        self.value_date.set(data1)
        self.top.after(1000, self.data_show)

    def heartbeat(self):
        global dstIP
        dstIP = self.value_dstIP.get()
        port = 23

        try:
            b = create_connection((dstIP, port), timeout=3)
            if b:
                self.value_heartBeat.set('板卡已连接')
                b.close()

        except:
            self.value_heartBeat.set('板卡未连接')

    def clear_log(self):
        self.msg.delete(1.0, 'end')

    def save_log(self):
        content = self.msg.get(1.0, 'end')
        log_name = self.value_date.get()
        log_name = log_name.replace(':', '_')
        log_name = log_name.replace(' ', '#')
        log_name = log_name + '.log'
        log = open('log\%s' % (log_name), 'a')
        log.write(content)
        self.msg.insert('end', '----- 日志已保存! -----\n', 'INT')

    def testList(self, row, col):
        itemList = []
        i = j = 0
        while i < row:
            while j < col:
                pos=(i, j)
                itemList.append(pos)
                j += 1
            i += 1
            j = 0
        return itemList


class IntVarNoDefault(IntVar):
    def __init__(self):
        self._default = ''
        IntVar.__init__(self)


class Dialog:

    def __init__(self, master, width, height, daltx=30, dalty=30, title=''):
        self.root = Toplevel(master)
        self.root.title(title)
        self.root.withdraw()
        # self.root.attributes('-toolwindow', 1, '-alpha', 0.9)
        self.root.attributes('-toolwindow', 1)
        f = font.nametofont('TkDefaultFont')
        f.configure(family='宋体')
        self.root.transient(master)
        m_x = master.winfo_rootx()
        m_y = master.winfo_rooty()
        self.root.geometry('=%dx%d+%d+%d' % (width, height, m_x+daltx, m_y+dalty))
        self.root.maxsize(width, height)
        self.root.deiconify()
        self.frame_1 = Frame(self.root)
        self.frame_1.grid()
        self.canvas = Canvas(self.frame_1, highlightcolor='#ECE9D8', state='disabled', insertborderwidth=200)
        self.canvas.grid(padx=4, column=1, row=0, pady=10)
        self.frame_2 = Frame(self.root)
        self.frame_2.grid(pady=5)
        # from PLTTest import protocol_test
        self.button_test = ttk.Button(self.frame_2, text='测试', width=5)
        self.button_test.grid(column=1, row=0, padx=5)

        self.root.focus_set()
        self.root.grab_set()
        # for table use
        self.table = ''

    def add_widget(self):
        pass

GUI = MsgGui()


def main():
    MsgGui()
    mainloop()


if __name__ == '__main__':
    main()

