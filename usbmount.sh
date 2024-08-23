#!/bin/bash

# Define the USB device and mount point
USB_DEVICE="/dev/sda1"
MOUNT_POINT="/mnt/sandisk"

# Check if the mount point directory exists, if not, create it
if [ ! -d "$MOUNT_POINT" ]; then
    mkdir -p "$MOUNT_POINT"
fi

# Mount the USB device to the mount point
mount $USB_DEVICE $MOUNT_POINT

# Check if the mount was successful
if [ $? -eq 0 ]; then
    echo "USB mounted successfully at $MOUNT_POINT"
else
    echo "Failed to mount USB at $MOUNT_POINT"
    exit 1
fi
