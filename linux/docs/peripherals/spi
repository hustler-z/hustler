+------------------------------------------------------------------------------+
| SPI ESSENTIALS                                                               |
+------------------------------------------------------------------------------+

Controller                    Peripheral
*---------------*             *---------------*
|            SS |------------▶| CS            |
|          SCLK |------------▶| SCLK          |
|          MOSI |------------▶| DIN           |
|          MISO |◀------------| DOUT          |
*---------------*             *---------------*

The serial peripheral interface is an APB slave device. A four wire full duplex
serial protocol from Motorola. There are four possible combinations for the
serial clock phase and polarity.

The clock phase (SCPH) determines whether the serial transfer begins with the
falling edge of slave select signals or the first edge of the serial clock. The
slave select line is held high when the SPI is idle or disabled.

--------------------------------------------------------------------------------

In regular mode, an individual chip select for each subnode is required from the
main. Once the chip select signal is enabled (pulled low) by the main, the clock
and data on the MOSI/MISO lines are available for the selected subnode. If
multiple chip select signals are enabled, data on the MISO line is corrupted,
as there is no way for the main to identify which subnode is transmitting the
data.

In daisy-chain mode, the subnodes are configured such that the chip select
signal for all subnodes is tied together and data propagates from one subnode
to the next. In this configuration, all subnodes receive the same SPI clock at
the same time. The data from the main is directly connected to the first subnode
and that subnode provides data to the next subnode and so on.

--------------------------------------------------------------------------------

---*                                     *--- CS
    \                                   /
     *---------------------------------*

        *--*  *--*               *--*
        |  |  |  |               |  |
--------*  *--*  *-- xxxxxxxxx --*  *-------- SCLK

------ xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx ------ DIN

------ xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx ------ DOUT

@SPI modes

Clock Polarity and Clock Phase

-------------------------------------------------------------------------
CPOL CPHA   Clock Polarity in Idle State            Clock Phase
-------------------------------------------------------------------------
0    0      Logic low                     Data sampled on rising edge and
                                          shifted out on the falling edge
-------------------------------------------------------------------------
0    1      Logic low                    Data sampled on the falling edge
                                       and shifted out on the rising edge
-------------------------------------------------------------------------
1    0      Logic high                   Data sampled on the falling edge
                                       and shifted out on the rising edge
-------------------------------------------------------------------------
1    1      Logic high                    Data sampled on the rising edge
                                      and shifted out on the falling edge
-------------------------------------------------------------------------


--------------------------------------------------------------------------------
