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

// Author: David Alexander

#include "pbbam/BamRecordImpl.h"
#include "pbbam/IndexedFastaReader.h"
#include "pbbam/Orientation.h"
#include "htslib/faidx.h"

#include <cstdlib>

namespace PacBio {
namespace BAM {


IndexedFastaReader::IndexedFastaReader()
    : filename_(""),
      handle_(nullptr)
{}

IndexedFastaReader::IndexedFastaReader(const std::string& filename)
    : IndexedFastaReader()
{
    Open(filename);
}

IndexedFastaReader::~IndexedFastaReader()
{
    Close();
}

bool IndexedFastaReader::Open(const std::string &filename)
{
    Close();
    faidx_t* handle = fai_load(filename.c_str());
    if (handle == nullptr)
        return false;
    else
    {
        filename_ = filename;
        handle_ = handle;
        return true;
    }
}

void IndexedFastaReader::Close()
{
    filename_ = "";
    if (handle_ != nullptr)
        fai_destroy(handle_);
    handle_ = nullptr;
}

#define REQUIRE_FAIDX_LOADED if (handle_ == nullptr) throw std::exception()

std::string IndexedFastaReader::Subsequence(const std::string& id,
                                            Position begin,
                                            Position end) const
{
    REQUIRE_FAIDX_LOADED;

    int len;
    // Derek: *Annoyingly* htslib seems to interpret "end" as inclusive in
    // faidx_fetch_seq, whereas it considers it exclusive in the region spec in
    // fai_fetch.  Can you please verify?
    char* rawSeq = faidx_fetch_seq(handle_, id.c_str(), begin, end - 1, &len);
    if (rawSeq == nullptr)
        throw std::exception();
    else {
        std::string seq(rawSeq);
        free(rawSeq);
        return seq;
    }
}

std::string IndexedFastaReader::Subsequence(const char *htslibRegion) const
{
    REQUIRE_FAIDX_LOADED;

    int len;
    char* rawSeq = fai_fetch(handle_, htslibRegion, &len);
    if (rawSeq == nullptr)
        throw std::exception();
    else {
        std::string seq(rawSeq);
        free(rawSeq);
        return seq;
    }
}

std::string
IndexedFastaReader::ReferenceSubsequence(const BamRecordImpl& bamRecord,
                                         Orientation orientation,
                                         bool gapped) const
{
    // TODO: implement me!   This can be done once BamRecord exposes
    // "reference name"

    // (the ref name is the primary identifier... the integer key is a
    // local key that makes sense only within the BAM file.)

    // (we could also have conveniences like this:
    //   GenomicInterval BamRecord::ReferenceInterval();
    //   PacBioReadInterval PacBioBamRecord::QueryInterval();
    //   PacBioReadInterval PacBioBamRecord::AlignedInterval(); )
    return std::string();
}


int IndexedFastaReader::NumSequences() const
{
    REQUIRE_FAIDX_LOADED;
    return faidx_nseq(handle_);
}

bool IndexedFastaReader::HasSequence(const std::string& name) const
{
    REQUIRE_FAIDX_LOADED;
    return (faidx_has_seq(handle_, name.c_str()) != 0);
}

int IndexedFastaReader::SequenceLength(const std::string& name) const
{
    REQUIRE_FAIDX_LOADED;
    int len = faidx_seq_len(handle_, name.c_str());
    if (len < 0)
        throw std::exception();
    else return len;
}


}}  // PacBio::BAM
