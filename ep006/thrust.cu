#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>
#include <thrust/functional.h>
#include <thrust/reduce.h>
#include <thrust/transform.h>
#include <vector>

// GPU version of compute function
float compute(const thrust::device_vector<float> &a,
              const thrust::device_vector<float> &b) {

  // Create a temporary vector to store the absolute differences
  thrust::device_vector<float> diff(a.size());

  // Calculate absolute differences between corresponding elements
  thrust::transform(thrust::device, a.begin(), a.end(), b.begin(), diff.begin(),
                    [] __device__(float x, float y) { return fabsf(x - y); });

  // Find the maximum difference using reduction
  float max_diff = thrust::reduce(thrust::device, diff.begin(), diff.end(),
                                  0.0f, thrust::maximum<float>());

  return max_diff;
}

int main() {
  // Data gen
  size_t size = 1 * (1 << 28);
  std::vector<float> a(size);
  std::vector<float> b(size);

  std::printf("Each vector contains %zu elements\n", a.size());

  size_t allocated_bytes_a = a.capacity() * sizeof(float);
  std::printf("Each vector allocated %zu bytes (%.2f GB)\n", allocated_bytes_a,
              allocated_bytes_a / (1024.0 * 1024.0 * 1024.0));

  // Fill the vectors with random values
  std::printf("Generating numbers...\n");
  std::mt19937 gen(69);
  std::normal_distribution<float> normal_dist(0.5f, 0.15f);

  auto rand_start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < size; i++) {
    a[i] = normal_dist(gen);
    b[i] = normal_dist(gen);
  }
  auto rand_end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> rand_elapsed = rand_end - rand_start;
  std::printf("Time: Data generation: %.6f s\n", rand_elapsed.count());

  // Transfer to GPU
  std::printf("Transferring data to GPU...\n");
  auto transfer_start = std::chrono::high_resolution_clock::now();

  thrust::device_vector<float> d_a = a;
  thrust::device_vector<float> d_b = b;
  thrust::device_vector<float> d_diff(size);

  auto transfer_end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> transfer_elapsed =
      transfer_end - transfer_start;
  std::printf("Time: Data transfer to GPU: %.6f s\n", transfer_elapsed.count());

  // Compute on GPU
  std::printf("Calculating on GPU...\n");
  auto start = std::chrono::high_resolution_clock::now();

  float max_diff = compute(d_a, d_b);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::printf("Time: Compute (GPU): %.6f s\n", elapsed.count());

  std::printf("Max difference: %.6f\n", max_diff);

  return 0;
}
