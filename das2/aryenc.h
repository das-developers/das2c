/* Copyright (C) 2024  Chris Piker <chris-piker@uiowa.edu>
 *
 * This file is part of das2C, the Core Das2 C Library.
 * 
 * das2C is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * das2C is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 2.1 along with das2C; if not, see <http://www.gnu.org/licenses/>. 
 */

/** @file aryenc.h Encoding/Decoding arrays to and from buffers */

#ifndef _das_aryenc_h_
#define _das_aryenc_h_

#include <stdint.h>

#include <das2/value.h>
#include <das2/array.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Reading and writing array data to buffers */
typedef struct das_aryenc {

	uint32_t uProc; /* Internal processing flags setup on the call to _init */

   int nAryValSz;  /* The size of each array value in internal buffer */

   int nBufValSz;  /* Width of a single value in the external buffer, ignored for vtText */

	das_val_type vtBuf; /* The value type in the external buffer */

   das_val_type vtAry; /* Cached here for rapid access */

	DasAry* pAry;  /* The array for which values are encoded/decoded */

   ubyte cSep;   /* Split strings on this byte value, in addition to null */

   uint32_t uMaxString; /* If we are storing fixed strings, this is set */

   das_units timeUnits; /* If ascii times are to be stored as an integral type
                           this is needed */

} DasAryEnc;

/** Has the memory for this encoder been initialized? */
DAS_API bool DasAryEnc_isValid(const DasAryEnc* pThis);

/** Initialize a serial buffer decoder/encoder
 * 
 * @param pThis A pointer to the memory area to initialize
 * 
 * @param pAry A pointer to the array which either receive or supply values.
 *        Values will be encoded so that they match the value type of the
 *        array. 
 *        @warning If the basic parameters of this array, such as it's value
 *        type or rank are changed, then DasAryEnc_init() must be re-called.
 * 
 * @param sSemantic The purpose of the data to store in the buffer, should
 *        be one of 'bool','int','real','datatime','string'.  This determines
 *        the kinds of calculations that may be performed on the data once
 *        in memory. 
 * 
 * @param sEncType The basic encoding type of data in the buffer, one of:
 *        - byte   : 8-bit signed integer
 *        - ubyte  : 8-bit un-signed integer
 *        - BEint  : A signed integer 2+ bytes long, most significant byte first
 *        - BEuint : An un-signed integer 2+ bytes long MSB first
 *        - LEint  : Little-endian version of BEint
 *        - LEuint : Little-endian version of BEuint
 *        - BEreal : An IEEE-754 floating point value, MSB first
 *        - LEreal : An IEEE-754 floating point value, LSB first
 *        - utf8   : A string of text bytes
 * 
 * @param nSzEach the number of bytes in an item.  For variable length
 *        items (which is common with the utf8 encoding) use -1.
 * 
 * @param cSep A single byte used to mark the end of a byte sequence for
 *        string data.  By default any space character marks the end of
 *        a string.  Use 0 to ignore.
 * 
 * @param epoch If time data needs to be converted from UTC strings an epoch
 *        will be needed.  Otherwise this field can be NULL
 * 
 * @returns DAS_OKAY if an decoder/encoder for can be created for the given
 *        arguments, an error code otherwise.
 * 
 * @note For 'string' semantic data where the last index in the array is 
 *       ragged DasAry_markEnd() will be called after each string is read.
 *       Otherwise, no string larger then the last index will be written
 *       and zeros will be appended to fill out the last index when reading
 *       data.
 */
DAS_API DasErrCode DasAryEnc_init(
   DasAryEnc* pThis, DasAry* pAry, const char* sSemantic, const char* sEncType,
   uint16_t uSzEach, ubyte cSep, das_units epoch
);

/** Read values from a simple buffer into an array
 * 
 * @param pThis An encoder
 
 * @param pBuf A pointer to the memory to read
 
 * @param nBufLen The length of the buffer parse into the array.  Note that
 *        even for string data the function trys to read nLen bytes.  Null
 *        values do not terminate parsing but do indicate the end of an 
 *        individual utf-8 encoded item.
 *
 * @param nExpect The number of values to try and read.  Reading less then 
 *        this does not trigger an error return.  If the caller considers
 *        reading less values then expect to be an error, compare *pRead 
 *        with the number provided for nExpect.  If any number of values 
 *        can be read, set this to -1.
 * 
 * @param pRead A pointer to a location to hold the number of values read
 *        or NULL.  If NULL, the number of values read will not be returned
 * 
 * @returns the number of unread bytes or a negative ERR code if a data conversion
 *        error occured.
 * */
DAS_API int DasAryEnc_read(
   DasAryEnc* pThis, const ubyte* pBuf, size_t nBufLen, int nExpect, int* pRead
);

/** Release the reference count on the array given to this encoder/decoder */
DAS_API void DasAryEnc_deInit(DasAryEnc* pThis);

#ifdef __cplusplus
}
#endif


#endif /* _das_aryenc_h_ */
