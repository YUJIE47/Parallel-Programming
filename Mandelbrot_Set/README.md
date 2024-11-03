## Description
- Project Goal
    In this assignment, you are asked to parallelize the sequential **Mandelbrot Set** program and learn the following skills:

    - Get familiar with thread programming using **Pthread** and **OpenMP**.
    - Combine process and thread to implement a **hybrid parallelism** solution.
    - Understand the importance of **load balance**.

- Problem Description
    The **Mandelbrot Set** is a set of complex numbers that are quasi-stable when computed by iterating the function:
    - Z_k = C, where k = 0
    - Z_k = Z_{k-1}^2 + C, where k >= 1
    
    where:
    - C is some complex number (C = a + bi).
    - Z_k is the k-th iteration of the complex number.
    - If |Z_k| <= 2 for any k, C belongs to the Mandelbrot Set.


    
    After plotting the Mandelbrot Set, it can be visualized with intricate patterns, displaying self-similarity at various scales, which is characteristic of fractals.

- Working Items
    In this assignment, you are asked to parallelize the sequential **Mandelbrot Set** program by implementing the following two versions:
    - **pthread**: Single node shared memory programming using **Pthread**.
    - **hybrid**: Single node hybrid parallelism programming using **MPI + OpenMP**
       - **MPI** is used to balance tasks among processes, and **OpenMP** threads are used to perform computations.
       - **Pthread** library could also be used to create additional threads for communications.

## Environment
- CPU: Intel(R) Core(TM) i7-14700K, 20 cores per socket, 28 logical CPUs
- OS: Ubuntu 20.04
- Slurm: 20.11.10
## Run Code
After run `make` to compile
- Pthread
    `srun -c $t ./pra_pthread $out $iter $x0 $x1 $y0 $y1 $w $h`
- Hybrid
    `srun -n $procs -c $t ./pra_hybrid $out $iter $x0 $x1 $y0 $y1 $w $h`
      The meanings of the arguments are:

    - **$procs** - `int`; [1, 48]: number of processes. Always 1 for the Pthread version.
    - **$t** - `int`; [1, 12]: number of threads per process. (technically, this is the number of CPUs you can use per process; you are allowed to use more or fewer threads)
    - **$out** - `string`: the path to the output file.
    - **$iter** - `int`; [1, 2×10^8]: number of iterations. (the largest int is around 2.1×10^9)
    - **$x0** - `double`; [-10, 10]: inclusive bound of the real axis.
    - **$x1** - `double`; [-10, 10]: non-inclusive bound of the real axis.
    - **$y0** - `double`; [-10, 10]: inclusive bound of the imaginary axis.
    - **$y1** - `double`; [-10, 10]: non-inclusive bound of the imaginary axis.
    - **$w** - `int`; [1, 16000]: number of points in the x-axis for output.
    - **$h** - `int`; [1, 16000]: number of points in the y-axis for output.

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


