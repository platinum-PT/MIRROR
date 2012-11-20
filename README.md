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

    $ sudo modprobe mirror mirror=ethA/ethB/ethC@ethZ
    Mirror ethA, ethB, ethC, ... to ethZ

    $ sudo modprobe mirror mirror=eth2/eth3@eth0
    Mirror eth2 and eth3 to eth0.

    $ sudo modprobe mirror mirror=eth0@eth1
    Mirror eth0 to eth1.
