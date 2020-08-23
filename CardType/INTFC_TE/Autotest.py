"""An SignalAnalyzer class and some helper functions.

NOTE:  The ROHDE&SCHWARZ SignalAnalzer's port is 5025, different from Aglinet's port 5023

Only tested on Agilent N9000A CXA Signal Analyzer.

Note:
It's user's responsibility adding delay between calling these helper functions

Here is a nice test:
python signalanalyzerlib.py 192.168.1.3 5023
"""


import sys
import os
import time
import struct
import configInterface as conf
from configparser import ConfigParser

try:
    import SOCKS; socket = SOCKS; del SOCKS
except ImportError:
    import socket

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

__all__ = ["SA"]
__version__ = '0.1'

# Line terminators (we always output CRLF, but accept any of CRLF, CR, LF)
CRLF = '\r\n'


# The class itself
class SA:
  

    '''An Signal Analyzer connection class.

    To create a connection, call the class using these arguments:
        host, port
'''

    debugging = 0
    host = ''
    port = 0 
    sock = None
    file = None
    welcome = None

    # Initialization method (called by class instantiation).
    def __init__(self, host='', port=0):
        if host:
            self.connect(host, port)
    
    def connect(self, host='', port=0):
        '''Connect to host(SA).  Arguments are:
        - host: hostname to connect to (string)
        - port: port to connect to (integer)'''
        if host != '':
            self.host = host
        if port > 0:
            self.port = port
        self.sock = socket.create_connection((self.host, self.port))
        self.file = self.sock.makefile('rb')
    
        
#self.welcome = self.getresp()
#return self.welcome 

    def getwelcome(self):
        '''Get the welcome messgage from the analyzer.
        (this is read and squirreled away by connect())'''
        if self.debugging:
            print('*welcome*', repr(self.welcome))
        return self.welcome

    def set_debuglevel(self, level):
        '''Set the debugging level.
        The required argument level means:
        0: no debugging output(default)
        1: print commands and responses but not body text etc.
        2: also print raw lines read and sent before stripping CR/LF'''
        self.debugging = level
    debug = set_debuglevel

    # Internal: return one line from the analyzer, stripping CRLF.
    # Raise EOFError if the connection is closed.
    def getline(self,s):
        #line = self.file.readline()
        #print(line)
        #s=self.skipprompt()
        # if self.debugging > 1:
        #     print('*get*', repr(line))
        if not s: raise EOFError
        if s[-2:] == CRLF: s = s[:-2]
        elif s[-1:] in CRLF: s = s[:-1]
        return s

    # Internal: get a response from the analyzer, which may possibly
    # consist of multiple lines.  Return a single string with no
    # trailing CRLF.  If the response consists of multiple lines,
    # these are separated by '\n' characters in the string
    def getmultiline(self,s):
        line = self.getline(s)
##        print('getmultiline:  ',line)
        if '?' in line: # query cmd contains a '?', so does the response
            nextline = self.skipprompt()
            line = line + ('\n' + nextline)
##            print('-'*30,line)
        return line

    # Internal: get a response from the analyzer.
    def getresp(self,s):
        resp = self.getmultiline(s)
        if self.debugging: print('*resp*', repr(resp))
        return resp

    # Internal: send one line to the analyzer, appending CRLF
    def putline(self, line):
        line = (line + CRLF).encode()
        if self.debugging > 1: print('*put*', repr(line))
        self.sock.sendall(line)
    
    # Internal: send one command to the analyzer (through putline())
    def putcmd(self, line):
        if self.debugging: print('*cmd*', repr(line))
        self.putline(line)
        #print(line)

    def sendcmd(self, cmd):
        '''Send a command and return the response.'''
        self.putcmd(cmd)
        return self.getresp()

    # Internal: skip 'SCPI>' prompt in the response and strip
    def skipprompt(self):
        s=self.file.readline()
##        print(s)
        if s[:6] == 'SCPI> ' or s[:6] == 'scpi> ':
            s = s[6:]
        # FIXME: Don't know why such strange bytes always in the first response
        if b'\xff\xfb\x01\xff\xfb\x03' == s[:6]: 
            s = s[6:]
           # print(s)
        
        return s.strip().decode()
        
    
    # Internal: get result from multiline response
    def extractresult(self, s):
        return s

    def voidcmd(self, cmd):
        '''Send a command and expect a response equal with the command.'''
        self.putcmd(cmd)
