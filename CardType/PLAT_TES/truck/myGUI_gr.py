
# coding = utf-8

from tkinter import *
from functools import partial
from tkinter.scrolledtext import *
import time
from socket import create_connection
from configparser import ConfigParser
from tkinter import tix
from tkinter import ttk
import queue

inert_message_queue = queue.Queue()
myLabel = partial(Label, width=10, height=2)
i = 0

config_para = ConfigParser()
config_para.read('para.ini')

MCT_IP = config_para.get('eNB', 'MCT')
platform_type = config_para.get('eNB', 'TYPE')
# print(platform_type)

notebook_map = {'IT': 10, 'MCT': 15, 'BBP': 15, 'GES': 5, 'FM':3}
testItem_MCT = ['PHY', 'CPLD', 'SWITCH', 'DDR', 'NandFlash', 'NorFlash', 'USB', '光模块', '温度', '功耗', 'GPS',
               'EEPROM', 'NULL1', 'NULL2', 'NULL3']
testItem_BBP = ['SWITCH', 'SRIO路由', 'SRIO数据', 'AIF', 'SDRAM', '温度', '功耗', 'EEPROM', 'DSP链路', 'DSP_DDR',
                '复位', 'FPGA加载', 'DSP加载', 'SRIO效率', 'NULL1']
testItem_GES = ['SWITCH', '模式', '温度', 'EEPROM', '复位']
testItem_FM = ['风速控制', '风速测试', '复位']
testItem_IT = ['下行_0', '上行_0', '下行_1', '上行_1', 'HMI_MB', 'HMI_MF', 'HMI_MG', 'AFC', '状态', '同步']


class MainGUI:
    
    def __init__(self):

        """
        tkinter root basic option
        """

        self.top = tix.Tk(className='平台测试_CZZ')
        self.top.geometry('=930x550+300+100')
        self.top.maxsize(930, 570)
        self.top.iconbitmap(default='myicon.ico')
        self.notebook_map = notebook_map

        '''
        variables in self
        '''
        self.value_dstIP = StringVar()
        self.value_eth3 = StringVar()
        self.value_dstIP.set(MCT_IP)
        self.value_date = StringVar()
        self.value_heartBeat = StringVar()
        self.value_heartBeat.set('板卡未连接')

        self.value = []
      
        self.value_testResult = StringVar()
        self.value_testResult.set('正在测试..')

        self.value_AFC = StringVar()

        j = 0
        while j < 10:
            a = IntVar()
            self.value.append(a)
            j += 1

        self.value_select_IT = IntVar()

        self.value_testTime = IntVar()
        self.value_testTime.set(1)

        '''
          main UI
        '''
        self.frame_item=LabelFrame(self.top, relief=FLAT)
        self.frame_item.grid(column=0, row=0, sticky=N, pady=5)

        self.notebook=ttk.Notebook(self.frame_item, padding=10)
        self.notebook.grid()

        self.frame_IT=Frame(self.notebook, name='it')
        self.frame_MCT=Frame(self.notebook, name='mct')
        self.frame_BBP=Frame(self.notebook, name='bbp')
        self.frame_GES=Frame(self.notebook, name='ges')
        self.frame_FM=Frame(self.notebook, name='fm')

        self.notebook.add(self.frame_IT, text='集成')
        self.notebook.add(self.frame_MCT, text='主控板')
        self.notebook.add(self.frame_BBP, text='基带板')
        self.notebook.add(self.frame_GES, text='交换板')
        self.notebook.add(self.frame_FM, text='风扇板')

        self.frame_control=LabelFrame(self.frame_item, relief=GROOVE, borderwidth=2, text='实验控制', foreground='blue')
        self.frame_control.grid(sticky=W, padx=10)

        self.entry_control_1=tix.LabelEntry(self.frame_control)
        self.entry_control_1.subwidget_list['label']['text'] = '试验次数：'
        self.entry_control_1.subwidget_list['entry']['textvariable']=self.value_testTime
        self.entry_control_1.grid(column=0, row=0, padx=10)

        self.button_control_1=ttk.Button(self.frame_control,text='设定', width=5)
        self.button_control_1.grid(column=1, row=0, padx=8)

        self.null_control_1=Label(self.frame_control)
        self.null_control_1.grid(column=2,row=0,padx=130)

        self.button_control_2=Button(self.frame_control,text='结果显示',relief='groove')
        self.button_control_2.grid(column=3,row=0,padx=5)
        
        self.msg_control_1=Message(self.frame_control,width=500,textvariable=self.value_testResult,justify='center',foreground='blue',font=('Arial',9))
        self.msg_control_1.grid(row=1,pady=5,stick=W,column=0,columnspan=4)


        self.frame_default=Frame(self.frame_item,relief=FLAT)
        self.frame_default.grid(sticky=W)

        self.label_default_1=myLabel(self.frame_default,text='  时间：')
        self.label_default_1.grid(column=0,row=0,sticky=W)
        self.label_default_3=Label(self.frame_default,textvariable=self.value_date,justify='left')
        self.dataShow()
        self.label_default_3.grid(column=1,row=0,pady=5,sticky=W)

        self.null_default_1=Label(self.frame_default)
        self.null_default_1.grid(column=2,row=0,padx=140,pady=10)

        self.label_default_2=myLabel(self.frame_default,textvariable=self.value_heartBeat,foreground='blue')
        self.label_default_2.grid(column=3,row=0,columnspan=2)
