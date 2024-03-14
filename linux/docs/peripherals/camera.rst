+------------------------------------------------------------------------------+
| CAMERA ESSENTIALS                                                            |
+------------------------------------------------------------------------------+

The parallel and BT.656 buses transport one bit of data on each clock cycle per
data line. The parallel bus uses synchronisation and other additional signals
whereas BT.656 embeds synchronisation.

--------------------------------------------------------------------------------
CSI-2 is a data bus intended for transferring images from cameras to the host
SoC.

    *---------------------------*
    | CSI-2 transmitter drivers |
    *---------------------------*

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

        ◀-------------------------------------(x)----------
                                               ▲
                                               |
                                        driver_register()
                                           :
                                           +- driver_find()
                                           :
                                           +- bus_add_driver()
                                           :
                                           +- driver_add_groups()
                                           :
                                           +- deferred_probe_extend_timeout()

--------------------------------------------------------------------------------