#self.skipprompt(self.getresp()) == cmd)

    def resultcmd(self, cmd):
        '''Send a command and expect a response which is a number.'''
        self.putcmd(cmd)
        return self.extractresult(self.getresp(self.skipprompt()))

    def close(self):
        '''Close the connection without assuming anything about it.'''
        if self.file:
            self.file.close()
            self.sock.close()
            self.file = self.sock = None

    # Check "Agilent X-Series Signal Analyzer User's and Programmer's Reference"
    # for the details of the following helper functions
    def rst(self):
        cmd = '*RST '
        self.voidcmd(cmd)
        
    #FREQKEY    
    def freq_cent(self, freq, unit='M'):
        cmd = 'FREQ:CENT ' + str(freq) + unit + 'Hz'
        self.voidcmd(cmd)

    def freq_start(self, freq, unit='K'):
        cmd = 'FREQ:START ' + str(freq) + unit + 'Hz'
        self.voidcmd(cmd)

    def freq_stop(self, freq, unit='K'):
        cmd = 'FREQ:STOP ' + str(freq) + unit + 'Hz'
        self.voidcmd(cmd)

    #SPANKEY
    def freq_span(self, span, unit='M'):
        cmd = 'FREQ:SPAN ' + str(span) + unit + 'Hz'
        self.voidcmd(cmd)

    def freq_span_full(self):
        cmd = 'FREQ:SPAN:FULL' 
        self.voidcmd(cmd)
        
    #AMPTKEY    
    def inp_att(self, lev):
        cmd = 'INP:ATT ' + str(lev) + 'dB'
        self.voidcmd(cmd)

    def inp_att_auto_on(self):
        cmd = 'INP:ATT:AUTO ON ' 
        self.voidcmd(cmd)    

    def disp_wind_trac_y_rlev(self, lev):
        cmd = 'DISP:WIND:TRAC:Y:RLEV ' + str(lev) + 'dBm'
        self.voidcmd(cmd)
        
    def disp_wind_trac_y_rlev_offs(self, lev):
        cmd = 'DISP:WIND:TRAC:Y:RLEV:OFFS ' + str(lev) + 'dB'
        self.voidcmd(cmd)

    #BWKEY
    def band(self, width, unit='K'):
        cmd = 'BAND ' + str(width) + unit + 'Hz'
        self.voidcmd(cmd)

    def band_vid(self, width, unit='K'):
        cmd = 'BAND:VID ' + str(width) + unit + 'Hz'
        self.voidcmd(cmd)
    #SWEEPKEY
    def init_cont_on(self):
        cmd = 'INIT:CONT ON'
        self.voidcmd(cmd)
        
    def init_cont_off(self):
        cmd = 'INIT:CONT OFF'
        self.voidcmd(cmd)

    def init_conm(self):
        cmd = 'INIT:CONM'
        self.voidcmd(cmd)
        
    def swe_time_auto_on(self):
        cmd = 'SWE:TIME:AUTO ON '
        self.voidcmd(cmd)

    def swe_time(self, times):
        cmd = 'SWE:TIME ' + str(times)+ 'ms'
        self.voidcmd(cmd)
        
    #TRACEKEY
    def disp_trac1_mode_maxh(self):
        cmd = 'DISP:TRAC1:MODE MAXH '
        self.voidcmd(cmd)

    def disp_trac1_mode_minh(self):
        cmd = 'DISP:TRAC1:MODE MINH '
        self.voidcmd(cmd)

    def disp_trac1_mode_average(self):
        cmd = 'DISP:TRAC1:MODE AVERAGE '
        self.voidcmd(cmd)
        
    def disp_trac1_mode_writ(self):
        cmd = 'DISP:TRAC1:MODE WRIT '
        self.voidcmd(cmd)    
    
    def det_rms(self):
        cmd = 'DET RMS '
        self.voidcmd(cmd)
        
    def det_aver(self):
        cmd = 'DET AVER '
        self.voidcmd(cmd)
        
    #TRIGKEY    
    def trig_sour_ext(self):
        cmd = 'TRIG:SOUR EXT'
        self.voidcmd(cmd)
        
    def trig_sour_imm(self):
        cmd = 'TRIG:SOUR IMM'
        self.voidcmd(cmd)

    def trig_sour_vid(self):
        cmd = 'TRIG:SOUR VID'
        self.voidcmd(cmd)    
        
    def trig_slop_pos(self):
        cmd = 'TRIG:SLOP POS'
        self.voidcmd(cmd)
        
    def trig_slop_neg(self):
        cmd = 'TRIG:SLOP NEG'
        self.voidcmd(cmd)

    def trig_hold(self, times):
        cmd = 'TRIG:HOLD ' + str(times)+ 'us'
        self.voidcmd(cmd)    
        
    def swe_egat_on(self):
        cmd = 'SWE:EGAT ON'
        self.voidcmd(cmd)
        
    def swe_egat_type_edge(self):
        cmd = 'SWE:EGAT:TYPE EDGE'
        self.voidcmd(cmd)
        
    def swe_egat_hold(self, times):
        cmd = 'SWE:EGAT:HOLD ' + str(times)+ 'us'
        self.voidcmd(cmd)
        
    def swe_egat_leng(self, times):
        cmd = 'SWE:EGAT:LENG ' + str(times)+ 'us'
        self.voidcmd(cmd)

    #MEASKEY
    def calc_mark_func_pow_off(self):
        cmd = 'CALC:MARK:FUNC:POW OFF'
        self.voidcmd(cmd)
        
    def calc_mark_func_pow_sel_acp(self):
        cmd = 'CALC:MARK:FUNC:POW:SEL ACP'
        self.voidcmd(cmd)
        
    def pow_ach_acp(self,num):
        cmd = 'POW:ACH:ACP '+str(num)
        self.voidcmd(cmd)
        
    def pow_ach_bwid_chan1(self, width, unit='M'):
        cmd = 'POW:ACH:BWID:CHAN1 ' + str(width) + unit + 'Hz'
        self.voidcmd(cmd)
        
    def pow_ach_bwid_ach(self, width, unit='M'):
        cmd = 'POW:ACH:BWID:ACH ' + str(width) + unit + 'Hz'
        self.voidcmd(cmd)
        
    def pow_ach_spac_chan(self, width, unit='M'):
        cmd = 'POW:ACH:SPAC:CHAN ' + str(width) + unit + 'Hz'
        self.voidcmd(cmd)
        
    def pow_ach_spac(self, width, unit='M'):
        cmd = 'POW:ACH:SPAC ' + str(width) + unit + 'Hz'
        self.voidcmd(cmd)
        
    def calc_mark_func_pow_res_acp(self):
        cmd = 'CALC:MARK:FUNC:POW:RES? ACP'
        return self.resultcmd(cmd)

    def calc_mark_func_pow_sel_obw(self):
        cmd = 'CALC:MARK:FUNC:POW:SEL OBW'
        self.voidcmd(cmd)
        
    def pow_bwid_99pct(self):
        cmd = 'POW:BWID 99PCT'
        self.voidcmd(cmd)

    def calc_mark_func_pow_res_obw(self):
        cmd = 'CALC:MARK:FUNC:POW:RES? OBW'
        return self.resultcmd(cmd)

    def swe_mode_esp(self):
        cmd = 'SWE:MODE ESP'
        self.voidcmd(cmd)
        
    def esp_pres(self,sem):
        cmd ="ESP:PRES '%s.xml'"%sem
        self.voidcmd(cmd)

    def swe_mode_list(self):
        cmd = 'SWE:MODE LIST'
        self.voidcmd(cmd)
        
    def mmem_load_stat_1(self,tse):
        cmd = 'MMEM:LOAD:STAT 1 '+'tse'+'.dfl'
        self.voidcmd(cmd)
        
    #LINESKEY
    def calc_dlin1_stat_on(self):
        cmd = 'CALC:DLIN1:STAT ON'
        self.voidcmd(cmd)

    def calc_dlin1(self,lev):
        cmd = 'CALC:DLIN1 '+str(lev)+'dBm'
        self.voidcmd(cmd)

    def calc_dlin2_stat_on(self):
        cmd = 'CALC:DLIN2:STAT ON'
        self.voidcmd(cmd)

    def calc_dlin2(self,lev):
        cmd = 'CALC:DLIN2 '+str(lev)+'dBm'
        self.voidcmd(cmd)
    
    def calc_tlin1_stat_on(self):
        cmd = 'CALC:TLIN1:STAT ON'
        self.voidcmd(cmd)

    def calc_tlin1(self,times):
        cmd = 'CALC:TLIN1 '+str(times)+'us'
        self.voidcmd(cmd)    
        
    #MARKKEY    
    def calc_mark_max(self):
        cmd = 'CALC:MARK:MAX'
        self.voidcmd(cmd)
        
    def calc_mark_min(self):
        cmd = 'CALC:MARK:MIN'
        self.voidcmd(cmd)
        
    def calc_mark_max_next(self):
        cmd = 'CALC:MARK:MAX:NEXT'
        self.voidcmd(cmd)

    def calc_mark_rlev(self):
        cmd = 'CALC:MARK:RLEV'
        self.voidcmd(cmd)

    def calc_mark_x(self):
        cmd = 'CALC:MARK:X?'
        return self.resultcmd(cmd)

    def calc_mark_y(self):
        cmd = 'CALC:MARK:Y?'
        return self.resultcmd(cmd)

    def calc_mark_aoff(self):
        cmd = 'CALC:MARK:AOFF'
        self.voidcmd(cmd)

    def calc_mark_mode_pos(self):
        cmd = 'CALC:MARK:MODE POS'
        self.voidcmd(cmd)
    
    #PRINTKEY
    def hcop_dev_lang_jpeg(self):
        cmd = 'HCOP:DEV:LANG JPEG'
        self.voidcmd(cmd)
    def hcop_dev_col_on(self):
        cmd = 'HCOP:DEV:COL ON'
        self.voidcmd(cmd)
        
    def hcop_cmap_def4(self):
        cmd = 'HCOP:CMAP:DEF4'
        self.voidcmd(cmd)
        
    def hcop_dest_mmem(self, photo):
        cmd = "HCOP:DEST 'MMEM'"
        self.voidcmd(cmd)
        cmd = "MMEM:NAME 'C:\\R_S\\instr\\user\\auto_test_qll\\%s.JPEG'" % photo
        self.voidcmd(cmd)
           
    def hcop(self):
        cmd = 'HCOP'
        self.voidcmd(cmd)
        
    def hcop_next(self):
        cmd = 'HCOP:NEXT'
        self.voidcmd(cmd)
        
    #PRINT LIST
    def trace1_data_list(self):
        cmd = ':TRACe1:DATA? LIST'
        return self.resultcmd(cmd)
    
    def trac_data_trace1(self):
        cmd = 'TRAC:DATA? TRACE1'
        return self.resultcmd(cmd)

    def calc_lim_fail(self):
        cmd = 'CALC:LIM:FAIL?'
        return self.resultcmd(cmd)
    
    def set_harm_num(self, num):
        cmd = 'CALC:MARK:FUNC:HARM:NHAR ' + str(num) 
        self.voidcmd(cmd)

    def set_harm_on_off(self, switch):
        if switch == 'ON':
            cmd = 'CALC:MARK:FUNC:HARM:STAT ' + switch 
            self.voidcmd(cmd)
        elif switch == 'OFF':
            cmd = 'CALC:MARK:FUNC:HARM:STAT ' + switch 
            self.voidcmd(cmd)
        else:
            pass

    def read_harm_value(self):
        cmd = 'CALC:MARK:FUNC:HARM:LIST?' 
        return self.resultcmd(cmd)

    def calc_mark_aoff(self):
        cmd = 'CALC:MARK:AOFF'
        self.voidcmd(cmd)
        
    def str_to_float(self,liter):
        length=len(liter)
        i=0
        reliter=liter
        for count in range(0,length):
            reliter[i]=float(liter[i])
            i+=1
        return reliter

    #MODE
    def inst_lte(self):
        cmd = 'INST LTE'
        self.voidcmd(cmd)

    def inst_san(self):
        cmd = 'INST SAN'
        self.voidcmd(cmd)
        
    #GENERAL
    def conf_dl_bw(self,width):
        cmd = 'CONF:DL:BW BW'+str(width)+'_00'
        self.voidcmd(cmd)

    def conf_dl_cycp_auto(self):
        cmd = 'CONF:DL:CYCP AUTO'
        self.voidcmd(cmd)

    def conf_dl_cycp_norm(self):
        cmd = 'CONF:DL:CYCP NORM'
        self.voidcmd(cmd)

    def disp_trac_y_rlev_offs(self,lev):
        cmd = 'DISP:TRAC:Y:RLEV:OFFS '+str(lev)+'dB'
        self.voidcmd(cmd)

    def fram_coun_stat_on(self):
        cmd = 'FRAM:COUN:STAT ON'
        self.voidcmd(cmd)

    def fram_coun_stat_off(self):
        cmd = 'FRAM:COUN:STAT OFF'
        self.voidcmd(cmd)

    def fram_coun_auto_on(self):
        cmd = 'FRAM:COUN:AUTO ON'
        self.voidcmd(cmd)

    def fram_coun_auto_off(self):
        cmd = 'FRAM:COUN:AUTO OFF'
        self.voidcmd(cmd)

    def conf_oop_nfr(self,fra):
        cmd = 'CONF:OOP:NFR '+str(fra)
        self.voidcmd(cmd)

    def oop_ncor_on(self):
        cmd = 'OOP:NCOR ON'
        self.voidcmd(cmd)

    def oop_ncor_off(self):
        cmd = 'OOP:NCOR OFF'
        self.voidcmd(cmd)
        
    #MIMO
    def conf_dl_mimo_conf_tx1(self):
        cmd = 'CONF:DL:MIMO:CONF TX1'
        self.voidcmd(cmd)

    def conf_dl_mimo_asel_ant1(self):
        cmd = 'CONF:DL:MIMO:ASEL ANT1'
        self.voidcmd(cmd)
        
    #ADVANCED
    def swap_on(self):
        cmd = 'SWAP ON'
        self.voidcmd(cmd)

    def swap_off(self):
        cmd = 'SWAP OFF'
        self.voidcmd(cmd)
        
    #TRIGGER
    def trig_mode_ext(self):
        cmd = 'TRIG:MODE EXT'
        self.voidcmd(cmd)

    def trig_hold(self,width):
        cmd = 'TRIG:HOLD '+str(width)+'us'
        self.voidcmd(cmd)    

    def trig_lev(self,lev):
        cmd = 'TRIG:LEV '+str(lev)+'v'
        self.voidcmd(cmd)
   
    #SPECTRUM 
    def swe_egat_auto_on(self):
        cmd = 'SWE:EGAT:AUTO ON'
        self.voidcmd(cmd)

    def swe_egat_auto_off(self):
        cmd = 'SWE:EGAT:AUTO OFF'
        self.voidcmd(cmd)

    def freq_span_auto_on(self):
        cmd = 'FREQ:SPAN:AUTO ON'
        self.voidcmd(cmd)

    def pow_sem_cat_a(self):
        cmd = 'POW:SEM:CAT A'
        self.voidcmd(cmd)
  
    def pow_sem_cat_b(self):
        cmd = 'POW:SEM:CAT B'
        self.voidcmd(cmd)

    def pow_ach_aach_eutra(self):
        cmd = 'POW:ACH:AACH EUTRA'
        self.voidcmd(cmd)

    def pow_ach_txch_coun(self,num):
        cmd = 'POW:ACH:TXCH:COUN '+str(num)
        self.voidcmd(cmd)

    def pow_ach_band_chan2(self,width):
        cmd = 'POW:ACH:BAND:CHAN2 BW'+str(width)+'_00'
        self.voidcmd(cmd)

    def pow_ach_spac_chan(self,width):
        cmd = 'POW:ACH:SPAC:CHAN '+str(width)+'MHz'
        self.voidcmd(cmd)

    def pow_ncor_on(self):
        cmd = 'POW:NCOR ON'
        self.voidcmd(cmd)

    def pow_ncor_off(self):
        cmd = 'POW:NCOR OFF'
        self.voidcmd(cmd)
   
    #DL DEMOD    
    def dl_dem_cest_tgpp(self):
        cmd = 'DL:DEM:CEST TGPP'
        self.voidcmd(cmd)

    def dl_dem_evmc_tgpp(self):
        cmd = 'DL:DEM:EVMC TGPP'
        self.voidcmd(cmd)

    def dl_dem_auto_on(self):
        cmd = 'DL:DEM:AUTO ON'
        self.voidcmd(cmd)

    def dl_dem_auto_off(self):
        cmd = 'DL:DEM:AUTO OFF'
        self.voidcmd(cmd)

    def dl_dem_cbsc_on(self):
        cmd = 'DL:DEM:CBSC ON'
        self.voidcmd(cmd)

    def dl_dem_cbcs_off(self):
        cmd = 'DL:DEM:CBCS OFF'
        self.voidcmd(cmd)

    def dl_dem_best_on(self):
        cmd = 'DL:DEM:BEST ON'
        self.voidcmd(cmd)

    def dl_dem_best_off(self):
        cmd = 'DL:DEM:BEST OFF'
        self.voidcmd(cmd)

    def dl_form_pscd_phydet(self):
        cmd = 'DL:FORM:PSCD PHYDET'
        self.voidcmd(cmd)

    def dl_form_pscd_pdcch(self):
        cmd = 'DL:FORM:PSCD PDCCH'
        self.voidcmd(cmd)    
        
    def dl_dem_prd_auto(self):
        cmd = 'DL:DEM:PRD AUTO'
        self.voidcmd(cmd)

    def dl_dem_prd_all0(self):
        cmd = 'DL:DEM:PRD ALL0'
        self.voidcmd(cmd)

    def dl_dem_mcf_on(self):
        cmd = 'DL:DEM:MCF ON'
        self.voidcmd(cmd)

    def dl_dem_mcf_off(self):
        cmd = 'DL:DEM:MCF OFF'
        self.voidcmd(cmd)

    def dl_trac_phas_off(self):
        cmd = 'DL:TRAC:PHAS OFF'
        self.voidcmd(cmd)

    def dl_trac_phas_pilpay(self):
        cmd = 'DL:TRAC:PHAS PILPAY'
        self.voidcmd(cmd)

    def dl_trac_phas_pil(self):
        cmd = 'DL:TRAC:PHAS PIL'
        self.voidcmd(cmd)

    def dl_trac_time_on(self):
        cmd = 'DL:TRAC:TIME ON'
        self.voidcmd(cmd)

    def dl_trac_time_off(self):
        cmd = 'DL:TRAC:TIME OFF'
        self.voidcmd(cmd)
        
    #DL FRAME CONFIG
    def conf_dl_tdd_udc(self,num):
        cmd = 'CONF:DL:TDD:UDC '+str(num)
        self.voidcmd(cmd)

    def conf_dl_tdd_spsc(self,num):
        cmd = 'CONF:DL:TDD:SPSC '+str(num)
        self.voidcmd(cmd)

    def conf_dl_plc_cid_auto(self):
        cmd = 'CONF:DL:PLC:CID AUTO'
        self.voidcmd(cmd)

    #FILE MANAGER
    def mmem_load_tmod_dl(self,model,width):
        cmd = "MMEM:LOAD:TMOD:DL 'E-TM%s__%sMHz'"%(model,width)
        self.voidcmd(cmd)
        
    #RUN CONT
    def init(self):
        cmd = 'INIT'
        self.voidcmd(cmd)
        
    #ACLR
    def calc2_feed_spec_acp(self):
        cmd = "CALC2:FEED 'SPEC:ACP'"
        self.voidcmd(cmd)
        
    def calc1_mark_func_pow_res(self):
        cmd = 'CALC1:MARK:FUNC:POW:RES?' 
        return self.resultcmd(cmd)

    #ALLOCATION SUMMARY
    def calc2_feed_stat_asum(self):
        cmd = "CALC2:FEED 'STAT:ASUM'"
        self.voidcmd(cmd)

    def trac_data_trace1(self):
        cmd = 'TRAC:DATA? TRACE1' 
        return self.resultcmd(cmd)
    
    #ON/OFF POWER
    def calc2_feed_pvt_oop(self):
        cmd = "CALC2:FEED 'PVT:OOP'"
        self.voidcmd(cmd)

    def oop_atim(self):
        cmd = "OOP:ATIM"
        self.voidcmd(cmd)

    def calc_lim_oop_offp(self):
        cmd = 'CALC:LIM:OOP:OFFP?' 
        return self.resultcmd(cmd)
    
    def calc_lim_oop_tran_ris(self):
        cmd = 'CALC:LIM:OOP:TRAN? RIS' 
        return self.resultcmd(cmd)
    
    def calc_lim_oop_tran_fall(self):
        cmd = 'CALC:LIM:OOP:TRAN? FALL' 
        return self.resultcmd(cmd)

    #SPECTRUM EMISSION MASK
    def calc2_feed_spec_sem(self):
        cmd = "CALC2:FEED 'SPEC:SEM'"
        self.voidcmd(cmd)

    #LIST/GRAPH
    def disp_tabl_on(self):
        cmd = "DISP:TABL ON"
        self.voidcmd(cmd)

    def disp_tabl_off(self):
        cmd = "DISP:TABL OFF"
        self.voidcmd(cmd)

    #EVM PRINT QPSK 
    def fetc_summ_evm_dsqp(self):
        cmd = 'FETC:SUMM:EVM:DSQP?' 
        return self.resultcmd(cmd)
    
    #EVM PRINT 16QAM 
    def fetc_summ_evm_dsst(self):
        cmd = 'FETC:SUMM:EVM:DSST?' 
        return self.resultcmd(cmd)

    #EVM PRINT 64QAM
    def fetc_summ_evm_dssf(self):
        cmd = 'FETC:SUMM:EVM:DSSF?' 
        return self.resultcmd(cmd)
    
    #FREQ ERROR
    def fetc_summ_ferr(self):
        cmd = 'FETC:SUMM:FERR?' 
        return self.resultcmd(cmd)

    #RSTP
    def fetc_summ_rstp(self):
        cmd = 'FETC:SUMM:RSTP?' 
        return self.resultcmd(cmd)

    #OSTP
    def fetc_summ_ostp(self):
        cmd = 'FETC:SUMM:OSTP?' 
        return self.resultcmd(cmd)

    #POWER
    def fetc_summ_pow(self):
        cmd = 'FETC:SUMM:POW?' 
        return self.resultcmd(cmd)
    
    #===========================================================================
    # ///////ul_test
    #===========================================================================
    def rst(self):
        cmd = '*RST '
        self.voidcmd(cmd)
        
    def freq(self, freq, unit='M'):
        cmd = 'FREQ ' + str(freq) + unit + 'Hz'
        self.voidcmd(cmd)
        
    def freq_offset(self, freq, unit='M'):
        cmd = 'FREQ:OFFSET ' + str(freq) + unit + 'Hz'
        self.voidcmd(cmd)

    def pow(self, lev):
        cmd = 'POW '+ str(lev)+ 'dBm'
        self.voidcmd(cmd)

    def rosc_sour_ext(self):
        cmd = 'ROSC:SOUR EXT' 
        self.voidcmd(cmd)

    def rosc_ext_freq_10mhz(self):
        cmd = 'ROSC:EXT:FREQ 10MHz'
        self.voidcmd(cmd)

    def rosc_ext_rfof_off(self):
        cmd = 'ROSC:EXT:RFOF OFF'
        self.voidcmd(cmd)

    def rosc_ext_sban_wide(self):
        cmd = 'ROSC:EXT:SBAN WIDE'
        self.voidcmd(cmd)

    def rosc_adj_off(self):
        cmd = 'ROSC:ADJ OFF'
        self.voidcmd(cmd)
    
    def rosc_ext_sban_wide(self):
        cmd = 'ROSC:EXT:SBAN WIDE'
        self.voidcmd(cmd)

    def sour_bb_arb_wav_sel(self,dire,name):
        cmd = "SOUR:BB:ARB:WAV:SEL '%s\\%s'"%(dire,name)
        self.voidcmd(cmd)
        
    def bb_arb_trig_seq_aret(self):
        cmd = 'BB:ARB:TRIG:SEQ ARET'
        self.voidcmd(cmd)
        
    def bb_arb_trig_sour_ext(self):
        cmd = 'BB:ARB:TRIG:SOUR EXT'
        self.voidcmd(cmd)

    def bb_arb_trig_ext_sync_oupt_on(self):
        cmd = 'BB:ARB:TRIG:EXT:SYNC:OUTP ON'
        self.voidcmd(cmd)

    def bb_arb_trig_del(self,num):
        cmd = 'BB:ARB:TRIG:DEL ' + str(num)
        self.voidcmd(cmd)

    def bb_arb_trig_inh_0(self):
        cmd = 'BB:ARB:TRIG:INH 0'
        self.voidcmd(cmd)

    def sour_bb_arb_stat_on(self):
        cmd = 'SOUR:BB:ARB:STAT ON'
        self.voidcmd(cmd)

    def sour_bb_arb_stat_off(self):
        cmd = 'SOUR:BB:ARB:STAT OFF'
        self.voidcmd(cmd)

    def bb_arb_trig_outp1_mode_unch(self):
        cmd = 'BB:ARB:TRIG:OUTP1:MODE UNCH'
        self.voidcmd(cmd)

    def bb_arb_trig_outp2_mode_unch(self):
        cmd = 'BB:ARB:TRIG:OUTP2:MODE UNCH'
        self.voidcmd(cmd)

    def bb_arb_trig_outp1_del_0(self):
        cmd = 'BB:ARB:TRIG:OUTP1:DEL 0'
        self.voidcmd(cmd)

    def bb_arb_trig_outp2_del_0(self):
        cmd = 'BB:ARB:TRIG:OUTP2:DEL 0'
        self.voidcmd(cmd)

    def bb_arb_cloc_sour_int(self):
        cmd = 'BB:ARB:CLOC:SOUR INT'
        self.voidcmd(cmd)

    def bb_arb_cloc_sysnc_mode_none(self):
        cmd = 'BB:ARB:CLOC:SYSNC:MODE NONE'
        self.voidcmd(cmd)

    def outp_on(self):
        cmd = 'OUTP ON'
        self.voidcmd(cmd)

    def outp_off(self):
        cmd = 'OUTP OFF'
        self.voidcmd(cmd)


