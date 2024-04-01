+------------------------------------------------------------------------------+
| CAMERA ESSENTIALS                                                            |
+------------------------------------------------------------------------------+

- V4L2 CORE -





--------------------------------------------------------------------------------
- DVP -

DVP is parallel port transmission, the speed is slow, the transmission bandwidth
is low.

*------------*
|       PCLK | => Pixel Clock
| MCLK(XCLK) | => External Clock Input
|      VSYNC | => Vertical Synchronization
|      HSYNC | => Horizontal Synchronization
|    D[0:11] | => Parallel Port Data can be 8/10/12 bit data size
*------------*

@BT656
@BT1120

The parallel and BT.656 buses transport one bit of data on each clock cycle per
data line. The parallel bus uses synchronisation and other additional signals
whereas BT.656 embeds synchronisation.

--------------------------------------------------------------------------------
- MIPI -

CSI-2 is a data bus intended for transferring images from cameras to the host
SoC.

*-----------------*
|   Application   |
*-----------------*
         |
*-----------------*
| Pixel to Byte   |
| Packing Formats |
+-----------------*
         |
*-----------------*
|    Low Level    |
|    Protocol     |
*-----------------*
         |
*-----------------*
| Lane Management |
|      Layer      |
*-----------------*
         |
*-----------------*
|    PHY Layer    | -> D-PHY & C-PHY
*-----------------*

D-PHY can switch between differential (High Speed) and single-ended (Low Power)
mode in real time depending on the need to transfer large amounts of data or to
conserve power to prolong the battery life. The D-PHY interface is capable of
operating in simplex or duplex configuration with single data lane or multiple
data lanes, giving a flexibility to avail the links as needed. In addition,
clock is always uni-directional (Master to Slave) and is in quadrature phase
with data.

--------------------------------------------------------------------------------
MIPI is differential serial transmission, fast speed, anti-interference.

*---------------------*                *---------------------*
|     CSI Transmitter |                | CSI Receiver        |
|             Data N+ | -------------▶ | Data N+             |
|             Data N- | -------------▶ | Data N-             |
|             ...     |                |                     |
|             Data 1+ | -------------▶ | Data 1+             |
|             Date 1- | -------------▶ | Date 1-             |
|                     |                |                     |
|             Clock+  | -------------▶ | Clock+              |
|             Clock-  | -------------▶ | Clock-              |
|                     |                |                     |
|          CCI Slave  |                | CCI Master          |
|                SCL  | ◀------------- | SCL                 |
|                SDA  | ◀------------▶ | SDA                 |
*---------------------*                *---------------------*

                                    [+] 16 for D-PHY and 7 for C-PHY
                                                  ▲
                                                  |
@ pixel_rate = link_freq * 2 * nr_of_lanes * 16 / k / bits_per_sample
                   |       |
                   |       ▼
                   |   [+] data is transferred on both rising
                   |       and falling edge of the signal
                   |
                   ▼
[+] tell the receiver the frequency of the bus (V4L2_CID_LINK_FREQ)

module_i2c_driver()
       |
       +- module_driver()
                |
                ▼
        (a) i2c_add_driver()
                  |
                  +- i2c_register_driver()

          (i2c_bus_type)

◀---------------(x)----------------(*)
                                    ▲
                                    |
                            driver_register()
                                :
                                +- driver_find()
                                :
                                +- bus_add_driver() => Add a driver to the bus
                                :
                                +- driver_add_groups()
                                :
                                +- deferred_probe_extend_timeout()
                                                :
                                                +- schedule_delayed_work()
                                                            |
                                                *-------------------------*
                                                | put work task in global |
                                                | workqueue after delay.  |
                                                *-------------------------*

--------------------------------------------------------------------------------
- SERDES -

*--------*   MIPI/DVP     *------------*       *--------------*
| sensor |---------------▶| serializer |------▶| deserializer |
*--------*  YUV/RGB/RAW   *------------*       *--------------*
                                                       |
                                                       | (soc)
                                                       ▼
                                           *------------------------*
                                           |                        |
                                       *-------*                *-------*
                                       |  DVP  |                | D-PHY |
                                       *-------*                *-------*
                                           |                        |
                                           |                 *----------*
                                           |                 | CSI HOST |
                                           |                 *----------*
                                           |                        |
                                       *--------------------------------*
                                       |              VICAP             |
                                       *--------------------------------*
                                                       :
                                                       ▼
                                       *--------------------------------*
                                       |              ISP               |
                                       *--------------------------------*

Depends on sensor with [no] ISP:

(a) sensor with ISP    => YUV image format
(b) sensor with no ISP => RAW/RGB image format

--------------------------------------------------------------------------------
SerDes is a functional block that Serializes and Deserializes digital data used
in high-speed chip-to-chip communication.

@ serializer   => Parallel-to-Serial Conversion
@ Deserializer => Serial-to-Parallel Conversion

       *------------*         *--------------*
------▶|            |         |              |------▶
       :            :         :              :
------▶| Serializer |--------▶| Deserializer |------▶
       :            :         :              :
------▶|            |         |              |------▶
       *------------*         *--------------*

(a) Data Transmission Efficiency
(b) High Data Rates
(c) Reduced Electromagnetic Interference (EMI)
(d) Long-Distance Communication
(e) Integration
(f) Scalability

--------------------------------------------------------------------------------
