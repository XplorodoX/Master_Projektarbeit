# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-src"
  "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-build"
  "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-subbuild/pybind11-populate-prefix"
  "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-subbuild/pybind11-populate-prefix/tmp"
  "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp"
  "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-subbuild/pybind11-populate-prefix/src"
  "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/PC/Desktop/projekt_lecon_master_sem2/Master_Projektarbeit/build_validation/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
