
set(GUI_CORE_NAME "arche-gui-core")

# Gather source and header files
file(GLOB_RECURSE HEADER_FILES "Public/*.h")
file(GLOB_RECURSE SOURCE_FILES "Private/*.cpp")

# Create the static library
add_library(${GUI_CORE_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${GUI_CORE_NAME} PUBLIC "Public")

# Add alias for easier linking
add_library(arche::gui-core ALIAS ${GUI_CORE_NAME})