/* Copyright (C) 2017-2018 Chris Piker <chris-piker@uiowa.edu>
 *
 * This file is part of libdas2, the Core Das2 C Library.
 *
 * Libdas2 is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * Libdas2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 2.1 along with libdas2; if not, see <http://www.gnu.org/licenses/>.
 */

/** @file variable.h correlated data and coordinate variables */

#ifndef _das_variable_h_
#define _das_variable_h_

#include <das2/array.h>
#include <das2/datum.h>

#include "units.h"

#ifdef __cplusplus
extern "C" {
#endif
	
/** @addtogroup datasets
 * @{
 */

/* Current max length of a vector (internal index) can be changed */
#define D2V_MAX_VEC_LEN 4  

enum var_type { 
	D2V_CONST, D2V_SEQUENCE, D2V_ARRAY, D2V_UNARY_OP, D2V_BINARY_OP
};

#ifdef _D
#error macro _D already defined, pick something else
#else
#define _D DEGEN
#endif 


/** Dataset index to array index mapping macros.
 * 
 * The scalar macros map the locations of atomic items to a an external
 * index allowing some indexes to be degenerate.
 * 
 * The vector macros do the same but they also assume check the array to
 * make sure that there is one extra index right after all the ones 
 * that are mentioned. 
 */
#define SCALAR_0  0, (NULL), 0
#define SCALAR_1(I)  1, (int8_t[DASIDX_MAX]){I,_D,_D,_D,_D,_D,_D,_D}, 0
#define SCALAR_2(I,J)  2, (int8_t[DASIDX_MAX]){I,J,_D,_D,_D,_D,_D,_D}, 0
#define SCALAR_3(I,J,K)  3, (int8_t[DASIDX_MAX]){I,J,K,_D,_D,_D,_D,_D}, 0
#define SCALAR_4(I,J,K,L)  4, (int8_t[DASIDX_MAX]){I,J,K,L,_D,_D,_D,_D}, 0
#define SCALAR_5(I,J,K,L,M)  5, (int8_t[DASIDX_MAX]){I,J,K,L,M,_D,_D,_D}, 0
#define SCALAR_6(I,J,K,L,M,N)  6, (int8_t[DASIDX_MAX]){I,J,K,L,M,N,_D,_D}, 0
#define SCALAR_7(I,J,K,L,M,N,O)  7, (int8_t[DASIDX_MAX]){I,J,K,L,M,N,O,_D}, 0
#define SCALAR_8(I,J,K,L,M,N,O,P)  8, (int8_t[DASIDX_MAX]){I,J,K,L,M,N,O,P}, 0

#define VEC_0   0, (int8_t[DASIDX_MAX]){_D,_D,_D,_D,_D,_D,_D,_D}, 1
#define VEC_1(I)  1, (int8_t[DASIDX_MAX]){I,_D,_D,_D,_D,_D,_D,_D}, 1
#define VEC_2(I,J)  2, (int8_t[DASIDX_MAX]){I,J,_D,_D,_D,_D,_D,_D}, 1
#define VEC_3(I,J,K)  3, (int8_t[DASIDX_MAX]){I,J,K,_D,_D,_D,_D,_D}, 1
#define VEC_4(I,J,K,L)  4, (int8_t[DASIDX_MAX]){I,J,K,L,_D,_D,_D,_D}, 1
#define VEC_5(I,J,K,L,M)  5, (int8_t[DASIDX_MAX]){I,J,K,L,M,_D,_D,_D}, 1
#define VEC_6(I,J,K,L,M,N)  6, (int8_t[DASIDX_MAX]){I,J,K,L,M,N,_D,_D}, 1
#define VEC_7(I,J,K,L,M,N,O)  7, (int8_t[DASIDX_MAX]){I,J,K,L,M,N,O,_D}, 1


/* Internal function for merging variable, and dimension shapes.  Different
 * rules apply for arrays to variable shape merges.
 * 
 * Combinding index rules:
 * 
 *    '*' + '-'    = '*'
 *    '*' + Number = Number      ('*' means undefined length, represented by)
 *    '-' + Number = Number      ('-' means no dependency,    negative nums )
 *    Big Number + Small Number = Small Number
 * 
 */
DAS_API void das_varindex_merge(int nRank, ptrdiff_t* pDest, ptrdiff_t* pSrc);

/* Internal function for merging length in a particular dimension. */
DAS_API ptrdiff_t das_varlength_merge(ptrdiff_t nLeft, ptrdiff_t nRight);

#define D2V_EXP_UNITS 0x02
#define D2V_EXP_RANGE 0x04
#define D2V_EXP_SUBEX 0x08
#define D2V_EXP_INTR  0x10
#define D2V_EXP_TYPE  0x20

/** Set index printing direction.
 *
 * Switch printing of variable index order in _toStr() calls.  Does not affect
 * the internal layout of the data.  The default print order is "Fastest
 * index last."
 * 
 * WARNING: This function is NOT thread safe.
 */
DAS_API void das_varindex_prndir(bool bFastLast);

/** Das2 fexible variables 
 * 
 * Like arrays, das2 variables are objects which produce values given a set
 * of indicies.  Unlike arrays the indicies provided need not correspond to
 * the actual layout of the data on the disk.  
 * 
 * To illustrate the difference between arrays and das2 variables consider the
 * following arrays containing a typical set of values for time, frequency, 
 * amplitude spectrogram.
 *
 * @code
 *  double time[1440];       // Time values at which frequency sweeps were triggered
 *  double frequency[42];    // Center frequencies for each energy channel
 *  double energy[1440][42]; // Energies measured for each frequency band in each sweep
 * @endcode
 *
 * So to get a correlated triplet of two coordinate values at index (14,34) 
 * would look like
 *
 * @code
 *  x = time[14] 
 *  y = frequency[34]
 *  z = energy[14][34]
 * @endcode
 *
 * In contrast if the time, frequency and energy data were stored in a 
 * correlated set of das2 variables accessing the values would look like 
 * this:
 *
 * @code
 * DasVar* vTime = new_DasVarArry(time, MAP_2(0,DASIDX_UNUSED));
 * DasVar* vFrequency = new_DasVarArray(frequency, MAP_2(DASIDX_UNUSED, 0));
 * DasVar* vEnergy = new_DasVarArray(energy, MAP_2(0, 1));
 * 
 * // A correlated set is now:
 *  x = fTime([14,34]);
 *  y = fFreqency([14,34]);
 *  z = fEnergy([14,34]);
 * @endcode
 *
 * In addition to wrapping arrays, das2 variables may produce data via 
 * calculations involving other variables.  For example:
 *
 * @code
 * 
 * // The premise here is that we are reading in an array of Mars Express 
 * // spacecraft altitudes.  We know the delay timing for the MARSIS instrument
 * // and need a way to get the altitude at which the radar return signal was
 * // generated.  We'll assume that there is nothing to bounce off of near
 * // the spacecraft except the planet below.  So the altitude at which the
 * // signal was generated is just the craft altitude minus a range which is
 * // calculated from the return time.
 * 
 * // Create a dynamic array of altitudes.  We don't know how big this will
 * // get or how large it is any any moment.
 * Array* pAltAry = new_DasAry("altitude", etDouble, 0, NULL, RANK_1(0));
 * 
 * // Create a fixed array of delay times.  There are 80 delay time measurements
 * // of the MARSIS radar for each altitude.
 * Array* pDelayAry = new_DasAry("delay", etDouble, 0, NULL, RANK_1(80));
 * // Fill in array values here
 * 
 * // We want an automatic variable that will give us the altitude of the 
 * // return signals no mater how many altitude values we get.  To do this
 * // we need to do the calculation:
 * //
 * //  Signal_Altitude = Craft_Altitude - (Speed_of_Light/2) * Delay_Time
 * //
 * // on any altitude we happen to have.  Since we have a variable number of
 * // altitudes but a fixed number of delay times it's natural to store these
 * // values in a 2-index array, namely: 
 * // 
 * // Signal_Altitude[i][j] where i marks the altitude and j marks the delay.
 * //
 * // So we are going to use the MAP_2 macro to map real array indices into
 * // virtual indexes.
 * 
 * // Map index 0 pAlt to index 0 of a Rank 2 index space
 * DasVar* pAlt = new_DasVar_array(pAltAry, MAP_2(0, DEGEN), Units_fromStr("km"));
 * 
 * // Map index 0 of pDelay to index 1 of a Rank 2 index space
 * DasVar* pDelay = new_DasVar_array(pDelayAry, MAP_2(DEGEN, 0), "μs");
 * 
 * // We need a constant. (Memory note, Variables copy the contents of the
 * // constant internally so it's okay to initialize constants with stack
 * // variables.)
 * int nConst = 299792 / 2;
 * DasVar* pConst = new_DasVar_const(etInt, &nConst, "km s**-1");
 * 
 * // Multiply the constant times the delay time to get the range.  Constants
 * // always return the same value for any index provided, thus they can be
 * // combined with anything.
 * DasVar* pRange = DasVar_binaryOp(&pConst, "*", pDelay);
 * 
 * // Subtract the range from the spacecraft altitude.  Since vAlt and vDelay
 * // have the same rank (due to index remapping) we can setup an element by
 * // element binary operation
 * DasVar* pSigAlt = DasVar_binaryOp( &pAlt, "-", &pRange);
 *
 * // Here's how to get data out of the resulting variable
 * double sigHeight = DasVar_double(&vSigAlt, IDX2(1234, 77));
 * 
 * // The component variables are evaluated in the same index space
 * double craftAlt = DasVar_double(&vAlt, IDX2(1234,77));
 * double range = DasVar_double(&vRange, IDX2(1234,77));
 * 
 * // If the same set of data are going to be evaluated multiple times it
 * // might be faster to just re-write all the internal data at a certain
 * // point into a new array.
 * DasVar* pEvalAlt = DasVar_evaluate(pSigAlt);
 * 
 * @endcode
 *	
 * In the example above the variable tree was created manually.  To support
 * Das2.3 general streams expressions like the following will be parsable
 * by the dataset object:
 * @code
 * 
 * DasVar* pSigAlt; 
 * pSigAlt = Dataset_eval(pDs, "$altitude[i] - (3.0e9 / 2)*delay[j]", "km" );
 * 
 * 
 * @endcode
 * 
 * To keep variables light weight (especially constants) they use stack value
 * semantics instead of heap value semantics.  This is a pain because you can't
 * just return variables from functions, as they may reference other variables
 * and you end up with dangling pointers.  On the other hand, using stack
 * semantics results in less heap fragmentation and thus faster code on current
 * processors.  Also simultaneous data reads are thread safe.  Writes can be too
 * so long as different threads write to different index ranges.
 * 
 * @see Dataset
 * @see Dimension
 * @see Array
 * @extends DasDesc
 */
typedef struct das_variable{
	enum var_type vartype;  /* CONST, ARRAY, SEQUENCE, UNARY_OP, BINARY_OP ... */
	das_val_type  vt;       /* vtUByte, vtText, vtTime, vtVector ... */
	
   size_t        vsize;    /* The size in bytes of each value in the variable
	                         * for non-scalar items, this yields the unusual value
                            * of sizeof(void*) */
   
   /* Number of external indexes.  Many of these may not be used and are
    * thus marked as degenerate */
   int nExtRank;

   /* Number of internal indexes, essentially the item rank.  Is zero except
      for text strings and geometric vectors */
   int nIntRank;
	
   /* Since it is possible to create variables in all kinds of ways (not just
    * backing arrays) we have to have our own units storage location.
    * Transforming backing arrays such that they are no longer in the units
    * they had when the variable was created will NOT automatically update this
    * variable. 
    */
   das_units units;

   /* Reference count on this variable.  Needed to make sure it's not
    * deleted out from under us. */
   int nRef;

	
	/* Get identifier for this variable, may be NULL for anoymous vars */
	const char* (*id)(const struct das_variable* pThis);
	
	/* Get full shape of this variable */
	int (*shape)(const struct das_variable* pThis, ptrdiff_t* pShape);

   /* Get the internal shape of this variable.
    *
    * Combine this with DasVar_intrType to get the purpose of the internal
    * indicies.
    */
   int (*intrShape)(const struct das_variable* pThis, ptrdiff_t* pComp);
	
	/* Write an expression (i.e. a representation) of this variable to a 
	 * buffer.
	 *
    * @param uFlags - D2V_EXP_UNITS include units in the expression
	 *                 D2V_EXP_RANGE include the range in the expression
    *                 D2V_EXP_INTER include internal component information
	 * 
	 * @returns The write point to add more text to the buffer
	 */
	char* (*expression)(const struct das_variable* pThis, char* sBuf, int nLen, 
			              unsigned int uFlags);
	
	/* Get the external length of this variable at a partial index */
	ptrdiff_t (*lengthIn)(
		const struct das_variable* pThis, int nIdx, ptrdiff_t* pLoc
	);
	
	/** Get a value at a specified index */
	bool (*get)(
		const struct das_variable* pThis, ptrdiff_t* pIdx, das_datum* pDatum
	);
	
	bool (*isFill)(
		const struct das_variable* pThis, const ubyte* pCheck, das_val_type vt	
	);
	
	/* Does this variable provide simple numbers */
	bool (*isNumeric)(const struct das_variable* pThis);

	DasAry* (*subset)(
		const struct das_variable* pThis, int nRank, const ptrdiff_t* pMin,
		const ptrdiff_t* pMax
	);
	
	/** Increment the reference count for this variable and return the new count */
	int (*incRef)(struct das_variable* pThis);
	
	/** Returns the number of remaining references to this variable,
	 * if the reference count drops to 0, the structure is deleted and
	 * the reference count for any owned subvariables or arrays is decremented
	 * which may trigger further deletions
	 */
	int (*decRef)(struct das_variable* pThis);


   /** Encodes the header */
	
} DasVar;


/** Create a new variable from unary operation on an existing variable.
 * 
 * Create a virtual variable from Operation(SubVar) as needed on an element 
 * by element basis, for example "Var1**-2", or "- Var1".  For efficency, 
 * simple powers are combined into the operator.
 * 
 * The new variable does not allocate any storage, Getting elements from this
 * variable will result in a sub-variable lookup and a calculation based on
 * the given operator.
 * 
 * If a variable is to be iterated over multiple times the function 
 * new_DasVarEval() can be used to run this calculation and any sub calculations
 * over all internal arrays and output the result into a new storage array.
 * 
 * @param sOp a string of lowercase letters or numbers describing the operation
 *        to apply.  The following strings are understood: "-", "**2", "**3"
 *        "**-2", "**-3", "ln", "log", "sqrt", "curt", "sin", "cos", "tan"
 *        For vectors the operation: "norm" is understood
 *
 * @param pVar The variable to modify
 *
 * @returns A new DasVar allocated on the heap with it's reference count set
 *          to one.
 * @memberof DasVar
 * @see new_DasVarEval
 */
DAS_API DasVar* new_DasVarUnary(const char* sOp, const DasVar* pVar);

/* Internal version for use by the expression lexer */
DAS_API DasVar* new_DasVarUnary_tok(int nOpTok, const DasVar* pVar);

/** Create a new variable from a binary operation on two other variables
 *
 * Create a virtual variable from  Var1 Operator Var2, for example Var1 + Var2.
 * 
 * The new variable does not allocate any storage though it does pre-calculate
 * any needed scaling factors.  Getting elements from this variable will result
 * in two sub-variable lookups and a calculation based on the given operator.
 * 
 * The two variables must produce the same number of values when given the 
 * same set of indices.  Most variables do not have an internal index so this
 * is usually satisfied.
 * 
 * The two variables must have units that can be combined using given operator.
 * Typically this means they must have the same units, but epoch units can be
 * combined with pure "length" (i.e. duration) units under the addition and
 * subtraction operators but two epoch units cannot be combined.
 * 
 * The new variable created by this binary combination will the units of the
 * right sub-variable.  Output of the left sub-variable will be scaled if 
 * needed before being combined on an element by element basis with the right
 * sub-variable.
 * 
 * If a variable is to be iterated over multiple times the function 
 * new_DasVarEval() can be used to run this calculation and any sub calculations
 * over all internal arrays and output the result into a new storage array
 *
 * @param sId A name for this new variable.  Use NULL for an anonymous 
 *        variable.  Anoymous variables are ususally for sub-expressions that
 *        aren't intended to be a top level data access point.
 * 
 *        Names never hurt, an clean up expression displays.  When in doubt,
 *        give the result of the binary operation a name. 
 *
 * @param pLeft the left index variable for the binary operation
 *
 * @param sOp the operation to preform, The following strings are understood
 *           "+","-","/","*","pow","dot","cross"
 *        The last two operations only work if the two variables are a vector.
 * 
 * @param pRight the indexed variable in the binary operation
 * 
 * @returns the new variable or NULL if an error occurred such as an unknown
 *          operator string.
 * @memberof DasVar
 * @see new_DasVarEval
 */
DAS_API DasVar* new_DasVarBinary(
	const char* sId, DasVar* pLeft, const char* sOp, DasVar* pRight
);


/** Create a constant value on the heap
 *
 * Wrap a constant value as a variable allowing for binary operations.
 * 
 * When asked for it's length in an index dimension, constant variables
 * report as "Function" of every index position if they are scalars.  Otherwise
 * the last index reports as the length of the vector or text string as appropriate.
 *
 * @param sId A short name for this constant, ex: 'c' for the speed of light
 * 
 * @param pDatum A pointer to a datum to wrap.
 * 
 * @memberof DasVar
 */
DAS_API DasVar* new_DasConstant(const char* sId, const das_datum* pDm);


/** Create a simple linear sequence variable
 * 
 * A simple sequence variable is linear in a single index.  Many measurements
 * happen in a parameter that progresses as a simple linea function of a 
 * single index.  For example time offest for a single A/D capture from the
 * start of the capture sequence.
 * 
 * @param sId An identifier for this sequence, follows rules for array ids
 * 
 * @param vt the value type must be one of the values in ::das_val_type
 * 
 * @param vSz the size in bytes for the value type, only used for vtByteSeq types
 * 
 * @param pMin The minimum value for the sequence
 * 
 * @param pInterval The interval between values of the sequence.
 * 
 * @param nDsRank The rank of the total index space, same as the length of pMap
 *
 * @param pMap A mapping from ::DasDs indices to this sequence's lone index.
 *             The mape can only have *one* value set to 0, the rest must be
 *             marked digenerate.
 *
 * @param units The units for values produced by this sequence except for 
 *              sequences of type vtTime.  For time sequences, this is the
 *              units of the *interval* only.  Output datums from the sequence
 *              will have units of UNIT_UTC.
 *
 * @return A DasVar structure allocated on the heap.
 * 
 * @memberof DasVar
 */
DAS_API DasVar* new_DasVarSeq(
	const char* sId, das_val_type vt, size_t vSz, const void* pMin, 
	const void* pInterval, int nExtRank, int8_t* pMap, int nIntRank, 
   das_units units
);

/** Create a variable backed by an Array
 *
 * This variable will be backed by an array though the array indicies do not
 * have to match the variable indicies.  For example an array of frequencies
 * for a time, frequency spectrogram might only have a single index [i], but
 * the variable could access these as index [i][j] where j for the function maps
 * to i for the array and i for the variable is ignored.
 *
 * @param pAry The array which contains coordinate values
 * 
 * @param nExtRank The external rank of the variable.  This should match 
 *          it's enclosing dataset, though some indicies can be marked as
 *          degenerate and are thus not mapped to the backing array.
 * 
 *          Don't set this directly if you can avoid it.  Use the 
 *          SCALAR_N and VECTOR_N macros instead.
 * 
 * @param pMap The mapping of external indexs to DasAry indexes.  Not every
 *          external index needs to be mapped.
 *
 *          Don't set this directly if you can avoid it.  Use the 
 *          SCALAR_N and VECTOR_N macros instead.
 * 
 * @param nIntRank The number of additional array indexes used to make
 *          the internal structure of Rank 1 items such as strings or
 *          Geometric Vectors.
 *
 *          Don't set this directly if you can avoid it.  Use the 
 *          SCALAR_N and VECTOR_N macros instead.
 *
 * @return A pointer to the new variable object allocated on the heap
 * 
 * @memberof DasVar
 */
DAS_API DasVar* new_DasVarArray(DasAry* pAry, int nExtRank, int8_t* pMap, int nIntRank);


/** Create a vector backed by an array
 * 
 * This variable must have one and only one internal index, and that index
 * must be fixed.  These conditions allow the variable to be a vector. 
 * 
 * Vectors can be operated on using the scalar binary operations:
 * 
 *     "+","-","/","*","pow"
 * 
 * but be aware that these operate in a component wise manner.  So:
 * @code
 *    vA = new_DasVecArray(...);
 *    vB = new_DasVecArray(...);
 *    vC = new_DasVarBinary(NULl, vA, "+", vB);
 * @endcode
 * 
 * will produce a vector where each value is vC[index] = vA[index] * vB[index]
 * 
 * Vector binary operations include:
 * 
 *    "cross", "dot"
 * 
 * which only work if two vectors have the same frame.
 * 
 * @param pAry The array which contains data values
 * 
* @param nExtRank The external rank of the variable.  This should match 
 *          it's enclosing dataset, though some indicies can be marked as
 *          degenerate and are thus not mapped to the backing array.
 * 
 *          Don't set this directly if you can avoid it.  Use the 
 *          SCALAR_N and VECTOR_N macros instead.
 * 
 * @param pMap The mapping of external indexs to DasAry indexes.  Not every
 *          external index needs to be mapped.
 *
 *          Don't set this directly if you can avoid it.  Use the 
 *          SCALAR_N and VECTOR_N macros instead.
 * 
 * @param nIntRank The number of additional array indexes used to make
 *          the internal structure of Rank 1 items such as strings or
 *          Geometric Vectors.
 *
 *          Don't set this directly if you can avoid it.  Use the 
 *          SCALAR_N and VECTOR_N macros instead.
 * 
 * @param sFrame A named coordinate frame for the vector.  If two vectors have
 *             different coordinate frames, they cannot be the subject of 
 *             binary operations.  Cannot be NULL.
 * 
 * @param nFrameId The integer id of this frame.  GeoVec datums only store
 *             the frame ID, not the name for faster comparisons
 * 
 * @param frametype The lower byte of DasFrame.flags
 * 
 * @param pDir A mapping between coordinate directions and the components 
 *             of each vector, may be NULL.
 * 
 * @param nDirs The number of directions in the vector direction map, can be 0
 */
DAS_API DasVar* new_DasVarVecAry(
   DasAry* pAry, int nExtRank, int8_t* pMap, int nIntRank, 
   const char* sFrame, ubyte nFrameId, ubyte frametype, ubyte nDirs,
   const ubyte* pDir
);

/** Increment the reference count on a variable 
 * 
 * @returns the new number of references to this variable
 * @memberof DasVar
 */
DAS_API int inc_DasVar(DasVar* pThis);

/** Decrement the reference count on a variable 
 *
 * If the reference count of a variable drops to zero then the variable 
 * decrements the reference count on all other variables and arrays that it
 * may be using and then free's it's own memory.  
 * 
 * You should set any local pointers refering to this variable to NULL after
 * calling dec_DasVar as it may no longer exist.
 * 
 * @returns the number of remaining references
 * 
 * @memberof DasVar
 */
DAS_API int dec_DasVar(DasVar* pThis);


/** Get number of references */
int ref_DasVar(const DasVar* pThis);

/** Get id token for variable, may be NULL for anoymous vars */
const char* DasVar_id(const DasVar* pThis);

/** Get the type of variable */
enum var_type DasVar_type(const DasVar* pThis);

/** Get the type of values held by the variable */
das_val_type DasVar_valType(const DasVar* pThis);

/** Get the size in bytes of each value */
size_t DasVar_valSize(const DasVar* pThis); 

/** Get the units for the values. 
 * 
 * @warning For some vectors, only the magnitude has these units.
 *          To determine if this is the case use 
 *  */
das_units DasVar_units(const DasVar* pThis); 


/** Get the backing array if present 
 * 
 * @returns NULL if the variable is not backed directly by an array
 * 
 * @memberof DasVar
 */
DAS_API DasAry* DasVarAry_getArray(DasVar* pThis);

/** Evaluate all sub-variable expressions and a single array variable
 */
DAS_API DasVar* new_DasVarEval(const DasVar* pVar);

/** Getting data from a variable */

/** Answer the question: is one variable orthogonal in index space to another.
 * 
 * @param pThis pointer to the first variable object
 * @param pOther pointer to the second variable object
 * @return True if the indicies that trigger a change in the first variable's
 *         output are completly separate from the indices that change the 
 *         second variable's output
 * @memberof DasVar
 */
DAS_API bool DasVar_orthoginal(const DasVar* pThis, const DasVar* pOther);


/** Return the current shape of this variable.
 *
 * Cause this variable to inspect it's managed array or sub-variables and
 * determine the current extents in index space.
 * 
 * @param pThis The variable for which the shape is desired
 * 
 * @param pShape a pointer to an array of size DASIDX_MAX.  Each element
 *        of the array will be filled in with either one of the following:
 *        
 *        * An integer from 0 to LONG_MAX to indicate the valid index range.
 * 
 *        * The value D2IDX_UNUSED to indicate the given index position is 
 *          ignored by this variable
 * 
 *        * The value D2IDX_RAGGED to indicate that the valid index is variable
 *          and depends on the values of other indices.
 * 
 *        * The value D2IDX_FUNC to indicate that the values are not stored
 *          but rather calculated from the given index itself.  This is true
 *          for variables backed by un-bounded sequences instead of arrays.
 * 
 * @returns The rank of the underlying storage or generation mechanism for the
 *          variable.  This is typically *not* a useful item but could be if
 *          attempting to minimize DasVar_copy memory usage.
 * 
 * @memberof DasVar
 */
DAS_API int DasVar_shape(const DasVar* pThis, ptrdiff_t* pShape);

/** Return the internal composition of this variable
 * 
 * Variables may contain scalars, or they may contain items with internal
 * structure.  For example an array strings has an internal index on the 
 * byte number, geometric vectors have an internal index that represents
 * the component in each direction.
 * 
 * @param  pThis The variable for which the shape is desired
 * 
 * @param pShape a pointer to an array of size DASIDX_MAX - 1.  Each element
 *        of the array will be filled in with either one of the following
 * 
 *        * An integer from 0 to LONG_MAX to indicate the valid index range.
 *          for a typicall geometric vector, this will be the number 3
 * 
 *        * The value D2IDX_RAGGED to indicate that the valid index is variable
 *          and depends on the values of the external indicies.  This is 
 *          common for string data.
 * 
 * @returns The rank of the interal components.  In the case of scalars this
 *          will be 0, and pShape is not touched.
 *
 * @memberof DasVar
 */
DAS_API int DasVar_intrShape(const DasVar* pThis, ptrdiff_t* pShape);


/** Return the current max value index value + 1 for any partial index
 * 
 * This is a more general version of DasVar_shape that works for both cubic
 * arrays and with ragged dimensions, or sequence values.
 * 
 * @param pThis A pointer to a DasVar structure
 * @param nIdx The number of location indices which may be less than the 
 *             number needed to specify an exact value.
 * @param pLoc A list of values for the previous indexes, must be a value 
 *             greater than or equal to 0
 * @return The number of sub-elements at this index location or D2IDX_UNUSED
 *         if this variable doesn't depend on a given location, or D2IDX_FUNC
 *         if this variable returns computed results for this location.
 * 
 * @see DasAry_lengthIn
 */
DAS_API ptrdiff_t DasVar_lengthIn(const DasVar* pThis, int nIdx, ptrdiff_t* pLoc);

/** Get a string representation of this variable.
 * 
 * @param pThis a pointer to variable in question
 *
 * @param sBuf a buffer to hold the output, 128 bytes should be more 
 *        than enough unless describing a deeply nested set of 
 *        binary operation variables are preset.
 *
 * @param nLen the length of the string buffer.  This function will
 *        not write more than nLen - 1 bytes to the buffer and will
 *        insure NULL termination
 */
DAS_API char* DasVar_toStr(const DasVar* pThis, char* sBuf, int nLen);


/** Are the values in this variable convertable to doubles? 
 *
 * DasVar can hold any enum das_val_type.  Many times applications just
 * are expecting numeric values that can be converted to doubles.  This
 * function returns true if the underlying array elements are of type:
 *
 *   vtUShort, vtShort, vtInt, vtLong, vtFloat, vtDouble, vtTime
 *
 * if the array value type is: 
 *
 *   vtUByte
 *
 * then values from this variable are considered to be convertable to
 * doubles if the underlying DasAry doesn't indicate that the values are
 * actually strings by way of the DasAry_getUsage() function.
 *
 * For variables that are backed combinations of other variables instead
 * of an array, sub-variables in the expression tree are consulted to
 * get the answer.  The first false return sticks.
 *
 * @see DasVar  for a general description of variable expressions
 *
 * @return True if the output a DasVar_getDatum() call (or the use of
 *         an iterator) will produce datums whose values are convertable
 *         to doubles.  False othewise.
 */
DAS_API bool DasVar_isNumeric(const DasVar* pThis);


/** Get a value given an index
 *
 * This is the "slow boat from China" way to retrieve elements but it always
 * works, even for non-orthogonal data sets, ragged arrays and variables built
 * on expressions involving other variables.  This is useful when re-gridding
 * a data set onto a rectangular array such as a pixel or voxel raster.
 *
 * @param pThis the variable in question
 *
 * @param pIdx The location to retrieve.  Unmapped indices are ignored.
 * 
 * @param pDatum pointer to a datum structure to fill in with the value
 *
 * @return false if the indices represented by pIdx are invalid, a pointer to
 *         the data value otherwise.  It is up to the caller to cast the
 *         pointer to the appropriate type and difference it to get the value.
 * @memberof DasVar
 */
DAS_API bool DasVar_get(
	const DasVar* pThis, ptrdiff_t* pIdx, das_datum* pDatum
);


/** Check to see if a value is a fill value for this variable
 * 
 * The two ways to check fill would be to get the data value and store it in
 * some external variable, or to call this function and ask if a value is a
 * fill value.  Since das variables can contain many different fundamental 
 * types (int, double, das_time, const char* etc.) it's easier for applications
 * build to this library to use this function, as all casting is handled
 * by the variable itself.
 *
 * 
 * @param pThis the variable in question.
 * @param pCheck a pointer to the value to check for equivalence to fill.
 * @param vt The type of the value to check.
 *
 * @returns true if this is a fill value, false otherwise.
 */
DAS_API bool DasVar_isFill(
	const DasVar* pThis, const ubyte* pCheck, das_val_type vt
);

/** Is this a simple variable or more than one variable combinded via operators?
 * 
 * If variable actually represents an expression tree of variables combined
 * via operations
 * @memberof DasVar
 */
DAS_API bool DasVar_isComposite(const DasVar* pVar);


/** Copy a subset of a variable into a memory buffer.
 *
 * Any evaluations needed to convert sequences, constants, binary operations
 * etc. are handled and all values are output in a single continuous array.
 * 
 * Indexes that are sliced to a single value are removed from the returned 
 * shape function.  Selection is by inclusive lower bound and an exclusive
 * upper bound. 
 * 
 * Giving away data memory buffers:
 *   The output DasAry may or may not own it's own memory.  If it does you
 *   can take the memory and delete the array using DasAry_disownElements().
 *   Otherwise, get the size and pointer to the memory using DasAry_getIn()
 *   and make a copy.
 * 
 * @param pThis the variable in question.
 * 
 * @param nRank The length of the subset range arrays, which should be the
 *             same as the rank of the dataset.  This argument is set when
 *             using the range macros (i.e. RNG_1, RNG_2, ...)
 *
 * @param pMin The inclusive lower bound for each index.  Must be nRank
 *             elements long. This argument is set when using range macros.
 *
 * @param pMax The exclusive upper bound for each index.  Must be nRank 
 *             elements long.  This argument is set when using range macros.
 *
 * @returns A pointer to a DasAry containing the selected range, or NULL if
 *          there is a problem.  The output DasAry may or may not own it's
 *          own memory.  The output DasAry *will* always be rectangular,
 *          reguardless of any underlying raggedness.  Calling DasAry_shape()
 *          on the return value will give non-zeor, non-negative values.
 * 
 * @memberof DasVar
 */
DAS_API DasAry* DasVar_subset(
	const DasVar* pThis, int nRank, const ptrdiff_t* pMin, const ptrdiff_t* pMax
);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _das_variable_h */
