#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import serial
import subprocess
import argparse
import os

class SerialCtrl:

    def __init__(self, **kwargs):
        return

    def init(self, port_s, baudrate_n):
        self.console = serial.Serial(
            port = port_s,
            baudrate = baudrate_n,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        )

    def send_cmd(self, tx, rx, wait, retries=1, wait_time=1):
        ret = False
        self.console.write(tx)

        for loop in range(retries):
            self.console.write(b'\r')
            if wait:
                time.sleep(wait_time)        
            info = self.console.read(100000)
            if self.is_title_of(info, rx):
                print("{} success".format(tx))
                ret = True
                break
            
        if not ret:
            print("{} failed".format(tx))

        return ret

    def is_title_of(self, info, titles):
        for t in titles:
            if t in info:
                return True

        return False

    def get_cur_sh(self, no_elf_bootup_rx, elf_boot_up_rx):
        self.console.write(b'\r\r\r\r\r')
        info = self.console.read(100000)
        if self.is_title_of(info, no_elf_bootup_rx):
            return "no_elf_bootup"
        elif self.is_title_of(info, elf_boot_up_rx):
            return "elf_bootup"
        else:
            return "unkonwn shell"

    def show_elf_version(self, no_elf_bootup_rx, elf_boot_up_rx):
        if "elf_bootup" == self.get_cur_sh(no_elf_bootup_rx, elf_boot_up_rx):
            self.console.write(b'\r')
            self.console.write(b'version')
            self.console.write(b'\r')
        
        return

if __name__ == '__main__':
    parser = argparse.ArgumentParser('seril_trans - a serial cmd input for phytium dev')

    # serial port to connect
    parser.add_argument(
        '--port', '-p',
        help='Serial port to connect board',
        default='/dev/ttyS3'
    )

    # baudrate to set
    parser.add_argument(
        '--baudrate', '-br',
        help='Baudrate for serial connection',
        default='115200'
    )

    # board ip
    parser.add_argument(
        '--boardip', '-bi',
        help='IPv4 address for developer board',
        default='192.168.4.20'
    )

    # host ip
    parser.add_argument(
        '--hostip', '-hi',
        help='IPv4 address for developer host',
        default='192.168.4.50'
    )

    # gateway ip
    parser.add_argument(
        '--gatewayip', '-gi',
        help='IPv4 address for developer host-board gateway',
        default='192.168.4.1'
    )

    # boot address
    parser.add_argument(
        '--bootaddr', '-ba',
        help='Boot address to load image',
        default='0x90100000'
    )

    # elf file
    parser.add_argument(
        '--elffile', '-ea',
        help='ELF image to load',
        default='baremetal.elf',
        type=argparse.FileType('rb'))    

    # get input args
    args = parser.parse_args()

    # establish serial connection
    ser_ctrl = SerialCtrl()
    ser_ctrl.init(str(args.port), int(args.baudrate))

    no_elf_bootup_rx = [b'ft2004#', b'FT2004#', b'd2000#', b'D2000#', b'e2000#', b'E2000#'] 
    elf_bootup_rx = [b'phytium:/$']

    print('reboot board and load image ....')

    tx = ''
    ret = True
    # check current shell header, is it u-boot or elf image
    cur_shell = ser_ctrl.get_cur_sh(no_elf_bootup_rx, elf_bootup_rx)

    if "no_elf_bootup" == cur_shell:    
        pass
    elif "elf_bootup" == cur_shell:
        # reset elf, wait 10 seconds
        ret = ser_ctrl.send_cmd(b'reboot', no_elf_bootup_rx, True, 2, 20)
        if True != ret:
            exit(6)
    else:
        print(cur_shell)
        exit(7)    

    # set ip addr of board
    tx = 'setenv ipaddr {}'.format(args.boardip)
    ret = ser_ctrl.send_cmd(bytes(tx, encoding="utf8"), no_elf_bootup_rx, False, 2)
    if True != ret:
        exit(2)

    # set ip addr of server host
    tx = 'setenv serverip {}'.format(args.hostip)
    ret = ser_ctrl.send_cmd(bytes(tx, encoding="utf8"), no_elf_bootup_rx, False, 2)
    if True != ret:
        exit(3)

    # set ip gateway
    tx = 'setenv gatewayip {}'.format(args.gatewayip)
    ret = ser_ctrl.send_cmd(bytes(tx, encoding="utf8"), no_elf_bootup_rx, False, 2)
    if True != ret:
        exit(4)

    # boot elf image
    tx = 'tftpboot {} {}'.format(args.bootaddr, os.path.basename(args.elffile.name))
    ret = ser_ctrl.send_cmd(bytes(tx, encoding="utf8"), no_elf_bootup_rx, True, 2, 5)
    if True != ret:
        exit(5)

    # unpack elf image and jump
    tx = 'bootelf -p {}'.format(args.bootaddr)
    ser_ctrl.send_cmd(bytes(tx, encoding="utf8"), elf_bootup_rx, True)
    if True != ret:
        exit(6)

    exit(0)