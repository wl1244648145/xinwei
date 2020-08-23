import getpass
import telnetlib, time, threading
import serial, os

serial4=serial.Serial()
serial4.baudrate = 115200
serial4.port = 0

serial4.open()
reset_flag = 0
read_flag = 0

HOST = "172.33.12.201"
userName = 'root'
passWord = '12345678'

cpldr54 = 'bsp_cpld_read_reg 54'
cpldr55 = 'bsp_cpld_read_reg 55'
cpldr56 = 'bsp_cpld_read_reg 56'
cpldr57 = 'bsp_cpld_read_reg 57'
cpldr71 = 'bsp_cpld_read_reg 71'

cpldw58_0 = 'call bsp_cpld_write_reg(58,0)'
cpldw58_1 = 'call bsp_cpld_write_reg(58,1)'
cpldw64 = 'call bsp_cpld_write_reg(64,1)'

fpgar192 = 'bsp_fpga_read_addr 192\n'
fpgar193 = 'bsp_fpga_read_addr 193\n'

fpgaw139 = 'bsp_fpga_write_addr 139,1\n'
fpgaw141 = 'bsp_fpga_write_addr 141,1\n'
fpgaw143 = 'bsp_fpga_write_addr 143,1\n'
fpgaw11  = 'bsp_fpga_write_addr 11,0xffff\n'

fpgar11 = 'bsp_fpga_read_addr 11\n'
fpgar139 = 'bsp_fpga_read_addr 139\n'
fpgar141 = 'bsp_fpga_read_addr 141\n'
fpgar143 = 'bsp_fpga_read_addr 143\n'

cpldw26_1 = 'bsp_cpld_write_reg 26,1\n'
cpldw26_0 = 'bsp_cpld_write_reg 26,0\n'

cpldw8_1 = 'bsp_cpld_write_reg 8,8\n'
cpldw8_0 = 'bsp_cpld_write_reg 8,0\n'

DSP4READY = 'slot5 dsp4 ready'
DSP3READY = 'slot5 dsp3 ready'
DSP2READY = 'slot5 dsp2 ready'
DSP1READY = 'slot5 dsp1 ready'

trxPLL = 'lmk04808_ReadLock 1\n'
boot_message = 'U-Boot 2011.12-gc6d9d50 (Jul 28 2016 - 18:42:03)\n'

EMREBOOT='power_off\n'
BTSA = 'cd /mnt/btsa'
GDB = './gdb - 880'
SRIO_STATUS = 'Bsp_Get_Cps1616_Port_Status'
BBP_RESET = 'bsp_bbp_reset 2\n'

succTime = 0
pllSendTime = 1
irState = 0
regValue = 0
fpga_516_value = 0
issue_flag = 0


def serial_log_record_dspReady():
    global issue_flag,reset_flag,read_flag
    i = 1
    while 1:
        file = open(r'%d.txt'%(i),'ab')
        line = serial4.readline()
        if DSP1READY in line:
            issue_flag += 1
            print(line)
        elif DSP2READY in line:
            issue_flag += 1
            print(line)
        elif DSP3READY in line:
            issue_flag += 1
            print(line)
        elif DSP4READY in line:
            issue_flag += 1
            print(line)
        else:
            print(line)
    
        file.write(line)
        if os.path.getsize(r'%d.txt' % (i)) >= 5000000:
            file.close()
            i += 1

def serial_log_record_302issue():
    global issue_flag,reset_flag,read_flag
    i = 1
    while 1:
        file = open(r'%d.txt'%(i),'ab')
        line = serial4.readline()
        if b'c0000302' in line:
            issue_flag = 1
            print(line)
        else:
            print(line)
    
        file.write(line)
        if os.path.getsize(r'%d.txt' % (i)) >= 5000000:
            file.close()
            i += 1
            

def emreboot():
    i=1
    while 1:
        for char in EMREBOOT:
            serial4.write(char.encode('ascii'))
            time.sleep(1)
        print('整机掉电%d次'%(i))
        time.sleep(300)
        i+=1

def logRead():
    while 1:
#       file=open(r'%d.txt'%(i),'ab')
        line=serial4.readline()
        if (b'value = 1=0x' in line) or (b'value = 0=0x' in line):
            lineStr=line.decode('ascii')
            lineExp=lineStr.split('=')[-1]
            lineExp.strip().strip()
            pllstate=lineExp.split('x')[-1]
            
            if int(pllstate)==1:
                global succTime,pllSendTime
                succTime += 1
            print('PLL状态:%d[%d]'%(int(pllstate),pllSendTime))
            pllSendTime += 1

        elif b'ret =' in line:
            lineStr=line.decode('ascii')
            lineExp=lineStr.split('(')[-1]
            lineExp.strip().strip()
            fpgaReg=int(lineExp[:-3])
            print(lineStr)
            if fpgaReg != 26 and fpgaReg != 8:
                global fpga_516_value
                fpga_516_value = fpgaReg
                



