/*
The MIT License (MIT)

Copyright (c) 2014 Julian Ingram

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

#include "jarr.h"

// the length in bits of a single element
const jarr_element_length_t jarr_element_length
        = (jarr_element_length_t) sizeof (jarr_element_t) * CHAR_BIT;

struct jarr jarr_init(jarr_element_t * const _arr,
                      jarr_length_t const _length_bits)
{
    struct jarr j;
    j.arr = _arr;
    jarr_set_length(&j, _length_bits);
    return j;
}

// last_element = limiter_element = arr if length_elements = 0
// last_element = arr = limiter_element - 1 if length_elements = 1

void jarr_set_length(struct jarr * const j, jarr_length_t const _length_bits)
{
    j->length_bits = _length_bits;
    j->bme = j->length_bits % jarr_element_length;
    j->mask = (j->bme != (jarr_element_length_t) 0U) ? ((jarr_element_t) - 1
            >> (jarr_element_length - j->bme)) : (jarr_element_t) - 1;
    j->length_elements = jarr_bltoel(j->length_bits);
    jarr_set_limits(j);
}

void jarr_clear_section(struct jarr * const j, jarr_length_t const length,
                        jarr_length_t const startbit)
{
    jarr_element_t* element_tbc = j->arr + jarr_bitoei(startbit);
    jarr_length_t const limiter_bit = startbit + length;
    jarr_element_t const* const last_element_tbc = j->arr
            + jarr_bitoei(limiter_bit - 1);

    if (element_tbc == last_element_tbc)
    {
        *element_tbc &= (length == jarr_element_length)
                ? (jarr_element_t) 0 : ~((((jarr_element_t) 1 << length)
                - (jarr_element_t) 1) << (startbit % jarr_element_length));
    }
    else
    {
        *element_tbc &= ~((jarr_element_t) - 1 << (startbit
                % jarr_element_length));
        ++element_tbc;

        while (element_tbc < last_element_tbc)
        {
            *element_tbc = (jarr_element_t) 0;
            ++element_tbc;
        }

        jarr_element_length_t lme = limiter_bit % jarr_element_length;
        *element_tbc &= lme ? (jarr_element_t) - 1 << lme : (jarr_element_t) 0;
    }
}

void jarr_set_section(struct jarr * const j, jarr_length_t const length,
                      jarr_length_t const startbit)
{
    jarr_element_t* element_tbs = j->arr + jarr_bitoei(startbit);
    jarr_length_t const limiter_bit = startbit + length;
    jarr_element_t const* const last_element_tbs = j->arr
            + jarr_bitoei(limiter_bit - (jarr_length_t) 1);

    if (element_tbs == last_element_tbs)
    {
        *element_tbs |= (length == jarr_element_length) ? (jarr_element_t)
                - 1 : (((jarr_element_t) 1 << length) - (jarr_element_t) 1)
                << (startbit % jarr_element_length);
    }
    else
    {
        *element_tbs |= (jarr_element_t) - 1 << (startbit
                % jarr_element_length);
        ++element_tbs;

        while (element_tbs < last_element_tbs)
        {
            *element_tbs = (jarr_element_t) - 1;
            ++element_tbs;
        }

        jarr_element_length_t lme = limiter_bit % jarr_element_length;
        *element_tbs |= lme ? ~((jarr_element_t) - 1 << lme) : (jarr_element_t)
                - 1;
    }
}

void jarr_write_section(struct jarr * const j, struct jarr const* const input,
                        jarr_length_t const startbit)
{
    jarr_element_length_t const lshift = startbit % jarr_element_length;
    jarr_element_t* element = j->arr + jarr_bitoei(startbit);
    jarr_element_t const* input_element = input->arr;
    if (lshift != (jarr_element_length_t) 0)
    {
        jarr_element_length_t const rshift = jarr_element_length
                - lshift;
        jarr_element_t tmp_element = j->arr[jarr_bltoel(input->length_bits
                                                        + startbit)
                - (size_t) 1];

        *element &= ((jarr_element_t) - 1 >> rshift);

        while (input_element < input->last_element)
        {
            *element |= *input_element << lshift;
            ++element;
            *element = *input_element >> rshift;
            ++input_element;
        }

        if (input->bme > rshift)
        {
            *element |= *input_element << lshift;
            ++element;
            jarr_element_t tmp_mask = ((jarr_element_t) 1
                    << (input->bme - rshift)) - (jarr_element_t) 1;
            *element &= ~tmp_mask;
            *element |= (*input_element >> rshift) & tmp_mask;
        }
        else if (input->bme < rshift && input->bme)
        {
            *element |= jarr_get_lev(input) << lshift;
            *element |= tmp_element
                    & ((jarr_element_t) - 1 << (lshift + input->bme));
        }
        else if (input->bme == rshift)
        {
            *element |= *input_element << lshift;
        }
        else // bme == 0
        {
            *element |= *input_element << lshift;
            ++element;
            jarr_element_t tmp_mask = (jarr_element_t) - 1 >> rshift;
            *element &= ~tmp_mask;
            *element |= (*input_element >> rshift) & tmp_mask;
        }
    }
    else
    {
        // this handles the case where rshift = element_size which would
        // cause undefined behaviour when used in the shift operation
        while (input_element < input->last_element)
        {
            *element = *input_element;
            ++element;
            ++input_element;
        }
        *element &= ~input->mask;
        *element |= jarr_get_lev(input);
    }
}

void jarr_read_section(struct jarr const* const j, struct jarr * const output,
                       jarr_length_t const startbit)
{
    jarr_element_length_t const rshift = startbit % jarr_element_length;
    jarr_element_t const *element = j->arr + jarr_bitoei(startbit);
    jarr_element_t *output_element = output->arr;

    if (rshift != (jarr_element_length_t) 0U)
    {
        jarr_element_length_t const lshift = jarr_element_length - rshift;
        while (output_element < output->last_element)
        {
            *output_element = *element >> rshift;
            ++element;
            *output_element |= *element << lshift;
            ++output_element;
        }
        // handle the last element
        *output_element = *element >> rshift;
        ++element;
        if (element != j->limiter_element)
        {
            *output_element |= *element << lshift;
        }
    }
    else
    {
        // this handles the case where rshift = element_size which would
        // cause undefined behaviour when used in the shift operation
        while (output_element != output->limiter_element)
        {
            *output_element = *element;
            ++element;
            ++output_element;
        }
    }
}

void jarr_bw_and(struct jarr * const out, struct jarr const* const in1,
                 struct jarr const* const in2)
{
    jarr_element_t* out_element = out->arr;
    jarr_element_t const* in1_element = in1->arr;
    jarr_element_t const* in2_element = in2->arr;
    while (in1_element != in1->limiter_element)
    {
        *out_element = *in1_element & *in2_element;
        ++out_element;
        ++in1_element;
        ++in2_element;
    }
}

void jarr_bw_or(struct jarr * const out, struct jarr const* const in1,
                struct jarr const* const in2)
{
    jarr_element_t* out_element = out->arr;
    jarr_element_t const* in1_element = in1->arr;
    jarr_element_t const* in2_element = in2->arr;
    while (in1_element != in1->limiter_element)
    {
        *out_element = *in1_element | *in2_element;
        ++out_element;
        ++in1_element;
        ++in2_element;
    }
}

void jarr_bw_xor(struct jarr * const out, struct jarr const* const in1,
                 struct jarr const* const in2)
{
    jarr_element_t* out_element = out->arr;
    jarr_element_t const* in1_element = in1->arr;
    jarr_element_t const* in2_element = in2->arr;
    while (in1_element != in1->limiter_element)
    {
        *out_element = *in1_element ^ *in2_element;
        ++out_element;
        ++in1_element;
        ++in2_element;
    }
}

void jarr_bw_not(struct jarr * const out, struct jarr const* const in)
{
    jarr_element_t* out_element = out->arr;
    jarr_element_t const* in_element = in->arr;
    while (in_element != in->limiter_element)
    {
        *out_element = ~(*in_element);
        ++out_element;
        ++in_element;
    }
}

unsigned char jarr_add(struct jarr * const out, struct jarr const* const in1,
                       struct jarr const* const in2, unsigned char carry)
{
    jarr_element_t* out_element = out->arr;
    jarr_element_t const* in1_element = in1->arr;
    jarr_element_t const* in2_element = in2->arr;

    while (in1_element < in1->last_element)
    {
        *out_element = jarr_add_elements(&carry, *in1_element, *in2_element);
        ++out_element;
        ++in1_element;
        ++in2_element;
    }

    // evaluate the input possibly partial
    if (in1->bme == (jarr_element_length_t) 0U)
    {
        *out_element = jarr_add_elements(&carry, *in1_element, *in2_element);
    }
    else
    {
        *out_element = jarr_add_elements(&carry, jarr_get_lev(in1),
                                         jarr_get_lev(in2));
        carry = jarr_read(out, out->length_bits);
    }
    return carry;
}

void jarr_lshift(struct jarr * const out, struct jarr const* const in,
                 jarr_length_t const shift)
{
    size_t shift_elements = jarr_bitoei(shift);
    jarr_element_length_t lshift_bits = shift % jarr_element_length;
    jarr_element_t* from_element = in->last_element - shift_elements;
    jarr_element_t* to_element = out->last_element;
#if jarr_handle_0_shift != 0
    if (lshift_bits != (jarr_element_length_t) 0)
    {
#endif
        if (lshift_bits != 0)
        {
            jarr_element_length_t rshift_bits = jarr_element_length - lshift_bits;
            while (from_element > in->arr)
            {
                *to_element = *from_element << lshift_bits;
                --from_element;
                *to_element |= *from_element >> rshift_bits;
                --to_element;
            }
        }
        else
        {
            while (from_element > in->arr)
            {
                *to_element = *from_element;
                --from_element;
                --to_element;
            }
        }
        *to_element = *from_element << lshift_bits;
        while (to_element > out->arr)
        {
            --to_element;
            *to_element = (jarr_element_t) 0;
        }
#if jarr_handle_0_shift != 0
    }
    else
    {
        jarr_copy(out, in);
    }
#endif
}

void jarr_rshift(struct jarr * const out, struct jarr const* const in,
                 jarr_length_t const shift)
{
    size_t shift_elements = jarr_bitoei(shift);
    jarr_element_length_t rshift_bits = shift % jarr_element_length;
    jarr_element_t* from_element = in->arr + shift_elements;
    jarr_element_t* to_element = out->arr;
#if jarr_handle_0_shift != 0
    if (rshift_bits != (jarr_element_length_t) 0)
    {
#endif
        if (from_element < in->last_element)
        {
            if (rshift_bits != 0)
            {
                jarr_element_length_t lshift_bits = jarr_element_length
                        - rshift_bits;

                while (from_element < in->last_element - 1)
                {
                    *to_element = *from_element >> rshift_bits;
                    ++from_element;
                    *to_element |= *from_element << lshift_bits;
                    ++to_element;
                }

                *to_element = *from_element >> rshift_bits;
                *to_element |= jarr_get_lev(in) << lshift_bits;
                ++to_element;
            }
            else
            {
                while (from_element < in->last_element)
                {
                    *to_element = *from_element;
                    ++from_element;
                    ++to_element;
                }
            }
        }

        *to_element = jarr_get_lev(in) >> rshift_bits;
        while (to_element < out->last_element)
        {
            ++to_element;
            *to_element = (jarr_element_t) 0;
        }
#if jarr_handle_0_shift != 0
    }
    else
    {
        jarr_copy(out, in);
    }
#endif
}
