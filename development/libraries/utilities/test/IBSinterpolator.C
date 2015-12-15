#include "../src/IBSinterpolator.h"
#include <gtest/gtest.h>

//define test
void doTest(ibs::Interpolator & interp, double xe[], double fel[], double fec[], const int esize){

	for (int i = 0; i < esize; ++i)
	{
	EXPECT_NEAR(interp.compute(xe[i],ibs::Interpolator::Extrapolation(0)),fel[i],1e-10);
	EXPECT_NEAR(interp.compute(xe[i],ibs::Interpolator::Extrapolation(1)),fec[i],1e-10);
	}
}


TEST(IBSinterpolator,IBSinterpolator)
{

	const int xsize = 3, esize = 7;
	double xs[xsize], fs[xsize], xe[esize], fel[esize], fec[esize];
	ibs::Interpolator interp;

	//assigned x and f values
	xs[0] = 0.0;    fs[0] = 1.0;
	xs[1] = 1.0;	fs[1] = 0.0;
	xs[2] = 2.0;	fs[2] = 1.0;

	//add points to the interpolation table
	interp.addPoint(xs[0], fs[0]);
	interp.addPoint(xs[1], fs[1]);
	interp.addPoint(xs[2], fs[2]);

	//expected f values at the x points (fec=constant,fel=linear extrapolation)
	xe[0] = -1;	 fel[0] = 2;	fec[0] = 1;
	xe[1] = 0;	 fel[1] = 1;	fec[1] = fel[1];
	xe[2] = 0.5; fel[2] = 0.5;	fec[2] = fel[2];
	xe[3] = 1;	 fel[3] = 0;	fec[3] = fel[3];
	xe[4] = 1.5; fel[4] = 0.5;	fec[4] = fel[4];
	xe[5] = 2;	 fel[5] = 1;	fec[5] = fel[5];
	xe[6] = 3;	 fel[6] = 2;	fec[6] = 1;

	doTest(interp, xe, fel, fec, esize);
}
