// Copyright (c) 2014-2015, Pacific Biosciences of California, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted (subject to the limitations in the
// disclaimer below) provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//  * Neither the name of Pacific Biosciences nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY PACIFIC
// BIOSCIENCES AND ITS CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL PACIFIC BIOSCIENCES OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

// Author: Derek Barnett

#ifdef PBBAM_TESTING
#define private public
#define protected public
#endif

#include "TestData.h"
#include <gtest/gtest.h>
#include <htslib/sam.h>
#include <pbbam/BamFile.h>
#include <pbbam/BamReader.h>
#include <pbbam/BamWriter.h>
#include <pbbam/EntireFileQuery.h>
#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
using namespace PacBio;
using namespace PacBio::BAM;
using namespace std;

struct Bam1Deleter
{
    void operator()(bam1_t* b) {
        if (b)
            bam_destroy1(b);
        b = nullptr;
    }
};

struct SamFileDeleter
{
    void operator()(samFile* file) {
        if (file)
            sam_close(file);
        file = nullptr;
    }
};

struct BamHdrDeleter
{
    void operator()(bam_hdr_t* hdr) {
        if (hdr)
            bam_hdr_destroy(hdr);
        hdr = nullptr;
    }
};

const string inputBamFn        = tests::Data_Dir + "/ex2.bam";
const string goldStandardSamFn = tests::Data_Dir + "/ex2.sam";
const string generatedBamFn    = tests::Data_Dir + "/generated.bam";
const string generatedSamFn    = tests::Data_Dir + "/generated.sam";

static inline
int Samtools_Bam2Sam(const string& bamFilename,
                     const string& samFilename)
{
    const std::string& convertArgs = string("view -h ") + bamFilename + string(" > ")  + samFilename;
    const std::string& convertCommandLine = tests::Samtools_Bin + string(" ") + convertArgs;
    return system(convertCommandLine.c_str());
}

static inline
int Diff_Sam2Sam(const string& fn1,
                 const string& fn2)
{
    const std::string& diffCommandLine = string("diff ") + fn1 + string(" ") + fn2;
    return system(diffCommandLine.c_str());
}

static inline
void RemoveGeneratedFiles(const string& fn1,
                          const string& fn2)
{
    remove(fn1.c_str());
    remove(fn2.c_str());
}

// sanity check for rest of tests below
TEST(EndToEndTest, ReadPureHtslib_WritePureHtslib)
{
    // open input BAM file
    PBBAM_SHARED_PTR<samFile> inputBam(sam_open(inputBamFn.c_str(), "r"), SamFileDeleter());
    EXPECT_TRUE(inputBam != 0);
    PBBAM_SHARED_PTR<bam_hdr_t> header(sam_hdr_read(inputBam.get()), BamHdrDeleter());

    // open output BAM file
    PBBAM_SHARED_PTR<samFile> outputBam(sam_open(generatedBamFn.c_str(), "wb"), SamFileDeleter());
    sam_hdr_write(outputBam.get(), header.get());

    // copy BAM file
    PBBAM_SHARED_PTR<bam1_t> record(bam_init1(), Bam1Deleter());
    while (sam_read1(inputBam.get(), header.get(), record.get()) >= 0)
        sam_write1(outputBam.get(), header.get(), record.get());

    // need to close files before comparing (to flush any buffers)
    inputBam.reset();
    outputBam.reset();

    // convert to sam & diff against gold standard
    const int convertRet = Samtools_Bam2Sam(generatedBamFn, generatedSamFn);
    const int diffRet    = Diff_Sam2Sam(goldStandardSamFn, generatedSamFn);
    EXPECT_EQ(0, convertRet);
    EXPECT_EQ(0, diffRet);

    // clean up
    RemoveGeneratedFiles(generatedBamFn, generatedSamFn);
}

TEST(EndToEndTest, ReadRawData_WriteRawData)
{
    // open input BAM file
    BamReader reader;
    const bool inputOpenedOk = reader.Open(inputBamFn);
    EXPECT_TRUE(inputOpenedOk);

    // open output BAM file
    BamWriter writer(generatedBamFn, *reader.Header().get());
    EXPECT_TRUE(writer);

    // copy BAM file
    PBBAM_SHARED_PTR<bam1_t> record(bam_init1(), Bam1Deleter());
    while (reader.GetNext(record))
        writer.Write(record);

    // need to close files before comparing (to flush any buffers)
    reader.Close();
    writer.Close();

    // convert to sam & diff against gold standard
    const int convertRet = Samtools_Bam2Sam(generatedBamFn, generatedSamFn);
    const int diffRet    = Diff_Sam2Sam(goldStandardSamFn, generatedSamFn);
    EXPECT_EQ(0, convertRet);
    EXPECT_EQ(0, diffRet);

    // clean up
    RemoveGeneratedFiles(generatedBamFn, generatedSamFn);
}

