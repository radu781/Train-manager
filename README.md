# Train manager
Command line interface application for managing train schedules (as a user).
It simulates a real life Client-Server architecture, where the clients request data from the server, and the server provides the data. The server's data about the trains is stored in an `.xml` file.

### Functionality
The clients send commands to the server.
Currently supported commands:
- _command validation_

Commands to be added:
- `today`: information about the current day's trains schedule
- `departures`: information about departures in the upcoming hour, including trains that are late
- `arrivals`: information about arrivals in the upcoming hour, including trains that are late

### Components
Train manager consists of:
- a Client side (multiple concurrent Client connections are supported)
- a Server side (only one Server is supported)

This is a multi-threaded application where:
- two threads are used in the client for sending and receiving data (excluding the main thread)
- _2n_ threads (approximately) are used in the server, two for each client

### Requirements
For Client and Server instances:
- g++ version 9.3.0
- make version 4.2.1
- any unix system (no cross platform compatibility so far)

### Build and start
```shell
git clone https://github.com/radu781/Train-manager.git
cd Train-manager

# This will use all debug optimizations
make

./Server/bin/main
./Client/bin/main
```
