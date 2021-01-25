# Ethbridge

Basic implementation of Layer 2 bridging functionality which allows two clients to talk to each other over local LAN as long as they are directly connected to the switch/router. This can bypass firewall restrictions or blocked incoming ports, or allow communcation if one or both are prse

## Requirements

* `OSX/Linux  (currently only tested/built on Linux but should work on OSX with some makefile changes)`

* `libpcap0.8`

To install libpcap, use

Linux: 
```sh 
sudo apt install libpcap libpcap-dev
```
OSX:
```sh 
brew install libpcap
```

No windows support is currently planned. 

## Description

The software uses libpcap to send and recieve raw ethernet packets. In the ethernet header, the protocol for these packets  (which would normally be ETH_P_IP or `0x0800` for IP packets) is set to 0xffff, which is not assigned to any protocol currently. Together with a filter on the mac address of the other end (specified in command line options) forms a bridge that passes data back and forth, datagram style without any additional states or checks or security (those are left to the attaching protocols to implement). 

## Building

Run the make command. Executable created will be  `./build/ethbridge` 

```sh 
make
```

## Usage

Each communicating computer must run the following command. **Note that multiple instances of this process will not work for communicating with more than one computer, as there is no separation of the respective channels when sending or recieving raw packets**

Sudo privilege is required due to raw packet capture. 

```sh
ethbridge -i [interface-regex] -m [mac_address] -o [output_specification]
```
```txt
 interface-regex: the regular expression to match to the ethernet name on w to communicate (saves you from typing longer ones manually)

 mac_address: the mac address of the computer on the other end of the bridge

 output_specification: the local communcation protocol. Options are:
  - STDIO           read from STDIN, write to STDOUT
  - TCP:L:[port]    listen on tcp connection on 127.0.0.1:port, and forward data
  - TCP:C:[port]    connect to 127.0.0.1:port, and forward data

```
## Examples
----
### Simple text chat, ether end can 

`Computer 1:`
```sh
sudo ethbridge -i eth -m aa:bb:cc:dd:01 -o STDIO 0</dev/tty
```
`Computer 2:`
```sh
sudo ethbridge -i eth -m aa:bb:cc:dd:02 -o STDIO 0</dev/tty
```
----
### TCP connection bridge

`Computer 1:`
```sh
nc -l 5585
sudo ethbridge -i eth -m aa:bb:cc:dd:01 -o TCP:C:5585
```
`Computer 2:`
```sh
sudo ethbridge -i eth -m aa:bb:cc:dd:02 -o TCP:L:5585
nc 127.0.0.1 5585
```
----

### Remote Shell 

`Computer 1:`
```sh
mkfifo ./testpipe
/bin/bash 0<./testpipe | sudo ethbridge -i eth -m aa:bb:cc:dd:01 -o STIDO >./testpipe
```
`Computer 2:`
```sh
sudo ethbridge -i eth -m aa:bb:cc:dd:02 -o STDIO 0</dev/tty
```