def stringToDig(myString):
    pool = []
    validNum = ''
    for i in range(len(myString)):
        temp = myString[i]
        if (myString[i]>='0'and myString[i]<='9')or (myString[i]=='.' or myString[i]=='-'):
            validNum= validNum+(myString[i])
            if i==(len(myString)-1):
                pool.append(float(validNum))
                validNum = ''
                continue
        else:
            if not validNum:
                continue
            else:
                pool.append(float(validNum))
                validNum = ''
                continue
    return pool


def recvMacCmd(filter, delay):
    adhandle, alldevs = conf.configInterface()
    filterCondition = str(filter).encode('ascii')
    conf.filterPacket(adhandle, alldevs, filterCondition)
    flag1 = 0
    timeStart = time.clock()
    timeMap = 0
    while((flag1 == 0) and (timeMap < delay)):
        flag,retVal = conf.getPacket(adhandle)

        if flag >0:
            buffer = struct.pack(">"+"B"*2,*retVal[24:26])
            cmdTyp = struct.unpack('>H', buffer)
            if(cmdTyp[0] ==0x3816):
                buffer = struct.pack(">"+"B"*len(retVal),*retVal)
                stringRule = '>'+'H'*10+'i'+'H'*8+'i'*159
                retVal = struct.unpack(stringRule, buffer)
                flag1=1
        timeMap = (time.clock()-timeStart)

    conf.freeDevice(alldevs)
    return(flag1, retVal[-3:-1])


