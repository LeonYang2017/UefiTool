#!/usr/bin/python
# -*- coding: utf-8 -*-
import wmi
import sys

c = wmi.WMI()
maclist = []
for interface in c.Win32_NetworkAdapterConfiguration(IPEnabled=1):
    maclist.append(interface.MACAddress)
count = len(maclist)
print("Total MAC address is:",count)
for i in maclist:
    print("MAC:",i)
for i in range(count):
    tt = 0
    #print(i)
    if i == count-1:
        continue
    tpmmac1 = str(maclist[i]).strip().split(":")
    tpmmac2 = str(maclist[i+1]).strip().split(":")
    for i in tpmmac1:
        for j in tpmmac2:
            if i == j:
                tt += 1
    if tt == 6:
        print("Two MAC addresses are the same!")
        print(maclist[i])
        print(maclist[i+1])
        sys.exit(1)
    if tt == 5:
        if tpmmac1[5] >= tpmmac2[5]:
            print("Failed!!! MAC 1 is greater than MAC 2!")
            print(maclist[i])
            print(maclist[i + 1])
            sys.exit(1)
        if tpmmac1[5] <= tpmmac2[5]:
            print("SUCCESS! MAC 1 is less than MAC 2!")
            print(maclist[i])
            print(maclist[i + 1])
            sys.exit(0)
sys.exit(0)