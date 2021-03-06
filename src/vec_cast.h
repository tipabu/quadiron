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
#ifndef __QUAD_VEC_CAST_H__
#define __QUAD_VEC_CAST_H__

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "vec_vector.h"

namespace quadiron {

namespace vec {

template <typename Ts, typename Td, typename Tw>
inline void pack_next(
    const std::vector<Ts*>& src,
    const std::vector<Td*>& dest,
    int n,
    size_t size)
{
    for (int i = 0; i < n; i++) {
        Tw* tmp = reinterpret_cast<Tw*>(src.at(i));
        std::copy_n(tmp, size, dest.at(i));
    }
}

/*
 * Cast buffers of source to a type corresponding to 'word_size'
 * Copy casted elements to buffers of destination
 *
 *  sizeof(Ts) <= sizeof(Td)
 *
 * @param src: source vector
 * @param dest: destination vector
 * @param n: number of buffers per vector
 * @param size: number of elements per destination buffer
 * @param word_size: number of bytes used to store data in each element of
 *  destination buffers
 * @return
 */
template <typename Ts, typename Td>
inline void pack(
    const std::vector<Ts*>& src,
    const std::vector<Td*>& dest,
    int n,
    size_t size,
    size_t word_size)
{
    assert(sizeof(Td) >= word_size);
    assert(word_size % sizeof(Ts) == 0);
    // get only word_size bytes from each element
    switch (word_size) {
    case 1:
        pack_next<Ts, Td, uint8_t>(src, dest, n, size);
        break;
    case 2:
        pack_next<Ts, Td, uint16_t>(src, dest, n, size);
        break;
    case 4:
        pack_next<Ts, Td, uint32_t>(src, dest, n, size);
        break;
    case 8:
        pack_next<Ts, Td, uint64_t>(src, dest, n, size);
        break;
    case 16:
        pack_next<Ts, Td, __uint128_t>(src, dest, n, size);
        break;
    default:
        break;
    }
}

template <typename Ts, typename Td, typename Tw>
inline void unpack_next(
    const std::vector<Ts*>& src,
    const std::vector<Td*>& dest,
    int n,
    size_t size)
{
    for (int i = 0; i < n; i++) {
        Tw* tmp = reinterpret_cast<Tw*>(dest.at(i));
        std::copy_n(src.at(i), size, tmp);
    }
}

/*
 * Cast buffers of destination to a type corresponding to 'word_size'
 * Copy elements from source buffers to casted buffers
 *
 *  sizeof(Ts) >= sizeof(Td)
 *
 * @param src: source vector
 * @param dest: destination vector
 * @param n: number of buffers per vector
 * @param size: number of elements per source buffer
 * @param word_size: number of bytes used to store data in each element of
 *  source buffers
 * @return
 */
template <typename Ts, typename Td>
inline void unpack(
    const std::vector<Ts*>& src,
    const std::vector<Td*>& dest,
    int n,
    size_t size,
    size_t word_size)
{
    assert(sizeof(Ts) >= word_size);
    assert(word_size % sizeof(Td) == 0);
    // get only word_size bytes from each element
    switch (word_size) {
    case 1:
        unpack_next<Ts, Td, uint8_t>(src, dest, n, size);
        break;
    case 2:
        unpack_next<Ts, Td, uint16_t>(src, dest, n, size);
        break;
    case 4:
        unpack_next<Ts, Td, uint32_t>(src, dest, n, size);
        break;
    case 8:
        unpack_next<Ts, Td, uint64_t>(src, dest, n, size);
        break;
    case 16:
        unpack_next<Ts, Td, __uint128_t>(src, dest, n, size);
        break;
    default:
        break;
    }
}

/*
 * Get and cast mem of Buffers<Ts> to a vector of Td*
 */
template <typename Ts, typename Td>
std::vector<Td*>* cast_mem_of_vecp(vec::Buffers<Ts>* s)
{
    int i;
    int n = s->get_n();

    // std::cout << "\ninput: "; s->dump();

    const std::vector<Ts*> mem_s = s->get_mem();
    std::vector<Td*>* mem_d = new std::vector<Td*>(n, nullptr);
    for (i = 0; i < n; i++) {
        mem_d->at(i) = reinterpret_cast<Td*>(mem_s.at(i));
    }

    return mem_d;
}

} // namespace vec
} // namespace quadiron

#endif