def signal(sa_ip_address, freq, power):
    sa = SA('%s' % sa_ip_address, 5025)
    sa.rst()
    sa.freq(freq)
    sa.pow(power)
    sa.outp_on()
    sa.close()


def signal_wanted(power_wanted,signal_ip_wanted,freq,offset_wanted,sour_wanted,sample):
    sa=SA('%s'%signal_ip_wanted,5025)
    sa.rst()
    sa.freq(freq)
    sa.freq_offset(offset_wanted)
    sa.rosc_sour_ext()
    sa.rosc_ext_freq_10mhz()
    sa.rosc_ext_rfof_off()
    sa.rosc_ext_sban_wide()
    sa.rosc_adj_off()
    sa.rosc_ext_sban_wide()
    sa.sour_bb_arb_wav_sel('CE_qll_1','%s'%sour_wanted)
    sa.bb_arb_trig_seq_aret()
    sa.bb_arb_trig_sour_ext()
    sa.bb_arb_trig_ext_sync_oupt_on()
    sa.bb_arb_trig_del(sample)
    sa.bb_arb_trig_inh_0()
    sa.sour_bb_arb_stat_on()
    sa.bb_arb_trig_outp1_mode_unch()
    sa.bb_arb_trig_outp2_mode_unch()
    sa.bb_arb_trig_outp1_del_0()
    sa.bb_arb_trig_outp2_del_0()
    sa.bb_arb_cloc_sour_int()
    sa.bb_arb_cloc_sysnc_mode_none()
    sa.pow(power_wanted)
    sa.outp_on()
    sa.close()


def signal_unwanted(power_unwanted, signal_ip_unwanted, freq_unwanted, offset_unwanted, sour_unwanted):
    sa = SA('%s'%signal_ip_unwanted, 5025)
    sa.rst()
    sa.freq(freq_unwanted)
    sa.freq_offset(offset_unwanted)
    #===========================================================================
    # sa.rosc_sour_ext()
    # sa.rosc_ext_freq_10mhz()
    # sa.rosc_ext_rfof_off()
    # sa.rosc_ext_sban_wide()
    # sa.rosc_adj_off()
    # sa.rosc_ext_sban_wide()
    #===========================================================================
    sa.sour_bb_arb_wav_sel('CE_qll_1','%s'%sour_unwanted)
    #===========================================================================
    # sa.bb_arb_trig_seq_aret()
    # sa.bb_arb_trig_sour_ext()
    # sa.bb_arb_trig_ext_sync_oupt_on()
    # sa.bb_arb_trig_del(sample)
    # sa.bb_arb_trig_inh_0()
    # sa.sour_bb_arb_stat_on()
    # sa.bb_arb_trig_outp1_mode_unch()
    # sa.bb_arb_trig_outp2_mode_unch()
    # sa.bb_arb_trig_outp1_del_0()
    # sa.bb_arb_trig_outp2_del_0()
    # sa.bb_arb_cloc_sour_int()
    # sa.bb_arb_cloc_sysnc_mode_none()
    #===========================================================================
    sa.pow(power_unwanted)
    sa.outp_on()
    sa.close()


def Ref_Sen_Lvl(freq, sample, offset_wanted, sour_wanted, freq_unwanted, offset_unwanted,
                sour_unwanted, spectrum_offset, power,mask, enter_exit,
                test_mode, test_band, power_wanted, power_unwanted):
    freq_count = (float(freq)-1700)*20
    signal_wanted(power_wanted, freq, offset_wanted, sour_wanted, sample)
    power_wanted = -80.0
    step = 1
    bler = 0.00
    while (bler>0.01 or step==1):
        power_wanted = power_wanted-step
        # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)

        filter = "ether src 30:31:32:33:34:3f  "
        delay =20
        flag,retCmd = recvMacCmd(filter,delay)
        if flag>0:
           print(retCmd)
           totalBlockNumber = retCmd[0]
           BlockERROR = retCmd[1]
           bler1 = float(BlockERROR)/totalBlockNumber
           myGUI_grid.GUI.msg.insert('end', 'power_wanted11: %s dBm\n' %(power_wanted), 'INT')
           myGUI_grid.GUI.msg.insert('end', 'bler11: %f%%\n' %(bler1), 'INT')
        if bler>0.01:
            step=-0.1
        print(power_wanted)
        print(bler)
        myGUI_grid.GUI.msg.insert('end', 'power_wanted22: %s dBm\n' %(power_wanted), 'INT')
        myGUI_grid.GUI.msg.insert('end', 'bler22: %f%%\n' %(bler1), 'INT')


def Dynamic_Range(freq,sample,offset_wanted,sour_wanted,freq_unwanted,offset_unwanted,
                sour_unwanted,spectrum_offset,power,mask,enter_exit,
                test_mode,test_band,power_wanted,power_unwanted):
    freq_count=(float(freq)-1700)*20 
    power_wanted=-80.0
    power_unwanted=-50
    step=1
    bler=0.00
    while (bler>0.01 or step==1):
        power_wanted = power_wanted + step
        # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
        
        filter = "ether src 30:31:32:33:34:3f  "
        delay =20
        flag, retCmd = recvMacCmd(filter, delay)
        if flag > 0:
           print(retCmd)
           totalBlockNumber = retCmd[0]
           BlockERROR= retCmd[1]   
           bler1 = float(BlockERROR)/totalBlockNumber
        myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' % (power_wanted), 'INT')
        myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' % bler, 'INT')
        if bler > 0.01:
            step = -0.1
            print(power_wanted)
            print(bler)
        myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' %(power_wanted), 'INT')
        myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')    


def ACS(freq, sample, offset_wanted, sour_wanted, freq_unwanted, offset_unwanted,
                sour_unwanted, spectrum_offset, power, mask, enter_exit,
                test_mode, test_band, power_wanted, power_unwanted):
    freq_count = (float(freq)-1700)*20
    signal_wanted(power_wanted, freq, offset_wanted, sour_wanted, sample)
    signal_unwanted(power_unwanted, freq_unwanted,offset_unwanted, sour_unwanted)
    if  test_band == '0':
        power_wanted = -89
        power_unwanted = -50
        step = 1
        bler = 0.00
        while (bler>0.01 or step==1):
            power_wanted = power_wanted - step
            # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
        
            filter = "ether src 30:31:32:33:34:3f  "
            delay = 20
            flag,retCmd = recvMacCmd(filter,delay)
            if flag > 0:
               print(retCmd)
               totalBlockNumber = retCmd[0]
               BlockERROR = retCmd[1]
               bler1 = float(BlockERROR)/totalBlockNumber
               myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' %(power_wanted), 'INT')    
               myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')
        if  bler>0.01:
            step = -0.1
            print(power_wanted)
            print(bler)
            myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' %(power_wanted), 'INT')
            myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')
    elif test_band == '2':
        power_wanted = -84.3
        power_unwanted = -50
        step = 1
        bler = 0.00
        while (bler>0.01 or step==1):
            power_wanted = power_wanted+step
           # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
        
            filter = "ether src 30:31:32:33:34:3f  "
            delay = 20
            flag, retCmd = recvMacCmd(filter, delay)
            if flag>0:
                print(retCmd)
                totalBlockNumber = retCmd[0]
                BlockERROR= retCmd[1]
                bler1 = float(BlockERROR)/totalBlockNumber
                myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' %(power_wanted), 'INT')
                myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')
        if bler > 0.01:
            step = -0.1
            print(power_wanted)
            print(bler)
            myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' %(power_wanted), 'INT')
            myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')
    else:
        time.sleep(0.02)


def Blocking(freq,sample,offset_wanted,sour_wanted,freq_unwanted,offset_unwanted,
                sour_unwanted,spectrum_offset,power,mask,enter_exit,
                test_mode,test_band,power_wanted,power_unwanted):
    freq_count=(float(freq)-1700)*20 
    signal_wanted(power_wanted,freq,offset_wanted,sour_wanted,sample)
    signal_unwanted(power_unwanted,freq_unwanted,offset_unwanted,sour_unwanted)
    if  test_band=='0':
        power_wanted=-89
        power_unwanted=-50
        step=1
        bler=0.00
        while (bler>0.01 or step==1):
            power_wanted=power_wanted+step
            # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
        
            filter = "ether src 30:31:32:33:34:3f  "
            delay =20
            flag, retCmd = recvMacCmd(filter,delay)
            if flag>0:
               print(retCmd)
               totalBlockNumber = retCmd[0]
               BlockERROR = retCmd[1]
               bler1 = float(BlockERROR)/totalBlockNumber
               myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' % power_wanted, 'INT')
               myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')
        if  bler>0.01:
            step=-0.1
            print(power_wanted)
            print(bler)
            myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' %(power_wanted), 'INT')
            myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')
    elif test_band=='2':
         power_wanted=-84.3
         power_unwanted=-50
         step=1
         bler=0.00
         while (bler>0.01 or step==1):
           power_wanted=power_wanted-step
           # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
        
           filter = "ether src 30:31:32:33:34:3f  "
           delay = 20
           flag, retCmd = recvMacCmd(filter,delay)
           if flag>0:
              print(retCmd)
              totalBlockNumber = retCmd[0]
              BlockERROR = retCmd[1]
              bler1 = float(BlockERROR)/totalBlockNumber
              myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' %(power_wanted), 'INT')    
              myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')
         if bler > 0.01:
            step = -0.1
            print(power_wanted)
            print(bler)
            myGUI_grid.GUI.msg.insert('end', 'power_wanted: %s dBm\n' %(power_wanted), 'INT')
            myGUI_grid.GUI.msg.insert('end', 'bler: %.2f\n' %(bler), 'INT')
    else:
        time.sleep(0.02)
    

