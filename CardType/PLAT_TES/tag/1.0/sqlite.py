
# -*- coding: utf-8 -*-
# 20150525 python 3.3.2 @ db

import sqlite3
import csv
import data
import logging
import configparser

# global variable
test_id_list = []
test_id_bbp = []
test_id_fm = []
test_id_mct = []
test_id_it = []
test_id_em = []
test_id_ges = []
test_id_fs = []
test_id_es = []

test_id_eeprom = []
test_id_eeprom_total = []

send_times_statistics = {}
rec_times_statistics = {}
send_times_eeprom = {}
rec_times_eeprom = {}
send_times_update = {}
rec_times_update = {}

config_para = configparser.ConfigParser()
config_para.read('para.ini')

platform_type = config_para.get('eNB', 'TYPE')


def db_generator(db, csv, sheet):
    conn = sqlite3.connect(db)
    sql_create = """CREATE TABLE "main".%s ("NO" INTERGER PRIMARY KEY NOT NULL, "TestCase" TEXT NOT NULL, "PlatType" INTEGER NOT NULL,
    "BoardType" INTEGER NOT NULL, "TestID" INTEGER NOT NULL, "TestTime" INTEGER NOT NULL,
    "FailTime" INTEGER NOT NULL, "SuccTime" INTEGER NOT NULL, "ParaLen" INTEGER NOT NULL)"""%sheet

#    .csv file need save as..,just change ext isnot open success
    data_all = open(csv).readlines()

    c = conn.cursor()
    c.execute(sql_create)
    conn.commit()

    cnt = 0

    for entry in data_all:

        value=entry.strip()
        value1=value.split(',')
        # print(value1)

##        int var use %s,str should use '%s'
        sql_insert = "INSERT INTO %s values ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' )"\
                     %(sheet, value1[0], value1[1], value1[2], value1[3], value1[4], value1[5], value1[6], value1[7],value1[8])
        cnt += 1
        c.execute(sql_insert)
    conn.commit()
    c.close()

    return cnt


def query_single_TestID(number, sheet):
    # data_struct = 'PlatType, BoardType, TestID, TestTime, FailTime, SuccTime, ParaLen'
    sql_query="SELECT * FROM {1} WHERE {0} in (TestID)".format(number, sheet)

    conn=sqlite3.connect('Testlist.db')
    c=conn.cursor()
    c.execute(sql_query)
    result=c.fetchall()
    # result=c.fetchone()
    c.close()
    # print(result)
    return result

def query_single_NO(number, sheet):
    # data_struct = 'PlatType, BoardType, TestID, TestTime, FailTime, SuccTime, ParaLen'
    sql_query="SELECT * FROM {1} WHERE {0} in (NO)".format(number, sheet)

##    print(sql_query)
    conn=sqlite3.connect('Testlist.db')
    c=conn.cursor()
    c.execute(sql_query)
    result=c.fetchall()
    # result=c.fetchone()
    c.close()
    # print(result)
    return result

def query_value(test_id, sheet):
    # data_struct = 'PlatType, BoardType, TestID, TestTime, FailTime, SuccTime, ParaLen'
    sql_query="SELECT TestCase FROM {1} WHERE {0} in (TestID)".format(test_id, sheet)
    conn=sqlite3.connect('Testlist.db')
    c=conn.cursor()
    c.execute(sql_query)
    result=c.fetchall()
    # result=c.fetchone()
    c.close()
    # print(result)
    return result


def query_column(db, column_name, sheet):

    # sql_query="SELECT {0} FROM Testlist WHERE testID IN (1,42)".format(item)
    # sql_query="SELECT {0} FROM Testlist LIMIT {1},{2}".format(item,start,sums)
    sql_query = "SELECT {0} FROM {1}".format(column_name, sheet)
    conn=sqlite3.connect(db)
    c=conn.cursor()
    c.execute(sql_query)
    result=c.fetchall()
    c.close()
    # print(result)
    return result


# data generator
def data_generator(sheet):
    global test_id_bbp, test_id_fm, test_id_it, test_id_mct, test_id_em, test_id_ges, test_id_fs, test_id_es
    global test_id_eeprom_total
    test_id_sql = query_column('Testlist.db', 'TestID', sheet)
    # print(test_id_sql)

    if sheet == 'eBBU_case':
        for item in test_id_sql:
            test_id_list.append(item[0])
            send_times_statistics[item[0]] = 0
            rec_times_statistics[item[0]] = [0, 0, 0]

        test_id_fm += test_id_list[:4]
        test_id_bbp += test_id_list[4:20]
        test_id_mct += test_id_list[20:33]
        test_id_it += test_id_list[33:41]
        test_id_em += test_id_list[41:45]
        test_id_fs += test_id_list[45:59]
        test_id_es += test_id_list[59:]

    elif sheet == 'CZZ_case':
        for item in test_id_sql:
            test_id_list.append(item[0])
            send_times_statistics[item[0]] = 0
            rec_times_statistics[item[0]] = [0, 0, 0]
        test_id_ges += test_id_list[:5]
        test_id_fm += test_id_list[5:8]
        test_id_bbp += test_id_list[8:23]
        test_id_mct += test_id_list[23:38]
        test_id_it += test_id_list[38:]

    elif sheet == 'CZZ_eeprom':
        for item in test_id_sql:
            test_id_eeprom.append(item[0])
            send_times_eeprom[item[0]] = 0
            rec_times_eeprom[item[0]] = [0, 0, 0]
        # print(send_times_eeprom)
        test_id_eeprom_total = test_id_eeprom[:24]

    elif sheet == 'eBBU_eeprom':
        for item in test_id_sql:
            test_id_eeprom.append(item[0])
            send_times_eeprom[item[0]] = 0
            rec_times_eeprom[item[0]] = [0, 0, 0]
        # print(test_id_eeprom)
        test_id_eeprom_total = test_id_eeprom[:24]
        # print(test_id_eeprom_total)


