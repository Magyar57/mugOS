#!/bin/bash

# Scripts to run "Bochs" debugger
# It is an emulater and debugger for x86 CPUs (handles both Intel and AMD)
# https://bochs.sourceforge.io/

# You might need following packages:
# - bochs
# - bochs-bios
# - bochs-debugger

bochs -f Bochs.cfg 
