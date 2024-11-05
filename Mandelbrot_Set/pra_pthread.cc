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

#include <pthread.h> 
#include <time.h>
#include <nvtx3/nvToolsExt.h>

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

typedef struct {
    int thread_id;
    int width;
    int height;
    int iters;
    double left;
    double right;
    double lower;
    double upper;
    int* image;
    int num_threads;
    struct timespec start_time, end_time;
} ThreadData;

void* mandelbrot_thread(void* arg) {  
    ThreadData* data = (ThreadData*)arg;
    int thread_id = data->thread_id;
    int height = data->height;
    int width = data->width;
    double left = data->left;
    double right = data->right;
    double lower = data->lower;
    double upper = data->upper;
    int iters = data->iters;
    int* image = data->image;
    int num_threads = data->num_threads;

    clock_gettime(CLOCK_MONOTONIC, &data->start_time);

    char range_name[32];
    snprintf(range_name, sizeof(range_name), "Thread %d iteration", thread_id);
    nvtxRangePush(range_name);

    for (int j = thread_id; j < height; j += num_threads) {  
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
            image[j * width + i] = repeats;
        }  
    }

    nvtxRangePop();

    clock_gettime(CLOCK_MONOTONIC, &data->end_time);
    double thread_time_used = (data->end_time.tv_sec - data->start_time.tv_sec) +
                              (data->end_time.tv_nsec - data->start_time.tv_nsec) / 1e9;
    printf("Thread %d execution time: %f seconds\n", thread_id, thread_time_used);

    return NULL;
}

int main(int argc, char** argv) {
    // detect how many CPUs are available
    cpu_set_t cpu_set;
    sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
    int num_threads = CPU_COUNT(&cpu_set);
    printf("%d cpus available\n", num_threads);


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

    // allocate memory for image 
    int* image = (int*)malloc(width * height * sizeof(int));
    assert(image);

    // Time measurement: Start
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    // create threads for mandelbrot set calculation
    pthread_t threads[num_threads]; 
    ThreadData thread_data[num_threads]; 
    for (int t = 0; t < num_threads; ++t) {
        thread_data[t] = (ThreadData){t, width, height, iters, left, right, lower, upper, image, num_threads};
        pthread_create(&threads[t], NULL, mandelbrot_thread, &thread_data[t]);
    }

    // join threads 
    for (int t = 0; t < num_threads; ++t) { 
        pthread_join(threads[t], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);

    // Calculate the time used
    double time_used = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_nsec - t_start.tv_nsec) / 1e9;
    printf("Total execution time: %f seconds\n", time_used);

    // draw and cleanup
    write_png(filename, iters, width, height, image);
    free(image);
}

// cmd: srun --mpi=pmi2 -c4  ./wrapper.sh ./pra_pthread out_p.png 2602 -3 0.2 -3 0.2 979 2355
// cmd: srun --mpi=pmi2 -c4  ./pra_pthread out_p.png 2602 -3 0.2 -3 0.2 979 2355
