#!/bin/bash

function create_device
{
	TOP_DIR=$(pwd)/../

	losetup /dev/loop0 disk	
	if [ $? -ne 0 ]; then
		echo "Unable to setupna loop device"
		exit -1
	fi

	insmod $TOP_DIR/bitmapdriver/tracker.ko
	if [ $? -ne 0 ]; then
		echo "Unable to load bitdriver"
		exit -1
	fi

	insmod $TOP_DIR/dmtarget/intercept.ko
	if [ $? -ne 0 ]; then
		echo "Unable to load interceptor"
		exit -1
	fi

	echo "Creating device"
	echo "echo 0 10000 intercept /dev/loop0 0| dmsetup create intercept"
	echo 0 10000 intercept /dev/loop0 0| dmsetup create intercept
}

function release_device
{
	echo "releasing device....................."
	echo "pausing device"
	dmsetup clear /dev/mapper/intercept

	echo "Suspending Device ................"
	sleep 2
	dmsetup suspend /dev/mapper/intercept

	echo "Removing Device.."
	sleep 2
	dmsetup remove /dev/mapper/intercept

	echo "Removing modules"
	sleep 2
	rmmod intercept 
	rmmod tracker
	losetup -D	
}

case $1 in
	create) create_device ;;
	release) release_device ;;
esac
