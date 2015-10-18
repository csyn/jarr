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

#ifndef JARR_H
#define	JARR_H

#include <limits.h>
#include <stdlib.h>

#define jarr_handle_0_shift 0

typedef size_t jarr_length_t; // must serve as both the length in bits and a
// bit index
typedef unsigned char jarr_element_t; // serves as the type for the bit array,
// must be unsigned
typedef unsigned char jarr_element_length_t; // serves as the type for indexing
// a bit inside an element

const jarr_element_length_t jarr_element_length;

struct jarr
{
    // the element array containing/representing the bit array
    jarr_element_t* arr;
    jarr_element_t* limiter_element;
    jarr_element_t* last_element;
    jarr_length_t length_bits;
    jarr_element_t mask;
    size_t length_elements;
    // length_bits % element_length_bits
    jarr_element_length_t bme;
};

struct jarr jarr_init(jarr_element_t * const _arr,
                      jarr_length_t const _length_bits);
void jarr_set_length(struct jarr * const j, jarr_length_t const _length_bits);
void jarr_clear_section(struct jarr * const j, jarr_length_t const length,
                        jarr_length_t const startbit);
void jarr_set_section(struct jarr * const j, jarr_length_t const length,
                      jarr_length_t const startbit);
void jarr_write_section(struct jarr * const j, struct jarr const* const input,
                        jarr_length_t const startbit);
void jarr_read_section(struct jarr const* const j, struct jarr * const output,
                       jarr_length_t const startbit);
void jarr_bw_and(struct jarr * const out, struct jarr const* const in1,
                 struct jarr const* const in2);
void jarr_bw_or(struct jarr * const out, struct jarr const* const in1,
                struct jarr const* const in2);
void jarr_bw_xor(struct jarr * const out, struct jarr const* const in1,
                 struct jarr const* const in2);
void jarr_bw_not(struct jarr * const out, struct jarr const* const in);
unsigned char jarr_add(struct jarr * const out, struct jarr const* const in1,
                       struct jarr const* const in2, unsigned char carry);
void jarr_lshift(struct jarr * const out, struct jarr const* const in,
                 jarr_length_t const shift);
void jarr_rshift(struct jarr * const out, struct jarr const* const in,
                 jarr_length_t const shift);

// bit index to element index

inline static size_t jarr_bitoei(jarr_length_t const bit_index)
{
    return bit_index / jarr_element_length;
}

// bit length to element length

inline static size_t jarr_bltoel(jarr_length_t const bit_length)
{
    return (bit_length + jarr_element_length - 1)
            / jarr_element_length;
}

// to be called whenever the element array is moved

inline static void jarr_set_limits(struct jarr * const j)
{
    j->limiter_element = j->arr + j->length_elements;
    j->last_element = (j->length_bits != (jarr_length_t)0U) ? j->limiter_element
            - 1 : j->limiter_element;
}

// sets a bit

inline static void jarr_set(struct jarr * const j, jarr_length_t const bit)
{
    *(j->arr + (bit / jarr_element_length)) |= ((jarr_element_t)1 << (bit
            % jarr_element_length));
}

// clears a bit

inline static void jarr_clear(struct jarr * const j, jarr_length_t const bit)
{
    *(j->arr + (bit / jarr_element_length)) &= ~((jarr_element_t)1 << (bit
            % jarr_element_length));
}

// toggles a bit

inline static void jarr_toggle(struct jarr * const j, jarr_length_t const bit)
{
    *(j->arr + (bit / jarr_element_length)) ^= ((jarr_element_t)1 << (bit
            % jarr_element_length));
}

// reads a bit

inline static unsigned char jarr_read(struct jarr const* const j,
                                      jarr_length_t const bit)
{
    return (((*(j->arr + (bit / jarr_element_length))
	      >> (bit % jarr_element_length)) & (jarr_element_t)1)
            != (jarr_element_t)0) ? (unsigned char)1 : (unsigned char)0;
}

// clears the array

inline static void jarr_clear_all(struct jarr * const j)
{
    jarr_element_t* element;
    for (element = j->arr; element != j->limiter_element; ++element)
    {
      *element = (jarr_element_t)0;
    }
}

// sets the array

inline static void jarr_set_all(struct jarr * const j)
{
    jarr_element_t* element;
    for (element = j->arr; element != j->limiter_element; ++element)
    {
        *element = (jarr_element_t)-1;
    }
}

// returns the value of the last element, use this function as any bits more
// significant than the bit at index length_bits - 1 are not guaranteed to
// be 0

inline static jarr_element_t jarr_get_lev(struct jarr const* const j)
{
    return *j->last_element & j->mask;
}

// copies one jarr onto another, they must be of the same size

inline static void jarr_copy(struct jarr * const out,
                             struct jarr const* const in)
{
    jarr_element_t* to_element = in->arr;
    jarr_element_t const* from_element = out->arr;
        
    while (from_element != in->limiter_element)
    {
        *to_element = *from_element;
        ++from_element;
        ++to_element;
    }
}

inline static jarr_element_t jarr_add_elements(unsigned char* const carry,
                                               jarr_element_t in1,
                                               jarr_element_t const in2)
{
    in1 += *carry;
    *carry = (in1 < *carry) ? (unsigned char)1 : (unsigned char)0;
    in1 += in2;
    *carry = ((*carry != (unsigned char)0) || (in1 < in2)) ? (unsigned char)1
            : (unsigned char)0;
    return in1;
}

#endif