def csv_file_generator(number, sheet, csv_name):
    csv_file = open(csv_name, 'w', newline='')
    data_writer = csv.writer(csv_file)
    i=1
    while i<number:
        line = query_single_NO(i, sheet)
        data_writer.writerow(*line)
        i += 1

if platform_type == 'eBBU':
    data_generator('eBBU_case')
    data_generator('eBBU_eeprom')
    update_value = [('基带板MCU在线升级', 'BBHIIII', [2, 2, 0x0203, 1, 0, 0, 1]),
                    ('基带板CPLD在线升级', 'BBHIIII', [2, 2, 0x0204, 1, 0, 0, 1]),
                    ('主控板CPLD在线升级', 'BBHIIII', [2, 4, 0x0300, 1, 0, 0, 1]),
                    ('环境监控板MCU在线升级', 'BBHIIII', [2, 3, 0x0505, 1, 0, 0, 1]),
                    ('风扇板MCU在线升级', 'BBHIIII', [2, 1, 0x0103, 1, 0, 0, 1]),
                    ('基带板复位', 'BBHIIII', [2, 2, 0x0200, 1, 0, 0, 1]),
                    ('基带板DSP加载', 'BBHIIII', [2, 2, 0x0202, 1, 0, 0, 1]),
                    ('基带板FPGA加载', 'BBHIIII', [2, 2, 0x0201, 1, 0, 0, 1]),
                    ('环境监控板复位', 'BBHIIII', [2, 3, 0x0500, 1, 0, 0, 1]),
                    ('整机掉电', 'BBHIIII', [2, 3, 0x0501, 1, 0, 0, 1]),
                    ('风扇板复位', 'BBHIIII', [2, 1, 0x0100, 1, 0, 0, 1]),
                    ('主控板1PPS链路', 'BBHIIII', [2, 4, 0x030E, 1, 0, 0, 1]),
                    ('主控板TOD链路', 'BBHIIII', [2, 4, 0x030F, 1, 0, 0, 1]),
                    ('交换板FPGA_160加载', 'BBHIIII', [2, 5, 0x0603, 1, 0, 0, 1]),
                    ('交换板FPGA_325加载', 'BBHIIII', [2, 5, 0x0604, 1, 0, 0, 1]),
                    ('交换板MCU在线升级', 'BBHIIII', [2, 5, 0x0606, 1, 0, 0, 1]),
                    ('交换板复位', 'BBHIIII', [2, 5, 0x060C, 1, 0, 0, 1]),
                    ('同步板FPGA加载', 'BBHIIII', [2, 7, 0x0702, 1, 0, 0, 1]),
                    ('同步板MCU在线升级', 'BBHIIII', [2, 7, 0x0704, 1, 0, 0, 1]),
                    ('同步板复位', 'BBHIIII', [2, 7, 0x0709, 1, 0, 0, 1]),
                    ]

    config_value = [('主控板eth3配置', 'BBHIIII', [2, 0, 0x0405, 1, 0, 0, 4]), ]

    send_times_update = {0x204: 0, 0x203: 0, 0x0300: 0, 0x0505: 0, 0x0103: 0, 0x0200: 0, 0x0201: 0, 0x0202: 0, 0x0500: 0,
                         0x0501: 0, 0x0100: 0, 0x030E: 0, 0x030F: 0, 0x0603: 0, 0x0604: 0, 0x0606: 0, 0x060C: 0, 0x0702: 0,
                         0x0704: 0, 0x0709: 0}
    rec_times_update = {0x0204: [0, 0, 0], 0x0203: [0, 0, 0], 0x0300: [0, 0, 0], 0x0505: [0, 0, 0], 0x0103: [0, 0, 0],
                        0x0200: [0, 0, 0], 0x0201: [0, 0, 0], 0x0202: [0, 0, 0], 0x0500: [0, 0, 0], 0x0501: [0, 0, 0],
                        0x0100: [0, 0, 0], 0x030E: [0, 0, 0], 0x030F: [0, 0, 0], 0x0603: [0, 0, 0], 0x0604: [0, 0, 0],
                        0x0606: [0, 0, 0], 0x060C: [0, 0, 0], 0x0702: [0, 0, 0], 0x0704: [0, 0, 0], 0x0709: [0, 0, 0]}
    # checkbutton

elif platform_type == 'CZZ':
    data_generator('CZZ_case')
    data_generator('CZZ_eeprom')
    update_value = [('基带板MCU在线升级', 'BBHIIII', [1, 3, 0x0203, 1, 0, 0, 0]),
                    ('基带板CPLD在线升级', 'BBHIIII', [1, 3, 0x0204, 1, 0, 0, 0]),
                    ('主控板CPLD加载', 'BBHIIII', [1, 4, 0x0300, 1, 0, 0, 0]),
                    ('交换板MCU在线升级', 'BBHIIII', [1, 1, 0x0005, 1, 0, 0, 0]), ]
    config_value = [('主控板eth3配置', 'BBHIIII', [1, 0, 0x0405, 1, 0, 0, 4]), ]



if __name__=='__main__':
    # query_single_TestID(54274, 'CZZ_case')
    # query_single_NO(1, 'CZZ_eeprom')
    # print(query_value(54274, 'eBBU_case')[0][0])
    db_generator('Testlist.db', 'eBBU_case.csv', 'eBBU_case')
    # csv_file_generator(73, 'eBBU_eeprom', 'eBBU_eeprom.csv')
    # data_generator('CZZ_eeprom')

