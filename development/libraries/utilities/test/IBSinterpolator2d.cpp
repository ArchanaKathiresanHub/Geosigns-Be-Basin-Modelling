#include "../src/IBSinterpolator2d.h"
#include <gtest/gtest.h>

//define test
void doTest(ibs::Interpolator2d & interp2, double xe[], double ye[], double fel[], double fec[],const int esize){

	for (int i = 0; i < esize; ++i)
	{
	EXPECT_NEAR(interp2.compute(xe[i],ye[i],ibs::Interpolator2d::Extrapolation(0)),fel[i],1e-10);
	EXPECT_NEAR(interp2.compute(xe[i],ye[i],ibs::Interpolator2d::Extrapolation(1)),fec[i],1e-10);
	}
}

TEST(IBSinterpolator2d,IBSinterpolator2d)
{
	const int xsize = 4, esize = 7;
	double xs[xsize], ys[xsize], fs[xsize], xe[esize], ye[esize], fel[esize], fec[esize];
	ibs::Interpolator2d interp2;

	//assigned x and f values
	xs[0] = 0.0;  ys[0] = 0.0;  fs[0] = 0.0;
	xs[1] = 1.0;  ys[1] = 0.0;  fs[1] = 0.0;
	xs[2] = 1.0;  ys[2] = 1.0;  fs[2] = 1.0;
	xs[3] = 0.0;  ys[3] = 1.0;  fs[3] = 1.0;

	//add points in the interpolation table
	for (int i = 0; i < xsize; ++i) {
		interp2.addPoint(xs[i],ys[i],fs[i]);
	};

	//expected f values at the x,y points (fec=constant,fel=linear extrapolation)
	xe[0] = xs[0];  ye[0] = ys[0];    fel[0] = fs[0];   fec[0] = fs[0];
	xe[1] = xs[1];  ye[1] = ys[1];    fel[1] = fs[1];   fec[1] = fs[1];
	xe[2] = xs[2];  ye[2] = ys[2];    fel[2] = fs[2];   fec[2] = fs[2];
	xe[3] = xs[3];  ye[3] = ys[3];    fel[3] = fs[3];   fec[3] = fs[3];

	xe[4] = 0.5;   ye[4] = 0.5;   	  fel[4] = 0.5;     fec[4] = fel[4];
	xe[5] = 1.0;   ye[5] = 0.5;   	  fel[5] = 0.5;     fec[5] = fel[5];
	xe[6] = 1.0;   ye[6] = 2.0;   	  fel[6] = 2.0;     fec[6] = 1.0;

	doTest(interp2, xe, ye, fel, fec, esize);
}
