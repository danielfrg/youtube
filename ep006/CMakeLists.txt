cmake_minimum_required(VERSION 3.8)
project(RandomMaxDiff LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

set(CMAKE_CUDA_STANDARD 11)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O2 --extended-lambda")
set(CMAKE_CUDA_ARCHITECTURES 89)

add_executable(cpu cpu.cpp)
add_executable(kernel kernel.cu)
add_executable(thrust thrust.cu)

# Enable separable compilation for the CUDA target
set_target_properties(kernel PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_target_properties(thrust PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
