#coding=utf-8
import csv
import sys
import os
import time

RESET='U-Boot 2011.12'
FILENAME = 'C:\\Documents and Settings\\xuqiang\桌面\\temp\\com1(2017-5-8 17_38_07).txt'
SRIO_STATUS = 'srio port status:'   
DSP4READY = 'slot5 dsp4 ready'
DSP3READY = 'slot5 dsp3 ready'
DSP2READY = 'slot5 dsp2 ready'
DSP1READY = 'slot5 dsp1 ready'
FPGA160T = 'fsa_fpga_160t load success'
FPGA325T = 'fsa_fpga_325t load success'
FPGA_ES = '[192.168.6.100:6]fpga load success'
RST_CAUSE = 'open mem device'
ETH_STATUS = 'ppc_ethsw_get_port_status'

AFC_LOCK = 'AFC_FAST ---> AFC_LOCK'

PPC_ETH0 = 'PPC_ETHNET0_PORT is link up'
PPC_ETH1 = 'PPC_ETHNET1_PORT is link up'
PPC_ETH2 = 'PPC_ETHNET2_PORT is link up'
PPC_ETH3 = 'PPC_ETHNET3_PORT is link up'
PPC_IP1 = 'IPRAN_PHY1_PORT is link up'
PPC_IP2 = 'IPRAN_PHY2_PORT is link down'
PPC_TRAC = 'TRACERAN_PHY_PORT is link down'
PPC_SLOT2 = 'PPC_SLOT2_PORT is link down'
PPC_SLOT3 = 'PPC_SLOT3_PORT is link down'
PPC_SLOT4 = 'PPC_SLOT4_PORT is link down'
PPC_SLOT5 = 'PPC_SLOT5_PORT is link up'
PPC_SLOT6 = 'PPC_SLOT6_PORT is link up'
PPC_SLOT7 = 'PPC_SLOT7_PORT is link up'


def reset_summay():
   
    file = open(FILENAME, encoding='latin-1').readlines()
    csv_file = open('result.csv', 'w', newline='')
    csv_writer = csv.writer(csv_file)
    head = ['测试次数','日志行号','基带板dsp1','基带板dsp2','基带板dsp3','基带板dsp4','基带板srio',
            'es板fpga','fs板fpga325t','fs板fpga160t','复位原因','ppc ethsw','AFC']
    csv_writer.writerow(head)
    lineno = []
    ppc_eth_status = [PPC_ETH0, PPC_ETH1, PPC_ETH2, PPC_ETH3, PPC_IP1, PPC_IP2, PPC_TRAC, PPC_SLOT2,
                      PPC_SLOT3, PPC_SLOT4, PPC_SLOT5, PPC_SLOT6, PPC_SLOT7]
    i = 0
    j = 0
    while i<len(file):
        if RESET in file[i]:
            lineno.append(i)
            
        i=i+1
    print('共压力测试%d次' % len(lineno))

    while j < len(lineno)-1:
        content = file[lineno[j]:lineno[j+1]]
        logbuf = ['NULL']*len(head)
        logbuf[0] = j+1
        logbuf[1] = lineno[j]
        for message in content:
            if SRIO_STATUS in message:
                logbuf[6] = message
            elif DSP4READY in message:
                logbuf[5] = message
            elif DSP3READY in message:
                logbuf[4] = message
            elif DSP2READY in message:
                logbuf[3] = message
            elif DSP1READY in message:
                logbuf[2] = message
            elif FPGA160T in message:
                logbuf[9] = message
            elif FPGA325T in message:
                logbuf[8] = message
            elif FPGA_ES in message:
                logbuf[7] = message
            elif RST_CAUSE in message:
                index = content.index(message)
                reset_cause = content[index+1:index+3].__repr__()
                logbuf[10] = reset_cause
            elif ETH_STATUS in message:
                index = content.index(message)
                ethsw_status = content[index+1:index+32].__repr__()
                for status in ppc_eth_status:
                    if status not in ethsw_status:
                        logbuf[11] = 'NULL'
                        break
                    else:
                        logbuf[11] = 'OK'
            elif AFC_LOCK in message:
                logbuf[12] = 'OK'
            else:
                pass
        csv_writer.writerow(logbuf)
        
        j += 1


def afc_modulate_byte(filename):
    destination = b'30s='
    count = 0
    e = 0
    fd = open(filename, 'rb')
    fd_log = open('afc.log', 'w', encoding='latin-1')

    for line in fd:
        if destination in line:
            count += 1
            line_string = line.decode('latin-1')
            i_p30s = line_string.index('30s')
            p_line = line_string[: i_p30s]
            p_30s = line_string[i_p30s: i_p30s + 10].split('=')[-1].strip()
            try:
                i_p1s = p_line.index('1s')
                p_1s = p_line[i_p1s: i_p1s + 8].split('=')[-1].strip()
            except ValueError:
                p_1s = 0
                fd_log.write(line_string)
                e += 1
            f_line = line_string[i_p30s: ]
            try:
                i_f1s = f_line.index('1s')
                f_1s = f_line[i_f1s: i_f1s+7].split('=')[-1].strip()
            except ValueError:
                f_1s = 0
                fd_log.write(line_string)
                e += 1
            try:
                i_f20s = f_line.index('20s')
                f_20s = f_line[i_f20s: i_f20s+11].split('=')[-1].strip()
            except ValueError:
                f_20s = 0
                fd_log.write(line_string)
                e += 1
            try:
                if (abs(float(p_1s)) >= 25) or (abs(float(p_30s)) >= 20) or \
                        (abs(float(f_1s)) >= 3 or (abs(float(f_20s)) >= 0.2)):
                    fd_log.write(line_string)
            except ValueError:
                fd_log.write(line_string)
                e += 1

    print('count=', count, 'except=', e)


def judge(file, initial):
    a = initial
    l = []
    fd = open(file, encoding='latin-1').readlines()
    for line in fd:
        num_row = line.split('->')
        if len(num_row)>2:
            continue
        num = num_row[-1].strip()
        try:
            number = int(num)
        except ValueError:
            print(line, fd.index(line))
            continue

##        if number == 39:
##            print(line)
        if number>59 or number<0:
            print(line, fd.index(line))
            continue
        l.append(number)
        
##        if a != number:
##            print(line)
##            a = number+1
##        else:
##            a += 1
    print('max = %d, min = %d' % (max(l), min(l)))

        
              
    
if __name__ == '__main__':
##    afc_modulate_byte('C:\\Documents and Settings\\xuqiang\\桌面\\temp\\COM1(2017-8-16 15_57_58)_beidou_afc_stress.txt')
    judge('C:\\Documents and Settings\\xuqiang\\桌面\\6t_sec.txt', 23)


