#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

#include <cuda_runtime.h>

// Error-checking macro
#define CUDA_CHECK(call)                                                       \
  do {                                                                         \
    cudaError_t err = call;                                                    \
    if (err != cudaSuccess) {                                                  \
      fprintf(stderr, "CUDA error in %s at line %d: %s\n", __FILE__, __LINE__, \
              cudaGetErrorString(err));                                        \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

/**
 * Kernel 1: Compute absolute difference: C[i] = |A[i] - B[i]|
 */
__global__ void absDiffKernel(const float *A, const float *B, float *C,
                              size_t N) {
  size_t i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < N) {
    C[i] = fabsf(A[i] - B[i]);
  }
}

/**
 * Kernel 2: Parallel reduction to find maximum in C.
 * Writes one value (the max) per thread block into partialMax[blockIdx.x].
 */
__global__ void reduceMaxKernel(const float *input, float *partialMax,
                                size_t N) {
  extern __shared__ float sdata[];

  unsigned int tid = threadIdx.x;
  size_t i = blockIdx.x * blockDim.x + threadIdx.x;

  // Load elements into shared memory (or a very small sentinel if out of range)
  if (i < N) {
    sdata[tid] = input[i];
  } else {
    sdata[tid] = -FLT_MAX;
  }
  __syncthreads();

  // Reduction in shared memory
  for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
    if (tid < s) {
      sdata[tid] = fmaxf(sdata[tid], sdata[tid + s]);
    }
    __syncthreads();
  }

  // Write block result
  if (tid == 0) {
    partialMax[blockIdx.x] = sdata[0];
  }
}

/**
 * Utility function: parallel max reduction on d_in of size N.
 * The result is returned in *h_out on the host.
 */
void reduceMax(const float *d_in, float *h_out, size_t N, int blockSize) {
  int gridSize = (int)((N + blockSize - 1) / blockSize);

  // Allocate partial results
  float *d_partialMax = nullptr;
  CUDA_CHECK(cudaMalloc(&d_partialMax, gridSize * sizeof(float)));

  // Shared memory size
  const int SHARED_BYTES = blockSize * sizeof(float);

  const float *d_currentIn = d_in;
  size_t currentSize = N;

  while (true) {
    reduceMaxKernel<<<gridSize, blockSize, SHARED_BYTES>>>(
        d_currentIn, d_partialMax, currentSize);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    if (gridSize == 1) {
      // Final max in d_partialMax[0]
      CUDA_CHECK(cudaMemcpy(h_out, d_partialMax, sizeof(float),
                            cudaMemcpyDeviceToHost));
      break;
    }

    // Prepare next pass
    currentSize = gridSize;
    gridSize = (gridSize + blockSize - 1) / blockSize;
    d_currentIn = d_partialMax;
  }

  CUDA_CHECK(cudaFree(d_partialMax));
}

float compute(float *d_A, float *d_B, size_t size) {
  // Allocate device memory for result
  float *d_C = nullptr;
  CUDA_CHECK(cudaMalloc(&d_C, size * sizeof(float)));

  // Launch abs-diff kernel
  const int BLOCK_SIZE = 256;
  int gridSize = (int)((size + BLOCK_SIZE - 1) / BLOCK_SIZE);

  absDiffKernel<<<gridSize, BLOCK_SIZE>>>(d_A, d_B, d_C, size);
  CUDA_CHECK(cudaGetLastError());
  CUDA_CHECK(cudaDeviceSynchronize());

  // Reduce to find maximum
  float max_diff = 0.0f;
  reduceMax(d_C, &max_diff, size, BLOCK_SIZE);

  // Clean up
  CUDA_CHECK(cudaFree(d_C));

  return max_diff;
}

int main() {
  // Data Gen
  size_t size = 1 * (1 << 28);
  std::vector<float> h_A(size);
  std::vector<float> h_B(size);

  std::printf("Each vector contains %zu elements\n", size);

  size_t allocated_bytes = size * sizeof(float);
  std::printf("Each vector uses %zu bytes (%.2f GB)\n", allocated_bytes,
              allocated_bytes / (1024.0 * 1024.0 * 1024.0));

  // Fill the vectors with random values
  std::printf("Generating numbers...\n");
  std::mt19937 gen(69);
  std::normal_distribution<float> normal_dist(0.5f, 0.15f);

  auto rand_start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < size; i++) {
    h_A[i] = normal_dist(gen);
    h_B[i] = normal_dist(gen);
  }
  auto rand_end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> rand_elapsed = rand_end - rand_start;
  std::printf("Time: Data generation: %.6f s\n", rand_elapsed.count());

  // Allocate GPU memory
  std::printf("Moving data to GPU...\n");
  float *d_A = nullptr;
  float *d_B = nullptr;
  float *d_C = nullptr; // to store |A - B|
  CUDA_CHECK(cudaMalloc(&d_A, size * sizeof(float)));
  CUDA_CHECK(cudaMalloc(&d_B, size * sizeof(float)));
  CUDA_CHECK(cudaMalloc(&d_C, size * sizeof(float)));

  // Copy data to device and compute
  auto start_copy = std::chrono::high_resolution_clock::now();
  CUDA_CHECK(cudaMemcpy(d_A, h_A.data(), size * sizeof(float),
                        cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(d_B, h_B.data(), size * sizeof(float),
                        cudaMemcpyHostToDevice));
  auto end_copy = std::chrono::high_resolution_clock::now();
  double time_copy =
      std::chrono::duration<double>(end_copy - start_copy).count();
  std::printf("Time: Host to Device copy: %.6f s\n", time_copy);

  // Launch abs-diff kernel
  std::printf("Calculating...\n");
  auto start = std::chrono::high_resolution_clock::now();

  float max_diff = compute(d_A, d_B, size);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::printf("Time: Compute: %.6f s\n", elapsed.count());

  std::printf("Max difference: %.6f\n", max_diff);

  CUDA_CHECK(cudaFree(d_A));
  CUDA_CHECK(cudaFree(d_B));
  CUDA_CHECK(cudaFree(d_C));

  return 0;
}
