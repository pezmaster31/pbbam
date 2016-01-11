
set(PbmergeSrcDir ${PacBioBAM_ToolsDir}/pbmerge/src)

# create version header
set(PbMerge_VERSION ${PacBioBAM_VERSION})
configure_file(
    ${PbmergeSrcDir}/PbMergeVersion.h.in
    ${PbmergeSrcDir}/PbMergeVersion.h
)

# list source files
set(PBMERGE_SOURCES

    ${ToolsCommonDir}/BamFileMerger.h
    ${ToolsCommonDir}/BamFileMerger.inl
    ${ToolsCommonDir}/OptionParser.h
    ${ToolsCommonDir}/OptionParser.cpp

    ${PbmergeSrcDir}/main.cpp
)

# build pbmerge executable
include(PbbamTool)
create_pbbam_tool(
    TARGET  pbmerge
    SOURCES ${PBMERGE_SOURCES}
)

# cram tests
if (PacBioBAM_build_tests)
    set(PbmergeTestsDir ${PacBioBAM_TestsDir}/src/tools/pbmerge)
    add_test(
        NAME pbmerge_CramTests
        WORKING_DIRECTORY ${PacBioBAM_TestsDir}/scripts
        COMMAND "python" cram.py
            ${PbmergeTestsDir}/pacbio_ordering.t
            ${PbmergeTestsDir}/aligned_ordering.t
            ${PbmergeTestsDir}/mixed_ordering.t
    )
endif()
