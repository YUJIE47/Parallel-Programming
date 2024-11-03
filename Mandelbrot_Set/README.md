## Environment

## Run Code

## Plots: Scalability & Load Balancing & Profile
- Test Case: slow02 `54564 -0.34499 -0.34501 -0.61249 -0.61251 800 800`

- Performance Measurement: Time (Speedup)
    - Pthread
    <img src="image/pthread.png" alt="Image description" width="500" height="300">

    - Hybrid
    <img src="image/hybrid.png" alt="Image description" width="500" height="300">

- Performance Measurement: Work Balance
    - Pthread
        | Thread ID | Time (ms)  |
        |-----------|------------|
        | 0         | 5.074555   |
        | 1         | 5.084398   |
        | 2         | 5.090845   |
        | 3         | 5.084257   |
        | 4         | 5.090989   |
        | 5         | 5.093023   |
        | 6         | 5.097611   |
        | 7         | 5.111112   |
    - Hybrid
        | Rank | Time  |
        | ---- | ----- |
        | 0    | 14.41 |
        | 1    | 8.74  |
        | 2    | 10.72 |
        | 3    | 14.41 |


