# Task Monitor Library
Monitor and report performance indicators from Linux embedded systems (interface library)

## Description
This project provides runtime library any development interfaces for TaskMonitor component applications.
The interfaces are generated from protobuf interface files and all users of libtaskmonitor should also link with libprotobuf.

## Download
Clone this repository with:    
`# git clone https://gitlab.com/taskmonitor/libtaskmonitor.git`

## Dependencies
TaskMonitor library depends on the following libraries:   

| Library | Reference | Info |
| ------ | ------ | ------ |
| protobuf | https://developers.google.com/protocol-buffers | Data serialization |

## Build
`mkdir build && cd build && cmake .. && make `
