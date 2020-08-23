
#coding=utf-8

from tkinter import *
from functools import partial
from tkinter.scrolledtext import *
import time
from socket import create_connection
from configparser import ConfigParser
from tkinter import tix
from tkinter import ttk
import queue
import tkinter.font


myLabel = partial(Label, width=10, height=2)
i = 0

inert_message_queue = queue.Queue()
config_para = ConfigParser()
config_para.read('para.ini')

MCT_IP = config_para.get('eNB','MCT')

testItem_MCT = ['PHY', 'CPLD', 'SWITCH', 'DDR', 'NandFlash', 'NorFlash', 'USB', '光模块', '温度', '功耗', 'GPS',
                    'EEPROM', '版本', 'NULL2', 'NULL3']
testItem_BBP = ['SWITCH', 'SRIO路由', 'SRIO数据', 'AIF', 'SDRAM', '温度', '功耗', 'EEPROM', 'DSP链路',
                'DSP_DDR', 'SRIO效率', 'ARM版本', 'CPLD', '光同步', '光模块', 'DSP版本', 'NULL0', 'NULL1', 'NULL2', 'NULL3']
testItem_FM = ['风速控制', '风速', 'EEPROM', '版本', 'NULL0']
testItem_IT = ['HMI_MB', 'HMI_MF', 'HMI_ME', 'AFC', '启动状态', '同步开', '同步停', 'DSP_SRIO', 'NULL1', 'NULL2']
testItem_EM = ['干结点', '温度', '版本', '485口', 'NULL1']
testItem_FS = ['SWITCH', 'PLL', 'SRIO路由', 'SDRAM', '温度', '功耗', 'EEPROM', 'PHY', '光模块', '光同步', 'FPGA_DDR',
               '10G光同步', 'FPGA_SRIO', 'ARM版本', 'NULL1']
testItem_ES = ['SWITCH', 'PLL', 'SDRAM', '温度', 'ARM版本', 'EEPROM', '光模块', '电口同步', '光口同步', 'NULL0']

