## Problem description
Assume that the time needed to send an n-byte message is λ + n / β. Write a program implementing the “ping pong” test to determine λ (latency) and β (bandwidth) on your parallel computer. Design the program to run on exactly two processes. Process 0 records the time and then sends a message to process 1. After process 1 receives the message, it immediately sends it back to process 0. Process 0 receives the message and records the time. The elapsed time divided by 2 is the average message-passing time. Try sending messages multiple times, and experiment with messages of different lengths, to generate enough data points that you can estimate λ and β.

```
mpicc -Wall -o a.out main.c
mpirun -np 2 ./a.out 
```
