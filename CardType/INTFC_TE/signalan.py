"""An SignalAnalyzer class and some helper functions.

NOTE:  The ROHDE&SCHWARZ SignalAnalzer's port is 5025, different from Aglinet's port 5023

Only tested on Agilent N9000A CXA Signal Analyzer.

Note:
It's user's responsibility adding delay between calling these helper functions

Here is a nice test:
python signalanalyzerlib.py 192.168.1.3 5023
"""


import sys
import time
import struct
import configInterface as conf

try:
    import SOCKS; socket = SOCKS; del SOCKS
except ImportError:
    import socket

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

    def pow_ach_txch_count(self, num):
        cmd = 'POW:ACH:TXCH:COUN '+str(num)
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

    def calc_mark_func_pow_sel_owb(self):
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
        
    def esp_pres(self, sem):
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
        
    def str_to_float(self, liter):
        length = len(liter)
        i = 0
        reliter = liter
        for count in range(0, length):
            reliter[i] = float(liter[i])
            i += 1
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


if __name__ == '__main__':
    # test_OBW('172.33.12.19', 415, 20)
    sa = SA('172.33.12.223', 5025)
    # sa.rst()
    # sa.disp_wind_trac_y_rlev(10)
    sa.close()

    