##        self.heartbeat()


        self.frame_log=LabelFrame(self.top,height=50,relief=GROOVE,borderwidth=2,text='日志',foreground='blue')
        self.frame_log.grid(column=1,row=0,sticky=N,pady=15,ipady=5)

        self.msg=ScrolledText(self.frame_log,width=47,height=28,font=('Arial',10))
        self.msg.grid(padx=10,pady=5,sticky=W,column=0,columnspan=2)
        # self.insert_message()

        self.button_log_1=ttk.Button(self.frame_log,text='保存',width=5,command=self.saveLog)
        self.button_log_1.grid(row=1,column=0,padx=10,sticky=W)

        self.button_log_2=ttk.Button(self.frame_log,text='清除',width=5,command=self.clearLog)
        self.button_log_2.grid(row=1,column=1,padx=10,sticky=E)
        
        '''
          IT UI
         '''
        self.frame_IT_1=LabelFrame(self.frame_IT,relief=GROOVE,borderwidth=2,text='系统信息',foreground='blue')
        self.frame_IT_1.grid(pady=10,padx=10,sticky=W)
        self.label_IT_1=myLabel(self.frame_IT_1,text='目标IP：')
        self.label_IT_1.grid(column=0,row=0,sticky=W)

        self.null_IT_1=Label(self.frame_IT_1)
        self.null_IT_1.grid(column=2,row=0,padx=100)

        self.button_IT_1=ttk.Button(self.frame_IT_1,text='连接',width=5)
        self.button_IT_1.grid(column=3,row=0,padx=5)
        
        
        self.entry_IT_1=Entry(self.frame_IT_1,width=22,textvariable=self.value_dstIP,font=('Arial',9))
        self.entry_IT_1.grid(column=1,row=0,pady=5)

        self.label_IT_2=myLabel(self.frame_IT_1,text=' ETH_3：')
        self.label_IT_2.grid(column=0,row=1,sticky=W)

        self.button_IT_2=ttk.Button(self.frame_IT_1,text='设置',width=5)
        self.button_IT_2.grid(column=3,row=1,padx=5)


        self.entry_IT_2=Entry(self.frame_IT_1, width=22, textvariable=self.value_eth3, font=('Arial',9))
        self.entry_IT_2.grid(column=1,row=1,pady=5)

        self.label_IT_3=myLabel(self.frame_IT_1,text='AFC状态：')
        self.label_IT_3.grid(column=0,row=2,sticky=W)
        self.entry_IT_3=Entry(self.frame_IT_1,width=22,font=('Arial',9),state='readonly',textvariable=self.value_AFC)
        self.entry_IT_3.grid(column=1,row=2,pady=5)

        self.frame_IT_2=LabelFrame(self.frame_IT,relief=GROOVE,borderwidth=2,text='测试项',foreground='blue')
        self.frame_IT_2.grid(sticky=W,padx=10)

        self.testItem_IT = testItem_IT

        self.cbName_IT=['IT_Cbutton%s'%(x) for x in range(10)]
        global i
        while i<10:
            
            self.cbName_IT[i]=Checkbutton(self.frame_IT_2,text=self.testItem_IT[i],variable=self.value[i])
            self.cbName_IT[i].grid(column=testList(2,5)[i][1],row=testList(2,5)[i][0],sticky=W,padx=16,pady=5)
            i = i+1

        self.frame_IT_3=LabelFrame(self.frame_IT,relief=FLAT)
        self.frame_IT_3.grid(column=0,row=2,sticky=W,padx=10,pady=5)

        self.null_IT_2=Label(self.frame_IT_3)
        self.null_IT_2.grid(column=0,row=0,padx=168)

        self.button_IT_3=ttk.Button(self.frame_IT_3,text='开始',width=5)
        self.button_IT_3.grid(column=2,row=0,padx=10)

        self.cbutton_IT_1=ttk.Checkbutton(self.frame_IT_3,text='全选',variable=self.value_select_IT,command=self.selectAll_IT)
        self.cbutton_IT_1.grid(column=1,row=0,padx=20)


    # def msgshow(self):
    #     showinfo('message','add function')

    def dataShow(self):
        data=time.ctime()

        moth={'Oct':10,'Jan':1,'Feb':2,'Mar':3,'Apr':4,'May':5,'Jun':6,'Jul':7,'Aug':8,'Sep':9,'Nov':11,'Dec':12}
        week={'Sat':'六','Tue':'二','Mon':'一','Wed':'三','Thu':'四','Fri':'五','Sun':'日'}

        b = data.split()

        if b[0] in week:
            weekShow = week[b[0]]

        if b[1] in moth:
            mothShow = moth[b[1]]

        data1=str(b[-1])+'-'+str(mothShow)+'-'+str(b[2])+' '+str(b[3])

        self.value_date.set(data1)
        self.top.after(1000,self.dataShow)

    def heartbeat(self):
        dstIP = self.value_dstIP.get()
        port=23

        try:
            b=create_connection((dstIP,port),timeout=3)
            if b:
                self.value_heartBeat.set('板卡已连接')
                b.close()

        except:
            self.value_heartBeat.set('板卡未连接')

    def selectAll_IT(self):
        for j in range(10):
            if self.value_select_IT.get()==1:
                self.cbName_IT[j].select()
            else:
                self.cbName_IT[j].deselect()

    def clearLog(self):
        self.msg.delete(1.0,'end')

    def saveLog(self):
        content=self.msg.get(1.0,'end')
        logName=self.value_date.get()
        logName=logName.replace(':','_')
        logName=logName.replace(' ','#')
        logName=logName+'.log'
        log=open('log\%s'%(logName),'a')
        log.write(content)
        self.msg.insert('end','----- 日志已保存! -----\n','INT')