class msggui:
    
    def __init__(self, initdir=None):
        '''
        tkinter root basic option
        '''
        self.top = tix.Tk(className='平台测试_eBBU')
        # self.top.geometry('=960x700+300+100')
        self.top.geometry('=930x630+300+100')
        self.top.maxsize(930, 650)
        self.top.iconbitmap(default='myicon.ico')

        f = tkinter.font.nametofont('TkDefaultFont')
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

        self.value = []
        self.sub_slot_id_value = []
      
        self.value_testResult = StringVar()
        self.value_testResult.set('正在测试..')

        self.value_AFC = StringVar()
        
        j=0
        while j<10:
            a = IntVar()
            self.value.append(a)
            j += 1

        for sub_id in range(8):
            sub_value = IntVar()
            self.sub_slot_id_value.append(sub_value)

        self.value_select_IT = IntVar()

        self.value_testTime = IntVar()
        self.value_testTime.set(1)

            
        '''
          main UI
        '''
        self.frame_item = LabelFrame(self.top, relief=FLAT)
        self.frame_item.grid(column=0, row=0, sticky=N, pady=5)

        self.notebook = ttk.Notebook(self.frame_item, padding=10)
        self.notebook.grid()

        self.frame_IT = Frame(self.notebook, name='it')
        self.frame_MCT = Frame(self.notebook, name='mct')
        self.frame_BBP = Frame(self.notebook, name='bbp')
        self.frame_FM = Frame(self.notebook, name='fm')
        self.frame_EM = Frame(self.notebook, name='em')
        self.frame_FS = Frame(self.notebook, name='fs')
        self.frame_ES = Frame(self.notebook, name='es')

        self.notebook.add(self.frame_IT, text='集成')
        self.notebook.add(self.frame_MCT, text='主控板')
        self.notebook.add(self.frame_BBP, text='基带板')

        # self.notebook.add(self.frame_GES,text='交换板')
        self.notebook.add(self.frame_FM, text='风扇板')
        self.notebook.add(self.frame_EM, text='监控板')
        self.notebook.add(self.frame_FS, text='交换板')
        self.notebook.add(self.frame_ES, text='同步板')

        self.frame_control=LabelFrame(self.frame_item, relief=GROOVE, borderwidth=2, text='实验控制', foreground='blue')
        self.frame_control.grid(sticky=W, padx=10)

        self.entry_control_1=tix.LabelEntry(self.frame_control)
        self.entry_control_1.subwidget_list['label']['text'] = '试验次数：'
        self.entry_control_1.subwidget_list['entry']['textvariable'] = self.value_testTime
        self.entry_control_1.grid(column=0, row=0, padx=10)

        self.button_control_1=ttk.Button(self.frame_control, text='设定', width=5)
        self.button_control_1.grid(column=1,row=0,padx=8)

        self.null_control_1=Label(self.frame_control)
        self.null_control_1.grid(column=2,row=0,padx=130)

        self.button_control_2=Button(self.frame_control, text='结果显示', relief='groove')
        self.button_control_2.grid(column=3, row=0, padx=5)
        
        self.msg_control_1 = Message(self.frame_control, width=500, textvariable=self.value_testResult, justify='center', foreground='blue', font=('Arial',9))
        self.msg_control_1.grid(row=1, pady=5, stick=W, column=0, columnspan=4)

        self.frame_default=Frame(self.frame_item, relief=FLAT)
        self.frame_default.grid(sticky=W)

        self.label_default_1=myLabel(self.frame_default, text='  时间：')
        self.label_default_1.grid(column=0, row=0, sticky=W)
        self.label_default_3=Label(self.frame_default, textvariable=self.value_date, justify='left')
        self.dataShow()
        self.label_default_3.grid(column=1, row=0, pady=5, sticky=W)

        self.null_default_1=Label(self.frame_default)
        self.null_default_1.grid(column=2, row=0, padx=130, pady=10)

        self.label_default_2=myLabel(self.frame_default, textvariable=self.value_heartBeat, foreground='blue')
        self.label_default_2.grid(column=3, row=0, columnspan=2)

        self.frame_log = LabelFrame(self.top, height=50, relief=GROOVE, borderwidth=2, text='日志', foreground='blue')
        self.frame_log.grid(column=1, row=0, sticky=N, pady=15, ipady=5)

        self.msg=ScrolledText(self.frame_log, width=47, height=31, font=('Arial',10))
        self.msg.grid(padx=10, pady=5, sticky=W, column=0, columnspan=2)

        self.button_log_1=ttk.Button(self.frame_log, text='保存', width=5, command=self.saveLog)
        self.button_log_1.grid(row=1, column=0, padx=10, sticky=W)

        self.button_log_2=ttk.Button(self.frame_log, text='清除', width=5, command=self.clearLog)
        self.button_log_2.grid(row=1, column=1, padx=10, sticky=E)
        
        '''
          IT UI
         '''
        self.frame_IT_1=LabelFrame(self.frame_IT, relief=GROOVE, borderwidth=2, text='系统信息', foreground='blue')
        self.frame_IT_1.grid(pady=10, padx=10, sticky=W)

        self.label_IT_1=myLabel(self.frame_IT_1, text='目标IP：')
        self.label_IT_1.grid(column=0, row=0, sticky=W)
        self.null_IT_1=Label(self.frame_IT_1)
        self.null_IT_1.grid(column=2, row=0, padx=100)
        self.button_IT_1=ttk.Button(self.frame_IT_1, text='连接', width=5)
        self.button_IT_1.grid(column=3, row=0, padx=5)
        self.entry_IT_1=Entry(self.frame_IT_1, width=22, textvariable=self.value_dstIP, font=('Arial',9))
        self.entry_IT_1.grid(column=1, row=0, pady=5)

        self.label_IT_com = myLabel(self.frame_IT_1, text='目标COM：')
        self.label_IT_com.grid(column=0, row=1, sticky=W)
        self.button_IT_com = ttk.Button(self.frame_IT_1, text='连接', width=5)
        self.button_IT_com.grid(column=3, row=1, padx=5)
        self.combobox_IT_com = ttk.Combobox(self.frame_IT_1, width=5, values=['COM1', 'COM2', 'COM3', 'COM4', 'COM5',
                                                                              'COM6','COM7', 'COM8'],
                                            font=('Arial', 9), justify='center')
        self.combobox_IT_com.set('COM1')
        self.combobox_IT_com.grid(column=1, row=1, pady=5, sticky=W)

        self.label_IT_2=myLabel(self.frame_IT_1,text=' ETH_3：')
        self.label_IT_2.grid(column=0, row=2, sticky=W)

        self.button_IT_2=ttk.Button(self.frame_IT_1, text='设置',width=5)
        self.button_IT_2.grid(column=3,row=2,padx=8)

        self.entry_IT_2 = Entry(self.frame_IT_1, width=22, textvariable=self.value_eth3, font=('Arial',9))
        self.entry_IT_2.grid(column=1, row=2, pady=5)

        self.label_IT_3=myLabel(self.frame_IT_1, text='AFC状态：')
        self.label_IT_3.grid(column=0, row=3, sticky=W)
        self.entry_IT_3=Entry(self.frame_IT_1, width=22, font=('Arial',9), state='readonly', textvariable=self.value_AFC)
        self.entry_IT_3.grid(column=1, row=3, pady=5)

        self.frame_IT_2=LabelFrame(self.frame_IT, relief=GROOVE,borderwidth=2, text='测试项', foreground='blue')
        self.frame_IT_2.grid(sticky=W, padx=10)

        self.testItem_IT = testItem_IT

        self.cbName_IT = ['IT_Cbutton%s'%(x) for x in range(10)]
        global i
        while i < 10:
            
            self.cbName_IT[i] = Checkbutton(self.frame_IT_2, text=self.testItem_IT[i], variable=self.value[i])
            self.cbName_IT[i].grid(column=testList(2, 5)[i][1], row=testList(2, 5)[i][0], sticky=W, padx=16, pady=5)
            i += 1

        self.cbName_IT[-1].destroy()
        self.cbName_IT[-2].destroy()
        # self.cbName_IT[-3].destroy()

        self.frame_IT_3=LabelFrame(self.frame_IT, relief=FLAT)
        self.frame_IT_3.grid(column=0, row=2, sticky=W, padx=10, pady=5)

        self.MCTslot_IT_label = Label(self.frame_IT_3, text='主控槽位：')
        self.MCTslot_IT_label.grid(column=0, row=0)
        self.MCTslot_IT_combobox = ttk.Combobox(self.frame_IT_3, width=1, values=['0', '1'], font=('Arial',8),
                                                justify='center')
        self.MCTslot_IT_combobox.grid(column=1, row=0)
        self.MCTslot_IT_combobox.set('0')

        self.sub_slot_IT_label = Label(self.frame_IT_3, text='从板槽位：')
        self.sub_slot_IT_label.grid(column=0, row=2)
        self.sub_slot_id = [2, 3, 4, 5, 6, 7, 8, 9]
        for id in self.sub_slot_id:
            self.sub_slot_id_checkbutton = ttk.Checkbutton(self.frame_IT_3, text=str(id), variable=self.sub_slot_id_value[id - 2])
            self.sub_slot_id_checkbutton.grid(column=id-1, row=2)

        self.label_srio_send = Label(self.frame_IT_3, text='SRIO发送：')
        self.label_srio_send.grid(row=1, column=0, pady=8)
        self.combobox_srio_send = ttk.Combobox(self.frame_IT_3, width=1, values=['2', '3', '4', '5', '6', '7'], font=('Arial', 9), justify='center')
        self.combobox_srio_send.grid(column=1, row=1)
        self.combobox_srio_send.set('2')

        self.label_srio_receive = Label(self.frame_IT_3, text='SRIO接收：')
        self.label_srio_receive.grid(row=1, column=3, columnspan=2)
        self.combobox_srio_receive = ttk.Combobox(self.frame_IT_3, width=1, values=['2', '3', '4', '5', '6', '7'], font=('Arial', 9), justify='center')
        self.combobox_srio_receive.grid(column=5, row=1)
        self.combobox_srio_receive.set('3')

        self.frame_IT_4 = LabelFrame(self.frame_IT, relief=FLAT)
        self.frame_IT_4.grid(column=0, row=3, sticky=W, padx=10)

        self.button_IT_stop = ttk.Button(self.frame_IT_4, text='停止', width=5)
        self.button_IT_stop.grid(column=9, row=0, padx=10)

        self.button_IT_3 = ttk.Button(self.frame_IT_4, text='开始', width=5)
        self.button_IT_3.grid(column=8, row=0, padx=10)

        self.cbutton_IT_1 = ttk.Checkbutton(self.frame_IT_4, text='全选', variable=self.value_select_IT, command=self.selectAll_IT)
        self.cbutton_IT_1.grid(column=7, row=0, padx=20)

        self.null_IT_label1 = Label(self.frame_IT_4)
        self.null_IT_label1.grid(column=0, row=0, padx=140)


    # def msgshow(self):
    #     showinfo('message','add function')

    def dataShow(self):
        data = time.ctime()

        moth = {'Oct': 10, 'Jan': 1, 'Feb': 2, 'Mar': 3, 'Apr': 4, 'May': 5, 'Jun': 6, 'Jul': 7, 'Aug': 8, 'Sep': 9,
                'Nov': 11, 'Dec': 12}
        week = {'Sat': '六', 'Tue': '二', 'Mon': '一', 'Wed': '三', 'Thu': '四', 'Fri': '五', 'Sun': '日'}

        b=data.split()

        if b[0] in week:
            weekShow=week[b[0]]

        if b[1] in moth:
            mothShow = moth[b[1]]

        data1=str(b[-1]) +'-'+str(mothShow)+'-'+str(b[2])+' '+str(b[3])

        self.value_date.set(data1)
        self.top.after(1000, self.dataShow)

    def heartbeat(self):
        dstIP = self.value_dstIP.get()
        port=23

        try:
            b = create_connection((dstIP,port), timeout=3)
            if b:
                self.value_heartBeat.set('板卡已连接')
                b.close()

        except:
            self.value_heartBeat.set('板卡未连接')

    def selectAll_IT(self):
        for j in range(7):
            if self.value_select_IT.get() == 1:
                self.cbName_IT[j].select()
            else:
                self.cbName_IT[j].deselect()

    def clearLog(self):
        self.msg.delete(1.0, 'end')

    def saveLog(self):
        content = self.msg.get(1.0,'end')
        logName = self.value_date.get()
        logName = logName.replace(':','_')
        logName = logName.replace(' ','#')
        logName = logName+'.log'
        log = open('log\%s'%(logName),'a')
        log.write(content)
        self.msg.insert('end','----- 日志已保存! -----\n','INT')
        

