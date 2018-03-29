# -----------------------------------------------------------------------------
# Add module directories
# -----------------------------------------------------------------------------
if (NOT USE_SYSTEM_MALLOC)
    add_subdirectory(bmalloc)
endif ()

add_subdirectory(WTF)

add_subdirectory(WebCore)