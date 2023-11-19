# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/VSARM/sdk/pico/pico-sdk/tools/pioasm"
  "C:/Users/Vlad/Documents/Diagnoza-project/Proiect-diagnoza-2023_2024/src/Dev/Code/build/pioasm"
  "C:/Users/Vlad/Documents/Diagnoza-project/Proiect-diagnoza-2023_2024/src/Dev/Code/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm"
  "C:/Users/Vlad/Documents/Diagnoza-project/Proiect-diagnoza-2023_2024/src/Dev/Code/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/tmp"
  "C:/Users/Vlad/Documents/Diagnoza-project/Proiect-diagnoza-2023_2024/src/Dev/Code/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "C:/Users/Vlad/Documents/Diagnoza-project/Proiect-diagnoza-2023_2024/src/Dev/Code/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src"
  "C:/Users/Vlad/Documents/Diagnoza-project/Proiect-diagnoza-2023_2024/src/Dev/Code/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
  file(MAKE_DIRECTORY "C:/Users/Vlad/Documents/Diagnoza-project/Proiect-diagnoza-2023_2024/src/Dev/Code/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
