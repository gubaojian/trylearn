cmake_minimum_required(VERSION 2.8.12)
include(WebKitCommon)
set_property(DIRECTORY . PROPERTY FOLDER "WTF")



add_subdirectory(wtf)


# Expose B's public includes (including Boost transitively) to other
# subprojects through cache variable.
#set(WTF_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/Source/WTF/
#        CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