def testList(row, col):
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


class pageSetUp:

    def __init__(self, master, row, column, item, padx_cb, real):
        """
        class can use for all pages in notebook
        """
        self.master = master
        self.row = row
        self.column = column
        self.total = row*column
        self.real = real

        self.value = []
        i = 0
        while i < self.total:
            a = IntVar()
            self.value.append(a)
            i += 1
    
        self.value_select = IntVar()
        self.eeprom_select = IntVar()
        
        self.value_eeprom = [StringVar() for i in range(10)]

        self.key_eeprom=['设备代号', '硬件版本','  MAC1','  MAC2','生产序列','制造商','生产日期','卫星型号','风扇转速','工作温度']

        self.value_var=IntVar()
        self.value_var1=IntVar()
        self.value_reset = IntVar()
        self.value_reset1 = IntVar()
        self.value_reset2 = IntVar()

        self.frame_1 = LabelFrame(self.master, relief=GROOVE, borderwidth=2, text='单板信息', foreground='blue')
        self.frame_1.grid(pady=10, padx=10)
        
        self.option=['eeprom_test_key%s'%(x) for x in range(10)]
        self.entry=['eeprom_test_value%s'%(x) for x in range(10)]

        for i, name in enumerate(self.key_eeprom):
            if i % 2 == 0: a, b = 1, 2
            elif i % 2 == 1: a, b = 4, 5
                
            self.option[i] = Label(self.frame_1, text=name)
            self.option[i].grid(column=a, row=int(i/2), sticky=W)
            self.entry[i] = Entry(self.frame_1, width=20, textvariable=self.value_eeprom[i], font=('Arial',9))
            self.entry[i].grid(column=b, row=int(i/2), pady=5)
