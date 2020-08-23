
import ftplib,os.path
import socket,time
import os
import sys


def ftp_login(addr, user_name, password):
        try:
                f1 = ftplib.FTP(addr)
        except TimeoutError:
                print('BTS login timeout!')
                return -1
        try:
                f1.login(user_name, password)
                print('%s ftp login ok' % addr)
##                f1.debugging = 1
                return f1
        except ftplib.error_perm:
                print('BTS cannot login!')
                return
        


def ftp_upload(des_addr, local_dir='.', des_dir='/mnt/btsb/', user_name='root', password='12345678', fun=1):

        f1 = ftp_login(des_addr, user_name, password)
        if f1 == -1:
                return
        f1.cwd(des_dir)
        if os.path.isdir(local_dir):
                os.chdir(local_dir)
                file_list = os.listdir()
    
                # upload file
                if fun == 1:
                        for file in file_list:
                            path = des_dir
                            f1.storbinary('STOR %s' % (file), open(file, 'rb'))
                            print('%s uploading......' % file)

                # delete file
                if fun == 0:
                        f1.cwd(des_dir)
                        file_list = f1.nlst()
                        try:
                                for file in file_list:
                                    f1.delete(file)
                                    print('%s delete......'% (file))
                        except ftplib.error_perm:
                                print('no file to delete!!')
                                return
        elif os.path.isfile(local_dir):
                file=local_dir.split('\\')[-1]
                fileDir=local_dir[:-len(file)]
                os.chdir(fileDir)
##                print(des_dir+file,file)
                if fun == 1:
                        f1.storbinary('STOR %s'% (file),open(file,'rb'))
                        print('%s upload successful!'%(file))
                if fun == 0:
                        f1.delete(file)
                        print('%s delete!'%(file))
        else:
                print('目标文件不存在！\n')
               
        f1.quit()



def ftpD():
    filelist=[]
    fileLIST=[]
    
    localDir='E:\\McLTE\\ver\\fpga\\'
   
    try:
        f1.login('dl','dl')
    except ftplib.error_perm:
        print('cannot loggin!')
    
        return
    
    path='file/download/bts/XW7102/'
    title='平台版本'.encode(encoding='gbk').decode(encoding='latin-1')
    #title=b'\xc6\xbd\xcc\xa8\xb0\xe6\xb1\xbe'
    path=path+title
    
    filels=f1.retrlines('LIST %s'%(path),callback=filelist.append)
    f1.cwd(path)
    for file in filelist:
        fileAttr=file.split(' ')
        fileName=fileAttr[-1]
        fileType=fileAttr[0][0]
        if fileType=='-':
            fileLIST.append(fileName)

    for file in fileLIST:
        path=localDir+file
        f1.retrbinary('RETR %s'%(file),open(path,'wb').write)
        print('%s download.....'%(file))        
    f1.quit()


def test():
    path='file/download/bts/XW7102/'
    title='平台版本'.encode(encoding='gbk').decode(encoding='latin-1')
    path1=path+title   
    path2='mnt/btsb/'
    
    try:
        f1.login('dl','dl')
    except ftplib.error_perm:
        print('Cannot Loggin Server!')
        return
    
    try:
        f2.login()
    except ftplib.error_perm:
        print('Cannot Loggin Client!')    
        return
    
    f1.cwd(path1)
    f2.cwd(path2)
    fileList=f1.nlst()
    print(fileList)
   
    for file in fileList:
        print('transfer %s......'%(file))
        f1.voidcmd('TYPE I')
        f2.voidcmd('TYPE I')
        sock1=f1.transfercmd('RETR %s'%(file))
        sock2=f2.transfercmd('STOR %s'%(file))

        while 1:
    
                data=sock1.recv(1024)
                sock2.sendall(data)
                
                if len(data)==0:
                    break
                    sock1.close()
                    sock2.close()
        sock1.close()
        sock2.close()
                
                                           
        res1=f1.getresp()
        print('f1 >> %s'%(res1))
        res2=f2.getresp()
        print('f2 >> %s'%(res2))       
                             
    f1.quit()
    f2.quit()
    print('file transfer ok')

if __name__=='__main__':
    local_exe_file = 'E:\\PLTest\\cardtype_test_code_20170424\\output\\bsp_app'
    ftp_upload('172.33.12.189', local_dir=local_exe_file, des_dir='/mnt/btsa/',fun=0)
    ftp_upload('172.33.12.189', local_dir=local_exe_file, des_dir='/mnt/btsa/')
