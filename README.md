# Snapshot
This document is intended to explain what I an trying to do here.

Intention of the project is to record the changed blocks of the disks and 
build a snapshot from these points.

I have implemented a Device Mapper target and a small driver, tracker driver, which records changed blocks on the disk.

Job of the Device mapper is to send a write request to the tracker driver. Tracker Driver will store the changed blocks.
For this tracker will provide an interface to the Device Mapper Target.

Tracker will provide one more ioctl interface for user appliactions to fetch which blocks are changed.

 --------------
|  User        |
|      App     |
 --------------
         |                                          --------
         |                                         |  IOCTL |
        \|/                                          --------
  ----------------                ___             ----------------
 |  Device Mapper |------------> |Pl |---------->|  Target Driver |             
 |  Target        |              | I |            ----------------           
  ----------------               | F |
                                  ___
               
 
Build process:
export TOP_DIR= <Path to Bitmap Folder>

1. Go to the Snapshot/Bitmap/bitmapdriver
   make 
   it will build tracker.ko
2. In  Snapshot/Bitmap/dmtarget
  make
  It will build a interceptor.ko

3. Snapshot/Bitmap/Test
  Run the script : dmsetup_remove.sh It will load modules and create a device mapper target.
  