def testList(row, col):
    itemList = []
    i = j = 0
    while i < row:
        while j < col:
            pos = (i, j)
            itemList.append(pos)
            j += 1

        i += 1
        j = 0
    return itemList

# class can use for all pages in notebook


class pageSetUp:

    def __init__(self,master,row,column,item,padx_cb,real):
        self.master=master
        self.row=row
        self.column=column
        self.total=row*column
        self.real=real

        self.value=[]
        i=0
        while i<self.total:
            a=IntVar()
            self.value.append(a)
            i+=1
    
        self.value_select=IntVar()
        self.eeprom_select=IntVar()
        
        self.value_eeprom=[StringVar() for i in range(10)]

        self.key_eeprom=['设备代号','硬件版本','  MAC1','  MAC2','生产序列','制造商','生产日期','卫星型号','风扇转速','工作温度']

        self.value_var=IntVar()
        self.value_var1=IntVar()     
            
        self.frame_1=LabelFrame(self.master,relief=GROOVE,borderwidth=2,text='单板信息',foreground='blue')
        self.frame_1.grid(pady=10,padx=10)
        
        self.option=['eeprom_test_key%s'%(x) for x in range(10)]
        self.entry=['eeprom_test_value%s'%(x) for x in range(10)]

        for i,name in enumerate(self.key_eeprom):
            if i%2==0:a,b=1,2
            elif i%2==1:a,b=4,5
                
            self.option[i]=Label(self.frame_1,text=name)
            self.option[i].grid(column=a,row=int(i/2),sticky=W)
            self.entry[i]=Entry(self.frame_1,width=20,textvariable=self.value_eeprom[i],font=('Arial',9))
            self.entry[i].grid(column=b,row=int(i/2),pady=5)
##            print(self.option[i]['state'])

        self.entry[6]['state']='readonly'
        self.entry[0].insert(1,'C6448')
        self.entry[9].insert(1,'               -25 ~ 55')

        self.null_1=Label(self.frame_1)

        self.null_2=Label(self.frame_1)
        self.null_2.grid(column=6,row=0,padx=5)
        self.null_0=Label(self.frame_1)
        self.null_0.grid(column=0,row=0,padx=(padx_cb/2-4))

        self.button_1=ttk.Button(self.frame_1,text='读',width=3)
        self.button_1.grid(column=2,row=5,padx=20,pady=5)

        self.button_2=ttk.Button(self.frame_1,text='写',width=3)
        self.button_2.grid(column=5,row=5,sticky=E)

        self.frame_2=LabelFrame(self.master,relief=GROOVE,borderwidth=2,text='测试项',foreground='blue')
        self.frame_2.grid(sticky=W,padx=10)

        self.testItem=item
        
        self.cbName=['BBP_Cbutton%s'%(x) for x in range(self.total)]
        
        i=0
        while i<self.total:
            
            self.cbName[i]=Checkbutton(self.frame_2,text=self.testItem[i],variable=self.value[i])
            self.cbName[i].grid(column=testList(self.row,self.column)[i][1],row=testList(self.row,self.column)[i][0],sticky=W,padx=padx_cb,pady=5)
            i = i+1

        self.frame_3 = LabelFrame(self.master,relief=FLAT)
        self.frame_3.grid(column=0,row=2,sticky=W,padx=10,pady=5)

        self.null_3 = Label(self.frame_3)
        self.null_3.grid(column=0,row=0)

        self.null_4=Label(self.frame_3)
        self.null_4.grid(column=1,row=0)

        self.null_5=Label(self.frame_3)
        self.null_5.grid(column=4,row=0,padx=100)

        self.button_3=ttk.Button(self.frame_3,text='开始',width=5)
        self.button_3.grid(column=7,row=0,padx=10)

        self.cbutton_1=ttk.Checkbutton(self.frame_3,text='全选',variable=self.value_select,command=self.selectAll)
        self.cbutton_1.grid(column=6,row=0,padx=30)

    def selectAll(self):
        for j in range(self.real):
            if self.value_select.get()==1:
                self.cbName[j].select()
            else:
                self.cbName[j].deselect()


