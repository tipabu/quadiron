/* -*- mode: c++ -*- */
/*
 * Copyright 2017-2018 Scality
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __QUAD_PROPERTY_H__
#define __QUAD_PROPERTY_H__

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

#include <sys/types.h>

namespace quadiron {

static constexpr unsigned OOR_MARK = 1;

/** Ancillary data attached to values.
 *
 * A property carries extra-information (whose interpretation is left to the
 * reader) related to a specific value (identified by its location).
 * It wraps a map whose each element is a key/value where
 *  - key indicates the location of symbol whose value should be adjusted
 *  - value indicates value that could be used to adjust the symbol value
 * For prime fields, value is always 1.
 * For NF4, value is an uint32_t integer.
 */
class Properties {
  public:
    inline void add(const size_t loc, const uint32_t data)
    {
        keys.push_back(loc);
        values.push_back(data);
        items_nb++;
    }

    inline void clear()
    {
        keys.clear();
        values.clear();
        items_nb = 0;
    }

    const std::vector<size_t>& get_keys() const
    {
        return keys;
    }

    const std::vector<uint32_t>& get_values() const
    {
        return values;
    }

  private:
    std::vector<size_t> keys;
    std::vector<uint32_t> values;
    size_t items_nb = 0;

    friend std::istream& operator>>(std::istream& is, Properties& props);
    friend std::ostream& operator<<(std::ostream& os, const Properties& props);
};

} // namespace quadiron

#endif