##            print(self.option[i]['state'])

        self.entry[6]['state']='readonly'
        self.entry[9].insert(1, ' -20 ~ 55')
        self.entry[5].insert(1, 'CQXINWEI')
        self.null_1 = Label(self.frame_1)

        self.null_2 = Label(self.frame_1)
        self.null_2.grid(column=6, row=0, padx=5)
        self.null_0=Label(self.frame_1)
        self.null_0.grid(column=0, row=0, padx=(padx_cb/2-4))

        self.button_1=ttk.Button(self.frame_1,text='读',width=3)
        self.button_1.grid(column=2, row=5, padx=20, pady=5)

        self.button_2 = ttk.Button(self.frame_1,text='写', width=3)
        self.button_2.grid(column=5,row=5,sticky=E)

        self.frame_2 = LabelFrame(self.master, relief=GROOVE, borderwidth=2, text='测试项', foreground='blue')
        self.frame_2.grid(sticky=W, padx=10)

        self.testItem = item
        
        self.cbName=['BBP_Cbutton%s'%(x) for x in range(self.total)]
        
        i = 0
        while i < self.total:
            
            self.cbName[i] = Checkbutton(self.frame_2,text=self.testItem[i],variable=self.value[i])
            self.cbName[i].grid(column=testList(self.row,self.column)[i][1],row=testList(self.row,self.column)[i][0],sticky=W,padx=padx_cb,pady=5)
            i = i+1

        self.frame_3 = LabelFrame(self.master, relief=GROOVE, borderwidth=2, text='功能项', foreground='blue')
        self.frame_3.grid(column=0, row=2, sticky=W, padx=10, pady=5)
        self.null_frame3_1 = Label(self.frame_3)

        self.frame_4=LabelFrame(self.master,relief=FLAT)
        self.frame_4.grid(column=0, row=3, sticky=W, padx=10, pady=5)

        self.null_3 = Label(self.frame_3)
        self.null_3.grid(column=0,row=0)

        self.null_4 = Label(self.frame_3)
        self.null_4.grid(column=1,row=0)
        self.label_frame4_slot = Label(self.frame_4, text='槽位:')
        self.label_frame4_slot.grid(row=0, column=0)
        self.null_frame4_1 = Label(self.frame_4)

        self.button_3=ttk.Button(self.frame_4,text='开始',width=5)
        self.button_3.grid(column=8, row=0, padx=10)

        self.button_stop = ttk.Button(self.frame_4, text='停止', width=5)
        self.button_stop.grid(column=9, row=0, padx=10)

        self.cbutton_1 = ttk.Checkbutton(self.frame_4,text='全选',variable=self.value_select,command=self.selectAll)
        self.cbutton_1.grid(column=7,row=0,padx=30)

    def selectAll(self):
        for j in range(self.real):
            if self.value_select.get() == 1:
                self.cbName[j].select()
            else:
                self.cbName[j].deselect()