def force_pll():
    i=0
    while i < 5:
        serial4.write(trxPLL.encode())
        time.sleep(3)
        i=i+1
#    serial4.write('fpgaw 1,7,0\r\n'.encode())
#    time.sleep(1)
#    serial4.write('fpgar 1,7\r\n'.encode())


def force_frame():
    while 1:
        if reset_flag == 1:
            break

        serial4.write('reset\n'.encode())
        time.sleep(70)
        serial4.write('bsp_reset_fpga\n'.encode())
        time.sleep(1)
        serial4.write(fpgaw139.encode())
        time.sleep(0.5)
        serial4.write(fpgaw141.encode())
        time.sleep(0.5)
        serial4.write(fpgaw143.encode())
        time.sleep(0.5)

        serial4.write(fpgar139.encode())
        time.sleep(1)
        serial4.write(fpgar141.encode())
        time.sleep(1)
        serial4.write(fpgar143.encode())
        time.sleep(1)
        
        serial4.write(fpgaw11.encode())
        time.sleep(5)
        serial4.write(fpgar11.encode())
        global read_flag
        read_flag = 1
        time.sleep(5)
        read_flag = 0

def force_srio_status():
    while 1:
        global issue_flag
        issue_flag = 0
        i=1
        serial4.write(BBP_RESET.encode())
        print('基带板复位%d次\n'%i)
        i += 1
        time.sleep(300)
        print('302issue %d\n'% issue_flag)
        if issue_flag >0:
            break
        


class BTS_Telnet(telnetlib.Telnet):
    
    def read_anytime(self):
        i=0
        while i<4:
            self.fill_rawq()
            self.process_rawq()
            buf=self.cookedq
            print(buf)     
            self.cookedq = b''
            i += 1

        return buf

    def write_anytime(self,a):
        i=0
        while i<a:
            print('--'*20,i)
            time.sleep(1)
            i=i+1
            

def telnet2input():
    tn = BTS_Telnet(HOST)
    ##tn.debuglevel=1

    tn.read_until(b"login: ")
    tn.write(userName.encode('ascii')+b"\n")

    tn.read_until(b"Password: ")
    tn.write(passWord.encode('ascii')+ b"\n")

    tn.write(b"pidof 'lte_app'" + b"\n")
    a=tn.read_anytime().decode('ascii')
    ltePid=a.split('\r\n')[1]

    print('lte PID = %d'%(int(ltePid)))

    fileDir='/mnt/btsa/gdb - %s'%(ltePid)
    time.sleep(1)
    tn.write(fileDir.encode('ascii') + b"\n")
    time.sleep(1)
 
    return tn


def start_th_logRec():
    th_logRec=threading.Thread(target=serial_log_record_302issue)
    th_logRec.start()


def test_rebootBTS():
    i=0

    while i < 3:
        print(time.ctime())
        time.sleep(60)
        i += 1
    tn = telnet2input()
    tn.write(cpldw54.encode('ascii') + b"\n")
    time.sleep(1)
    tn.write(cpldw55.encode('ascii') + b"\n")
    time.sleep(1)
    tn.write(cpldw56.encode('ascii') + b"\n")
    time.sleep(1)
    tn.write(cpldw57.encode('ascii') + b"\n")
    time.sleep(1)
    tn.write(cpldw58_0.encode('ascii') + b"\n")
    time.sleep(1)
    tn.write(cpldw58_1.encode('ascii') + b"\n")

    force_frame()
    while i < 4:
        print(time.ctime())
        time.sleep(60)
        i += 1
    if regValue == 0:
        return

    else:
        print('FPGA寄存器和：%d'%(regValue))
        tn.write(cpldw64.encode('ascii') + b"\n")
        time.sleep(1)


def test_reboot_bbu():
    i=0
    tn = telnet2input()
    tn.write(b"call bsp_bbp_reset()" + b"\n")
    
    while i<1:
        print(time.ctime())
        time.sleep(60)
        i+=1
    force_pll()


def cpld_reboot():
    global issue_flag
    while 1:
        print('[启动次数 = %d！]'% (issue_flag))
        if issue_flag > 1:
            break
        serial4.write(BTSA.encode() + b'\n')
        time.sleep(1)
        serial4.write(GDB.encode() + b'\n')
        time.sleep(5)
        serial4.write(cpldw64.encode() + b'\n')
        issue_flag = 0
        i = 0
        while i <10:
            time.sleep(60)
            print('[等待%d分钟 ！]'%(i))
            i += 1

def main():

    start_th_logRec()
    # cpld_reboot()
##    force_frame()
    force_srio_status()


if __name__=='__main__':
    main()
##    telnet2input()



        
