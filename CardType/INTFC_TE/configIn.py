#-*- coding:utf-8 -*-
from winpcapy_master.winpcapy import *
from winpcapy_master.winpcapy_types import *
from ctypes  import *
import platform
# from pip.backwardcompat import raw_input

if platform.python_version()[0] =="3":
    raw_input = input


def configInterface(delay=20):
    alldevs = POINTER(pcap_if_t)()
    errbuf = create_string_buffer(PCAP_ERRBUF_SIZE)
    if pcap_findalldevs_ex(str(PCAP_SRC_IF_STRING).encode('ascii'),None,alldevs,errbuf)== -1:
        print ("Error in pcap_findalldevs()")
        
    i=0
    d = alldevs.contents
    while d:
        i = i+1
    #        print("%d. %s" %(i,d.name))
        if (d.description):
            pass
    #            print ("(%s)\n" % (d.description))
        else:
            pass
    #            print (" (No description available)\n")
        if d.next :
            d = d.next.contents
        else:
            d = False
    if (i==0):
    #        print ("\nNo interfaces found! Make sure WinPcap is installed.\n")
        sys.exit(-1)
    
    #   print "Enter the interface number (1-%d):" % i
    
     
    d = alldevs
    for i in range(0,i):
        if (str("VM").encode('ascii') in d.contents.description) or (str("VPN").encode('ascii')  in d.contents.description):
            d = d.contents.next
        else:
            break
    #===========================================================================
    # try:
    #===========================================================================
        if d.contents.name==None:
            print ("%device is empty" % i)
    #===========================================================================
    # except Exception, e:
    #     
    #     print  ("%device is empty" % i)
    #===========================================================================
    
    adhandle = pcap_open(d.contents.name,65536,PCAP_OPENFLAG_PROMISCUOUS,delay,None,errbuf)
    if(adhandle == None):
        print ("\nUnable to open the adapter. %s is not supported by WinPcap\n" % d.contents.name)
        pcap_freealldevs(alldevs)
        sys.exit(-1)
    if pcap_datalink(adhandle)!=1:
        pcap_freealldevs(alldevs)
        sys.exit(-1)
    return (adhandle,alldevs)

def filterPacket(adhandle,alldevs,filterCondition):
    fcode = bpf_program()
    NetMask = 0xffffff
    filter = filterCondition      
    if pcap_compile(adhandle,byref(fcode),filter,1,NetMask)<0:
        print ('\nError compiling filter: wrong syntax.\n')
        pcap_freealldevs(alldevs)
        sys.exit(-1)
     
    if pcap_setfilter(adhandle,byref(fcode))<0 :
        print('\nError setting the filter\n')
        pcap_freealldevs(alldevs)
        sys.exit(-1)  

def getPacket(adhandle):
    header = POINTER(pcap_pkthdr)()
    pkt_data = POINTER(u_char)()
    while True:
        res =pcap_next_ex(adhandle,header,pkt_data)
        
        if res==0:
            continue
        elif res>0:
            b = [pkt_data[i] for i in range(header.contents.len)]   
            flag = 1
            #===================================================================
            # print [hex(b[i]) for i in range(len(b))]
            #===================================================================
            break                   
                       
    retVal = [pkt_data[i] for i in range(header.contents.len)]
    return (flag,retVal)
def sendPacket(adhandle,data):
    packet = (c_ubyte*(len(data)*2))()
    i = 0
    for bit in data:
        packet[i] = bit>>8
        packet[i+1] = bit&0xff
        i+=2
         
    
    flag = pcap_sendpacket(adhandle,packet,len(data)*2)
    return flag

def freeDevice(alldevs):
    pcap_freealldevs(alldevs)