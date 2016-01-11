
set(PbindexdumpSrcDir ${PacBioBAM_ToolsDir}/pbindexdump/src)

# create version header
set(PbIndexDump_VERSION ${PacBioBAM_VERSION})
configure_file(
    ${PbindexdumpSrcDir}/PbIndexDumpVersion.h.in
    ${PbindexdumpSrcDir}/PbIndexDumpVersion.h
)

# list source files
set(PBINDEXDUMP_SOURCES

    ${ToolsCommonDir}/OptionParser.h
    ${ToolsCommonDir}/OptionParser.cpp

    ${PbindexdumpSrcDir}/CppFormatter.cpp
    ${PbindexdumpSrcDir}/CppFormatter.h
    ${PbindexdumpSrcDir}/IFormatter.h
    ${PbindexdumpSrcDir}/JsonFormatter.cpp
    ${PbindexdumpSrcDir}/JsonFormatter.h
    ${PbindexdumpSrcDir}/PbIndexDump.cpp
    ${PbindexdumpSrcDir}/PbIndexDump.h
    ${PbindexdumpSrcDir}/PbIndexDumpVersion.h
    ${PbindexdumpSrcDir}/Settings.h
    ${PbindexdumpSrcDir}/json.hpp
    ${PbindexdumpSrcDir}/main.cpp
)

# build pbindexdump executable
include(PbbamTool)
create_pbbam_tool(
    TARGET  pbindexdump
    SOURCES ${PBINDEXDUMP_SOURCES}
)

# cram tests
if (PacBioBAM_build_tests)
    set(PbindexdumpTestsDir ${PacBioBAM_TestsDir}/src/tools/pbindexdump)
    add_test(
        NAME pbindexdump_CramTests
        WORKING_DIRECTORY ${PacBioBAM_TestsDir}/scripts
        COMMAND "python" cram.py
            ${PbindexdumpTestsDir}/json.t
            ${PbindexdumpTestsDir}/cpp.t
    )
endif()