def test():
    '''Test program.
    Usage: sa [-d] host port 

    -d debug

    This test program will set the analyzer's center freq to 456MHz,
    set the freq span to 20MHz...  and output measured freq and power.
    '''
    import time

    if len(sys.argv) < 3:
        print(test.__doc__)
        sys.exit(0)

    debugging = 0
    while sys.argv[1] == '-d':
        debugging = debugging + 1
        del sys.argv[1]

    host = sys.argv[1]
    port = sys.argv[2]
    sa = SA(host, port)
    sa.set_debuglevel(debugging)
#print sa.getwelcome()
    sa.freq_cent(456, 'M')
    sa.freq_span(20, 'M')
    sa.disp_wind_trac_y_rlev(20)
    sa.band(10, 'K')
    sa.band_vid(10, 'K')
    sa.calc_mark_mode_pos()
    # must sleep here after calc_mark_mode_pos and calc_mark_max
    time.sleep(2)
    sa.calc_mark_max()
    time.sleep(5)
    currentfreq = sa.calc_mark_x()
    currentpower = sa.calc_mark_y()
    sa.calc_mark_aoff()
    sa.close()
    print('Current Freq:', currentfreq)
    print('Current Power:', currentpower)


def test_power_aclr(spectrum_ip, freq, span):
    sa = SA('%s'% spectrum_ip, 5025)
    sa.rst()
    sa.inst_san()
    sa.freq_cent(freq)
    sa.freq_span(span)


