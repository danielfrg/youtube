#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

int main() {
  // Data gen
  size_t size = 5 * (1 << 28);
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

  // Compute
  std::printf("Calculating on CPU...\n");
  auto start = std::chrono::high_resolution_clock::now();

  float max_diff = 0.0f;
  for (size_t i = 0; i < size; i++) {
    max_diff = std::max(max_diff, std::abs(a[i] - b[i]));
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::printf("Time: Compute: %.6f s\n", elapsed.count());

  std::printf("Max difference: %.6f\n", max_diff);

  return 0;
}
