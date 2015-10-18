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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BIT_MANIPULATIONS_LENGTH 	8192
#define SECTION_CLEAR_LENGTH 		8192
#define SECTION_CLEAR_REPS 		8192
#define SECTION_SET_LENGTH 		8192
#define SECTION_SET_REPS 		8192
#define SECTION_READ_LENGTH 		8192
#define SECTION_READ_REPS 		8192
#define SECTION_WRITE_LENGTH 		8192
#define SECTION_WRITE_REPS 		8192
#define AND_LENGTH 			8192
#define AND_REPS 			8192
#define OR_LENGTH 			8192
#define OR_REPS				8192
#define XOR_LENGTH 			8192
#define XOR_REPS 			8192
#define NOT_LENGTH 			8192
#define NOT_REPS 			8192
#define ADD_LENGTH 			8192
#define ADD_REPS 			8192
#define SUB_LENGTH 			8192
#define SUB_REPS 			8192
#define LSHIFT_LENGTH 			8192
#define LSHIFT_REPS 			8192
#define RSHIFT_LENGTH 			8192
#define RSHIFT_REPS 			8192

#define SEED (time(NULL))

static clock_t suite_start_time;
static clock_t start_time;

unsigned int rand_limited(unsigned int limit)
{
    unsigned int r_num = 0;
    if (limit)
    {
        if (limit > RAND_MAX)
        {
            limit = RAND_MAX;
        }
        static unsigned char rand_initialised = 0;
        if (!rand_initialised)
        {
            srand(SEED);
            rand_initialised = 1;
        }

        do
        {
            r_num = (unsigned int) (rand() / (unsigned int) (RAND_MAX / limit));
        }
        while (r_num >= limit);
    }
    return r_num;
}

unsigned int rand_limited_nz(unsigned int limit)
{
    unsigned int r_num;
    do
    {
        r_num = rand_limited(limit);
    }
    while (r_num == 0);
    return r_num;
}

unsigned char rand_char(void)
{
    return (unsigned char) (rand_limited(UCHAR_MAX + 1));
}

void rand_array(struct jarr * const ja)
{
    size_t i;
    for (i = 0; i < ja->length_elements; ++i)
    {
        ja->arr[i] = 0;
        size_t j;
        for (j = 0; j < sizeof (jarr_element_t); ++j)
        {
            ja->arr[i] |= (((jarr_element_t) rand_char()) << (CHAR_BIT * j));
        }
    }
}

void copy_array(struct jarr * const dest, struct jarr const* const source)
{
    size_t i;
    for (i = 0; i < dest->length_elements; ++i)
    {
        dest->arr[i] = source->arr[i];
    }
}

int compare_arrays(struct jarr const* const a, struct jarr const* const b)
{
    if (a->length_bits != b->length_bits)
    {
        return 0;
    }
    jarr_length_t i;
    for (i = 0; i < a->length_bits; ++i)
    {
        if (jarr_read(a, i) != jarr_read(b, i))
        {
            return 0;
        }
    }
    return 1;
}

// this prints the array backwards, it is a little easier to compare arrays of
// differing sizes in a terminal window like this

void print_array_reversed(struct jarr const* const a)
{
    jarr_length_t i = 0;
    while (i < a->length_bits)
    {
        if (jarr_read(a, i))
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
        ++i;
    }
    printf("\n");
}

