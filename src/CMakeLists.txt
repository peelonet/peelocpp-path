ADD_LIBRARY(
  peelocpp_path
  path.cpp
)
INSTALL(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/peelo
  DESTINATION include
  FILES_MATCHING PATTERN "*.hpp"
)
INSTALL(
  TARGETS peelocpp_path
  ARCHIVE
  DESTINATION lib
)
