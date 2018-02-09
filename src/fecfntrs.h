/* -*- mode: c++ -*- */
#ifndef __NTL_FECFNTRS_H__
#define __NTL_FECFNTRS_H__

#include "fec.h"
#include "fft2k.h"
#include "poly.h"
#include "vvec.h"
#include "vvecp.h"

/**
 * GF_2^2^k+1 based RS (Fermat Number Transform)
 * As suggested by the paper:
 * FNT-based Reed-Solomon Erasure Codes
 * by Alexandre Soro and Jerome Lacan
 */
template <typename T>
class FECFNTRS : public FEC<T> {
  private:
    FFT2K<T>* fft = nullptr;

  public:
    T n;
    T r;
    FECFNTRS(
        unsigned word_size,
        unsigned n_data,
        unsigned n_parities,
        size_t pkt_size = 8)
        : FEC<T>(FEC<T>::TYPE_2, word_size, n_data, n_parities, pkt_size)
    {
        assert(word_size < 4);
        // warning all fermat numbers >= to F_5 (2^32+1) are composite!!!
        T gf_p = (1ULL << (8 * word_size)) + 1;
        this->gf = new GFP<T>(gf_p);

        assert(_jacobi<T>(this->gf->get_prime_root(), this->gf->card()) == -1);

        // with this encoder we cannot exactly satisfy users request, we need to
        // pad n = minimal divisor of (q-1) that is at least (n_parities +
        // n_data)
        n = this->gf->get_code_len_high_compo(n_parities + n_data);

        // compute root of order n-1 such as r^(n-1) mod q == 1
        r = this->gf->get_nth_root(n);

        // std::cerr << "n=" << n << "\n";
        // std::cerr << "r=" << r << "\n";

        this->fft = new FFT2K<T>(this->gf, n, pkt_size);
    }

    ~FECFNTRS()
    {
        delete fft;
        delete this->gf;
    }

    int get_n_outputs()
    {
        return this->n;
    }

    /**
     * Encode vector
     *
     * @param output must be n
     * @param props must be exactly n
     * @param offset used to locate special values
     * @param words must be n_data
     */
    void encode(
        Vec<T>* output,
        std::vector<KeyValue*> props,
        off_t offset,
        Vec<T>* words)
    {
        VVec<T> vwords(words, n);
        fft->fft(output, &vwords);
        // max_value = 2^x
        T thres = fft->get_gf()->card() - 1;
        // check for out of range value in output
        for (unsigned i = 0; i < this->code_len; i++) {
            if (output->get(i) & thres) {
                char buf[256];
                snprintf(buf, sizeof(buf), "%zd:%d", offset, i);
                assert(nullptr != props[i]);
                props[i]->insert(std::make_pair(buf, "@"));
                output->set(i, 0);
            }
        }
    }

    void encode(
        Vecp<T>* output,
        std::vector<KeyValue*> props,
        off_t offset,
        Vecp<T>* words)
    {
        VVecp<T> vwords(words, n);
        fft->fft(output, &vwords);
        // check for out of range value in output
        int size = output->get_size();
        T thres = (fft->get_gf()->card() - 1);
        for (unsigned i = 0; i < this->code_len; i++) {
            T* chunk = output->get(i);
            for (int j = 0; j < size; j++) {
                if (chunk[j] & thres) {
                    char buf[256];
                    snprintf(
                        buf,
                        sizeof(buf),
                        "%zd:%d",
                        offset + j * this->word_size,
                        i);
                    assert(nullptr != props[i]);
                    props[i]->insert(std::make_pair(buf, "@"));
                    chunk[j] = 0;
                }
            }
        }
    }

    void decode_add_data(int fragment_index, int row)
    {
        // not applicable
        assert(false);
    }

    void decode_add_parities(int fragment_index, int row)
    {
        // we can't anticipate here
    }

    void decode_build()
    {
        // nothing to do
    }

    /**
     * Perform a Lagrange interpolation to find the coefficients of the
     * polynomial
     *
     * @note If all fragments are available ifft(words) is enough
     *
     * @param output must be exactly n_data
     * @param props special values dictionary must be exactly n_data
     * @param offset used to locate special values
     * @param fragments_ids unused
     * @param words v=(v_0, v_1, ..., v_k-1) k must be exactly n_data
     */
    void decode(
        Vec<T>* output,
        std::vector<KeyValue*> props,
        off_t offset,
        Vec<T>* fragments_ids,
        Vec<T>* words)
    {
        int k = this->n_data; // number of fragments received
        // vector x=(x_0, x_1, ..., x_k-1)
        Vec<T> vx(this->gf, k);
        for (int i = 0; i < k; i++) {
            vx.set(i, this->gf->exp(r, fragments_ids->get(i)));
        }

        for (int i = 0; i < k; i++) {
            int j = fragments_ids->get(i);
            char buf[256];
            snprintf(buf, sizeof(buf), "%zd:%d", offset, j);
            if (nullptr != props[j]) {
                if (props[j]->is_key(buf) && props[j]->at(buf) == "@")
                    words->set(i, fft->get_gf()->card() - 1);
            }
        }

        // Lagrange interpolation
        Poly<T> A(this->gf), _A(this->gf);

        // compute A(x) = prod_j(x-x_j)
        A.set(0, 1);
        for (int i = 0; i < k; i++) {
            Poly<T> _t(this->gf);
            _t.set(1, 1);
            _t.set(0, this->gf->sub(0, vx.get(i)));
            // _t.dump();
            A.mul(&_t);
        }
        // std::cout << "A(x)="; A.dump();

        // compute A'(x) since A_i(x_i) = A'_i(x_i)
        _A.copy(&A);
        _A.derivative();
        // std::cout << "A'(x)="; _A.dump();

        // evaluate n_i=v_i/A'_i(x_i)
        Vec<T> _n(this->gf, k);
        for (int i = 0; i < k; i++) {
            _n.set(i, this->gf->div(words->get(i), _A.eval(vx.get(i))));
        }
        // std::cout << "_n="; _n.dump();

        // compute N'(x) = sum_i{n_i * x^z_i}
        Poly<T> N_p(this->gf);
        for (int i = 0; i <= k - 1; i++) {
            N_p.set(fragments_ids->get(i), _n.get(i));
        }

        // We have to find the numerator of the following expression:
        // P(x)/A(x) = sum_i=0_k-1(n_i/(x-x_i)) mod x^n
        // using Taylor series we rewrite the expression into
        // P(x)/A(x) = -sum_i=0_k-1(sum_j=0_n-1(n_i*x_i^(-j-1)*x^j))
        Poly<T> S(this->gf);
        for (T i = 0; i <= n - 1; i++) {
            T val = this->gf->inv(this->gf->exp(r, i + 1));
            S.set(i, N_p.eval(val));
        }
        S.neg();
        S.mul(&A);
        // No need to mod x^n since only last n_data coefs are obtained
        // output is n_data length
        for (unsigned i = 0; i < this->n_data; i++)
            output->set(i, S.get(i));
    }
};

#endif