# mct board ui config

GUI = msggui()

GUI.msg.tag_configure('INT',foreground='blue')
GUI.msg.tag_configure('fail',foreground='red')

MCT = pageSetUp(GUI.frame_MCT, 3, 5, testItem_MCT, 15, 13)
MCT.cbName[14].destroy()
MCT.cbName[13].destroy()
MCT.option[8]['state'] = 'disabled'
MCT.entry[8]['state'] = 'readonly'
MCT.null_3 = Checkbutton(MCT.frame_3, text='CPLD升级', variable=MCT.value_var)
MCT.null_3.grid(column=0, row=0, padx=15)

MCT.syn_485 = Checkbutton(MCT.frame_3, text='1PPS', variable=MCT.value_reset)
MCT.syn_485.grid(column=1, row=0, padx=15, sticky=W)

MCT.syn_tod = Checkbutton(MCT.frame_3, text='TOD', variable=MCT.value_reset1)
MCT.syn_tod.grid(column=2, row=0, padx=26, sticky=W)

MCT.null_6 = ttk.Combobox(MCT.frame_4, width=1, values=['0', '1'], font=('Arial', 9), justify='center')
MCT.null_6.grid(column=2, row=0)
MCT.null_6.set('0')
MCT.null_frame3_1.grid(row=0, column=3, padx=98)
MCT.null_frame4_1.grid(column=3, padx=90)

MCT.null_1.grid(column=3,row=0,padx=28)


