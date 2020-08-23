
# -*- coding: utf-8 -*-
# 20150516 python3.3.2

__version__ = '0.1'

import socket
import struct, queue, threading, logging
import protocal
import configparser
import re
import time


__all__ = ['emsUdpServer', 'messageHandler']

test_times_statistics_lock = threading.Lock()
config_para = configparser.ConfigParser()
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

# logger = logging.getLogger('test')
socket_message = queue.Queue()
headType = '>IBBHIIII'
tailType = 'I'

error_flag = 0

myGUI_grid.GUI.msg.tag_configure('INT', foreground='blue')
myGUI_grid.GUI.msg.tag_configure('fail', foreground='red')


class emsUdpServer:
        timeout = None
        allow_reuse_address = True
        address_family = socket.AF_INET
        socket_type = socket.SOCK_DGRAM
        max_packet_size = 8192
        allow_reuse_address = False

        def __init__(self, server_address, RequestHandlerClass, bind_and_activate=True):
                """Constructor. May be extended, do NOT override."""
                self.server_address = server_address
                self.RequestHandlerClass = RequestHandlerClass
                self.__is_shut_down = threading.Event()
                self.__shutdown_request = False
                self.socket = socket.socket(self.address_family, self.socket_type)
                if bind_and_activate:
                        self.server_bind()
                        self.server_activate()

        def server_bind(self):
                if self.allow_reuse_address:
                        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDER, 1)
                self.socket.bind(self.server_address)

        def server_activate(self):
                # No need to call listen() for UDP.
                #self.socket.listen(5) #for TCP
                pass

        def serve_forever(self):
                self.__is_shut_down.clear()
                try:
                        while not self.__shutdown_request:
                                self._handle_request()
                finally:
                        self.__shutdown_request = False
                        self.__is_shut_down.set()

        def shutdown(self):
                """Stops the serve_forever loop.
                Blocks until the loop has finished. This must be called while
                serve_forever() is running in another thread, or it will
                deadlock.
                """
                self.__shutdown_request = True
                self.__is_shut_down.wait()

        def _handle_request(self):
                try:
                        request, client_address = self.get_request()
                except socket.error:
                        return
                if self.verify_request(request, client_address):
                        try:
                                self.process_request(request, client_address)
                        except:
                                self.handle_error(request, client_address)
                                self.shutdown_request(request)

        def get_request(self):
                data, client_addr = self.socket.recvfrom(self.max_packet_size)
                return (data, self.socket), client_addr

        def verify_request(self, request, client_address):
                """Verify the request.  May be overridden. 
                Return True if we should proceed with this request. """
                return True

        def process_request(self, request, client_address):
                """Call finish_request."""
                self.finish_request(request, client_address)
                self.shutdown_request(request) 
        
        def finish_request(self, request, client_address):
                """Finish one request by instantiating RequestHandlerClass."""
                self.RequestHandlerClass(request, client_address, self)

        def shutdown_request(self, request):
                """Called to shutdown and close an individual request."""
                # no need to shutdown anything in udp server
                pass
        
        def handle_error(self, request, client_address):
                print('-'*40)
                print('Exception happened during processing of request from')
                print(client_address)
                import traceback
                traceback.print_exc() # XXX But this goes to stderr!
                print('-'*40)


class messageHandler:
        def __init__(self, request, client_address, server):
                self.request = request
                self.client_address = client_address
                self.server = server

                try:
                        self.setup()
                finally:
                        self.finish()

        def setup(self):
                self.packet = self.request[0]
                self.socket= self.request[1]
                socket_message.put(self.packet)

        def finish(self):
                
                pass


