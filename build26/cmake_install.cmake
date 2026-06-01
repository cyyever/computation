# Install script for directory: /home/cyy/computation

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/cyy/computation/build26/test/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/cyy/computation/build26/libMyComputationLib.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/cyy/computation" TYPE FILE FILES
    "/home/cyy/computation/src/exception.hpp"
    "/home/cyy/computation/src/symbol.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/cyy/computation/automaton" TYPE FILE FILES "/home/cyy/computation/src/automaton/automaton.hpp")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/cyy/computation/context_free_lang" TYPE FILE FILES
    "/home/cyy/computation/src/context_free_lang/canonical_lr_grammar.hpp"
    "/home/cyy/computation/src/context_free_lang/cfg.hpp"
    "/home/cyy/computation/src/context_free_lang/cfg_production.hpp"
    "/home/cyy/computation/src/context_free_lang/cnf.hpp"
    "/home/cyy/computation/src/context_free_lang/common_grammar.hpp"
    "/home/cyy/computation/src/context_free_lang/dcfg.hpp"
    "/home/cyy/computation/src/context_free_lang/dk.hpp"
    "/home/cyy/computation/src/context_free_lang/dk_1.hpp"
    "/home/cyy/computation/src/context_free_lang/dk_base.hpp"
    "/home/cyy/computation/src/context_free_lang/dpda.hpp"
    "/home/cyy/computation/src/context_free_lang/endmarked_dpda.hpp"
    "/home/cyy/computation/src/context_free_lang/lalr_grammar.hpp"
    "/home/cyy/computation/src/context_free_lang/ll_grammar.hpp"
    "/home/cyy/computation/src/context_free_lang/lr_0_grammar.hpp"
    "/home/cyy/computation/src/context_free_lang/lr_0_item.hpp"
    "/home/cyy/computation/src/context_free_lang/lr_1_grammar.hpp"
    "/home/cyy/computation/src/context_free_lang/lr_1_item.hpp"
    "/home/cyy/computation/src/context_free_lang/lr_grammar.hpp"
    "/home/cyy/computation/src/context_free_lang/model_transform.hpp"
    "/home/cyy/computation/src/context_free_lang/pda.hpp"
    "/home/cyy/computation/src/context_free_lang/slr_grammar.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/cyy/computation/formal_grammar" TYPE FILE FILES "/home/cyy/computation/src/formal_grammar/grammar_symbol.hpp")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/cyy/computation/regular_lang" TYPE FILE FILES
    "/home/cyy/computation/src/regular_lang/dfa.hpp"
    "/home/cyy/computation/src/regular_lang/gnfa.hpp"
    "/home/cyy/computation/src/regular_lang/nfa.hpp"
    "/home/cyy/computation/src/regular_lang/regex.hpp"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/cyy/computation/turing_recognizable_lang" TYPE FILE FILES
    "/home/cyy/computation/src/turing_recognizable_lang/multi_tape_turing_machine.hpp"
    "/home/cyy/computation/src/turing_recognizable_lang/multi_tape_turing_machine_base.hpp"
    "/home/cyy/computation/src/turing_recognizable_lang/ntm.hpp"
    "/home/cyy/computation/src/turing_recognizable_lang/single_tape_turing_machine.hpp"
    "/home/cyy/computation/src/turing_recognizable_lang/turing_machine.hpp"
    "/home/cyy/computation/src/turing_recognizable_lang/turing_machine_base.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/MyComputationLib/MyComputationLibConfig.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/MyComputationLib/MyComputationLibConfig.cmake"
         "/home/cyy/computation/build26/CMakeFiles/Export/ca867bd3ca827d0ea105a4c02e68796f/MyComputationLibConfig.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/MyComputationLib/MyComputationLibConfig-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/MyComputationLib/MyComputationLibConfig.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MyComputationLib" TYPE FILE FILES "/home/cyy/computation/build26/CMakeFiles/Export/ca867bd3ca827d0ea105a4c02e68796f/MyComputationLibConfig.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MyComputationLib" TYPE FILE FILES "/home/cyy/computation/build26/CMakeFiles/Export/ca867bd3ca827d0ea105a4c02e68796f/MyComputationLibConfig-debug.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/cyy/computation/build26/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/cyy/computation/build26/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
