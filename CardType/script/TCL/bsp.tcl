for { set i 1} {$i<100} {incr i} {
set j [expr $i*1000+1]
tsend "Bsp_FreqSet $j"
#��ʱ1��
after 1000
}