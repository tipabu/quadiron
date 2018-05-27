#!/bin/bash

# Copyright 2017-2018 Scality
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

bin=$1
samples_nb=100
chunk_size=512
sce_type=enc_dec
show_type=1
threads_nb=1

if [ -z $1 ]
then
    1>&2 echo "error: missing argument"
    echo "usage: $0 PATH_TO_BENCH_DRIVER"
    exit 1;
fi

chunk_size=51200
# for rs-nf4
ec_type=rs-nf4
word_size=8
for k in 16; do
  for n in 256 1024; do
    m=$((n-k))
    ${bin} -e ${ec_type} -w ${word_size} -k ${k} -m ${m} -c ${chunk_size} -s ${sce_type} -g ${threads_nb} -f ${show_type}
    show_type=0
  done
done

# for rs-fnt with different packet sizes
ec_type=rs-fnt
word_size=2
for k in 16; do
  for n in 256 1024; do
    m=$((n-k))
    for pkt_size in 0 256 512; do
      ${bin} -e ${ec_type} -w ${word_size} -k ${k} -m ${m} -c ${chunk_size} -s ${sce_type} -g ${threads_nb} -f ${show_type} -p ${pkt_size}
    done
  done
done

# for all cases
sce_type=enc_dec
chunk_size=512
for ec_type in rs-gf2n-v rs-gf2n-c rs-gf2n-fft rs-gf2n-fft-add rs-gfp-fft rs-fnt rs-nf4; do
  for k in 5; do
    for m in 2; do
      for word_size in 1 2 4 8; do
        ${bin} -e ${ec_type} -w ${word_size} -k ${k} -m ${m} -c ${chunk_size} -s ${sce_type} -g ${threads_nb} -f ${show_type}
      done
    done
  done
done
