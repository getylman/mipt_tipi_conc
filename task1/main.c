#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define BLOCK_SIZE 32

#define x_par 2
#define y_par 2
#define THREADS_NUM x_par*y_par

typedef struct {
  unsigned h1;
  unsigned x_start;
  double** A;
  double** B;
  double** C;
} Foo;

double t_diff(struct timeval start, struct timeval end) {
  return end.tv_sec - start.tv_sec + 1e-6*(end.tv_usec - start.tv_usec);
}

double** allocate_and_fill(unsigned h, unsigned w) {
  double** matrix = (double**)malloc(h * sizeof(double*));
  for (unsigned i = 0; i < h; ++i) {
    matrix[i] = (double*)malloc(w * sizeof(double));
    for (unsigned j = 0; j < w; ++j) {
      matrix[i][j] = rand() % 1000;
    }
  }
  return matrix;
}

double** allocate_and_fill_default(unsigned h, unsigned w) {
  double** matrix = (double**)malloc(h * sizeof(double*));
  for (unsigned i = 0; i < h; ++i) {
    matrix[i] = (double*)calloc(w, sizeof(double));
  }
  return matrix;
}

void deallocate(double** matrix, unsigned h) {
  for (unsigned i = 0; i < h; ++i) {
    free(matrix[i]);
  }
  free(matrix);
}

void matrix_mult_simple(double** A, double** B, double** C, unsigned h1,
                        unsigned w1, unsigned w2) {
  for (unsigned i = 0; i < h1; ++i) {
    for (unsigned j = 0; j < w2; ++j) {
      for (unsigned k = 0; k < w1; ++k) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

void matrix_mult(double** A, double** B, double** C, unsigned h1, unsigned w1,
                 unsigned w2) {
  for (unsigned i = 0; i < h1; ++i) {
    for (unsigned k = 0; k < w1; ++k) {
      for (unsigned j = 0; j < w2; j += 4) {
        C[i][j] += A[i][k] * B[k][j];
        C[i][j + 1] += A[i][k] * B[k][j + 1];
        C[i][j + 2] += A[i][k] * B[k][j + 2];
        C[i][j + 3] += A[i][k] * B[k][j + 3];
      }
    }
  }
}

// preparetion
void matrix_mult_par_prep(Foo* datas, double** A, double** B, double** C,
                          unsigned h1, unsigned w1, unsigned w2) {
  for (unsigned i = 0; i < y_par; ++i) {
    for (unsigned j = 0; j < x_par; ++j) {
      // printf("%d\n", i * h1 / y_par);
      Foo data = {h1, j * w1 / x_par, A + (i * h1 / y_par), B, C + (i * h1 / y_par)};
      datas[i * x_par + j] = data;
    }
  }
}

void* matrix_mult_par_task(void* arg) {
  if (!arg) {
    return NULL;
  }
  Foo* data = (Foo*)arg;

  const unsigned y_step = data->h1 / y_par;
  const unsigned x_step = data->h1 / x_par;

  for (unsigned i = 0; i < y_step; i += BLOCK_SIZE) {
    for (unsigned j = data->x_start; j < (data->x_start + x_step);
         j += BLOCK_SIZE) {
      for (unsigned k = 0; k < data->h1; k += BLOCK_SIZE) {
        for (unsigned il = 0; il < BLOCK_SIZE; ++il) {
          for (unsigned kl = 0; kl < BLOCK_SIZE; ++kl) {
            for (unsigned jl = 0; jl < BLOCK_SIZE; jl += 4) {
              data->C[i + il][j + jl] +=
                  data->A[i + il][k + kl] * data->B[k + kl][j + jl];
              data->C[i + il][j + jl + 1] +=
                  data->A[i + il][k + kl] * data->B[k + kl][j + jl + 1];
              data->C[i + il][j + jl + 2] +=
                  data->A[i + il][k + kl] * data->B[k + kl][j + jl + 2];
              data->C[i + il][j + jl + 3] +=
                  data->A[i + il][k + kl] * data->B[k + kl][j + jl + 3];
            }
          }
        }
      }
    }
  }
  pthread_exit(NULL);
}

void matrix_mult_par(Foo* datas, pthread_t* threads) {
  for (unsigned i = 0; i < y_par; ++i) {
    for (unsigned j = 0; j < x_par; ++j) {
      pthread_create(threads + i * x_par + j, NULL, matrix_mult_par_task,
                     (void*)(datas + i * x_par + j));
    }
  }
  for (unsigned i = 0; i < THREADS_NUM; ++i) {
    pthread_join(threads[i], NULL);
  }
}

int main(int argc, char** argv) {
  if (argc != 4) {
    printf("Not enought args!\n");
    return 1;
  }

  srand(time(NULL));
  unsigned h1 = atoi(argv[1]);
  unsigned w1 = atoi(argv[2]);
  unsigned w2 = atoi(argv[3]);

  assert(h1 == w1 && w1 == w2);

  double** A = allocate_and_fill(h1, w1);
  double** B = allocate_and_fill(w1, w2);
  double** C = allocate_and_fill_default(h1, w2);

  Foo datas[THREADS_NUM];

  pthread_t threads[THREADS_NUM];

  matrix_mult_par_prep(datas, A, B, C, h1, w1, w2);

  struct timeval start, end;

  gettimeofday(&start, NULL);

  // matrix_mult_simple(A, B, C, h1, w1, w2);
  // matrix_mult(A, B, C, h1, w1, w2);
  matrix_mult_par(datas, threads);

  gettimeofday(&end, NULL);

  double elapsed_time = t_diff(start, end);

  printf("%f s\n", elapsed_time);

  deallocate(A, h1);
  deallocate(B, w1);
  deallocate(C, h1);
}