# bbp board ui config

BBP = pageSetUp(GUI.frame_BBP, 4, 5, testItem_BBP, 12, 16)
BBP.option[8]['state'] = 'disable'
BBP.option[7]['state'] = 'disable'
BBP.option[2]['state'] = 'disable'
BBP.option[3]['state'] = 'disable'
BBP.entry[8]['state'] = 'readonly'
BBP.entry[7]['state'] = 'readonly'
BBP.entry[2]['state'] = 'readonly'
BBP.entry[3]['state'] = 'readonly'

BBP.cbName[-1].destroy()
BBP.cbName[-2].destroy()
BBP.cbName[-3].destroy()
BBP.cbName[-4].destroy()

BBP.null_3=Checkbutton(BBP.frame_3, text='CPLD升级', variable=BBP.value_var)
BBP.null_3.grid(column=0, row=0, padx=12)

BBP.null_4 = Checkbutton(BBP.frame_3,text='MCU升级', variable=BBP.value_var1)
BBP.null_4.grid(column=1, row=0)

BBP.reset_checkbutton = Checkbutton(BBP.frame_3, text='复位', variable=BBP.value_reset)
BBP.reset_checkbutton.grid(column=2, row=0, padx=30)
BBP.reset_dsp_checkbutton = Checkbutton(BBP.frame_3, text='DSP加载', variable=BBP.value_reset1)
BBP.reset_dsp_checkbutton.grid(column=3, row=0, padx=18)
BBP.reset_fpga_checkbutton = Checkbutton(BBP.frame_3, text='FPGA加载', variable=BBP.value_reset2)
BBP.reset_fpga_checkbutton.grid(column=4, row=0, padx=12)

BBP.null_frame4_1.grid(column=3, padx=90)

BBP.null_6=ttk.Combobox(BBP.frame_4, width=1, values=['2','3','4','5','6','7'], font=('Arial',9), justify='center')
BBP.null_6.grid(column=2, row=0)
BBP.null_6.set('2')

BBP.null_1.grid(column=3, row=0, padx=28)


# fan board ui config


FM = pageSetUp(GUI.frame_FM, 1, 5, testItem_FM, 30, 4)
FM.cbName[4].destroy()
FM.label_frame4_slot['text']='速率：'

FM.null_4 = tix.Control(FM.frame_4, max=100, min=1, value=1)
FM.null_4.grid(column=1,row=0)

FM.null_6 = Label(FM.frame_4, text='通道：')
FM.null_6.grid(column=2,row=0)

FM.null_7 = ttk.Combobox(FM.frame_4, width=3, values=['ALL', 0, 1, 2], font=('Arial', 9), justify='center')
FM.null_7.grid(column=3, row=0)
FM.null_7.set('ALL')

FM.update_mcu_check = Checkbutton(FM.frame_3, text='MCU升级', variable=FM.value_var1)
FM.update_mcu_check.grid(row=0, padx=29)

FM.reset_checkbutton = Checkbutton(FM.frame_3, text='复位', variable=FM.value_reset)
FM.reset_checkbutton.grid(column=1, row=0, padx=37)
FM.null_frame3_1.grid(row=0, column=2, padx=112)
FM.null_frame4_1.grid(column=4, padx=37)

FM.null_0.grid(column=0, row=0, padx=5)
FM.null_1.grid(column=3, row=0, padx=25)

FM.option[7]['state'] = 'disable'
FM.option[2]['state'] = 'disable'
FM.option[3]['state'] = 'disable'
FM.entry[7]['state'] = 'readonly'
FM.entry[2]['state'] = 'readonly'
FM.entry[3]['state'] = 'readonly'
FM.entry[8].insert(1, '       |       |')

# peu board ui config

EM = pageSetUp(GUI.frame_EM, 1, 5, testItem_EM, 25, 4)
EM.cbName[4].destroy()
EM.null_frame2_1 = Label(EM.frame_2)
EM.null_frame2_1.grid(column=13, row=0, padx=30)

for i in range(10):
    EM.option[i]['state'] = 'disabled'
    EM.entry[i].delete(0, 'end')
    EM.entry[i]['state'] = 'readonly'
EM.button_1['state'] = 'disabled'
EM.button_2['state'] = 'disabled'

