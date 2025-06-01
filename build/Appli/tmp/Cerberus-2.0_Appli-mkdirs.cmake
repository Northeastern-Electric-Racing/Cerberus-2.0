# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/app/Appli"
  "/home/app/Appli/build"
  "/home/app/build/Appli"
  "/home/app/build/Appli/tmp"
  "/home/app/build/Appli/src/Cerberus-2.0_Appli-stamp"
  "/home/app/build/Appli/src"
  "/home/app/build/Appli/src/Cerberus-2.0_Appli-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/app/build/Appli/src/Cerberus-2.0_Appli-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/app/build/Appli/src/Cerberus-2.0_Appli-stamp${cfgdir}") # cfgdir has leading slash
endif()
