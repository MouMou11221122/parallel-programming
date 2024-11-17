### Problem description
This is a parallel floyd warshall algorithm.

### View the m * n matrix(actually n * n)
The matrix can be view by the UNIX command
```
od -tx matrix.bin
```
or
```
od -i matrix.bin
```
The first two 4-byte elements represent the number of rows (m) and the number of columns (n) of an m * n matrix, respectively. Each subsequent single 4-byte element represents an individual element.

