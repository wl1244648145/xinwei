__author__ = 'xuqiang'

from signalanalyzerlib import SA
from time import sleep

sa = SA('172.16.16.234', 5025)


def init_instrument(freq, yrelv, offset, rbw, sweepTime):
    sa.rst()
    sa.freq_cent(freq)
    sa.disp_wind_trac_y_rlev(yrelv)
    sa.disp_wind_trac_y_rlev_offs(offset)
    sa.band(rbw)
    sa.swe_time(sweepTime)
    sa.det_rms()
    sa.trig_sour_imm()


def test_power_aclr(band):
    sa.calc_mark_func_pow_sel_acp()
    sa.pow_ach_txch_count(1)
    sa.pow_ach_acp(2)
    sa.pow_ach_bwid_chan1(band)
    sa.pow_ach_bwid_ach(band)
    sa.pow_ach_spac_chan(band)
    sa.pow_ach_spac(band)
    sleep(5)
    value = sa.calc_mark_func_pow_res_acp()
    print(value)


def test_obw():
    sa.calc_mark_func_pow_sel_owb()
    sa.pow_bwid_99pct()
    sleep(5)
    obw = sa.calc_mark_func_pow_res_obw()
    print(obw)

if __name__ == '__main__':
    init_instrument(660, -12, 32, 30, 5000)
    test_power_aclr(20)
    test_obw()