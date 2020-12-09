#!/usr/bin/python
# -*- coding: utf-8 -*-

import wmi
import sys

c = wmi.WMI()
bios = c.Win32_BIOS()[0]
version = str(bios.SMBIOSBIOSVersion).split("(")[0].strip()
print("BIOS Version is : "+version)
if version.endswith("B"):
    print("This is SMT BIOS!")
    sys.exit(0)
if version.endswith("A"):
    print("This is Formal BIOS!")
    sys.exit(1)
print("BIOS version is not end with A or B!")
sys.exit(2)
