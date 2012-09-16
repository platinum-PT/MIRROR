# MIRROR

## Description

Port Mirroring.
Copy traffic from specified interfaces to MIRROR port.

Tested kernel version: 2.6.23, 2.6.29, 2.6.37

## Compile

 $ make

## Install

 $ sudo make install

## Usage

 $ sudo modprobe mirror ports=A[,B,C,...],Z
 Mirror ethA, ethB, ethC, ... to ethZ

 $ sudo modprobe mirror ports=2,3,0
 Mirror eth2 and eth3 to eth0.

 $ sudo modprobe mirror ports=0,1
 Mirror eth0 to eth1.