GUI = MainGUI()
MCT=pageSetUp(GUI.frame_MCT,3,5,testItem_MCT,15,12)
MCT.cbName[14].destroy()
MCT.cbName[13].destroy()
MCT.cbName[12].destroy()
MCT.option[8]['state']='disabled'
MCT.null_3=Checkbutton(MCT.frame_3,text='CPLD升级',variable=MCT.value_var)
MCT.null_3.grid(column=0,row=0,padx=15)
MCT.null_1.grid(column=3,row=0,padx=28)


BBP=pageSetUp(GUI.frame_BBP,3,5,testItem_BBP,12,10)
BBP.cbName[10]['state']='disable'
BBP.cbName[11]['state']='disable'
BBP.cbName[12]['state']='disable'
BBP.cbName[13]['state']='disable'
BBP.option[8]['state']='disable'
BBP.option[7]['state']='disable'
BBP.option[2]['state']='disable'
BBP.option[3]['state']='disable'
BBP.cbName[14].destroy()
BBP.null_3=Checkbutton(BBP.frame_3,text='CPLD升级',variable=BBP.value_var)
BBP.null_3.grid(column=0,row=0,padx=12)

BBP.null_4=Checkbutton(BBP.frame_3,text='MCU升级',variable=BBP.value_var1)
BBP.null_4.grid(column=1,row=0)

BBP.null_5.grid(column=4,row=0,padx=72)
BBP.null_1.grid(column=3,row=0,padx=28)


##TRX=pageSetUp(GUI.frame_TRX,2,4,testItem_GES)
GES=pageSetUp(GUI.frame_GES,1,5,testItem_GES,20,4)
GES.cbName[4]['state']='disable'

GES.null_3['text']='模式：'

GES.null_4=ttk.Combobox(GES.frame_3,width=6,values=['自适应','十兆','百兆','千兆'],font=('Arial',9),justify='center')
GES.null_4.grid(column=1,row=0)
GES.null_4.set('自适应')

GES.null_6=Label(GES.frame_3,text='端口：')
GES.null_6.grid(column=2,row=0)

GES.null_7=ttk.Combobox(GES.frame_3,width=2,values=[0,1,2,3],font=('Arial',9),justify='center')
GES.null_7.grid(column=3,row=0)
GES.null_7.set(0)

GES.mcu_update = Checkbutton(GES.frame_3,text='MCU升级',variable=GES.value_var1)
GES.mcu_update.grid(column=4,row=0, padx=25)

GES.option[8]['state']='disable'
GES.option[7]['state']='disable'
GES.option[2]['state']='disable'
GES.option[3]['state']='disable'

GES.null_5.grid(column=5,row=0,padx=3)
GES.null_1.grid(column=3,row=0,padx=25)

FM=pageSetUp(GUI.frame_FM, 1, 3, testItem_FM, 47, 2)
FM.cbName[-1]['state']='disable'

FM.null_3['text']='速率：'
    
FM.null_4=tix.Control(FM.frame_3,max=100,min=1,value=1)
FM.null_4.grid(column=1,row=0)

FM.null_6=Label(FM.frame_3,text='通道：')
FM.null_6.grid(column=2,row=0)

FM.null_7=ttk.Combobox(FM.frame_3,width=3,values=['ALL',0,1,2],font=('Arial',9),justify='center')
FM.null_7.grid(column=3,row=0)
FM.null_7.set('ALL')

FM.null_5.grid(column=4,row=0,padx=70)
FM.null_0.grid(column=0,row=0,padx=5)
FM.null_1.grid(column=3,row=0,padx=25)

FM.option[7]['state']='disable'
FM.option[2]['state']='disable'
FM.option[3]['state']='disable'


##BP=pageSetUp(GUI.frame_BP,2,4,testItem_GES)


# mainloop()

##def main():
##    msggui()
##    mainloop()
##
##    
##
##if __name__=='__main__':
##    main()

