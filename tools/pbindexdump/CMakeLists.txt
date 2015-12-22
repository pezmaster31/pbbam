
# create version header
set(PbIndexDump_VERSION ${PacBioBAM_VERSION})
configure_file(
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/PbIndexDumpVersion.h.in
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/PbIndexDumpVersion.h
)

# sources
set(PBINDEX_SOURCES
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/CppFormatter.cpp
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/CppFormatter.h
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/IFormatter.h
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/JsonFormatter.cpp
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/JsonFormatter.h
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/OptionParser.cpp
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/OptionParser.h
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/PbIndexDump.cpp
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/PbIndexDump.h
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/PbIndexDumpVersion.h
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/Settings.h
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/json.hpp
    ${PacBioBAM_RootDir}/tools/pbindexdump/src/main.cpp
)

# pbindexdump executable
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PacBioBAM_CXX_FLAGS}")
include_directories(${PacBioBAM_RootDir}/tools/pbindexdump/src ${PacBioBAM_INCLUDE_DIRS})
add_executable(pbindexdump ${PBINDEX_SOURCES})
set_target_properties(pbindexdump PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PacBioBAM_BinDir}
)
target_link_libraries(pbindexdump pbbam)

# cram tests
if (PacBioBAM_build_tests)
    add_test(
        NAME pbindexdump_CramTests
        WORKING_DIRECTORY ${PacBioBAM_TestsDir}/scripts
        COMMAND "python" cram.py
            ${PacBioBAM_TestsDir}/src/tools/pbindexdump/json.t
            ${PacBioBAM_TestsDir}/src/tools/pbindexdump/cpp.t
    )
endif()