TEST(EndToEndTest, ReadBamRecord_WriteBamRecord_SingleThread)
{
    // open input BAM file
    BamFile bamFile(inputBamFn);
    EXPECT_TRUE(bamFile.Error() == BamFile::NoError);

    // open output BAM file
    BamWriter writer(generatedBamFn, bamFile.Header(), BamWriter::DefaultCompression, 1);
    EXPECT_TRUE(writer);

    // copy BAM file
    EntireFileQuery entireFile(bamFile);
    EXPECT_TRUE(entireFile);
    for (const BamRecord& record : entireFile)
        writer.Write(record);
    writer.Close();  // need to close output file before comparing (to flush any buffers)

    // convert to sam & diff against gold standard
    const int convertRet = Samtools_Bam2Sam(generatedBamFn, generatedSamFn);
    const int diffRet    = Diff_Sam2Sam(goldStandardSamFn, generatedSamFn);
    EXPECT_EQ(0, convertRet);
    EXPECT_EQ(0, diffRet);

    // clean up
    RemoveGeneratedFiles(generatedBamFn, generatedSamFn);
}

TEST(EndToEndTest, ReadBamRecord_WriteBamRecord_APIDefaultThreadCount)
{
    // open input BAM file
    BamFile bamFile(inputBamFn);
    EXPECT_TRUE(bamFile.Error() == BamFile::NoError);

    // open output BAM file
    BamWriter writer(generatedBamFn, bamFile.Header());
    EXPECT_TRUE(writer);

    // copy BAM file
    EntireFileQuery entireFile(bamFile);
    EXPECT_TRUE(entireFile);
    for (const BamRecord& record : entireFile)
        writer.Write(record);
    writer.Close();  // need to close output file before comparing (to flush any buffers)

    // convert to sam & diff against gold standard
    const int convertRet = Samtools_Bam2Sam(generatedBamFn, generatedSamFn);
    const int diffRet    = Diff_Sam2Sam(goldStandardSamFn, generatedSamFn);
    EXPECT_EQ(0, convertRet);
    EXPECT_EQ(0, diffRet);

    // clean up
    RemoveGeneratedFiles(generatedBamFn, generatedSamFn);
}

TEST(EndToEndTest, ReadBamRecord_WriteBamRecord_SystemDefaultThreadCount)
{
    // open input BAM file
    BamFile bamFile(inputBamFn);
    EXPECT_TRUE(bamFile.Error() == BamFile::NoError);

    // open output BAM file
    BamWriter writer(generatedBamFn, bamFile.Header(), BamWriter::DefaultCompression, 0);
    EXPECT_TRUE(writer);

    // copy BAM file
    EntireFileQuery entireFile(bamFile);
    EXPECT_TRUE(entireFile);
    for (const BamRecord& record : entireFile)
        writer.Write(record);
    writer.Close();  // need to close output file before comparing (to flush any buffers)

    // convert to sam & diff against gold standard
    const int convertRet = Samtools_Bam2Sam(generatedBamFn, generatedSamFn);
    const int diffRet    = Diff_Sam2Sam(goldStandardSamFn, generatedSamFn);
    EXPECT_EQ(0, convertRet);
    EXPECT_EQ(0, diffRet);

    // clean up
    RemoveGeneratedFiles(generatedBamFn, generatedSamFn);
}

TEST(EndToEndTest, ReadBamRecord_WriteBamRecord_UserThreadCount)
{
    // open input BAM file
    BamFile bamFile(inputBamFn);
    EXPECT_TRUE(bamFile.Error() == BamFile::NoError);

    // open output BAM file
    BamWriter writer(generatedBamFn, bamFile.Header(), BamWriter::DefaultCompression, 6);
    EXPECT_TRUE(writer);

    // copy BAM file
    EntireFileQuery entireFile(bamFile);
    EXPECT_TRUE(entireFile);
    for (const BamRecord& record : entireFile)
        writer.Write(record);
    writer.Close();  // need to close output file before comparing (to flush any buffers)

    // convert to sam & diff against gold standard
    const int convertRet = Samtools_Bam2Sam(generatedBamFn, generatedSamFn);
    const int diffRet    = Diff_Sam2Sam(goldStandardSamFn, generatedSamFn);
    EXPECT_EQ(0, convertRet);
    EXPECT_EQ(0, diffRet);

    // clean up
    RemoveGeneratedFiles(generatedBamFn, generatedSamFn);
}

TEST(EndToEndTest, BamFileReuse)
{
    BamFile file;
    ASSERT_FALSE(file.IsOpen());
    ASSERT_TRUE(file.Filename().empty());
    ASSERT_FALSE((bool)file.Header());

    file.Open(inputBamFn);
    ASSERT_TRUE(file.IsOpen());
    ASSERT_FALSE(file.Filename().empty());
    ASSERT_TRUE((bool)file.Header());

    file.Close();
    ASSERT_FALSE(file.IsOpen());
    ASSERT_TRUE(file.Filename().empty());
    ASSERT_FALSE((bool)file.Header());

    file.Open(inputBamFn);
    ASSERT_TRUE(file.IsOpen());
    ASSERT_FALSE(file.Filename().empty());
    ASSERT_TRUE((bool)file.Header());
}