void fail(char const* const test, char const* const message)
{
    printf("%%TEST_FAILED%% time=%f testname=jarr ( %s ) message=%s\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC, test, message);
    exit(1);
}

void jassert(unsigned char const ex, char const* const test,
             char const* const message)
{
    if (!ex)
    {
        fail(test, message);
    }
}

void jarr_test_bit_maipulations(void)
{
    char test_str[] = "bit manipulations";
    printf("stest testing %s\n", test_str);

    jarr_element_t arr[BIT_MANIPULATIONS_LENGTH / jarr_element_length];
    struct jarr l = jarr_init(arr, (BIT_MANIPULATIONS_LENGTH
                              / jarr_element_length) *
                              (sizeof (jarr_element_t) * CHAR_BIT));
    unsigned int i;
    for (i = 0; i < (BIT_MANIPULATIONS_LENGTH
            / jarr_element_length) * (sizeof (jarr_element_t) * CHAR_BIT); i++)
    {
        jarr_clear(&l, i);
        jassert(!jarr_read(&l, i), test_str, "");
        jarr_set(&l, i);
        jassert(jarr_read(&l, i), test_str, "");
    }
}

void jarr_test_section_clear(void)
{
    char test_str[] = "section clear";
    printf("stest testing %s\n", test_str);
    unsigned int i;
    for (i = 0; i < SECTION_CLEAR_REPS; ++i)
    {
        // initialise an array to all 1s
        jarr_element_t arr[SECTION_CLEAR_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test = jarr_init(arr, rand_limited_nz(SECTION_CLEAR_LENGTH));
        jarr_set_all(&test);

        // take 2 random points
        jarr_length_t clear_p1 = rand_limited(test.length_bits + 1);
        jarr_length_t clear_p2;
        do
        {
            clear_p2 = rand_limited(test.length_bits + 1);
        }
        while (clear_p1 == clear_p2);

        // find the lower/higher one
        jarr_length_t clear_from;
        jarr_length_t clear_to;

        if (clear_p1 < clear_p2)
        {
            clear_from = clear_p1;
            clear_to = clear_p2;
        }
        else
        {
            clear_from = clear_p2;
            clear_to = clear_p1;
        }

        jarr_clear_section(&test, clear_to - clear_from, clear_from);

        unsigned int t;
        for (t = 0; t < test.length_bits; ++t)
        {
            if ((t < clear_from) || (t >= clear_to))
            {
                jassert((jarr_read(&test, t) == 1), test_str, "");
            }
            else
            {
                jassert((jarr_read(&test, t) == 0), test_str, "");
            }
        }
    }
}

void jarr_test_section_set(void)
{
    char test_str[] = "section set";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < SECTION_SET_REPS; ++i)
    {
        // initialise an array to all 0s
        jarr_element_t arr[SECTION_SET_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test = jarr_init(arr, rand_limited_nz(SECTION_SET_LENGTH));
        jarr_clear_all(&test);

        // take 2 random points
        jarr_length_t set_p1 = rand_limited(test.length_bits + 1);
        jarr_length_t set_p2;
        do
        {
            set_p2 = rand_limited(test.length_bits + 1);
        }
        while (set_p1 == set_p2);

        // find the lower/higher one
        jarr_length_t set_from;
        jarr_length_t set_to;

        if (set_p1 < set_p2)
        {
            set_from = set_p1;
            set_to = set_p2;
        }
        else
        {
            set_from = set_p2;
            set_to = set_p1;
        }

        jarr_set_section(&test, set_to - set_from, set_from);

        unsigned int t;
        for (t = 0; t < test.length_bits; ++t)
        {
            if ((t < set_from) || (t >= set_to))
            {
                jassert((jarr_read(&test, t) == 0), test_str, "");
            }
            else
            {
                jassert((jarr_read(&test, t) == 1), test_str, "");
            }
        }
    }
}

void jarr_test_section_read(void)
{
    char test_str[] = "section read";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < SECTION_READ_REPS; ++i)
    {
        // initialise a random array of random length
        jarr_element_t arr[SECTION_READ_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test = jarr_init(arr, rand_limited_nz(SECTION_READ_LENGTH));
        rand_array(&test);

        // take 2 random points
        jarr_length_t read_p1 = rand_limited(test.length_bits + 1);
        jarr_length_t read_p2;
        do
        {
            read_p2 = rand_limited(test.length_bits + 1);
        }
        while (read_p1 == read_p2);

        // find the lower/higher one
        jarr_length_t read_from;
        jarr_length_t read_to;

        if (read_p1 < read_p2)
        {
            read_from = read_p1;
            read_to = read_p2;
        }
        else
        {
            read_from = read_p2;
            read_to = read_p1;
        }

        // declare an array to read into
        jarr_element_t arr_section[SECTION_READ_LENGTH
                + (sizeof (jarr_element_t) * CHAR_BIT) - 1
                / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr section = jarr_init(arr_section, read_to - read_from);

        // perform the read
        jarr_read_section(&test, &section, read_from);

        jassert((section.length_bits == read_to - read_from), test_str,
                "incorrect length");

        unsigned int t;
        for (t = 0; t < section.length_bits; ++t)
        {
            jassert((jarr_read(&section, t) == jarr_read(&test, t + read_from)),
                    test_str, "");
        }
    }
}

void jarr_test_section_write(void)
{
    char test_str[] = "section write";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < SECTION_WRITE_REPS; ++i)
    {
        // initialise a random array of random length
        jarr_element_t arr[SECTION_WRITE_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test = jarr_init(arr, rand_limited_nz(SECTION_WRITE_LENGTH));
        rand_array(&test);

        // copy it
        jarr_element_t copy_of_arr[SECTION_WRITE_LENGTH
                + (sizeof (jarr_element_t) * CHAR_BIT) - 1
                / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr copy_of_test = jarr_init(copy_of_arr, test.length_bits);
        copy_array(&copy_of_test, &test);

        // take 2 random points
        jarr_length_t write_p1 = rand_limited(test.length_bits + 1);
        jarr_length_t write_p2;
        do
        {
            write_p2 = rand_limited(test.length_bits + 1);
        }
        while (write_p1 == write_p2);

        // find the lower/higher one
        jarr_length_t write_from;
        jarr_length_t write_to;

        if (write_p1 < write_p2)
        {
            write_from = write_p1;
            write_to = write_p2;
        }
        else
        {
            write_from = write_p2;
            write_to = write_p1;
        }

        // initialise an array to write into
        jarr_element_t arr_section[SECTION_WRITE_LENGTH
                + (sizeof (jarr_element_t) * CHAR_BIT) - 1
                / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr section = jarr_init(arr_section, write_to - write_from);
        rand_array(&section);

        // perform the write
        jarr_write_section(&test, &section, write_from);

        jassert((test.length_bits == copy_of_test.length_bits), test_str,
                "incorrect length");

        unsigned int t;
        for (t = 0; t < test.length_bits; ++t)
        {
            if ((t >= write_to) || (t < write_from))
            {
                jassert((jarr_read(&test, t) == jarr_read(&copy_of_test, t)),
                        test_str, "2");
            }
            else
            {
                jassert((jarr_read(&test, t) == jarr_read(&section,
                                                          t - write_from)),
                        test_str, "1");
            }
        }
    }
}

void jarr_test_and(void)
{
    char test_str[] = "and";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < AND_REPS; ++i)
    {
        size_t length = rand_limited_nz(AND_LENGTH);
        jarr_element_t arr[5][AND_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test[5] = {
            jarr_init(arr[0], length),
            jarr_init(arr[1], length),
            jarr_init(arr[2], length),
            jarr_init(arr[3], length),
            jarr_init(arr[4], length),
        };

        rand_array(&test[0]);
        rand_array(&test[1]);
        rand_array(&test[2]);

        // decide where to put the result
        struct jarr * args[3] = {
            &test[rand_limited(3)],
            &test[rand_limited(3)],
            &test[rand_limited(3)],
        };
            
        copy_array(&test[3], args[1]);
        copy_array(&test[4], args[2]);

        // perform the and
        jarr_bw_and(args[0], args[1], args[2]);

        // perform a naive and
        jarr_length_t index = 0;
        while (index < length)
        {
            if (!jarr_read(&test[4], index))
            {
                jarr_clear(&test[3], index);
            }
            ++index;
        }

        unsigned int t;
        for (t = 0; t < length; ++t)
        {
            jassert((jarr_read(args[0], t) == jarr_read(&test[3], t)),
                    test_str, "");
        }
    }
}

void jarr_test_or(void)
{
    char test_str[] = "or";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < OR_REPS; ++i)
    {
        size_t length = rand_limited_nz(OR_LENGTH);
        jarr_element_t arr[5][OR_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test[5] = {
            jarr_init(arr[0], length),
            jarr_init(arr[1], length),
            jarr_init(arr[2], length),
            jarr_init(arr[3], length),
            jarr_init(arr[4], length),
        };

        rand_array(&test[0]);
        rand_array(&test[1]);
        rand_array(&test[2]);

        // decide where to put the result
        struct jarr * args[3] = {
            &test[rand_limited(3)],
            &test[rand_limited(3)],
            &test[rand_limited(3)],
        };
        copy_array(&test[3], args[1]);
        copy_array(&test[4], args[2]);

        // perform the or
        jarr_bw_or(args[0], args[1], args[2]);

        // perform a naive or
        jarr_length_t index = 0;
        while (index < length)
        {
            if (jarr_read(&test[4], index))
            {
                jarr_set(&test[3], index);
            }
            ++index;
        }

        unsigned int t;
        for (t = 0; t < length; ++t)
        {
            jassert((jarr_read(args[0], t) == jarr_read(&test[3], t)),
                    test_str, "");
        }
    }
}

void jarr_test_xor(void)
{
    char test_str[] = "xor";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < XOR_REPS; ++i)
    {
        size_t length = rand_limited_nz(XOR_LENGTH);
        jarr_element_t arr[5][XOR_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test[5] = {
            jarr_init(arr[0], length),
            jarr_init(arr[1], length),
            jarr_init(arr[2], length),
            jarr_init(arr[3], length),
            jarr_init(arr[4], length),
        };

        rand_array(&test[0]);
        rand_array(&test[1]);
        rand_array(&test[2]);

        // decide where to put the result
        struct jarr * args[3] = {
            &test[rand_limited(3)],
            &test[rand_limited(3)],
            &test[rand_limited(3)],
        };

        copy_array(&test[3], args[1]);
        copy_array(&test[4], args[2]);

        // perform the xor
        jarr_bw_xor(args[0], args[1], args[2]);

        // perform a naive xor
        jarr_length_t index = 0;
        while (index < length)
        {
            if (jarr_read(&test[4], index))
            {
                jarr_toggle(&test[3], index);
            }
            ++index;
        }

        unsigned int t;
        for (t = 0; t < length; ++t)
        {
            jassert((jarr_read(args[0], t) == jarr_read(&test[3], t)),
                    test_str, "");
        }
    }
}

void jarr_test_not(void)
{
    char test_str[] = "not";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < NOT_REPS; ++i)
    {
        size_t length = rand_limited_nz(NOT_LENGTH);
        jarr_element_t arr[3][NOT_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test[3] = {
            jarr_init(arr[0], length),
            jarr_init(arr[1], length),
            jarr_init(arr[2], length),
        };

        rand_array(&test[0]);
        rand_array(&test[1]);

        copy_array(&test[2], &test[0]);

        // decide where to put the result
        struct jarr* result = &test[rand_limited(2)];

        // perform the not
        jarr_bw_not(result, &test[0]);

        unsigned int t;
        for (t = 0; t < length; ++t)
        {
            jassert((jarr_read(result, t) != jarr_read(&test[2], t)),
                    test_str, "");
        }
    }
}

void jarr_test_add(void)
{
    char test_str[] = "add";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < ADD_REPS; ++i)
    {
        size_t length = rand_limited_nz(ADD_LENGTH);
        jarr_element_t arr[5][ADD_LENGTH + (sizeof (jarr_element_t) * CHAR_BIT)
                - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test[5] = {
            jarr_init(arr[0], length),
            jarr_init(arr[1], length),
            jarr_init(arr[2], length),
            jarr_init(arr[3], length),
            jarr_init(arr[4], length),
        };

        rand_array(&test[0]);
        rand_array(&test[1]);
        rand_array(&test[2]);

        // decide where to put the result
        struct jarr * args[3] = {
            &test[rand_limited(3)],
            &test[rand_limited(3)],
            &test[rand_limited(3)],
        };

        copy_array(&test[3], args[1]);
        copy_array(&test[4], args[2]);

        unsigned char carry = rand_limited(2);
        unsigned char carry_copy = carry;

        // perform the add
        carry = jarr_add(args[0], args[1], args[2], carry);

        // perform a naive add
        jarr_length_t index = 0;
        while (index < length)
        {
            switch (jarr_read(&test[3], index)
                    + jarr_read(&test[4], index) + carry_copy)
            {
            case 0:
                jarr_clear(&test[3], index);
                carry_copy = 0;
                break;
            case 1:
                jarr_set(&test[3], index);
                carry_copy = 0;
                break;
            case 2:
                jarr_clear(&test[3], index);
                carry_copy = 1;
                break;
            case 3:
                jarr_set(&test[3], index);
                carry_copy = 1;
                break;
            }
            ++index;
        }

        while ((index < length) && carry_copy)
        {
            if (!jarr_read(&test[3], index))
            {
                jarr_set(&test[3], index);
                carry_copy = 0;
            }
            else
            {
                jarr_clear(&test[3], index);
            }
            ++index;
        }

        jassert((carry == carry_copy), test_str, "incorrect carry");

        unsigned int t;
        for (t = 0; t < length; ++t)
        {
            jassert((jarr_read(args[0], t) == jarr_read(&test[3], t)),
                    test_str, "");
        }
    }
}

void jarr_test_lshift(void)
{
    char test_str[] = "left shift";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < LSHIFT_REPS; ++i)
    {
        size_t length;
        do
        {
            length = rand_limited_nz(LSHIFT_LENGTH);
        }
        while (length == 1);
        jarr_element_t arr[3][LSHIFT_LENGTH + (sizeof (jarr_element_t)
                * CHAR_BIT) - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test[3] = {
            jarr_init(arr[0], length),
            jarr_init(arr[1], length),
            jarr_init(arr[2], length),
        };

        rand_array(&test[0]);
        rand_array(&test[1]);

        copy_array(&test[2], &test[0]);

        // decide where to put the result
        struct jarr* result = &test[rand_limited(2)];

        // generate the shift
        jarr_length_t shift = rand_limited_nz(length);

        // perform the lshift
        jarr_lshift(result, &test[0], shift);

        unsigned int t;
        for (t = 0; t < length; ++t)
        {
            if (t < shift)
            {
                jassert((jarr_read(result, t) == 0), test_str, "");
            }
            else
            {
                jassert((jarr_read(result, t) == jarr_read(&test[2],
                                                           t - shift)),
                        test_str, "");
            }
        }
    }
}

void jarr_test_rshift(void)
{
    char test_str[] = "right shift";
    printf("stest testing %s\n", test_str);

    unsigned int i;
    for (i = 0; i < RSHIFT_REPS; ++i)
    {
        size_t length;
        do
        {
            length = rand_limited_nz(RSHIFT_LENGTH);
        }
        while (length == 1);
        jarr_element_t arr[3][RSHIFT_LENGTH + (sizeof (jarr_element_t)
                * CHAR_BIT) - 1 / (sizeof (jarr_element_t) * CHAR_BIT)];
        struct jarr test[3] = {
            jarr_init(arr[0], length),
            jarr_init(arr[1], length),
            jarr_init(arr[2], length),
        };

        rand_array(&test[0]);
        rand_array(&test[1]);

        copy_array(&test[2], &test[0]);

        // decide where to put the result
        struct jarr* result = &test[rand_limited(2)];

        // generate the shift
        jarr_length_t shift = rand_limited_nz(length);

        // perform the rshift
        jarr_rshift(result, &test[0], shift);

        unsigned int t;
        for (t = 0; t < length; ++t)
        {
            if (t < (length - shift))
            {
                jassert((jarr_read(result, t) == jarr_read(&test[2],
                                                           t + shift)),
                        test_str, "");
            }
            else
            {
                jassert((jarr_read(result, t) == 0), test_str, "");
            }
        }
    }
}

int main(int argc, char** argv)
{
    printf("%%SUITE_STARTING%% jarr_test\n");
    printf("stest testing jarr\n");
    printf("%%SUITE_STARTED%%\n");

    suite_start_time = clock();

    printf("%%TEST_STARTED%% test1 (jarr_test)\n");
    start_time = clock();
    jarr_test_bit_maipulations();
    printf("%%TEST_FINISHED%% time=%fs test1 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test3 (jarr_test)\n");
    start_time = clock();
    jarr_test_section_clear();
    printf("%%TEST_FINISHED%% time=%fs test3 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test4 (jarr_test)\n");
    start_time = clock();
    jarr_test_section_set();
    printf("%%TEST_FINISHED%% time=%fs test4 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test5 (jarr_test)\n");
    start_time = clock();
    jarr_test_section_read();
    printf("%%TEST_FINISHED%% time=%fs test5 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test6 (jarr_test)\n");
    start_time = clock();
    jarr_test_section_write();
    printf("%%TEST_FINISHED%% time=%fs test6 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test7 (jarr_test)\n");
    start_time = clock();
    jarr_test_and();
    printf("%%TEST_FINISHED%% time=%fs test7 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test8 (jarr_test)\n");
    start_time = clock();
    jarr_test_or();
    printf("%%TEST_FINISHED%% time=%fs test8 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test9 (jarr_test)\n");
    start_time = clock();
    jarr_test_xor();
    printf("%%TEST_FINISHED%% time=%fs test9 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test10 (jarr_test)\n");
    start_time = clock();
    jarr_test_not();
    printf("%%TEST_FINISHED%% time=%fs test10 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test11 (jarr_test)\n");
    start_time = clock();
    jarr_test_add();
    printf("%%TEST_FINISHED%% time=%fs test11 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);


    printf("%%TEST_STARTED%% test12 (jarr_test)\n");
    start_time = clock();
    jarr_test_lshift();
    printf("%%TEST_FINISHED%% time=%fs test12 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%TEST_STARTED%% test13 (jarr_test)\n");
    start_time = clock();
    jarr_test_rshift();
    printf("%%TEST_FINISHED%% time=%fs test13 (jarr_test)\n",
           (double) (clock() - start_time) / CLOCKS_PER_SEC);

    printf("%%SUITE_FINISHED%% time=%fs\n", (double) (clock()
           - suite_start_time) / CLOCKS_PER_SEC);

    return 0;
}
