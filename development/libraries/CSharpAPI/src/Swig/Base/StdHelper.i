/*
 * Copyright (C) 2006 Shell International Exploration & Production.
 * All rights reserved. 
 * 
 * Developed under licence for Shell by PDS B.V.
 *
 * Confidential and proprietary source code of Shell. 
 * Do not distribute without written permission from Shell 
 */


// ---------------------------------------
// Helper for classes in the std namespace
// ---------------------------------------


/*
 * Swig does not have any support for iterators so this is a dummy
 * implementation of the std::iterator structure.
 *
 * Instantiate before derived classes.
 *
 * For instance
 *
 * %template() std::iterator< std::forward_iterator_tag, VoicePSS::Point >;
 *
 * class MyIterator : 
 *      public std::iterator< std::forward_iterator_tag, VoicePSS::Point >
 * {};
 *
 * Note %template(<empty>) indicates to swig that it does not
 * need to generate wrapper code.
 */

namespace std
{
    struct input_iterator_tag
    {};

    struct output_iterator_tag
    {};

    struct forward_iterator_tag : 
        public input_iterator_tag
    {};

    struct bidirectional_iterator_tag : 
        public forward_iterator_tag
    {};

    struct random_access_iterator_tag : 
        public bidirectional_iterator_tag
    {};


    // Note: This seems to be the only way to get SWIG to work
    // It does not accept default arguments of the type PtrT = (Type *)

    template <  class Category, 
                class Type, 
                class DiffT = int,
                class PtrT = void,  // should be Type*
                class RefT = void > // should be Type&
    struct iterator
    {};
}


// General name for vector of strings

%include <std_string.i>
%include <std_vector.i>
%include <std_pair.i>

namespace std
{
	%template(StringVector)		vector< string >;
}

// vector of integers
namespace std
{
	%template(UnsignedIntVector)		vector< unsigned int >;
}

// vector of doubles
namespace std
{
	%template(DoubleVector)		vector< double >;
}

// pair of doubles
namespace std
{
	%template(PairOfDoubles)		pair< double, double >;
}

// vestor pairs of doubles
namespace std
{
	%template(PairOfDoublesVector)		vector< pair< double, double > >;
}
