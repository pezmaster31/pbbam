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

#ifndef FRAMES_H
#define FRAMES_H

#include "pbbam/Config.h"
#include <vector>

namespace PacBio {
namespace BAM {

class PBBAM_EXPORT Frames
{
public:
    /// \name Conversion Methods
    /// \{

    /// Constructs a Frames object from encoded (lossy, 8-bit data).
    ///
    /// \note This method should probably not be needed often by client code working with frame data.
    /// It exists primarily for (internal) parsing & interpretation of the BAM file contents. The
    /// method is available, though, should the conversion operation be needed.
    ///
    /// \param[in] codedData encoded data
    /// \returns Frames object
    static Frames CodeToFrames(const std::vector<uint8_t>& codedData);

    /// Lossy encodes a container of (raw) frame values.
    ///
    /// \param[in] frames expanded frame data
    /// \returns lossy, 8-bit encoded frame codes
    static std::vector<uint8_t> Downsample(const std::vector<uint16_t>& frames);

    /// \}

public:
    /// \name Constructors & Related Methods
    /// \{

    Frames(void);
    Frames(const std::vector<uint16_t>& frames);
    Frames(const std::vector<uint8_t>::const_iterator first,
            const std::vector<uint8_t>::const_iterator last);


    Frames(std::vector<uint16_t>&& frames);
    Frames(const Frames& other);
    Frames(Frames&& other);
    Frames& operator=(const Frames& other);
    Frames& operator=(Frames&& other);
    ~Frames(void);

    /// \}

public:
    /// \name Access Data
    /// \{

    /// \returns Frame data in expanded (not encoded) form
    const std::vector<uint16_t>& Data(void) const;

    /// \}

public:
    /// \name Conversion Methods
    /// \{

    /// \returns Frame data in (lossy, 8-bit) encoded form.
    std::vector<uint8_t> Downsampled(void) const;

    /// \}

public:
    /// \name Iterators
    /// \{

    /// \returns A const_iterator to the beginning of the sequence.
    std::vector<uint16_t>::const_iterator cbegin(void) const;

    /// \returns A const_iterator to the element past the end of the sequence.
    std::vector<uint16_t>::const_iterator cend(void) const;

    /// \returns A const_iterator to the beginning of the sequence.
    std::vector<uint16_t>::const_iterator begin(void) const;

    /// \returns A const_iterator to the element past the end of the sequence.
    std::vector<uint16_t>::const_iterator end(void) const;

    /// \returns An iterator to the beginning of the sequence.
    std::vector<uint16_t>::iterator begin(void);

    /// \returns An iterator to the element past the end of the sequence.
    std::vector<uint16_t>::iterator end(void);

    /// \} 

public:
    /// \name Access Data
    /// \{

    /// Sets this record's data.
    ///
    /// \param[in] frames data in expanded (not encoded) form
    /// \returns reference to this object
    Frames& Data(const std::vector<uint16_t>& frames);

    /// Sets this record's data.
    ///
    /// This is an overloaded function, allowing move semantics
    /// (instead of copying the data).
    ///
    /// \param[in] frames data in expanded (not encoded) form
    /// \returns reference to this object
    Frames& Data(std::vector<uint16_t>&& frames);

    /// \}

private:
    std::vector<uint16_t> data_;
};

inline const std::vector<uint16_t>& Frames::Data(void) const
{ return data_; }

inline std::vector<uint8_t> Frames::Downsampled(void) const
{ return Frames::Downsample(data_); }

inline Frames& Frames::Data(const std::vector<uint16_t>& frames)
{ data_ = frames; return *this; }

inline Frames& Frames::Data(std::vector<uint16_t>&& frames)
{ data_ = std::move(frames); return *this; }

inline std::vector<uint16_t>::const_iterator Frames::cbegin(void) const
{ return data_.cbegin(); }

inline std::vector<uint16_t>::const_iterator Frames::cend(void) const
{ return data_.cend(); }

inline std::vector<uint16_t>::const_iterator Frames::begin(void) const
{ return data_.begin(); }

inline std::vector<uint16_t>::const_iterator Frames::end(void) const
{ return data_.end(); }

inline std::vector<uint16_t>::iterator Frames::begin(void)
{ return data_.begin(); }

inline std::vector<uint16_t>::iterator Frames::end(void)
{ return data_.end(); }

inline bool operator==(const Frames& lhs, const Frames& rhs)
{ return lhs.Data() == rhs.Data(); }

inline bool operator==(const Frames& lhs, const std::vector<uint16_t>& rhs)
{ return lhs.Data() == rhs; }

inline bool operator==(const std::vector<uint16_t>& lhs, const Frames& rhs)
{ return lhs == rhs.Data(); }

inline bool operator!=(const Frames& lhs, const Frames& rhs)
{ return !(lhs == rhs); }

inline bool operator!=(const Frames& lhs, const std::vector<uint16_t>& rhs)
{ return !(lhs == rhs); }

inline bool operator!=(const std::vector<uint16_t>& lhs, const Frames& rhs)
{ return !(lhs == rhs); }

} // namespace BAM
} // namespace PacBio

#endif // FRAMES_H