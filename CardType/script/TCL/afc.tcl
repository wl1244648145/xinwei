while {1==1} {
tsend "mnt/btsa/XH"
after 1000
#msend "attach&lte" "  XH  attach " 60000
tsend "attach&lte"
after 2000
tsend "bsp_power_ctrl 1"
after 240000
#tsend "AFC_PRINT_VAR"
#after 240000

#tsend "g_s8PFlag = 1"
#after 60000
}