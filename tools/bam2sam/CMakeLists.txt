
set(Bam2SamSrcDir ${PacBioBAM_ToolsDir}/bam2sam/src)

# create version header
set(Bam2Sam_VERSION ${PacBioBAM_VERSION})
configure_file(
    ${Bam2SamSrcDir}/Bam2SamVersion.h.in
    ${Bam2SamSrcDir}/Bam2SamVersion.h
)

# list source files
set(BAM2SAM_SOURCES

    ${ToolsCommonDir}/OptionParser.h
    ${ToolsCommonDir}/OptionParser.cpp

    ${Bam2SamSrcDir}/main.cpp
    ${Bam2SamSrcDir}/Bam2Sam.h
    ${Bam2SamSrcDir}/Bam2Sam.cpp
    ${Bam2SamSrcDir}/Bam2SamVersion.h
    ${Bam2SamSrcDir}/Settings.h
)

# build bam2sam executable
include(PbbamTool)
create_pbbam_tool(
    TARGET  bam2sam
    SOURCES ${BAM2SAM_SOURCES}
)

# cram tests
if (PacBioBAM_build_tests)
    set(Bam2SamTestsDir ${PacBioBAM_TestsDir}/src/tools/bam2sam)
    add_test(
        NAME bam2sam_CramTests
        WORKING_DIRECTORY ${PacBioBAM_TestsDir}/scripts
        COMMAND "python" cram.py
            ${Bam2SamTestsDir}/bam2sam.t
    )
endif()
