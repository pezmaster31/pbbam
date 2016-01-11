
set(PbindexSrcDir ${PacBioBAM_ToolsDir}/pbindex/src)

# create version header
set(PbIndex_VERSION ${PacBioBAM_VERSION})
configure_file(
    ${PbindexSrcDir}/PbIndexVersion.h.in
    ${PbindexSrcDir}/PbIndexVersion.h
)

# list source files
set(PBINDEX_SOURCES

    ${ToolsCommonDir}/OptionParser.h
    ${ToolsCommonDir}/OptionParser.cpp

    ${PbindexSrcDir}/main.cpp
    ${PbindexSrcDir}/PbIndex.h
    ${PbindexSrcDir}/PbIndex.cpp
    ${PbindexSrcDir}/PbIndexVersion.h
)

# build pbindex executable
include(PbbamTool)
create_pbbam_tool(
    TARGET  pbindex
    SOURCES ${PBINDEX_SOURCES}
)
