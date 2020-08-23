#coding=utf-8

import ftplib
import os
import telnetlib
import time

norflash_file = [b'u-boot.bin', b'uImage', b'p2041bbu.dtb', b'rootfs.bbu.uboot', b'rcw.bin']


def ftp_login(addr, user_name, password):
        f1 = ftplib.FTP(addr)
        try:
                f1.login(user_name, password)
                print('okkkk')
                return f1
        except ftplib.error_perm:
                print('BTS cannot login!')
                return


def ftp_upload(des_addr, local_dir='.', des_dir='/mnt/btsb/', user_name='root', password='12345678', fun=1):
        
            os.chdir(local_dir)
            file_list = os.listdir()
            f1 = ftp_login(des_addr, user_name, password)

            # upload file
            if fun == 1:
                    for file in file_list:
                        path = des_dir
                        f1.storbinary('STOR %s' % (path+file), open(file, 'rb'))
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
           
            f1.quit()


def telnet2input(host, user_name, password):
    try:
        tn = telnetlib.Telnet(host,timeout=30)
    except TimeoutError:
        print('Telnet连接失败')
        return
    # tn.debuglevel = 1

    tn.read_until(b"login: ")

    tn.write(user_name.encode('ascii') + b"\n")

    tn.read_until(b"Password: ")
    tn.write(password.encode('ascii') + b"\n")
    time.sleep(1)
    tn.write('cd /mnt/btsa'.encode('ascii') + b"\n")

    return tn


def norflash_update(host, user_name='root', password='12345678'):
    real_exist_file = []

    uboot_update = 'flashcp -v u-boot.bin /dev/mtd9'.encode('ascii') + b'\n'

    uImage1_update = 'flashcp -v uImage /dev/mtd6'.encode('ascii') + b'\n'
    uImage2_update = 'flashcp -v uImage /dev/mtd7'.encode('ascii') + b'\n'

    dtb1_update = 'flashcp -v p2041bbu.dtb /dev/mtd2'.encode('ascii') + b'\n'
    dtb2_update = 'flashcp -v p2041bbu.dtb /dev/mtd3'.encode('ascii') + b'\n'

    rootfs1_update = 'flashcp -v rootfs.bbu.uboot /dev/mtd4'.encode('ascii') + b'\n'
    rootfs2_update = 'flashcp -v rootfs.bbu.uboot /dev/mtd5'.encode('ascii') + b'\n'

    tn = telnet2input(host, user_name, password)

    norflash_cmd_list = {b'u-boot.bin': (uboot_update,),
                         b'uImage': (uImage1_update, uImage2_update),
                         b'p2041bbu.dtb': (dtb1_update, dtb2_update),
                         b'rootfs.bbu.uboot': (rootfs1_update, rootfs2_update),
                         b'rcw.bin': (),
                         }
    if tn:
        tn.write('ls'.encode('ascii') + b'\n')
        time.sleep(2)
        file_list = tn.read_very_eager()
        for file in norflash_file:
            if file in file_list:
                real_exist_file.append(file)
        print(real_exist_file)

        for file in real_exist_file:
            for cmd in norflash_cmd_list[file]:

                tn.write(cmd)
                print(cmd)
                verify_data = tn.read_until(b'Verifying kb', 60)
                if b'Verifying kb' in verify_data:
                    print('%s文件校验中...' % file.decode())
                elif b'No such file or directory' in verify_data:
                    print('%s文件不存在！' % file.decode())
                    continue
                else:
                    print(verify_data)
                tn.read_until(b'/mnt/btsa #')
                time.sleep(1)
            print('%s升级完成' % file.decode())

        print('NorFlash文件升级完成..')
        tn.close()

    else:
        return


if __name__ == '__main__':
   ftp_upload('172.33.12.188', fun=0)
   ftp_upload('172.33.12.188', fun=1)
   # norflash_update('172.33.12.188')




