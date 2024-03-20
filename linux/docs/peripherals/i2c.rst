+------------------------------------------------------------------------------+
| I2C ESSENTIALS                                                               |
+------------------------------------------------------------------------------+

An I2C system features two shared communication lines for all devices on the
bus. The two lines are used for bidirectional, half-duplex communication.

I2C allows for multiple controllers and multiple target devices, Pullup
resistors are required on both of these lines.

$ i2cdetect [-y] [-a] [-q|-r] I2CBUS [FIRST LAST]
$ i2cdump [-f] [-y] [-r first-last] [-a] I2CBUS ADDRESS [MODE [BANK [BANKREG]]]

*------------*
| I2C Master |
|            |----------------+-------------------+------▶ SDA
|            |                |                   |
|            |----------+-----|-------------+-----+------▶ SCL
*------------*          |     |             |     |
                    *-------------*     *-------------*
                    | I2C Slave 0 |     | I2C Slave N |
                    *-------------*     *-------------*

@SCL => a serial clock primarily controlled by the controller device. SCL is
used to synchronously clock data in or out of the target device.

@SDA => the serial data line. SDA is used to transmit data to or from target
devices.

--------------------------------------------------------------------------------

SDA
------*                                                   *------
       \                                                 /
        \                                               /
         *-------                               -------*
SDL
------------*                                       *------------
             \                                     /
              \                                   /
               *-------                   -------*

*-------+---------------+-----+----------+------------+-----+------*
| Start | Address Frame | R/W | ACK/NACK | Data Frame | *** | Stop |
*-------+---------------+-----+----------+------------+-----+------*

              (1)               (0)
         *----------*
        /            \
       /              \
------*                *----------------------- SDA

            *----*             *----*
           /      \           /      \
          /        \         /        \
---------*          *-------*          *------- SCL

@Start Condition: The SDA line switches from a high voltage level to a low
voltage level before the SCL line switches from high to low.

@Stop Condition: The SDA line switches from a low voltage level to a high
voltage level after the SCL line switches from low to high.

@Address Frame: A 7 or 10 bit sequence unique to each slave that identifies the
slave when the master wants to talk to it.

@Read/Write Bit: A single bit specifying whether the master is sending data to
the slave (low voltage level) or requesting data from it (high voltage level).

@ACK/NACK Bit: Each frame in a message is followed by an acknowledge/
no-acknowledge bit. If an address frame or data frame was successfully received,
an ACK bit is returned to the sender from the receiving device

--------------------------------------------------------------------------------
