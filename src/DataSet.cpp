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
/// \file DataSet.cpp
/// \brief Implements the DataSet class.
//
// Author: Derek Barnett

#include "pbbam/DataSet.h"
#include "pbbam/DataSetTypes.h"
#include "pbbam/internal/DataSetBaseTypes.h"
#include "DataSetIO.h"
#include "FileUtils.h"
#include "TimeUtils.h"
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <unordered_map>

namespace PacBio {
namespace BAM {
namespace internal {

static const std::string defaultVersion{ "4.0.0" };

static void GetAllFiles(const ExternalResources& resources,
                        std::vector<std::string>* result)
{
    for (const auto& resource : resources) {

         // store this resource's path 
        result->push_back(resource.ResourceId());

        // store any child indices
        for (const auto& idx : resource.FileIndices()) 
            result->push_back(idx.ResourceId());
        
        // recurse into any other child resources
        GetAllFiles(resource.ExternalResources(), result);
    }
}

static inline void InitDefaults(DataSet& ds)
{
    // provide default 'CreatedAt' & 'Version' attributes if not already present in XML

    if (ds.CreatedAt().empty())
        ds.CreatedAt(internal::ToIso8601(CurrentTime()));
    
    if (ds.Version().empty())
        ds.Version(internal::defaultVersion);
}

} // namespace internal

using internal::DataSetElement;
using internal::DataSetIO;
using internal::FileUtils;

DataSet::DataSet(void)
    : DataSet(DataSet::GENERIC)
{ 
    internal::InitDefaults(*this);
}

DataSet::DataSet(const DataSet::TypeEnum type)
    : d_(nullptr)
    , path_(FileUtils::CurrentWorkingDirectory())
{
    switch(type) {
        case DataSet::GENERIC             : d_.reset(new DataSetBase); break;
        case DataSet::ALIGNMENT           : d_.reset(new AlignmentSet); break;
        case DataSet::BARCODE             : d_.reset(new BarcodeSet); break;
        case DataSet::CONSENSUS_ALIGNMENT : d_.reset(new ConsensusAlignmentSet); break;
        case DataSet::CONSENSUS_READ      : d_.reset(new ConsensusReadSet); break;
        case DataSet::CONTIG              : d_.reset(new ContigSet); break;
        case DataSet::HDF_SUBREAD         : d_.reset(new HdfSubreadSet); break;
        case DataSet::REFERENCE           : d_.reset(new ReferenceSet); break;
        case DataSet::SUBREAD             : d_.reset(new SubreadSet); break;
        default:
            throw std::runtime_error("unsupported dataset type"); // unknown type
    }

    internal::InitDefaults(*this);
}

DataSet::DataSet(const BamFile& bamFile)
    : d_(DataSetIO::FromUri(bamFile.Filename()))
    , path_(FileUtils::CurrentWorkingDirectory())
{
    internal::InitDefaults(*this);
}

DataSet::DataSet(const std::string& filename)
    : d_(DataSetIO::FromUri(filename))
    , path_(FileUtils::DirectoryName(filename))
{
    // for FOFN contents and raw BAM filenames, we can just use the current
    // directory as the starting path.
    //
    // (any relative paths in the FOFN have already been resolved)
    //
    if (boost::algorithm::iends_with(filename, ".fofn") ||
        boost::algorithm::iends_with(filename, ".bam") ||
        boost::algorithm::iends_with(filename, ".fasta") ||
        boost::algorithm::iends_with(filename, ".fa"))
    {
        path_ = FileUtils::CurrentWorkingDirectory();
    }
    internal::InitDefaults(*this);
}

DataSet::DataSet(const std::vector<std::string>& filenames)
    : d_(DataSetIO::FromUris(filenames))
    , path_(FileUtils::CurrentWorkingDirectory())
{ 
    internal::InitDefaults(*this);
}

DataSet::DataSet(const DataSet& other)
    : path_(other.path_)
{
    DataSetBase* otherDataset = other.d_.get();
    DataSetElement* copyDataset = new DataSetElement(*otherDataset);
    d_.reset(static_cast<DataSetBase*>(copyDataset));
}

DataSet::DataSet(DataSet&& other)
    : d_(std::move(other.d_))
    , path_(std::move(other.path_))
{
    assert(other.d_.get() == nullptr);
}

DataSet& DataSet::operator=(const DataSet& other)
{
    DataSetBase* otherDataset = other.d_.get();
    DataSetElement* copyDataset = new DataSetElement(*otherDataset);
    d_.reset(static_cast<DataSetBase*>(copyDataset));
    path_ = other.path_;
    return *this;
}

DataSet& DataSet::operator=(DataSet&& other)
{
    d_ = std::move(other.d_);
    path_ = std::move(other.path_);
    return *this;
}

DataSet::~DataSet(void) { }

DataSet& DataSet::operator+=(const DataSet& other)
{
    *d_.get() += *other.d_.get();
    return *this;
}

std::vector<std::string> DataSet::AllFiles(void) const
{
    // get all files
    std::vector<std::string> result;
    internal::GetAllFiles(ExternalResources(), &result);

    // resolve relative paths
    std::transform(result.begin(), result.end(), result.begin(),
                   [this](const std::string& fn) { return this->ResolvePath(fn); });
    return result;
}

std::vector<BamFile> DataSet::BamFiles(void) const
{
    const PacBio::BAM::ExternalResources& resources = ExternalResources();
    
    std::vector<BamFile> result;
    result.reserve(resources.Size());
    for(const ExternalResource& ext : resources) {

        // only bother resolving file path if this is a BAM file
        boost::iterator_range<std::string::const_iterator> bamFound = boost::algorithm::ifind_first(ext.MetaType(), "bam");
        if (!bamFound.empty()) {
            const std::string fn = ResolvePath(ext.ResourceId());
            result.push_back(BamFile(fn));
        }
    }
    return result;
}

std::vector<std::string> DataSet::FastaFiles(void) const
{
    const PacBio::BAM::ExternalResources& resources = ExternalResources();

    std::vector<std::string> result;
    result.reserve(resources.Size());
    for(const ExternalResource& ext : resources) {

        // only bother resolving file path if this is a BAM file
        boost::iterator_range<std::string::const_iterator> fastaFound = boost::algorithm::ifind_first(ext.MetaType(), "fasta");
        if (!fastaFound.empty()) {
            const std::string fn = ResolvePath(ext.ResourceId());
            result.push_back(fn);
        }
    }
    return result;
}

DataSet DataSet::FromXml(const std::string& xml)
{
    DataSet result;
    result.d_ = DataSetIO::FromXmlString(xml);
    internal::InitDefaults(result);
    return result;
}

const NamespaceRegistry& DataSet::Namespaces(void) const
{ return d_->Namespaces(); }

NamespaceRegistry& DataSet::Namespaces(void)
{ return d_->Namespaces(); }

DataSet::TypeEnum DataSet::NameToType(const std::string& typeName)
{
    static std::unordered_map<std::string, DataSet::TypeEnum> lookup;
    if (lookup.empty()) {
        lookup["DataSet"]               = DataSet::GENERIC;
        lookup["AlignmentSet"]          = DataSet::ALIGNMENT;
        lookup["BarcodeSet"]            = DataSet::BARCODE;
        lookup["ConsensusAlignmentSet"] = DataSet::CONSENSUS_ALIGNMENT;
        lookup["ConsensusReadSet"]      = DataSet::CONSENSUS_READ;
        lookup["ContigSet"]             = DataSet::CONTIG;
        lookup["HdfSubreadSet"]         = DataSet::HDF_SUBREAD;
        lookup["ReferenceSet"]          = DataSet::REFERENCE;
        lookup["SubreadSet"]            = DataSet::SUBREAD;
    }
    return lookup.at(typeName); // throws if unknown typename
}

std::vector<std::string> DataSet::ResolvedResourceIds(void) const
{
    const PacBio::BAM::ExternalResources& resources = ExternalResources();

    std::vector<std::string> result;
    result.reserve(resources.Size());
    for(const ExternalResource& ext : resources) {
//        const string fn = ;
//        const string fn = internal::FileUtils::ResolvedFilePath(ext.ResourceId(), path_);
        result.push_back(ResolvePath(ext.ResourceId()));
    }
    return result;
}

std::string DataSet::ResolvePath(const std::string& originalPath) const
{ return internal::FileUtils::ResolvedFilePath(originalPath, path_); }

void DataSet::Save(const std::string& outputFilename)
{ DataSetIO::ToFile(d_, outputFilename); }

void DataSet::SaveToStream(std::ostream& out)
{ DataSetIO::ToStream(d_, out); }

std::set<std::string> DataSet::SequencingChemistries(void) const
{
    const std::vector<BamFile> bamFiles{ BamFiles() };

    std::set<std::string> result;
    for(const BamFile& bf : bamFiles) {
        if (!bf.IsPacBioBAM())
            throw std::runtime_error{ "only PacBio BAMs are supported" };
        const std::vector<ReadGroupInfo> readGroups{ bf.Header().ReadGroups() };
        for (const ReadGroupInfo& rg : readGroups)
            result.insert(rg.SequencingChemistry());
    }
    return result;
}

std::string DataSet::TypeToName(const DataSet::TypeEnum& type)
{
    switch(type) {
        case DataSet::GENERIC             : return "DataSet";
        case DataSet::ALIGNMENT           : return "AlignmentSet";
        case DataSet::BARCODE             : return "BarcodeSet";
        case DataSet::CONSENSUS_ALIGNMENT : return "ConsensusAlignmentSet";
        case DataSet::CONSENSUS_READ      : return "ConsensusReadSet";
        case DataSet::CONTIG              : return "ContigSet";
        case DataSet::HDF_SUBREAD         : return "HdfSubreadSet";
        case DataSet::REFERENCE           : return "ReferenceSet";
        case DataSet::SUBREAD             : return "SubreadSet";
        default:
            throw std::runtime_error("unsupported dataset type"); // unknown type
    }
}

// Exposed timestamp utils

std::string CurrentTimestamp(void)
{ return internal::ToDataSetFormat(internal::CurrentTime()); }

std::string ToDataSetFormat(const std::chrono::system_clock::time_point &tp)
{ return internal::ToDataSetFormat(tp); }

std::string ToDataSetFormat(const time_t &t)
{ return ToDataSetFormat(std::chrono::system_clock::from_time_t(t)); }

std::string ToIso8601(const std::chrono::system_clock::time_point &tp)
{ return internal::ToIso8601(tp); }

std::string ToIso8601(const time_t &t)
{ return ToIso8601(std::chrono::system_clock::from_time_t(t)); }

} // namespace BAM
} // namespace PacBio
