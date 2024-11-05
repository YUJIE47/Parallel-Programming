#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define PNG_NO_SETJMP
#include <sched.h>
#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <omp.h>
#include <mpi.h>
#include <time.h>

void write_png(const char* filename, int iters, int width, int height, const int* buffer) {
    FILE* fp = fopen(filename, "wb");
    assert(fp);
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png_ptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr);
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_filter(png_ptr, 0, PNG_NO_FILTERS);
    png_write_info(png_ptr, info_ptr);
    png_set_compression_level(png_ptr, 1);
    size_t row_size = 3 * width * sizeof(png_byte);
    png_bytep row = (png_bytep)malloc(row_size);
    for (int y = 0; y < height; ++y) {
        memset(row, 0, row_size);
        for (int x = 0; x < width; ++x) {
            int p = buffer[(height - 1 - y) * width + x];
            png_bytep color = row + x * 3;
            if (p != iters) {
                if (p & 16) {
                    color[0] = 240;
                    color[1] = color[2] = p % 16 * 16;
                } else {
                    color[0] = p % 16 * 16;
                }
            }
        }
        png_write_row(png_ptr, row);
    }
    free(row);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

int main(int argc, char** argv) {
    // MPI Init
    int rank, size, rc;

    rc = MPI_Init(&argc, &argv);

    if (rc != MPI_SUCCESS) {
        printf("Error starting MPI program. Terminating.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // detect how many CPUs are available
    cpu_set_t cpu_set;
    sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
    printf("%d cpus available\n", CPU_COUNT(&cpu_set));

    // argument parsing
    assert(argc == 9);
    const char* filename = argv[1];
    int iters = strtol(argv[2], 0, 10);
    double left = strtod(argv[3], 0);
    double right = strtod(argv[4], 0);
    double lower = strtod(argv[5], 0);
    double upper = strtod(argv[6], 0);
    int width = strtol(argv[7], 0, 10);
    int height = strtol(argv[8], 0, 10);

    // Calculate the number of rows each process will handle
    int rows_per_proc = height / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? height : start_row + rows_per_proc;

    // Allocate memory for each process's portion of the image
    int* local_image = (int*)malloc(width * rows_per_proc * sizeof(int));
    assert(local_image);

    // Time measurement: Start
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    // Mandelbrot set computation
    #pragma omp parallel for  
    for (int j = start_row; j < end_row; ++j) {
        double y0 = j * ((upper - lower) / height) + lower; 
        for (int i = 0; i < width; ++i) {
            double x0 = i * ((right - left) / width) + left; 

            int repeats = 0;
            double x = 0;
            double y = 0;
            double length_squared = 0;
            while (repeats < iters && length_squared < 4) {
                double temp = x * x - y * y + x0;
                y = 2 * x * y + y0;
                x = temp;
                length_squared = x * x + y * y;
                ++repeats;
            }
            local_image[(j - start_row) * width + i] = repeats;
        }
    }

    // Gather results from all processes to the root process
    int* image = NULL;
    if (rank == 0) {
        image = (int*)malloc(width * height * sizeof(int));
        assert(image);
    }
    MPI_Gather(local_image, width * rows_per_proc, MPI_INT, image, width * rows_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the time used
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double time_used = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_nsec - t_start.tv_nsec) / 1e9;

    printf("Rank %d execution time: %f seconds\n", rank, time_used);

    double max_time_used;
    MPI_Reduce(&time_used, &max_time_used, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total execution time across all ranks: %f seconds\n", max_time_used);
        write_png(filename, iters, width, height, image);
    }

    // Cleanup
    free(local_image);
    if (rank == 0) {
        free(image);
    }

    MPI_Finalize();
}

// cmd: srun --mpi=pmi2 -n4 -c4 ./wrapper.sh ./pra_hybrid out_hb.png 10000 -2 2 -2 2 800 800
// cmd: srun --mpi=pmi2 -n4 -c4 ./pra_hybrid out_hb.png 10000 -2 2 -2 2 800 800