def test_POW_ACLR(spectrum_ip, freq, spectrum_offset, sample=0, offset_wanted=0, sour_wanted=0, freq_unwanted=0,
                  offset_unwanted=0, sour_unwanted=0, power=0, mask=0, enter_exit=0, test_mode=0, test_band=0):
    freq_count=(float(freq)-1700)*20
    # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
    sa = SA('%s'% spectrum_ip, 5025)
    sa.rst()
    sa.inst_san()
    sa.freq_cent(freq)
    sa.freq_span(20)
    sa.band(30)
    sa.band_vid(100)
    sa.inp_att(25)
    sa.disp_wind_trac_y_rlev_offs(spectrum_offset)
    sa.disp_wind_trac_y_rlev(30)
    time.sleep(0.5)
    sa.swe_time_auto_on()
    sa.det_rms()
    sa.trig_sour_ext()
    sa.trig_slop_pos()
    sa.swe_egat_on()
    sa.swe_egat_type_edge()
    sa.swe_egat_hold(60)
    sa.swe_egat_leng(128)
    sa.calc_mark_func_pow_sel_acp()
    sa.pow_ach_acp(2)
    sa.pow_ach_bwid_chan1(15)
    sa.pow_ach_bwid_ach(15)
    sa.pow_ach_spac_chan(15)
    sa.pow_ach_spac(15)
    time.sleep(8)
    pow_aclr=sa.calc_mark_func_pow_res_acp()
    temp_pow_aclr = stringToDig(pow_aclr)
    myGUI_grid.GUI.msg.insert('end', 'power: %.2fdBm\n' %(temp_pow_aclr[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr1_lower: %.2fdBm\n' %(temp_pow_aclr[1]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr1_upper: %.2fdBm\n' %(temp_pow_aclr[2]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr2_lower: %.2fdBm\n' %(temp_pow_aclr[3]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr2_upper: %.2fdBm\n' %(temp_pow_aclr[4]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    print(pow_aclr)
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('mcwill_pow_aclr')
    sa.hcop_next()
    sa.close()


def test_OBW(spectrum_ip, freq, spectrum_offset, sample=0, offset_wanted=0, sour_wanted=0, freq_unwanted=0,
             offset_unwanted=0, sour_unwanted=0,  power=0, mask=0, enter_exit=0, test_mode=0, test_band=0):
    # freq_count = (float(freq)-1700)*20
    # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_san()
    sa.freq_cent(freq)
    sa.freq_span(30)
    sa.band(30)
    sa.band_vid(100)
    sa.inp_att(25)
    sa.disp_wind_trac_y_rlev_offs(spectrum_offset)
    sa.disp_wind_trac_y_rlev(30)
    sa.swe_time_auto_on()
    sa.det_rms()
    sa.trig_sour_ext()
    sa.trig_slop_pos()
    sa.swe_egat_on()
    sa.swe_egat_type_edge()
    sa.swe_egat_hold(60)
    sa.swe_egat_leng(128)
    sa.pow_bwid_99pct()
    sa.calc_mark_func_pow_sel_obw()
    time.sleep(1)
    obw = sa.calc_mark_func_pow_res_obw()
    temp_obw = stringToDig(obw)
    # sa.hcop_dev_col_on()
    # sa.hcop_cmap_def4()
    # sa.hcop_dev_lang_jpeg()
    # sa.hcop_dest_mmem('mcwill_obw')
    # sa.hcop_next()
    sa.close()
    # myGUI_grid.GUI.msg.insert('end','obw: %.2fMHz\n'%(temp_obw[0]/10**6), 'INT')
    # myGUI_grid.GUI.msg.see('end')
    print(obw)


def test_FREQERROR(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                   offset_unwanted, sour_unwanted, spectrum_offset, power, mask, enter_exit, test_mode, test_band):
    sa=SA('%s'%spectrum_ip,5025)
    sa.rst()
    sa.inst_san()
    sa.freq_cent(freq)
    sa.freq_span(0.05)
    sa.band(1)
    sa.band_vid(3)
    sa.inp_att(25)
    sa.disp_wind_trac_y_rlev_offs(spectrum_offset)
    sa.swe_time_auto_on()
    sa.disp_wind_trac_y_rlev(30)
    sa.det_rms()
    sa.calc_mark_max()
    freqerror_test=sa.calc_mark_x()
    temp_freqerror_test = stringToDig(freqerror_test)
    freqerror=temp_freqerror_test[0]-float(freq)*10**6
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('mcwill_freqerror')
    sa.hcop_next()
    sa.close()
    myGUI_grid.GUI.msg.insert('end', 'freqerror: %.2fHz\n'%(freqerror), 'INT')
    myGUI_grid.GUI.msg.see('end')
    print(freqerror)


def test_TSE(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
             offset_unwanted, sour_unwanted, spectrum_offset, power, mask, enter_exit, test_mode, test_band):
    freq_count=(float(freq)-1700)*20
    # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
    sa=SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_san()
    sa.freq_cent(freq)
    sa.freq_span(20)
    sa.inp_att(10)
    sa.disp_wind_trac_y_rlev(10)
    sa.disp_wind_trac_y_rlev_offs(spectrum_offset)
    time.sleep(0.5)
    sa.swe_time_auto_on()
    sa.det_rms()
    sa.trig_sour_ext()
    sa.trig_slop_pos()
    sa.swe_egat_on()
    sa.swe_egat_type_edge()
    sa.swe_egat_hold(60)
    sa.swe_egat_leng(128)    
    sa.calc_dlin1_stat_on()
    sa.calc_dlin1(-36)
    sa.band(1)
    sa.band_vid(3)
    sa.freq_start(9)
    sa.freq_stop(150)
    time.sleep(1)
    sa.calc_mark_max()
    freq_x=sa.calc_mark_x()
    temp_freq_x = stringToDig(freq_x)
    power_y=sa.calc_mark_y()
    temp_power_y = stringToDig(power_y)
    print(freq_x,power_y)
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('mcwill_tse')
    sa.hcop_next()
    time.sleep(1)
    myGUI_grid.GUI.msg.insert('end', 'tse_9k_150k_freq: %.2fkHz\n'%(temp_freq_x[0]/1000), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'tse_9k_150k_power: %.2fdBm\n'%(temp_power_y[0]),'INT')
    myGUI_grid.GUI.msg.see('end')
    pass
    sa.inp_att(20)
    sa.disp_wind_trac_y_rlev(10)
    sa.band(10)
    sa.band_vid(30)
    sa.freq_start(150)
    sa.freq_stop(30000)
    time.sleep(1)
    sa.calc_mark_max()
    freq_x=sa.calc_mark_x()
    temp_freq_x = stringToDig(freq_x)
    power_y=sa.calc_mark_y()
    temp_power_y = stringToDig(power_y)
    print(freq_x, power_y)
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('mcwill_tse')
    sa.hcop_next()
    time.sleep(2)
    myGUI_grid.GUI.msg.insert('end', 'tse_150k_30M_freq: %.2fMHz\n'%(temp_freq_x[0]/10**6), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'tse_150k_30M_power: %.2fdBm\n'%(temp_power_y[0]),'INT')
    myGUI_grid.GUI.msg.see('end')
    pass
    sa.inp_att(10)
    sa.disp_wind_trac_y_rlev(20)
    sa.band(100)
    sa.band_vid(300)
    sa.freq_start(30000)
    sa.freq_stop(1000000)
    time.sleep(3)
    sa.calc_mark_max()
    freq_x=sa.calc_mark_x()
    temp_freq_x = stringToDig(freq_x)
    power_y=sa.calc_mark_y()
    temp_power_y = stringToDig(power_y)
    print(freq_x,power_y)
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('mcwill_tse')
    sa.hcop_next()
    time.sleep(1)
    myGUI_grid.GUI.msg.insert('end', 'tse_30M_1G_freq: %.2fMHz\n'%(temp_freq_x[0]/10**6), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'tse_30M_1G_power: %.2fdBm\n'%(temp_power_y[0]),'INT')
    myGUI_grid.GUI.msg.see('end')
    pass
    sa.inp_att(20)
    sa.disp_wind_trac_y_rlev(30)
    sa.calc_dlin1(-30)
    sa.band(1000)
    sa.band_vid(3000)
    sa.freq_start(1000000)
    sa.freq_stop(7000000)
    time.sleep(2)
    sa.calc_mark_max()
    time.sleep(3)
    sa.calc_mark_max_next()
    freq_x=sa.calc_mark_x()
    temp_freq_x = stringToDig(freq_x)
    power_y=sa.calc_mark_y()
    temp_power_y = stringToDig(power_y)
    print(freq_x,power_y)
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('mcwill_tse')
    sa.hcop_next()
    myGUI_grid.GUI.msg.insert('end', 'tse_1G_7G_freq: %.2fGHz\n'%(temp_freq_x[0]/10**9), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'tse_1G_7G_power: %.2fdBm\n'%(temp_power_y[0]),'INT')
    myGUI_grid.GUI.msg.see('end')
    pass
    sa.close()


def test_SEM(spectrum_ip,freq,sample,offset_wanted,sour_wanted,freq_unwanted,
             offset_unwanted,sour_unwanted,spectrum_offset,power,mask,enter_exit,test_mode,test_band):
    freq_count=(float(freq)-1700)*20
    # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
    sa=SA('%s'%spectrum_ip,5025)
    sa.rst()
    sa.inst_san()
    sa.swe_mode_esp()
    sa.esp_pres(789)
    time.sleep(10)
    a=sa.calc_lim_fail()
    print(a)
    sem1=sa.trace1_data_list()
    sem1=sem1.split(',')
    sem=sem1
    sem=sa.str_to_float(sem1) 
    print(sem[1:8])
    print(sem[12:19])
    print(sem[23:30])
    print(sem[34:41])
    print(sem[45:52])
    print(sem[56:63])
    print(sem[67:74])
    print(sem[78:85])
    print(sem[89:96])
    print(sem[100:107])
    myGUI_grid.GUI.msg.insert('end', 'sem: %s\n'%(sem[1:8]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[12:19]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[23:30]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[34:41]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[45:52]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[56:63]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[67:74]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[78:85]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[89:96]), 'INT')
    myGUI_grid.GUI.msg.insert('end', '     %s\n'%(sem[100:107]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('mcwill_sem')
    sa.hcop_next()
    sa.close()


def test_SYN_DELAY(spectrum_ip,freq,sample,offset_wanted,sour_wanted,freq_unwanted,
                   offset_unwanted,sour_unwanted,spectrum_offset,power,mask,enter_exit,test_mode,test_band):
    freq_count=(float(freq)-1700)*20
    # downLink_UpLinkCmd(test_band,mask,freq_count,power,test_mode,enter_exit)
    sa=SA('%s'%spectrum_ip,5025)
    sa.rst()
    sa.freq_cent(freq)
    sa.freq_span(0)
    sa.band(10000)
    sa.band_vid(20000)
    sa.inp_att(30)
    sa.disp_wind_trac_y_rlev_offs(spectrum_offset)
    sa.disp_wind_trac_y_rlev(50)
##    time.sleep(0.5)
    sa.swe_time(0.02)
    sa.det_rms()
    sa.trig_sour_ext()
    sa.trig_slop_pos()
    sa.trig_hold(45)
    sa.calc_tlin1_stat_on()
    sa.calc_tlin1(53.5)
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('mcwill_syn_delay')
    sa.hcop_next()
    sa.close()    


def test_LTE_ACLR_11(spectrum_ip,freq,sample,offset_wanted,sour_wanted,freq_unwanted,
                     offset_unwanted,sour_unwanted,spectrum_offset,dl_bw):
    sa=SA('%s'%spectrum_ip,5025)
    sa.rst()
    sa.inst_lte()
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_on()
    sa.fram_coun_auto_on()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.init()
    #===========================================================================
    # sa.mmem_load_tmod_dl('1_1','%s'%dl_bw)
    #===========================================================================
    sa.calc2_feed_spec_acp()
    time.sleep(3)
    aclr11=sa.calc1_mark_func_pow_res()
    temp_aclr11 = stringToDig(aclr11)
    print('ACLR1_1:',aclr11)
    myGUI_grid.GUI.msg.insert('end', 'power:                  %.2fdBm\n' %(temp_aclr11[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr11_1_lower:     %.2fdBc\n' %(temp_aclr11[1]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr11_1_upper:     %.2fdBc\n' %(temp_aclr11[2]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr11_2_lower:      %.2fdBc\n' %(temp_aclr11[3]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr11_2_upper:      %.2fdBc\n' %(temp_aclr11[4]), 'INT')
    #===========================================================================
    # myGUI_grid.GUI.msg.insert('end', 'lte_aclr_11: %s\n'%(aclr11), 'INT')
    #===========================================================================
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_aclr_11')
    sa.hcop_next()
    sa.close()


def test_LTE_ACLR_12(spectrum_ip,freq,sample,offset_wanted,sour_wanted,freq_unwanted,
                     offset_unwanted,sour_unwanted,spectrum_offset,dl_bw):
    sa=SA('%s'%spectrum_ip,5025)
    sa.rst()
    sa.inst_lte()
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_on()
    sa.fram_coun_auto_on()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.init()
    #===========================================================================
    # sa.mmem_load_tmod_dl('1_2','%s'%dl_bw)
    #===========================================================================
    sa.calc2_feed_spec_acp()
    time.sleep(3)
    aclr12=sa.calc1_mark_func_pow_res()
    temp_aclr12 = stringToDig(aclr12)
    myGUI_grid.GUI.msg.insert('end', 'aclr12_1_lower: %.2fdBm\n' %(temp_aclr12[1]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr12_1_upper: %.2fdBm\n' %(temp_aclr12[2]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr12_2_lower: %.2fdBm\n' %(temp_aclr12[3]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'aclr12_2_upper: %.2fdBm\n' %(temp_aclr12[4]), 'INT')
    print('ACLR1_2:',aclr12)
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_aclr_12')
    sa.hcop_next()
    sa.close()


def test_LTE_POWER_RSTP_11(spectrum_ip,freq,sample,offset_wanted,sour_wanted,freq_unwanted,
                           offset_unwanted,sour_unwanted,spectrum_offset,dl_bw):
    sa=SA('%s'%spectrum_ip,5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_on()
    sa.fram_coun_auto_on()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    #===========================================================================
    # sa.mmem_load_tmod_dl('1_1','%s'%dl_bw) 
    #===========================================================================
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_dem_best_on()
    sa.disp_tabl_on()
    sa.init()
    time.sleep(3)
    power=sa.fetc_summ_pow()
    rstp=sa.fetc_summ_rstp()
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_power_rstp_11')
    sa.hcop_next()
    time.sleep(3)
    temp_power = stringToDig(power)
    temp_rstp = stringToDig(rstp)
    myGUI_grid.GUI.msg.insert('end', 'power11:                %.2fdBm\n' %(temp_power[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'rstp11:                   %.2fdBm\n' %(temp_rstp[0]), 'INT')
    #===========================================================================
    # myGUI_grid.GUI.msg.insert('end', 'lte_power_11: %s dBm\nlte_rstp_11: %s dBm\n'%(power,rstp), 'INT')
    #===========================================================================
    myGUI_grid.GUI.msg.see('end')
    print('POWER:',power)
    print('RSTP:',rstp)
    sa.close()


def test_LTE_SEM_11(spectrum_ip,freq,sample,offset_wanted,sour_wanted,freq_unwanted,
                    offset_unwanted,sour_unwanted,spectrum_offset,dl_bw):
    sa=SA('%s'%spectrum_ip,5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_on()
    sa.fram_coun_auto_on()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    #===========================================================================
    # sa.mmem_load_tmod_dl('1_1','%s'%dl_bw) 
    #===========================================================================
    sa.calc2_feed_spec_sem()
    sa.init()
    time.sleep(3)
    sem1=sa.trace1_data_list()
    sem1=sem1.split(',')
    sem=sem1
    sem=sa.str_to_float(sem1)
    print('SEM:')
    print(sem[1:8])
    print(sem[12:19])
    print(sem[23:30])
    print(sem[34:41])
    print(sem[45:52])
    print(sem[56:63])
    myGUI_grid.GUI.msg.insert('end', 'sem test result:   see save picture\n', 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_sem_11')
    sa.hcop_next()
    sa.close()


def test_LTE_ON_OFF_POWER_11(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                             offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa=SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(0)
    sa.fram_coun_stat_on()
    sa.fram_coun_auto_on()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    #===========================================================================
    # sa.mmem_load_tmod_dl('1_1','%s'%dl_bw) 
    #===========================================================================
    sa.calc2_feed_pvt_oop()
    sa.oop_atim()
    sa.init()
    time.sleep(3)
    oop=sa.trace1_data_list()
    print('OOP:',oop)
    temp_oop = stringToDig(oop)
    myGUI_grid.GUI.msg.insert('end', 'on_off_power11: %.2fdBm\n' %(temp_oop[3]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'falling_trans_period: %.2fus\n' %(temp_oop[5]*1e6), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'rising_trans_period: %.2fus\n' %(temp_oop[6]*1e6), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_on_off_power')
    sa.hcop_next()
    sa.close()


def test_LTE_TSE_11(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                    offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_san()
    sa.freq_cent(freq)
    sa.freq_span(20)
    sa.inp_att(25)
    sa.disp_wind_trac_y_rlev_offs(spectrum_offset)
    sa.disp_wind_trac_y_rlev(30)
    time.sleep(0.5)
    sa.swe_time_auto_on()
    sa.det_rms()
    sa.trig_sour_ext()
    sa.trig_slop_pos()
    sa.swe_egat_on()
    sa.swe_egat_type_edge()
    sa.swe_egat_hold(5200)
    sa.swe_egat_leng(6200)    
    sa.calc_dlin1_stat_on()
    sa.calc_dlin1(-36)
    sa.band(1)
    sa.band_vid(3)
    sa.freq_start(9)
    sa.freq_stop(150)
    sa.disp_trac1_mode_maxh()
    time.sleep(5)
    sa.calc_mark_max()
    tsex=sa.calc_mark_x()
    tsey=sa.calc_mark_y()
    temp_tsex = stringToDig(tsex)
    temp_tsey = stringToDig(tsey)
    print('TSE1: FREQ:%s       LEVEL:%s' % (tsex, tsey))
    myGUI_grid.GUI.msg.insert('end', 'tse_9k~150k     %.2fkHz,     %.2fdBm\n' % (temp_tsex[0]*1e-3, temp_tsey[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_tse')
    sa.hcop_next()
    time.sleep(2)
    pass
    sa.inp_att(25)
    sa.band(10)
    sa.band_vid(30)
    sa.freq_start(150)
    sa.freq_stop(30000)
    sa.disp_trac1_mode_maxh()
    time.sleep(4)
    sa.calc_mark_max()
    tsex=sa.calc_mark_x()
    tsey=sa.calc_mark_y()
    temp_tsex = stringToDig(tsex)
    temp_tsey = stringToDig(tsey)
    print('TSE1: FREQ:%s       LEVEL:%s' % (tsex, tsey))
    myGUI_grid.GUI.msg.insert('end', 'tse_150k~30M  %.2fMHz,   %.2fdBm\n' % (temp_tsex[0]*1e-6, temp_tsey[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_tse')
    sa.hcop_next()
    time.sleep(2)
    pass
    sa.inp_att(30)
    sa.band(100)
    sa.band_vid(300)
    sa.freq_start(30000)
    sa.freq_stop(1000000)
    sa.disp_trac1_mode_maxh()
    time.sleep(3)
    sa.calc_mark_max()
    sa.calc_mark_max_next()
    tsex=sa.calc_mark_x()
    tsey=sa.calc_mark_y()
    temp_tsex = stringToDig(tsex)
    temp_tsey = stringToDig(tsey)
    print('TSE1: FREQ:%s       LEVEL:%s' % (tsex, tsey))
    myGUI_grid.GUI.msg.insert('end', 'tse_30M~1G     %.2fMHz,   %.2fdBm\n' % (temp_tsex[0]*1e-6, temp_tsey[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_tse')
    sa.hcop_next()
    time.sleep(2)
    pass
    sa.inp_att(20)
    sa.calc_dlin1(-30)
    sa.band(1000)
    sa.band_vid(3000)
    sa.freq_start(1000000)
    sa.freq_stop(7000000)
    time.sleep(2)
    sa.disp_trac1_mode_maxh()
    sa.calc_mark_max()
    sa.calc_mark_max_next()
    time.sleep(3)
    tsex=sa.calc_mark_x()
    tsey=sa.calc_mark_y()
    temp_tsex = stringToDig(tsex)
    temp_tsey = stringToDig(tsey)
    print('TSE1: FREQ:%s       LEVEL:%s'%(tsex,tsey))
    myGUI_grid.GUI.msg.insert('end', 'tse_1G~7G       %.2fGHz,     %.2fdBm\n'%(temp_tsex[0]*1e-9,temp_tsey[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_tse')
    sa.hcop_next()
    pass
    sa.close()


def test_LTE_OBW_11(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                    offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_san()
    sa.freq_cent(freq)
    sa.freq_span(30)
    sa.band(30)
    sa.band_vid(100)
    sa.inp_att(30)
    sa.disp_wind_trac_y_rlev_offs(spectrum_offset)
    sa.disp_wind_trac_y_rlev(30)
    sa.swe_time(100)
    sa.det_rms()
    sa.trig_sour_ext()
    sa.trig_slop_pos()
    sa.swe_egat_on()
    sa.swe_egat_type_edge()
    sa.swe_egat_hold(60)
    sa.swe_egat_leng(128)
    sa.pow_bwid_99pct()
    sa.calc_mark_func_pow_sel_obw()
    time.sleep(10)
    obw=sa.calc_mark_func_pow_res_obw()
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_obw')
    sa.hcop_next()
    sa.close()
    print('OBW:',obw)
    temp_obw = stringToDig(obw)
    myGUI_grid.GUI.msg.insert('end', 'obw_11:                 %.2fMHz\n'%((temp_obw[0])*1e-6), 'INT')
    myGUI_grid.GUI.msg.see('end')


def test_LTE_EVM_FERR_OSTP_20(spectrum_ip,freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                              offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_off()
    sa.fram_coun_auto_off()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    #===========================================================================
    # sa.mmem_load_tmod_dl('2','%s'%dl_bw)
    #===========================================================================
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.disp_tabl_on()
    sa.init()
    time.sleep(3)
    evm = sa.fetc_summ_evm_dssf()
    while(evm == 'NAN'):
        time.sleep(1)
        evm = sa.fetc_summ_evm_dssf()
    ferr = sa.fetc_summ_ferr()
    ostp = sa.fetc_summ_ostp()
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_evm_ferr_ostp_2_0')
    sa.hcop_next()
    time.sleep(3)
    temp_evm = stringToDig(evm)
    temp_ferr = stringToDig(ferr)
    temp_ostp = stringToDig(ostp)
    print('EVM2_0_64QAM:',evm)
    print('FERR2_0:',ferr)
    print('OSTP2_0:',ostp)
    #===========================================================================
    # myGUI_grid.GUI.msg.insert('end', 'lte_evm_20_64qam: %s \nlte_ferr_20: %sHz dBm\nlte_ostp_20:%sdBm \n'%(evm,ferr,ostp), 'INT')
    #===========================================================================
    myGUI_grid.GUI.msg.insert('end', 'lte_evm_20_64qam:        %.2f\n'%(temp_evm[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_ferr_20:                    %.2fHz\n'%(temp_ferr[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_ostp_20:                   %.2fdBm\n'%(temp_ostp[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.close()


def test_LTE_EVM_FERR_OSTP_31(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                               offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_off()
    sa.fram_coun_auto_off()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    #===========================================================================
    # sa.mmem_load_tmod_dl('3_1','%s'%dl_bw)
    #===========================================================================
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.disp_tabl_on()
    sa.init()
    time.sleep(3)
    evm=sa.fetc_summ_evm_dssf()
    while(evm == 'NAN'):
        time.sleep(1)
        evm = sa.fetc_summ_evm_dssf()
    ferr = sa.fetc_summ_ferr()
    ostp = sa.fetc_summ_ostp()
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_evm_ferr_ostp_3_1')
    sa.hcop_next()
    time.sleep(3)
    temp_evm = stringToDig(evm)
    temp_ferr = stringToDig(ferr)
    temp_ostp = stringToDig(ostp)
    print('EVM3_1_64QAM:', evm)
    print('FERR3_1:', ferr)
    print('OSTP3_1:', ostp)
    myGUI_grid.GUI.msg.insert('end', 'lte_evm_31_64qam:       %.2f\n'%(temp_evm[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_ferr_31:                   %.2fHz\n'%(temp_ferr[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_ostp_31:                  %.2fdBm \n'%(temp_ostp[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.close()    


def test_LTE_EVM_FERR_32(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                         offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_off()
    sa.fram_coun_auto_off()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    #===========================================================================
    # sa.mmem_load_tmod_dl('3_2','%s'%dl_bw)
    #===========================================================================
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.disp_tabl_on()
    sa.init()
    time.sleep(3)
    evm1 = sa.fetc_summ_evm_dsqp()
    evm2 = sa.fetc_summ_evm_dsst()
    while(evm1 == 'NAN'):
        time.sleep(1)
        evm1 = sa.fetc_summ_evm_dsqp()
        evm2 = sa.fetc_summ_evm_dsst()
    ferr = sa.fetc_summ_ferr()
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_evm_ferr_3_2')
    sa.hcop_next()
    time.sleep(3)
    temp_evm1 = stringToDig(evm1)
    temp_evm2 = stringToDig(evm2)
    temp_ferr = stringToDig(ferr)
    print('EVM3_2_QPSK:', evm1)
    print('EVM3_2_16QAM:', evm2)
    print('FERR3_2:', ferr)
    myGUI_grid.GUI.msg.insert('end', 'lte_evm_32_qpsk:            %.2f\n'%(temp_evm1[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_evm_32_16qam:         %.2f\n'%(temp_evm2[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_ferr_32:                     %.2fHz \n'%(temp_ferr[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.close()


def test_LTE_EVM_FERR_33(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                         offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_off()
    sa.fram_coun_auto_off()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    #===========================================================================
    # sa.mmem_load_tmod_dl('3_3','%s'%dl_bw)
    #===========================================================================
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.disp_tabl_on()
    sa.init()
    time.sleep(3)
    evm1=sa.fetc_summ_evm_dsqp()
    evm2=sa.fetc_summ_evm_dsst()
    while(evm1 == 'NAN'):
        time.sleep(1)
        evm1=sa.fetc_summ_evm_dsqp()
        evm2=sa.fetc_summ_evm_dsst()
    ferr=sa.fetc_summ_ferr()
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_evm_ferr_3_3')
    sa.hcop_next()
    time.sleep(3)
    temp_evm1 = stringToDig(evm1)
    temp_evm2 = stringToDig(evm2)
    temp_ferr = stringToDig(ferr)
    print('EVM3_3_QPSK:', evm1)
    print('EVM3_3_16QAM:', evm2)
    print('FERR3_3:', ferr)
    myGUI_grid.GUI.msg.insert('end', 'lte_evm_33_qpsk:           %.2f\n'%(temp_evm1[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_evm_33_16qam:         %.2f\n'%(temp_evm2[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_ferr_33:                     %.2fHz\n'%(temp_ferr[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.close()


def test_LTE_RE_20(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                   offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_off()
    sa.fram_coun_auto_off()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    #===========================================================================
    # sa.mmem_load_tmod_dl('2','s%'%dl_bw)
    #===========================================================================
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.calc2_feed_stat_asum()
    sa.init()
    time.sleep(3)
    re1 = sa.trac_data_trace1()
    re1 = re1.split(',')
    re_qpsk = re1[38]
    re_64qam = re1[45]
    #===========================================================================
    # re_qpsk=float(re_qpsk)
    # re_64qam=float(re_64qam)
    #===========================================================================
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_re_2_0')
    sa.hcop_next()
    time.sleep(3)
    temp_re_qpsk = stringToDig(re_qpsk)
    temp_re_64qam = stringToDig(re_64qam)
    print('RE_PDCCH_QPSK_2_0:', re_qpsk)
    print('RE_PDSCH_64QAM_2_0:', re_64qam)
    myGUI_grid.GUI.msg.insert('end', 'lte_re_pdcch_qpsk_20:    %.2fdBm\n' % (temp_re_qpsk[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_re_pssch_64qam_20: %.2fdBm\n' % (temp_re_64qam[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.close()


def test_LTE_RE_31(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                   offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_off()
    sa.fram_coun_auto_off()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    #===========================================================================
    # sa.mmem_load_tmod_dl('3_1','%s'%dl_bw)
    #===========================================================================
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.calc2_feed_stat_asum()
    sa.init()
    time.sleep(3)
    re1 = sa.trac_data_trace1()
    re1 = re1.split(',')
    re_qpsk = re1[38]
    re_64qam = re1[45]
    #===========================================================================
    # re_qpsk=float(re_qpsk)
    # re_64qam=float(re_64qam)
    #===========================================================================
    sa.hcop_dev_col_on()
    sa.hcop_cmap_def4()
    sa.hcop_dev_lang_jpeg()
    sa.hcop_dest_mmem('lte_re_3_1')
    sa.hcop_next()
    time.sleep(3)
    temp_re_qpsk = stringToDig(re_qpsk)
    temp_re_64qam = stringToDig(re_64qam)
    print('RE_PDCCH_QPSK_3_1:', re_qpsk)
    print('RE_PDSCH_64QAM_3_1:', re_64qam)
    myGUI_grid.GUI.msg.insert('end', 'lte_re_pdcch_qpsk_31:   %.2f\n'%(temp_re_qpsk[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_re_pssch_64qam_31: %.2f\n'%(temp_re_64qam[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.close()


def test_LTE_RE_33(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                   offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_off()
    sa.fram_coun_auto_off()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    #===========================================================================
    # sa.mmem_load_tmod_dl('3_3','%s'%dl_bw)
    #===========================================================================
    
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.calc2_feed_stat_asum()
    sa.init()
    time.sleep(3)
    re1 = sa.trac_data_trace1()
    re1 = re1.split(',')
    if dl_bw == '10':
        re_16qam = re1[45]
        re_qpsk1 = re1[38]
        #=======================================================================
        # re_qpsk1=float(re_qpsk1)
        # re_16qam=float(re_16qam)
        #=======================================================================
        time.sleep(3)
        temp_re_qpsk1 = stringToDig(re_qpsk1)
        temp_re_16qam = stringToDig(re_16qam)
        print('RE_PDCCH_QPSK_3_3:', re_qpsk1)
        print('RE_PDSCH_16QAM_3_3:', re_16qam)
        myGUI_grid.GUI.msg.insert('end', 'lte_re_pdcch_qpsk_33:     %.2fdBm \n' % (temp_re_qpsk1[0]), 'INT')
        myGUI_grid.GUI.msg.insert('end', 'lte_re_pdsch_16qam_33:  %.2fdBm \n' % (temp_re_16qam[0]), 'INT')
        myGUI_grid.GUI.msg.see('end')
    else:    
        if re1[46] == '2':
           
           re_qpsk2 = re1[45]
           sa.hcop_dev_col_on()
           sa.hcop_cmap_def4()
           sa.hcop_dev_lang_jpeg()
           sa.hcop_dest_mmem('lte_re_pdsch_qpsk3_3')
           sa.hcop_next()
           while(re1[46] == '2'):
               time.sleep(1)
               re1 = sa.trac_data_trace1()
               re1 = re1.split(',')
           re_16qam = re1[45]
           sa.hcop_dev_col_on()
           sa.hcop_cmap_def4()
           sa.hcop_dev_lang_jpeg()
           sa.hcop_dest_mmem('lte_re_pdsch_16qam3_3')
           sa.hcop_next()
           re_qpsk1 = re1[38]
           #====================================================================
           # re_qpsk1=float(re_qpsk1)
           # re_qpsk2=float(re_qpsk2)
           # re_16qam=float(re_16qam)
           #====================================================================
           time.sleep(3)
           temp_re_qpsk1 = stringToDig(re_qpsk1)
           temp_re_qpsk2 = stringToDig(re_qpsk2)
           temp_re_16qam = stringToDig(re_16qam)
           print('RE_PDCCH_QPSK_3_3:', re_qpsk1)
           print('RE_PDSCH_QPSK_3_3:', re_qpsk2)
           print('RE_PDSCH_16QAM_3_3:', re_16qam)
           myGUI_grid.GUI.msg.insert('end', 'lte_re_pdcch_qpsk_33: %.2fdBm \n' % (temp_re_qpsk1[0]), 'INT')
           myGUI_grid.GUI.msg.insert('end', 'lte_re_pdsch_qpsk_33: %.2fdBm \n' % (temp_re_qpsk2[0]), 'INT')
           myGUI_grid.GUI.msg.insert('end', 'lte_re_pdsch_16qam_33: %.2fdBm \n' % (temp_re_16qam[0]), 'INT')
           myGUI_grid.GUI.msg.see('end')

        else:
           re_16qam = re1[45]
           sa.hcop_dev_col_on()
           sa.hcop_cmap_def4()
           sa.hcop_dev_lang_jpeg()
           sa.hcop_dest_mmem('lte_re_pdsch_16qam3_3')
           sa.hcop_next()

           while(re1[46] != '2'):
               time.sleep(1)
               re1 = sa.trac_data_trace1()
               re1 = re1.split(',')
           re_qpsk2 = re1[45]
           sa.hcop_dev_col_on()
           sa.hcop_cmap_def4()
           sa.hcop_dev_lang_jpeg()
           sa.hcop_dest_mmem('lte_re_pdsch_qpsk3_3')
           sa.hcop_next()
           re_qpsk1=re1[38]
           #====================================================================
           # re_qpsk1=float(re_qpsk1)
           # re_qpsk2=float(re_qpsk2)
           # re_16qam=float(re_16qam)
           #====================================================================
           time.sleep(3)
           temp_re_qpsk1 = stringToDig(re_qpsk1)
           temp_re_qpsk2 = stringToDig(re_qpsk2)
           temp_re_16qam = stringToDig(re_16qam)
           print('RE_PDCCH_QPSK_3_3:',re_qpsk1)
           print('RE_PDSCH_QPSK_3_3:',re_qpsk2)
           print('RE_PDSCH_16QAM_3_3:',re_16qam)
           myGUI_grid.GUI.msg.insert('end', 'lte_re_pdcch_qpsk_33: %.2fdBm \n'%(temp_re_qpsk1[0]), 'INT')
           myGUI_grid.GUI.msg.insert('end', 'lte_re_pdsch_qpsk_33: %.2fdBm \n'%(temp_re_qpsk2[0]), 'INT')
           myGUI_grid.GUI.msg.insert('end', 'lte_re_pdsch_16qam_33: %.2fdBm \n'%(temp_re_16qam[0]), 'INT')
   
        
def test_LTE_RE_32(spectrum_ip, freq, sample, offset_wanted, sour_wanted, freq_unwanted,
                   offset_unwanted, sour_unwanted, spectrum_offset, dl_bw):
    sa = SA('%s' % spectrum_ip, 5025)
    sa.rst()
    sa.inst_lte()
    time.sleep(5)
    sa.freq_cent(freq)
    sa.conf_dl_bw(dl_bw)
    sa.conf_dl_cycp_norm()
    sa.disp_trac_y_rlev_offs(spectrum_offset)
    sa.fram_coun_stat_off()
    sa.fram_coun_auto_off()
    sa.swe_time(20.1)
    sa.conf_oop_nfr(25)
    sa.oop_ncor_off()
    sa.conf_dl_mimo_conf_tx1()
    sa.conf_dl_mimo_asel_ant1()
    sa.swap_off()
    sa.trig_mode_ext()
    sa.trig_hold(-10)
    sa.trig_lev(1.4)
    sa.freq_span_auto_on()
    sa.pow_ach_aach_eutra()
    sa.pow_ach_txch_coun(2)
    sa.pow_ach_band_chan2(dl_bw)
    sa.pow_ach_spac_chan(dl_bw)
    time.sleep(3)
    sa.pow_ach_txch_coun(1)
    sa.pow_ncor_off()
    sa.pow_sem_cat_a()
    #===========================================================================
    # sa.mmem_load_tmod_dl('3_2','%s'%dl_bw)
    #===========================================================================
    sa.dl_dem_cest_tgpp()
    sa.dl_dem_evmc_tgpp()
    sa.dl_dem_cbsc_on()
    sa.dl_dem_auto_on()
    sa.dl_form_pscd_phydet()
    sa.dl_dem_best_on()
    sa.dl_dem_prd_auto()
    sa.dl_dem_mcf_off()
    sa.dl_trac_phas_off()
    sa.dl_trac_time_off()
    sa.conf_dl_tdd_udc(3)
    sa.conf_dl_tdd_spsc(8)
    sa.conf_dl_plc_cid_auto()
    sa.calc2_feed_stat_asum()
    sa.init()
    time.sleep(3)
    re1 = sa.trac_data_trace1()
    re1 = re1.split(',')

    if re1[46] == '2':
       while(re1[46] == '2'):
            time.sleep(0.5)
            re1=sa.trac_data_trace1()
            re1=re1.split(',') 
       re_16qam=re1[45]
       sa.hcop_dev_col_on()
       sa.hcop_cmap_def4()
       sa.hcop_dev_lang_jpeg()
       sa.hcop_dest_mmem('lte_re_pdsch_16qam3_2')
       sa.hcop_next()
       while(re1[46]!='2'):
            time.sleep(1)
            re1=sa.trac_data_trace1()
            re1=re1.split(',') 
       re_qpsk2=re1[45]
       sa.hcop_dev_col_on()
       sa.hcop_cmap_def4()
       sa.hcop_dev_lang_jpeg()
       sa.hcop_dest_mmem('lte_re_pdsch_qpsk3_2')
       sa.hcop_next()

    else:
       while(re1[46]=='2'):
            time.sleep(0.5)
            re1=sa.trac_data_trace1()
            re1=re1.split(',') 
       re_16qam=re1[45]
       sa.hcop_dev_col_on()
       sa.hcop_cmap_def4()
       sa.hcop_dev_lang_jpeg()
       sa.hcop_dest_mmem('lte_re_pdsch_16qam3_2')
       sa.hcop_next()
       
       while(re1[46]!='2'):
            time.sleep(1)
            re1=sa.trac_data_trace1()
            re1=re1.split(',') 
       re_qpsk2=re1[45]
       sa.hcop_dev_col_on()
       sa.hcop_cmap_def4()
       sa.hcop_dev_lang_jpeg()
       sa.hcop_dest_mmem('lte_re_pdsch_qpsk3_2')
       sa.hcop_next()
       
    re_qpsk1 = re1[38]
    #===========================================================================
    # re_qpsk1=float(re_qpsk1)
    # re_qpsk2=float(re_qpsk2)
    # re_16qam=float(re_16qam)
    #===========================================================================
    time.sleep(3)
    temp_re_qpsk1 = stringToDig(re_qpsk1)
    temp_re_qpsk2 = stringToDig(re_qpsk2)
    temp_re_16qam = stringToDig(re_16qam)
    print('RE_PDCCH_QPSK_3_2:', re_qpsk1)
    print('RE_PDSCH_QPSK_3_2:', re_qpsk2)
    print('RE_PDSCH_16QAM_3_2:',re_16qam)
    myGUI_grid.GUI.msg.insert('end', 'lte_re_pdcch_qpsk_32:    %.2f\n'%(temp_re_qpsk1[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_re_pdsch_qpsk_32:    %.2f\n'%(temp_re_qpsk2[0]), 'INT')
    myGUI_grid.GUI.msg.insert('end', 'lte_re_pdsch_16qam_32: %.2f\n'%(temp_re_16qam[0]), 'INT')
    myGUI_grid.GUI.msg.see('end')
    sa.close()
    
    
    
    

##    pow_aclr=repr(a)
##    sa.calc_mark_mode_pos()
##    time.sleep(2)
##    sa.calc_mark_max()
##    time.sleep(5)
    
##    currentfreq=sa.calc_mark_x()
##    currentpower=sa.calc_mark_y()
##    sa.calc_mark_aoff()
    
##    sing.stop()
    

if __name__ == '__main__':
    # test_OBW('172.33.12.19', 415, 20)
    sa = SA('172.33.12.223', 5025)
    # sa.rst()
    # sa.disp_wind_trac_y_rlev(10)
    sa.close()

    