def socket_message_handle():
        while 1:
                packet = socket_message.get()
                lenth = len(packet)
                paraLenth = lenth - 28
                messageType = headType+'B'*paraLenth+tailType
                data = struct.unpack(messageType, packet)
                # print(data)
                if hex(data[0]) == '0xaabbbeef' and hex(data[-1]) == '0xaa5555aa':
                        pltIdenty = data[1]
                        boardIdenty = data[2]
                        testID = data[3]
                        totalTimes = data[4]
                        succTimes = data[6]
                        failTimes = data[5]

                        dataLen = data[7]
                        slot = data[8]

                        if testID == 0xAAAA:
                                heartbeat_count = data[9]
                                printMsg = '接收BBU心跳[%d]\r\n' % heartbeat_count
                                myGUI_grid.inert_message_queue.put(printMsg)
                                rru_info_raw_data = data[10: -1]
                                # print(len(rru_info_raw_data))
                                rru_info_type = '!BBI'+'I'*3+ 'IIII'+'B'*32*4+ 'HHhhHHH'
                                rru_info_raw_type = '!'+'B'*(struct.calcsize(rru_info_type))
                                rru_info_raw = struct.pack(rru_info_raw_type, *rru_info_raw_data)
                                rru_info_real_data = struct.unpack(rru_info_type, rru_info_raw)
                                # print(rru_info_real_data)
                                sw_pkg_ver = hex(rru_info_real_data[3])
                                sw_app_ver = hex(rru_info_real_data[4])
                                sw_fpga_ver = hex(rru_info_real_data[5])

                                hw_master_ver = hex(rru_info_real_data[6])
                                hw_slave_ver = hex(rru_info_real_data[7])
                                hw_mpa_ver = hex(rru_info_real_data[8])
                                hw_spa_ver = hex(rru_info_real_data[9])

                                master_sn_byte = struct.pack('!'+'B'*32, *rru_info_real_data[10:42])
                                mster_sn_str = master_sn_byte.decode('gbk')
                                slave_sn_byte = struct.pack('!'+'B'*32, *rru_info_real_data[42:74])
                                slave_sn_str = slave_sn_byte.decode('gbk')
                                mpa_sn_byte = struct.pack('!'+'B'*32, *rru_info_real_data[74:106])
                                mpa_sn_str = mpa_sn_byte.decode('gbk')
                                spa_sn_byte = struct.pack('!'+'B'*32, *rru_info_real_data[106:138])
                                spa_sn_str = spa_sn_byte.decode('gbk')

                                max_tx_power = str(float(rru_info_real_data[138])*0.1)+'dBm'
                                max_rx_power = str(float(rru_info_real_data[139])*0.1)+'dBm'
                                ant_state = hex(rru_info_real_data[-2]).upper()
                                product_type = hex(rru_info_real_data[-1]).upper()
                                rru_info = rru_info_real_data[0:3]+ \
                                            (sw_pkg_ver, sw_app_ver, sw_fpga_ver, hw_master_ver, hw_slave_ver, hw_mpa_ver,
                                             hw_spa_ver, mster_sn_str, slave_sn_str, mpa_sn_str, spa_sn_str, max_tx_power,
                                            max_rx_power) + \
                                        rru_info_real_data[140: -2] + (ant_state, product_type)

                                for i in range(21):
                                        s = str(rru_info[i])
                                        myGUI_grid.GUI.value_eeprom[i].set(s)
                                protocal.sem_heartbeat.set()

                        elif testID in [0xA001, 0xA002, 0xA003, 0xA004, 0xA005, 0xA006, 0xA007, 0xA008, 0xA009, 0xA00A,
                                        0xA00B, 0xA00C, 0xA00D, 0xA00E, 0xA00F, 0xA010]:
                                cmd_message = protocal.protocol_dic[testID][0]
                                if succTimes == 1:
                                        myGUI_grid.inert_message_queue.put('%s成功...\n' % cmd_message)
                                if failTimes == 1:
                                        myGUI_grid.inert_message_queue.put('%s超时...\n' % cmd_message)

                        # 0xfefe表示打印信息
                        elif testID == 65278:

                                printData = data[8:-1]
                                printByte = struct.pack('B'*dataLen,*printData)
                                try:
                                        printMsg = printByte.decode('gbk')
                                except UnicodeDecodeError:
                                        printMsg = printByte.decode('latin-1')
                                printMsg = printMsg[:-1]

                                if 'cell ID' in printMsg:
                                        cell_ID = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_cell_response[0].set(cell_ID)
                                        # myGUI_grid.inert_message_queue.put(printMsg)
                                elif 'cell config result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_cell_response[1].set(result)

                                elif (('version query result' in printMsg) or ('version download result' in printMsg) or\
                                        ('version activate result' in printMsg)):
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_version_test.set(result)

                                elif 'RF status query result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_rfstatus_result.set(result)

                                elif 'RRU status query result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_rrustatus_response.set(result)

                                elif 'rf lo freq' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_rrustatus_para[0].set(result)

                                elif 'rf lo is' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        temp = temp.strip()
                                        if temp == '0':
                                                result = '锁定'
                                        elif temp == '1':
                                                result = '失锁'
                                        else:
                                                result = '错误'
                                        myGUI_grid.GUI.value_rrustatus_para[1].set(result)

                                elif 'clock is' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        temp = temp.strip()
                                        if temp == '0':
                                                result = '同步'
                                        elif temp == '1':
                                                result = '失步'
                                        else:
                                                result = '错误'
                                        myGUI_grid.GUI.value_rrustatus_para[2].set(result)

                                elif 'ir interface' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        temp = temp.strip()
                                        if temp == '1':
                                                result = '普通模式'
                                        elif temp == '2':
                                                result = '级联模式'
                                        else:
                                                result = '错误'
                                        myGUI_grid.GUI.value_rrustatus_para[3].set(result)

                                elif 'work mode' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        temp = temp.strip()
                                        if temp == '0':
                                                result = '未运营'
                                        elif temp == '1':
                                                result = '测试中'
                                        elif temp == '2':
                                                result = '运营中'
                                        elif temp == '3':
                                                result = '版本升级中'
                                        elif temp == '4':
                                                result = '故障'
                                        else:
                                                result = '错误'
                                        myGUI_grid.GUI.value_rrustatus_para[4].set(result)

                                elif 'main board temp' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_rrustatus_para[5].set(result)

                                elif 'slave board temp' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_rrustatus_para[6].set(result)

                                elif 'system time is' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_rrustatus_para[8].set(result)

                                elif 'running time' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_rrustatus_para[7].set(result)

                                elif 'RF CHN' in printMsg:
                                        i = printMsg.index('CHN')
                                        ahead = printMsg[i:].index('[')
                                        channel = int(printMsg[i:][ahead+1])
                                        if 'channel temp' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=2, value=temp)

                                        elif 'tx power is' in printMsg:
                                                tx_power = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=3, value=tx_power)

                                        elif 'rx power is' in printMsg:
                                                rx_power = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=5, value=rx_power)

                                        elif 'tx gain' in printMsg:
                                                tx_gain = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=4, value=tx_gain)

                                        elif 'rx gain' in printMsg:
                                                rx_gain = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=6, value=rx_gain)

                                        elif 'vswr is' in printMsg:
                                                vswr = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=7, value=vswr)

                                        elif 'vswr calc' in printMsg:
                                                vswr_calc_result = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=10, value=vswr_calc_result)

                                        elif 'rx power result' in printMsg:
                                                rx_power_result = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=9, value=rx_power_result)

                                        elif 'tx power result' in printMsg:
                                                tx_power_result = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_rfstatus.table.set(item=channel+1, column=8, value=tx_power_result)

                                elif 'Fiber status query' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_fiberstatus_response.set(result)

                                elif 'Fiber CHN' in printMsg:
                                        i = printMsg.index('CHN')
                                        ahead = printMsg[i:].index('[')
                                        channel = int(printMsg[i:][ahead+1])
                                        # print(printMsg, channel)
                                        if 'rx power' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_fiberstatus.table.set(item=channel+1, column=2, value=temp)

                                        elif 'tx power' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_fiberstatus.table.set(item=channel+1, column=3, value=temp)

                                        elif 'ineffect' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_fiberstatus.table.set(item=channel+1, column=4, value=temp)

                                        elif 'vendor' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_fiberstatus.table.set(item=channel+1, column=5, value=temp)

                                        elif 'transfer rate' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_fiberstatus.table.set(item=channel+1, column=6, value=temp)

                                        elif 'temp' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_fiberstatus.table.set(item=channel+1, column=7, value=temp)

                                        elif 'voltage' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_fiberstatus.table.set(item=channel+1, column=8, value=temp)

                                        elif 'current' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_fiberstatus.table.set(item=channel+1, column=9, value=temp)

                                elif 'power calibration response' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_calibration_result[0].set(result)

                                elif 'power calibration result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        temp = temp.strip()
                                        if temp == '00':
                                                result = '失败'
                                        else:
                                                result = temp
                                        myGUI_grid.GUI.value_calibration_result[1].set(result)

                                elif 'calibration RMS' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        temp = temp.strip()
                                        if temp == '00':
                                                result = '失败'
                                        else:
                                                result = temp
                                        myGUI_grid.GUI.value_calibration_result[2].set(result)

                                elif 'DPD result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        temp = temp.strip()
                                        if temp == '00':
                                                result = '失败'
                                        else:
                                                result = temp
                                        myGUI_grid.GUI.value_calibration_result[3].set(result)

                                elif 'calibration CHN' in printMsg:
                                        i = printMsg.index('CHN')
                                        ahead = printMsg[i:].index('[')
                                        channel = int(printMsg[i:][ahead+1])
                                        if 'tx power target' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_calibration.table.set(item=channel+1, column=2, value=temp)

                                        elif 'tx power offset' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_calibration.table.set(item=channel+1, column=3, value=temp)

                                        elif 'dl power gain' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_calibration.table.set(item=channel+1, column=4, value=temp)

                                        elif 'ul power gain' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_calibration.table.set(item=channel+1, column=5, value=temp)

                                        elif 'rx power' in printMsg:
                                                temp = printMsg.split(' ')[-1]
                                                myGUI_grid.GUI.sub_calibration.table.set(item=channel+1, column=6, value=temp)

                                elif 'alarm state' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        temp = temp.strip()
                                        if temp == '0':
                                                result = '无告警'
                                        elif temp == '1':
                                                result = '有告警'
                                        else:
                                                result = '错误'
                                        myGUI_grid.GUI.value_alarm_response.set(result)

                                elif 'freq IO' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[0].set(result)

                                elif 'freq band' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[1].set(result)

                                elif 'freq min' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[2].set(result)

                                elif 'freq max' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[3].set(result)

                                elif 'RF total channel' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[4].set(result)

                                elif 'RF Freq K' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[5].set(result)

                                elif 'RF Freq X' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[6].set(result)

                                elif 'hardware type' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[7].set(result)

                                elif 'product type' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_hardware_result[8].set(result)

                                elif 'reset result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_reboot_response.set(result)

                                elif 'FiberDelay measure' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_delay_test.set(result)

                                elif 'fiber delay config' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_delay_response.set(result)

                                elif 'parameter query result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_parameter_response.set(result)

                                elif 'vswr threshold' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_parameter_result[0].set(result)

                                elif 'board temp threshold' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_parameter_result[1].set(result)

                                elif 'rf channel temp threshold' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_parameter_result[2].set(result)

                                elif 'total time-slot' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_parameter_result[3].set(result)

                                elif 'dl time-slot' in printMsg:
                                        result = printMsg.split(' ')[-1]
                                        myGUI_grid.GUI.value_parameter_result[4].set(result)

                                elif 'system time config response' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_time_config_response.set(result)

                                elif 'system time config result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_time_config_result.set(result)

                                elif 'antenna state config result' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_mask_config_result.set(result)

                                elif 'antenna state config response' in printMsg:
                                        temp = printMsg.split(' ')[-1]
                                        result = result_show(temp)
                                        myGUI_grid.GUI.value_mask_config_response.set(result)

                                else:
                                        try:
                                                print(printMsg)
                                        except UnicodeEncodeError:
                                                print(str(printByte))
                                        myGUI_grid.inert_message_queue.put(printMsg)

                else:
                        # myGUI_grid.inert_message_queue.put('Unexpect message!\n')
                        print(packet)
                time.sleep(0.1)


def result_show(result):
        result = result.strip()
        if (result == '00') or (result == '0'):
                result = '成功'
        else:
                result = '失败'
        return result


def test():
        
        server = emsUdpServer(('172.33.12.21', 9000), messageHandler)
        server.serve_forever()


if __name__ == '__main__':
        test()

