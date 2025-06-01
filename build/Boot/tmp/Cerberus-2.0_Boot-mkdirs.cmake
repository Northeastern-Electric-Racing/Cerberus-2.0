# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/app/Boot"
  "/home/app/Boot/build"
  "/home/app/build/Boot"
  "/home/app/build/Boot/tmp"
  "/home/app/build/Boot/src/Cerberus-2.0_Boot-stamp"
  "/home/app/build/Boot/src"
  "/home/app/build/Boot/src/Cerberus-2.0_Boot-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/app/build/Boot/src/Cerberus-2.0_Boot-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/app/build/Boot/src/Cerberus-2.0_Boot-stamp${cfgdir}") # cfgdir has leading slash
endif()
