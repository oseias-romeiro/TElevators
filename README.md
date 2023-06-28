# TElevators
Project to elevators controller was developed to handle calls on every floor of a building. Based on the problem producer-consumer classic discussed.

## Problem
The problem addresses the scenario where several users need to move quickly through floors of a building. For this, there are one or more elevators to meet requests for a certain number of floors. Each request is unique, that is, there can only be one request per floor. When there is already a request for all floors, any other request must wait until it can make its call. Elevators are also on standby, if there are no requests to be met. To deal with the best path that the elevator should take, there is a decision maker that decides the path through the SCAN scheduler algorithm.


## Compile & Execute

```shell
gcc -pthread -o TElevator TElevator.c
./TElevator
```

