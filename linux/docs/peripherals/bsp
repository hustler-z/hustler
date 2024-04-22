+------------------------------------------------------------------------------+
| BSP ESSENTIALS                                                               |
+------------------------------------------------------------------------------+

(1) Driver Binding

*--------*                  *---------------*
| Device |<----- BUS ------>| Device Driver |
*--------*        ▲         *---------------*
                  |
                  : device_register()
                  |
The bus type structure contains a list of all devices that are on that bus type
in the system.

[+] include/linux/device/bus.h

bus_register() => register a driver-core subsystem

@struct bus_type
           |
           +- int (*match)(struct device *dev, struct device_driver *drv)
           :

whenever a new device or driver is added, callback match() is called to compare
a device against the IDs of a driver.

(2) Device Class

Upon the successful completion of probe, the device is registered with the class
to which it belongs. Device drivers belong to one and only one class, and that
is set in the driver's devclass field.

Devres - Managed Device Resource
                    |
                    +- Avoid possible resource leak on driver detach.

(3) Platform Device

*-----------------*                           *------------------------*
| Platform Device |<----- Platform Bus ------>| Platform Device Driver |
*-----------------*            ▲              *------------------------*
                               |
                               : platform_device_register()
                               |
Platform devices are devices that typically appear as autonomous entities in the
system.

The platform_device.dev.bus_id is the canonical name for the devices:
@platform_device.name => driver matching

--------------------------------------------------------------------------------
What container_of() does is to obtain a pointer to the containing struct from a
pointer to a member by a simple subtraction using the offsetof() macro from
standard C, which allows something similar to object oriented behaviours.
Notice that the contained member must not be a pointer, but an actual member for
this to work.

--------------------------------------------------------------------------------
