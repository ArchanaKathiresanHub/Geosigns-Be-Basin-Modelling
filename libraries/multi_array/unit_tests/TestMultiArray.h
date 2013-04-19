#ifndef TESTMULTIARRAY_H
#define TESTMULTIARRAY_H

#include <cxxtest/TestSuite.h>

#include "../RMContiguousCopyArray.h"
#include "../CMContiguousCopyArray.h"
#include "../CloseOpenInterval.h"
#include "../Tuple.h"
#include "../SteppedCopyArray.h"
#include "../Step2IndexMap.h"

using namespace multi_array;

struct TestData {
  double x;
  int y;
};

class TestMultiArray : public CxxTest::TestSuite
{
public:

	void test_global_multi_array_RMContiguousCopyArray_RMContiguousCopyArray()
	{
		CloseOpenInterval<3,int> domain(tuple(0,0,1), tuple(4,5,7));

		RMContiguousCopyArray<3,TestData> rm_array(domain);
		CMContiguousCopyArray<3,TestData> cm_array(domain);

  	    for (RMContiguousCopyArray<3,TestData>::ForwardPosCursor c1 = rm_array.forwardPosCursor(); c1.advance();) {
		    Tuple<3,int> pos = c1.pos();
		    int value = pos[0] + pos[1] + pos[2];
		    (*c1).y = value;
		}

		for (CMContiguousCopyArray<3,TestData>::ForwardPosCursor c2 = cm_array.forwardPosCursor(); c2.advance();) {
		    Tuple<3,int> pos = c2.pos();
		    int value = pos[0] + pos[1] + pos[2];
		    (*c2).y = -value;
		}

		RMContiguousCopyArray<3,TestData> copy_array1(cm_array);
		CMContiguousCopyArray<3,TestData> copy_array2(rm_array);

		Tuple<3,int> tuple1;
		tuple1[0] = 1;
		tuple1[1] = rm_array.indexMap().strides()[0];
		tuple1[2] = rm_array.indexMap().strides()[1];

		Tuple<3,int> tuple2(rm_array.indexMap().strides(),1);

		CloseOpenInterval<4,int> clop1(cons(closeOpenInterval(tuple(0),tuple(2)),rm_array.domain()));
		CloseOpenInterval<4,int> clop2(snoc(cm_array.domain(),closeOpenInterval(tuple(0),tuple(2))));

		ClosedInterval<4,int> clsd1(close(clop1));
		ClosedInterval<5,int> clsd2(cons(closedInterval(tuple(0),tuple(2)),clsd1));
		ClosedInterval<5,int> clsd3(snoc(clsd1,closedInterval(tuple(0),tuple(2))));

		Step2IndexMap<3> indexMap(cons(1,rm_array.indexMap().strides()));
		SteppedCopyArray<3,TestData> copy_array3(rm_array, indexMap);

		CloseOpenInterval<3,int> sliceDomain(tuple(1,1,2), tuple(3,4,6));
		RMStrideRefArray<3,TestData> rm_slice( rm_array.slice(sliceDomain) );
		CMStrideRefArray<3,TestData> cm_slice( cm_array.slice(sliceDomain) );

		for (RMStrideRefArray<3,TestData>::ForwardCursor s1 = rm_slice.forwardCursor(); s1.advance(); ) {
		    (*s1).y *= 2;
		}

		for (CMStrideRefArray<3,TestData>::ForwardCursor s2 = cm_slice.forwardCursor(); s2.advance(); ) {
		    (*s2).y *= 2;
		}

		SteppedRefArray<1,TestData> rm_column( rm_slice.column(tuple(1,1)) );
		RMContiguousRefArray<1,TestData> rm_row( rm_slice.row(tuple(1,2)) );
		CMContiguousRefArray<1,TestData> cm_column( cm_slice.column(tuple(1,1)) );
		SteppedRefArray<1,TestData> cm_row( cm_slice.row(tuple(1,2)) );

		for (SteppedRefArray<1,TestData>::ForwardCursor i1 = rm_column.forwardCursor(); i1.advance(); )
		    (*i1).x = 1;

		for (RMContiguousRefArray<1,TestData>::ForwardCursor i2 = rm_row.forwardCursor(); i2.advance(); )
		    (*i2).x = 1;

		for (CMContiguousRefArray<1,TestData>::ForwardCursor i3 = cm_column.forwardCursor(); i3.advance(); )
		    (*i3).x = 1;

		for (SteppedRefArray<1,TestData>::ForwardCursor i4 = cm_row.forwardCursor(); i4.advance(); )
		    (*i4).x = 1;
	}

};

#endif // TESTMULTIARRAY_H
