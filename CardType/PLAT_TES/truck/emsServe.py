
# -*- coding: utf-8 -*-
# 20150516 python3.3.2

__version__ = '0.1'

import socket
import struct, queue, threading, logging
import sqlite
import configparser
import re


__all__ = ['emsUdpServer', 'messageHandler']

test_times_statistics_lock = threading.Lock()
config_para = configparser.ConfigParser()
config_para.read('para.ini')

platform_type = config_para.get('eNB', 'TYPE')
if platform_type == 'CZZ':
    import myGUI_grid_CZZ as myGUI_grid
if platform_type == 'eBBU':
    import myGUI_grid_eBBU as myGUI_grid

logger = logging.getLogger('test')
socket_message = queue.Queue()
drying_pattern = re.compile('干接点[0-7]测试失败')

headType = '>IBBHIIII'
tailType = 'I'

recValue = sqlite.rec_times_statistics
eepromRec = sqlite.rec_times_eeprom

rec_times_update = sqlite.rec_times_update
error_flag = 0

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
                data=struct.unpack(messageType, packet)

                if hex(data[0]) == '0xaabbbeef' and hex(data[-1]) == '0xaa5555aa':
                        pltIdenty = data[1]
                        boardIdenty = data[2]
                        testID = data[3]
                        totalTimes = data[4]
                        succTimes = data[6]
                        failTimes = data[5]

                        dataLen = data[7]
                        if testID in recValue:
                                test_times_statistics_lock.acquire()
                                if testID == 775:
                                        recValue[testID][0] += float(succTimes/2)
                                        recValue[testID][1] += float(failTimes/2)
                                        # print('------OK--------\n')
                                elif testID == 780 or testID == 526:
                                        recValue[testID][0] += float(succTimes/3)
                                        recValue[testID][1] += float(failTimes/3)
                                else:
                                        recValue[testID][0] += succTimes
                                        recValue[testID][1] += failTimes
                                test_times_statistics_lock.release()

                                if dataLen == paraLenth:
                                        pass

                                else:
                                        logger.info('数据长度有误 !')
                                        insert_message = '接收参数长度有误 !\n'
                                        myGUI_grid.inert_message_queue.put(insert_message)

                        elif testID in rec_times_update:
                                rec_times_update[testID][0] += succTimes
                                rec_times_update[testID][1] += failTimes

                        elif testID in eepromRec:
                                eepromRec[testID][0] += succTimes
                                eepromRec[testID][1] += failTimes

                                if dataLen == paraLenth:
                                        pass

                                else:
                                        logger.info('数据长度有误 !')
                                        insert_message = '接收参数长度有误 !\n'
                                        myGUI_grid.inert_message_queue.put(insert_message)


                        # 0xfefe表示打印信息
                        elif testID == 65278:

                                printData=data[8:-1]
                                printByte=struct.pack('B'*dataLen,*printData)
                                try:
                                        printMsg=printByte.decode('gbk')
                                except UnicodeDecodeError:
                                        printMsg = 'ffffffffffffffffffffffffff\n\n'
                                printMsg = printMsg[:-1]

                                if 'AIF' in printMsg:
                                        if ('DSP4' in printMsg) or ('dsp4' in printMsg):
                                                pass
                                        else:
                                                # logger.info(printMsg)
                                                myGUI_grid.inert_message_queue.put(printMsg)

                                elif '可追踪星数' in printMsg:
                                        i=printMsg.index('可追踪星数')
                                        j=i+5
                                        number=printMsg[j:-3]
                                        number=int(number)
                                        if number <= 3:

                                                myGUI_grid.inert_message_queue.put('GPS可追踪星数小于3！\n')
                                                recValue[778][0] -= 1
                                                recValue[778][1] += 1
                                        else:
                                                # logger.info(printMsg)
                                                myGUI_grid.inert_message_queue.put(printMsg)

                                elif 'DeviceID=' in printMsg:
                                        msg=printMsg.split('=')
                                        expMsg=msg[-1]
                                        expMsg=expMsg.strip()
                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[0].set(expMsg)
                                        elif '基带板' in printMsg:
                                                myGUI_grid.BBP.value_eeprom[0].set(expMsg)
                                        elif '交换板' in printMsg:
                                            try:
                                                myGUI_grid.GES.value_eeprom[0].set(expMsg)
                                            except AttributeError:
                                                myGUI_grid.FS.value_eeprom[0].set(expMsg)
                                        elif '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[0].set(expMsg)
                    
                                        elif '增强板' in printMsg:
                                                myGUI_grid.ES.value_eeprom[0].set(expMsg)
                                        else:
                                                pass
                                elif 'BoardType=' in printMsg:
                                        msg=printMsg.split('=')
                                        expMsg=msg[-1]
                                        expMsg=expMsg.strip()
                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[1].set(expMsg)
                                        elif '基带板' in printMsg:
                                                myGUI_grid.BBP.value_eeprom[1].set(expMsg)
                                        elif '交换板' in printMsg:
                                            try:
                                                myGUI_grid.GES.value_eeprom[1].set(expMsg)
                                            except AttributeError:
                                                myGUI_grid.FS.value_eeprom[1].set(expMsg)
                                                
                                        elif '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[1].set(expMsg)
                                        elif '增强板' in printMsg:
                                                myGUI_grid.ES.value_eeprom[1].set(expMsg)
                                        else:
                                                pass
                                elif 'MACADDR1=' in printMsg:
                                        msg=printMsg.split('=')
                                        expMsg=msg[-1].upper()
                                        expMsg=expMsg.strip()
                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[2].set(expMsg)
                                        elif '基带板' in printMsg:
                                                myGUI_grid.BBP.value_eeprom[2].set(expMsg)
                                        elif '交换板' in printMsg:
                                                myGUI_grid.GES.value_eeprom[2].set(expMsg)
                                        elif '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[2].set(expMsg)
                                        else:
                                                pass

                                elif 'MACADDR2=' in printMsg:
                                        msg=printMsg.split('=')
                                        expMsg=msg[-1].upper()
                                        expMsg=expMsg.strip()
                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[3].set(expMsg)
                                        elif '基带板' in printMsg:
                                                myGUI_grid.BBP.value_eeprom[3].set(expMsg)
                                        elif '交换板' in printMsg:
                                                myGUI_grid.GES.value_eeprom[3].set(expMsg)
                                        elif '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[3].set(expMsg)

                                        else:
                                                pass

                                elif 'ProductSN=' in printMsg:
                                        msg=printMsg.split('=')
                                        expMsg=msg[-1]
                                        expMsg=expMsg.strip()
                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[4].set(expMsg[:15])
                                        elif '基带板' in printMsg:
                                                myGUI_grid.BBP.value_eeprom[4].set(expMsg[:14])
                                        elif '交换板' in printMsg:
                                            try:
                                                myGUI_grid.GES.value_eeprom[4].set(expMsg[:15])
                                            except AttributeError:
                                                myGUI_grid.FS.value_eeprom[4].set(expMsg[:15])
                                    
                                        elif '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[4].set(expMsg)
                                        elif '增强板' in printMsg:
                                                myGUI_grid.ES.value_eeprom[4].set(expMsg)

                                        else:
                                                pass

                                elif 'Manufacture=' in printMsg:
                                        msg=printMsg.split('=')
                                        expMsg=msg[-1]
                                        expMsg=expMsg.strip()
                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[5].set(expMsg)
                                        elif '基带板' in printMsg:
                                                myGUI_grid.BBP.value_eeprom[5].set(expMsg)
                                        elif '交换板' in printMsg:
                                            try:
                                                myGUI_grid.GES.value_eeprom[5].set(expMsg)
                                            except AttributeError:
                                                myGUI_grid.FS.value_eeprom[5].set(expMsg)
                                               
                                        elif '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[5].set(expMsg)
                            
                                        elif '增强板' in printMsg:
                                                myGUI_grid.ES.value_eeprom[5].set(expMsg)
                                        else:
                                                pass

                                elif 'ProductDate=' in printMsg:
                                        msg = printMsg.split('=')
                                        preExpMsg = msg[-1]
                                        mothValue = preExpMsg[:2]
                                        mothValue = int(mothValue,base=16)
                                        dayValue = preExpMsg[2:4]
                                        dayValue = int(dayValue,base=16)
                                        yearValue = preExpMsg[4:]
                                        yearValue = int(yearValue,base=16)

                                        expMsg = str(yearValue) + '-' + str(mothValue) + '-'+str(dayValue)
                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[6].set(expMsg)
                                        elif '基带板' in printMsg:
                                                myGUI_grid.BBP.value_eeprom[6].set(expMsg)
                                        elif '交换板' in printMsg:
                                            try:
                                                myGUI_grid.GES.value_eeprom[6].set(expMsg)
                                            except AttributeError:
                                                myGUI_grid.FS.value_eeprom[6].set(expMsg)
                                               
                                        elif '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[6].set(expMsg)
                                        
                                        elif '增强板' in printMsg:
                                                myGUI_grid.ES.value_eeprom[6].set(expMsg)
                                        else:
                                                pass
                                elif 'SatelliteReceiver=' in printMsg:
                                        msg = printMsg.split('=')
                                        expMsg = msg[-1]
                                        expMsg = expMsg.strip()
                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[7].set(expMsg)

                                        else:
                                                pass
                                elif 'Fan_InitialSpeed' in printMsg:
                                        msg = printMsg.split('=')
                                        fan2_speed = msg[-1]
                                        fan0_speed_msg = msg[1]
                                        fan0_speed = fan0_speed_msg.split(',')[0]

                                        fan1_speed_msg = msg[2]
                                        fan1_speed = fan1_speed_msg.split(',')[0]
                                        # expMsg = fan0_speed + '|' + fan1_speed + '|' + fan2_speed
                                        expMsg = ('%d|%d|%d' % (int(fan0_speed, 16), int(fan1_speed, 16), int(fan2_speed, 16)))
                                        expMsg = expMsg.strip()
                                        # print(expMsg)
                                        if '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[8].set(expMsg)

                                        else:
                                                pass
                                elif 'TemperatureThreshold=' in printMsg:
                                        msg = printMsg.split('=')
                                        preExpMsg = msg[-1].split(' ')
                                        expMsg = preExpMsg[-1]+'~'+preExpMsg[0]

                                        if '主控板' in printMsg:
                                                myGUI_grid.MCT.value_eeprom[9].set(expMsg)
                                        elif '基带板' in printMsg:
                                                myGUI_grid.BBP.value_eeprom[9].set(expMsg)
                                        elif '交换板' in printMsg:
                                            try:
                                                myGUI_grid.GES.value_eeprom[9].set(expMsg)
                                            except AttributeError:
                                                myGUI_grid.FS.value_eeprom[9].set(expMsg)
                                               
                                        elif '风扇板' in printMsg:
                                                myGUI_grid.FM.value_eeprom[9].set(expMsg)
                                        elif '增强板' in printMsg:
                                                myGUI_grid.ES.value_eeprom[9].set(expMsg)

                                        else:
                                                pass
                                elif 'error' in printMsg:
                                        global error_flag
                                        error_flag = 1
                                        myGUI_grid.inert_message_queue.put('%s' % (printMsg))

                                # elif drying_pattern.search(printMsg):
                                #         pass

                                # elif '干结点状态' in printMsg:
                                #         bit0123 = [1, 2, 4, 8]
                                #         drying_msg = printMsg.split(',')
                                #         drying0123 = drying_msg[0].split('x')[-1]
                                #         drying0123 = int(drying0123, 16)
                                #
                                #         drying4567 = drying_msg[-1].split('x')[-1].strip()[:-1]
                                #         drying4567 = int(drying4567, 16)
                                #         if drying0123 != 0 or drying4567 != 0:
                                #                 fail_node = '失败节点：'
                                #                 for bit in bit0123:
                                #                         if drying0123 & bit == bit:
                                #                                 fail_node += str(bit0123.index(bit))
                                #                 for bit in bit0123:
                                #                         if drying4567 & bit == bit:
                                #                                 fail_node += str(bit0123.index(bit) + 4)
                                #
                                #                 myGUI_grid.inert_message_queue.put(fail_node+'\n')
                                #         myGUI_grid.inert_message_queue.put(printMsg)


                                elif ('AFC状态:' in printMsg) and boardIdenty==0:
                                        state=printMsg.split(':')[-1][:-1]
                                        myGUI_grid.GUI.value_AFC.set(state)

                                else:
                                        # logger.info(printMsg)
                                        myGUI_grid.inert_message_queue.put(printMsg)

                        else:
                            logger.info('Wrong test id: %d', testID)

                else:

                            logger.info('Unexpect message!!')
                                # myGUI_grid.inert_message_queue.put('Unexpect message!\n')
                                # print(packet)


def test():
        
        server = emsUdpServer(('172.33.12.21', 9000), messageHandler)
        server.serve_forever()


if __name__ == '__main__':
        test()

