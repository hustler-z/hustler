--------------------------------------------------------------------------------

(1) load the binary to the board on uboot via serial port

    (a) enter kermit which has ~/.kermrc configured already;

.kermrc:
*-----------------------*
set line /dev/ttyUSB0
set speed 1500000
set carrier-watch off
set hardshake none
set flow-control none
robust
set file type bin
set file name lit
set rec pack 1000
set send pack 1000
set window 5
*-----------------------*

    (b) enter 'c' to connect to the target serial port;
    (c) enter uboot and enter 'loadb [address]';
    (c) press 'ctrl + \ + c' back to kermit;
    (d) enter 'send [path to target binary]';
    (e) enter 'c' again and enter 'go [address]' to execute;

--------------------------------------------------------------------------------
