# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "ide\\CMakeFiles\\doruk_ide_autogen.dir\\AutogenUsed.txt"
  "ide\\CMakeFiles\\doruk_ide_autogen.dir\\ParseCache.txt"
  "ide\\doruk_ide_autogen"
  )
endif()
