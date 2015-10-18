# jarr #


An array of bits with optimised bitwise operations

jarr is an array of bits, like the C++ class bitset, however it is always
represented in memory as 1 bit -> 1 bit. It also provides optimised bitwise
operations and unaligned section read/write functionality.

The init function takes in an array of type jarr_element_t, this array must
have the required space allocated to cover the length in bits specified as the
other argument to this function. There is a jarr_set_length method provided to
change the length of the bit array after the call to the init function, however
be aware that changing the length of the bit array and not making the required
changes to the allocated element array that contains it may result in reading
from/writing to invalid memory. If it is only required to move the array (with
no change to its length) then the jarr_set_limits function is provided to update
the pointers stored on the struct using the new value of arr.

Most of these functions do not handle jarrs of length 0, if your program might
generate these please check for this externally.

`struct jarr jarr_init(jarr_element_t* const _arr,
                      jarr_length_t const _length_bits);`

Takes an pointer to a chunk of memory and a length in bits and returns a jarr
with these attributes. Does not modify the contents of the memory.

The memory pointed to by *_arr* should be large enough to hold the array of
bits.

`void jarr_set_length(struct jarr* const j, jarr_length_t const _length_bits);`

Modifies the length of a jarr.

`void jarr_clear_section(struct jarr* const j, jarr_length_t const length,
                        jarr_length_t const startbit);`

Clears a section of the jarr of length *length* starting at *starbit*. *length*
should not be equal to 0.

`void jarr_set_section(struct jarr* const j, jarr_length_t const length,
                      jarr_length_t const startbit);`

Sets a section of the jarr of length *length* starting at *starbit*. *length*
should not be equal to 0.

`void jarr_write_section(struct jarr* const j, struct jarr const* const input,
                        jarr_length_t const startbit);`

Overwrites a section of a jarr with annother, starting at *startbit* and
continuing for the length of the input jarr.

`void jarr_read_section(struct jarr const* const j, struct jarr* const output,
                       jarr_length_t const startbit);`

Reads a section of a jarr into annother, starting at *startbit* and
continuing for the length of the input jarr.

`void jarr_bw_and(struct jarr* const out, struct jarr const* const in1,
                 struct jarr const* const in2);`

Ands 2 jarrs, putting the result in another. It is possible to put the result
in one of the input jarrs. All three jarrs must be the same length.

`void jarr_bw_or(struct jarr* const out, struct jarr const* const in1,
                 struct jarr const* const in2);`

Ors 2 jarrs, putting the result in another. It is possible to put the result
in one of the input jarrs. All three jarrs must be the same length.

`void jarr_bw_xor(struct jarr* const out, struct jarr const* const in1,
                 struct jarr const* const in2);`

Xors 2 jarrs, putting the result in another. It is possible to put the result
in one of the input jarrs. All three jarrs must be the same length.

`void jarr_bw_not(struct jarr* const out, struct jarr const* const in);`

Inverts all the bits of a jarr, putting the result in another jarr. It is
possible to put the result back into the input jarr. Both jarrs must be the
same length.

`unsigned char jarr_add(struct jarr* const out, struct jarr const* const in1,
                       struct jarr const* const in2, unsigned char carry);`

Adds 2 jarrs, putting the result in another. It is possible to put the result
in one of the input jarrs. All three jarrs must be the same length.

`unsigned char jarr_sub(struct jarr* const out, struct jarr const* const in1,
                       struct jarr const* const in2, unsigned char borrow);`

Subtracts 2 jarrs, putting the result in another. It is possible to put the
result in one of the input jarrs. All three jarrs must be the same length.

`void jarr_lshift(struct jarr* const out, struct jarr const* const in,
                 jarr_length_t const shift);`

Shifts the bits of a jarr left by *shift* bits, putting the result in another
jarr. It is possible to put the result back into the input jarr. Both jarrs
must be the same length.

`void jarr_rshift(struct jarr* const out, struct jarr const* const in,
                 jarr_length_t const shift);`

Shifts the bits of a jarr right by *shift* bits, putting the result in another
jarr. It is possible to put the result back into the input jarr. Both jarrs
must be the same length.

`inline static size_t jarr_bitoei(jarr_length_t const bit_index);`

Converts a bit index to an element index.

`inline static size_t jarr_bltoel(jarr_length_t const bit_length)`

converts a bit length to an element length.

`inline static void jarr_set_limits(struct jarr * const j)`

Updates the pointers to the limits of the array, call this whenever the array
is moved in memory.

`inline static void jarr_set(struct jarr * const j, jarr_length_t const bit)`

Sets the bit at index *bit*.

`inline static void jarr_clear(struct jarr * const j, jarr_length_t const bit)`

Clears the bit at index *bit*.

`inline static void jarr_toggle(struct jarr * const j, jarr_length_t const bit)`

Toggles the bit at index *bit*.

`inline static unsigned char jarr_read(struct jarr const* const j,
                                      jarr_length_t const bit)`

Reads the bit at index *bit*.

`inline static void jarr_clear_all(struct jarr* const j)`

Clears all the bits in the array.

`inline static void jarr_set_all(struct jarr* const j)`

Sets all the bits in the array.

`inline static jarr_element_t jarr_get_lev(struct jarr const* const j)`

Returns the value of the last element, use this function as any bits more
significant than the bit at index *length_bits - 1* are not guaranteed to be 0.
