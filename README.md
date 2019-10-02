# AlphaChat

## Setup

1. Clone this repository and cd into it
```bash
$ git clone https://github.com/alphadose/Alpha-Chat

$ cd Alpha-Chat
```

2. Build project binaries
```bash
$ make all
* Building Server
* Building Client
* Done
```

## Hosting the Chat-Server
```bash
$ ./server <port>
Starting server.
Waiting for incoming connections...
```

## Connecting to the Chat-Server via Client
```bash
$ ./client <ip_address_of_server> <port>
```

## Cleaning binaries
```bash
$ make clean
* Done
```
