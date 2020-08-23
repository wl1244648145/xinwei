import paramiko
import os, os.path
import time
import threading
import ftp    


class mySFTP:

        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        src_file_list = []
        
        def __init__(self,ip, port, usrname, password):
                self.ip = ip
                self.port = port
                self.usrname = usrname
                self.password = password
                
                try:
                    self.ssh.connect(hostname=self.ip, port=self.port, username=self.usrname, password=self.password)
                    print('%s ssh connect succed!' % self.ip)
                    
                except:
                     print('%s ssh connect failed!' % self.ip)
                     raise
                try:
                    self.sftp = self.ssh.open_sftp()
                    print('%s sftp connect succed!' % self.ip)
                except:
                    print('%s sftp connect failed!' % self.ip)
            

        def local_file_list(self, path):
            if os.path.isdir(path):
                    files = os.listdir(path)
                    for file in files:
                        file_dir = path + '\\' + file
                        if os.path.isfile(file_dir):
                            self.src_file_list.append(file_dir)
                        elif os.path.isdir(file_dir):
                            self.local_file_list(file_dir)
                        else:
                            print('path is not a file or direction!')

            elif os.path.isfile(path):
##                    print('file11')
                    self.src_file_list.append(path)
        
            else:
                    print('path is not a file or a direction!')
                    
        def compile_ver(self, compile_dir):
             stdin,stdout,stderr = self.ssh.exec_command('cd %s;make' % compile_dir)
##             compile_info = stdout.readlines()
             compile_info = stderr.readlines()
             i = 0
             while(i<len(compile_info)):
                 print(compile_info[i])
                 i+=1
             print('--------- compile complete --------')

        def upload_file(self, dst_code_dir):
            for file in self.src_file_list:
                index = file.index('CardType')
                sub_dir = file[index:].replace('\\', '/')
                server_file = dst_code_dir + sub_dir
        
                try:
                    self.sftp.put(file, server_file)
                    print('%s upload success!!' % server_file)
                except FileNotFoundError:
                    rindex = server_file.rindex('/')
                    self.ssh.exec_command('mkdir -p %s' % server_file[:rindex])
                    time.sleep(2)
                    self.sftp.put(file, server_file)
                    print('%s upload success!!' % server_file)
            

        def download_file(self, remote_exe_file, local_exe_file):
            self.sftp.get(remote_exe_file, local_exe_file)
            print('%s download success!!' % local_exe_file)
            self.ssh.exec_command('rm %s' % remote_exe_file)


def test():

    dst_ip = '172.33.12.250'
    dst_port = 22
    usrname = 'root'
    password = 'xinwei'
    dst_code_Dir= '/mnt/test/'

    local_code_Dir1 = 'E:\\PLTest\\CardType\\codeplatform\\bsp\\modules\\rrutest'
    local_code_Dir2 = 'E:\\PLTest\\CardType\\codeplatform\\bsp\\modules\\boardtest\\src\\bsp_boardtest.c'
    local_code_Dir3 = 'E:\\PLTest\\CardType\\codeplatform\\bsp\\com_inc\\bsp_boardtest_ext.h'
  
    compile_dir = '/mnt/test/CardType'
    remote_exe_file = '/mnt/test/CardType/output/bsp_app'
    local_exe_file = 'E:\\PLTest\\CardType\\output\\bsp_app'

    sftp = mySFTP(dst_ip, dst_port, usrname, password)
    sftp.local_file_list(local_code_Dir1)
    time.sleep(1)
    sftp.local_file_list(local_code_Dir2)
    time.sleep(1)
    sftp.local_file_list(local_code_Dir3)
    time.sleep(1)

    sftp.upload_file(dst_code_Dir)
    time.sleep(1)
##    print(sftp.src_file_list)

    sftp.compile_ver(compile_dir)
    time.sleep(1)
    sftp.download_file(remote_exe_file, local_exe_file)
    sftp.sftp.close()
    sftp.ssh.close()

    cmd = input('Download?(Y/N)')
    if cmd == 'Y':
            try:
                    ftp.ftp_upload('172.33.12.188', local_dir=local_exe_file, des_dir='/mnt/btsa', fun=0)
            except 550:
                    pass
            finally:
                    ftp.ftp_upload('172.33.12.188', local_dir=local_exe_file, des_dir='/mnt/btsa')
    else:
            return
                  

if __name__ == '__main__':

    test()
   







                
