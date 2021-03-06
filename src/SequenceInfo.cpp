
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
//
// File Description
/// \file SequenceInfo.cpp
/// \brief Implements the SequenceInfo class.
//
// Author: Derek Barnett

#include "pbbam/SequenceInfo.h"
#include "SequenceUtils.h"
#include <sstream>

namespace PacBio {
namespace BAM {
namespace internal {

static std::string token_SN = std::string("SN");
static std::string token_LN = std::string("LN");
static std::string token_AS = std::string("AS");
static std::string token_M5 = std::string("M5");
static std::string token_SP = std::string("SP");
static std::string token_UR = std::string("UR");

} // namespace internal

SequenceInfo::SequenceInfo(const std::string& name,
                           const std::string& length)
    : name_(name)
    , length_(length)
{ }

SequenceInfo SequenceInfo::FromSam(const std::string& sam)
{
    // pop off '@SQ\t', then split rest of line into tokens
    const std::vector<std::string>& tokens = internal::Split(sam.substr(4), '\t');
    if (tokens.empty())
        return SequenceInfo();

    SequenceInfo seq;
    std::map<std::string, std::string> custom;

    // iterate over tokens
    for (const std::string& token : tokens) {
        const std::string& tokenTag   = token.substr(0,2);
        const std::string& tokenValue = token.substr(3);

        // set sequence info
        if      (tokenTag == internal::token_SN) seq.Name(tokenValue);
        else if (tokenTag == internal::token_LN) seq.Length(tokenValue);
        else if (tokenTag == internal::token_AS) seq.AssemblyId(tokenValue);
        else if (tokenTag == internal::token_M5) seq.Checksum(tokenValue);
        else if (tokenTag == internal::token_SP) seq.Species(tokenValue);
        else if (tokenTag == internal::token_UR) seq.Uri(tokenValue);

        // otherwise, "custom" tag
        else
            custom[tokenTag] = tokenValue;
    }

    seq.CustomTags(custom);
    return seq;
}

bool SequenceInfo::IsValid(void) const
{
    if (name_.empty())
        return false;

    // use long instead of int32_t, just to make sure we can catch overflow
    const long l = atol(length_.c_str());
    return l >= 0 && l <= INT32_MAX;
}

std::string SequenceInfo::ToSam(void) const
{
    std::stringstream out;
    out << "@SQ"
        << internal::MakeSamTag(internal::token_SN, name_);

    if (!length_.empty())     out << internal::MakeSamTag(internal::token_LN, length_);
    if (!assemblyId_.empty()) out << internal::MakeSamTag(internal::token_AS, assemblyId_);
    if (!checksum_.empty())   out << internal::MakeSamTag(internal::token_M5, checksum_);
    if (!species_.empty())    out << internal::MakeSamTag(internal::token_SP, species_);
    if (!uri_.empty())        out << internal::MakeSamTag(internal::token_UR, uri_);

    // append any custom tags
    std::map<std::string, std::string>::const_iterator customIter = custom_.cbegin();
    std::map<std::string, std::string>::const_iterator customEnd  = custom_.cend();
    for ( ; customIter != customEnd; ++customIter )
        out << internal::MakeSamTag(customIter->first, customIter->second);

    return out.str();
}

} // namespace BAM
} // namespace PacBio