EM.null_1.grid(column=3, row=0, padx=25)
EM.update_mcu_check = Checkbutton(EM.frame_3, text='MCU升级', variable=EM.value_var)
EM.update_mcu_check.grid(column=0, row=0, padx=24)

EM.reset_checkbutton = Checkbutton(EM.frame_3, text='复位', variable=EM.value_reset)
EM.reset_checkbutton.grid(column=1, row=0, padx=20)
EM.reset_power_checkbutton = Checkbutton(EM.frame_3, text='整机掉电', variable=EM.value_reset1)
EM.reset_power_checkbutton.grid(column=2, row=0, padx=29)
EM.null_frame3_1.grid(row=0, column=3, padx=71)

EM.null_6 = ttk.Combobox(EM.frame_4, width=1, values=['8', '9'], font=('Arial', 9), justify='center')
EM.null_6.grid(column=1, row=0)
EM.null_6.set('8')

EM.null_frame4_1.grid(row=0, column=2, padx=95)

# fs board ui config

FS = pageSetUp(GUI.frame_FS, 3, 5, testItem_FS, 11, 14)
FS.null_1.grid(column=3, row=0, padx=29)
FS.cbName[-1].destroy()

FS.entry[8]['state'] = 'readonly'
FS.entry[7]['state'] = 'readonly'
FS.entry[2]['state'] = 'readonly'
FS.entry[3]['state'] = 'readonly'

FS.option[8]['state'] = 'disable'
FS.option[7]['state'] = 'disable'
FS.option[2]['state'] = 'disable'
FS.option[3]['state'] = 'disable'

FS.null_3=Checkbutton(FS.frame_3, text='FPGA_160加载', variable=FS.value_var)
FS.null_3.grid(column=0, row=0, padx=12)
FS.null_4 = Checkbutton(FS.frame_3,text='FPGA_325加载', variable=FS.value_var1)
FS.null_4.grid(column=1, row=0, padx=35)
FS.update_mcu_check = Checkbutton(FS.frame_3, text='MCU升级', variable=FS.value_reset)
FS.update_mcu_check.grid(column=2, row=0, padx=10)
FS.reset_check = Checkbutton(FS.frame_3, text='复位', variable=FS.value_reset1)
FS.reset_check.grid(column=3, row=0, padx=24)

FS.null_6 = ttk.Combobox(FS.frame_4, width=1, values=['6', '7'], font=('Arial', 9), justify='center')
FS.null_6.grid(column=1, row=0)
FS.null_6.set('7')

FS.null_frame4_1.grid(row=0, column=2, padx=50)

# es board ui config

ES = pageSetUp(GUI.frame_ES, 2, 5, testItem_ES, 14, 9)
ES.null_1.grid(column=3, row=0, padx=29)
ES.cbName[9].destroy()

ES.entry[8]['state'] = 'readonly'
ES.entry[7]['state'] = 'readonly'
ES.entry[2]['state'] = 'readonly'
ES.entry[3]['state'] = 'readonly'

ES.option[8]['state'] = 'disable'
ES.option[7]['state'] = 'disable'
ES.option[2]['state'] = 'disable'
ES.option[3]['state'] = 'disable'

ES.null_3=Checkbutton(ES.frame_3, text='FPGA加载', variable=ES.value_var)
ES.null_3.grid(column=0, row=0, padx=12)
ES.null_4 = Checkbutton(ES.frame_3,text='MCU升级', variable=ES.value_var1)
ES.null_4.grid(column=1, row=0, padx=5)
ES.reset_check = Checkbutton(ES.frame_3, text='复位', variable=ES.value_reset)
ES.reset_check.grid(column=2, row=0, padx=19)

ES.null_frame3_1.grid(row=0, column=3, padx=107)

ES.null_6 = ttk.Combobox(ES.frame_4, width=1, values=['2', '3', '4', '5', '6', '7'], font=('Arial', 9), justify='center')
ES.null_6.grid(column=1, row=0)
ES.null_6.set('5')

ES.null_frame4_1.grid(row=0, column=2, padx=95)


# mainloop()

# def main():
#    msggui()
#    mainloop()
#
#
#
# if __name__=='__main__':
#    main()

