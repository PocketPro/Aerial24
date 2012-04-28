/*
 *  GolfSwing_math.h
 *  Zelus
 *
 *  Created by Gord Parke on 10-08-11.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __GOLFSWING_MATH_H
#define __GOLFSWING_MATH_H

#include "GolfSwing_types.h"

/******* Conversion methods ******************/
#define GS_MPS_TO_MPH(x) x * 2.23693629;

/******* Debugging methods *******************/
#define GSPrintMatrix(m,r,c,p) {                                    \
			for (int i = 0; i < (r); ++i){                          \
				printf("| ");                                       \
				for (int j = 0; j < (c); ++j)                       \
					if (signbit(m[(c)*i + j]))                      \
						printf("%." #p "f ",m[(c)*i + j]);          \
					else                                            \
						printf(" %." #p "f ",m[(c)*i + j]);         \
				printf("|\n");                                      \
			}                                                       \
		}

#define GSSprintMatrix(s,m,r,c,p) {                                 \
            strcpy(s, "");                                          \
            for (int i = 0; i < (r); ++i){                          \
                sprintf(s,"%s| ",s);                                \
                for (int j = 0; j < (c); ++j)                       \
                    if (signbit(m[(c)*i + j]))                      \
                        sprintf(s,"%s%." #p "f ",s,m[(c)*i + j]);   \
                    else                                            \
                        sprintf(s,"%s%." #p "f ",s,m[(c)*i + j]);   \
                sprintf(s,"%s|\n",s);                               \
            }                                                       \
        }

/******** Other macros **********************/
#define GSMin(x,y) ((x) < (y) ? (x) : (y))
#define GSMax(x,y) ((x) > (y) ? (x) : (y))


/********** SINGLE PRECISION ******************/
/* Single-Vector Integer to Floating-Point Conversion */
void GSVectorInt8ToFloat(GSFloat vOut[], GSInt8 vIn[], GSInt size);
void GSVectorInt16ToFloat(GSFloat vOut[], GSInt16 vIn[], GSInt size);
void GSVectorInt32ToFloat(GSFloat vOut[], GSInt vIn[], GSInt size);

/* Vector output addition functions */
void GSVectorAdd(GSFloat vOut[], GSFloat vIn1[],GSFloat vIn2[], GSInt size);
void GSVectorSubtract(GSFloat vOut[], GSFloat vMinuend[],GSFloat vSubtrahend[], GSInt size);

/* Vector output multiplication functions */
void GSVectorMatrixMultiply(GSFloat vOut[], GSFloat vVect[], GSFloat mMatrix[], GSInt size);
void GSVectorMatrixTransposeMultiply(GSFloat vOut[], GSFloat vVect[], GSFloat mMatrix[], GSInt size);
void GSVectorScalarMultiply(GSFloat vOut[], GSFloat vIn[], GSFloat scalar, GSInt size);
void GSVectorScalarDivide(GSFloat vOut[], GSFloat vIn[], GSFloat scalar, GSInt size);
void GSVectorCrossProduct(GSFloat vOut[], GSFloat vIn1[], GSFloat vIn2[]);

/* Vector output single-vector fucntions */
void GSVectorAbsoluteValue(GSFloat vOut[], GSFloat vIn[], GSInt size);
void GSVectorMeanVector(GSFloat vOut[], GSFloat vIn[], GSInt vectorSize, GSInt strideBetweenVectors, GSInt windowSize);
void GSVectorVectorWeightedMean(GSFloat vOut[], GSFloat vIn1[], GSFloat vIn2[], GSFloat weight1, GSFloat weight2, GSInt size);
void GSVectorNormalize(GSFloat vOut[], GSFloat vIn[], GSInt size);

/* Vector output vector copying functions */
void GSVectorSubmatrixCopy(GSFloat vOut[], GSFloat vIn[], GSInt submatrixCols, GSInt submatrixRows,
					 GSInt vOutSupermatrixCols, GSInt vInSupermatrixCols);
void GSVectorCopy(GSFloat vOut[], GSFloat vIn[], GSInt size);

/*Vector output other functions */
void GSVectorMultiplyScalarAddVector(GSFloat vOut[], GSFloat vIn[], GSFloat scalar, GSFloat vAdd[], GSInt size);
void GSVectorLinearInterpolate(GSFloat vOut[], GSFloat vInLower[], GSFloat vInUpper[], GSFloat indexFraction, GSInt size);
void GSVectorVariance(GSFloat vOut[], GSInt16 vIn[], GSInt vectorSize, GSInt vectorStride, GSInt numVectors);

/* Scalar output functions */
GSFloat GSVectorSumMags(GSFloat vIn[], GSInt size);
GSFloat GSVectorSumElements(GSFloat vIn[], GSInt size);
GSFloat GSVectorMagnitude(GSFloat vIn[], GSInt size);
GSFloat GSVectorAngleToVector(GSFloat vIn1[], GSFloat vIn2[]);
GSFloat GSMatrixDeterminant(GSFloat mIn[]);
GSFloat GSVectorDotProduct(GSFloat vIn1[], GSFloat vIn2[], GSInt size);

/* Matrix output functions */
void GSMatrixRotationFromVector(GSFloat mOut[], GSFloat vIn[], GSFloat tolerance); 
void GSMatrixMatrixMultiply(GSFloat mOut[], GSFloat mIn1[], GSFloat mIn2[], GSInt size);
void GSMatrixTranspose(GSFloat mOut[], GSFloat mIn[], GSInt size);
void GSMatrixOrthogonalityTest(GSFloat mOut[], GSFloat mIn[], GSInt size);


/********** DOUBLE PRECISION ******************/
/* Single-Vector Integer to Floating-Point Conversion */
void GSVectorInt8ToDouble(GSDouble vOut[], GSInt8 vIn[], GSInt size);
void GSVectorInt16ToDouble(GSDouble vOut[], GSInt16 vIn[], GSInt size);
void GSVectorInt32ToDouble(GSDouble vOut[], GSInt vIn[], GSInt size);

/* Vector output addition functions */
void GSVectorAddD(GSDouble vOut[], GSDouble vIn1[], GSDouble vIn2[], GSInt size);
void GSVectorSubtractD(GSDouble vOut[], GSDouble vMinuend[],GSDouble vSubtrahend[], GSInt size);

/* Vector output multiplication functions */
void GSVectorMatrixMultiplyD(GSDouble vOut[], GSDouble vVect[], GSDouble mMatrix[], GSInt size);
void GSVectorMatrixRectangularMultiplyD(GSDouble vOut[], GSDouble vVect[], GSDouble mMatrix[], GSInt rows, GSInt cols);

void GSVectorMatrixTransposeMultiplyD(GSDouble vOut[], GSDouble vVect[], GSDouble mMatrix[], GSInt size);
void GSVectorScalarMultiplyD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSInt size);
void GSVectorScalarDivideD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSInt size);
void GSVectorScalarAddD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSInt size);
void GSVectorScalarSubtractD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSInt size);
void GSVectorCrossProductD(GSDouble vOut[], GSDouble vIn1[], GSDouble vIn2[]);

/* Vector output single-vector fucntions */
void GSVectorAbsoluteValueD(GSDouble vOut[], GSDouble vIn[], GSInt size);
void GSVectorMeanVectorD(GSDouble vOut[], GSDouble vIn[], GSInt vectorSize, GSInt strideBetweenVectors, GSInt windowSize);
void GSVectorVectorWeightedMeanD(GSDouble vOut[], GSDouble vIn1[], GSDouble vIn2[], GSDouble weight1, GSDouble weight2, GSInt size);
GSInt GSVectorNormalizeD(GSDouble vOut[], GSDouble vIn[], GSInt size);
void GSVectorSortIndexesD(GSInt vIndiciesOut[], GSDouble vIn[], GSInt n, GSInt bAsc);

/* Vector output vector copying functions */
void GSVectorSubmatrixCopyD(GSDouble vOut[], GSDouble vIn[], GSInt submatrixCols, GSInt submatrixRows,
						   GSInt vOutSupermatrixCols, GSInt vInSupermatrixCols);
void GSVectorCopyD(GSDouble vOut[], GSDouble vIn[], GSInt size);

/*Vector output other functions */
void GSVectorMultiplyScalarAddVectorD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSDouble vAdd[], GSInt size);
void GSVectorLinearInterpolateD(GSDouble vOut[], const GSDouble vInLower[], const GSDouble vInUpper[], GSDouble indexFraction, GSInt size);
void GSVectorWindowSumD(GSDouble vOut[], GSDouble vIn[], GSDouble outStride, GSDouble inStride, GSDouble outputSize, GSDouble windowSize);


/* Scalar output functions */
GSDouble GSVectorSumMagsD(GSDouble vIn[], GSInt size);
GSDouble GSVectorSumElementsD(GSDouble vIn[], GSInt size);
GSDouble GSVectorMagnitudeD(GSDouble vIn[], GSInt size);
GSDouble GSVectorAngleToVectorD(GSDouble vIn1[], GSDouble vIn2[]);
GSDouble GSVectorAngleInRangeToVectorD(GSDouble vInFrom[], GSDouble vInTo[], GSDouble vSignDirection[], GSInt bSignDirectionPos,
                                       GSDouble minDomainBound, GSInt bMinDomainHard);
GSDouble GSMatrixDeterminantD(GSDouble mIn[]);
GSDouble GSVectorDotProductD(GSDouble vIn1[], GSDouble vIn2[], GSInt size);


/* Matrix output functions */
void GSMatrixRotationFromVectorD(GSDouble mOut[], GSDouble vIn[], GSDouble tolerance); 
void GSMatrixMatrixMultiplyD(GSDouble mOut[], GSDouble mIn1[], GSDouble mIn2[], GSInt size);
void GSMatrixMatrixRectangularMultiplyD(GSDouble mOut[], GSDouble mIn1[], GSDouble mIn2[],
										GSInt mIn1Rows, GSInt mIn2Cols, GSInt mIn1Cols);
void GSMatrixTransposeD(GSDouble mOut[], GSDouble mIn[], GSInt size);
void GSMatrixTransposeOutOfPlaceD(GSDouble mOut[], GSDouble mIn[], GSInt mInRows, GSInt mInCols);
void GSMatrixOrthogonalityTestD(GSDouble mOut[], GSDouble mIn[], GSInt size);
void GSMatrixOrthogonalityCharacterizationD(GSDouble *determinant, GSDouble *angleMag, GSDouble vAngle[3], GSDouble mIn[]);
void GSMatrixFromOuterProductD(GSDouble mOut[], GSDouble vIn1[], GSDouble vIn2[], GSInt size);
void GSMatrixMakeIdentity(GSDouble mOut[], GSInt size);
GSErr GSMatrixFrameTransformationFromEquivalentWorldVectors(GSVectorElement_t mTransformation[], 
                                                           const GSVectorElement_t vSourceFrameVectors[],
                                                           const GSVectorElement_t vDestinationFrameVectors[],
                                                           GSInt numVectors, GSInt stride);

/* Random numbers */
GSFloat GSRandomNumberUniform(GSLong *seedNegToStart);
GSFloat GSRandomNumberGaussian(GSLong *seedNegToStart);

/* Quaternion Functions */
void GSQuaternionFromVector(GSVectorElement_t qQuaternion[], const 
                            GSVectorElement_t vVector[]);
void GSQuaternionProductMatrix(GSVectorElement_t mQuaterionProduct[], GSVectorElement_t qQuaterion[]);
void GSQuaternionProductMatrixBar(GSVectorElement_t mQuaterionProductBar[], GSVectorElement_t qQuaterion[]);
void GSQuaterionToRotationMatrix(GSVectorElement_t mRotation[], GSVectorElement_t qQuaterion[]);

/* Statistical functions */
GSDouble GSVectorVarianceD(GSDouble v1[], GSInt size);
GSDouble GSVectorMeanD(GSDouble v1[], GSInt size);
GSDouble GSGammaQ(GSFloat a, GSFloat x);

/* Extrapolation functions */
void GSVectorPolynomialEvaluation(GSVectorElement_t y[], GSVectorElement_t x[], GSInt xNum, GSVectorElement_t polyCoeff[], GSInt polyOrder);
GSErr GSVectorPolynomialExtrapolateD(GSVectorElement_t extrapolatedY[], GSVectorElement_t extrapolatedX[], GSInt extrapolatedSize,
                                     GSVectorElement_t fitX[], GSVectorElement_t fitY[], GSInt fitSize,
                                     GSInt polyOrder);
GSErr GSVectorPolynomialExtrapolate_withWeightAndDerivD(GSVectorElement_t extrapolatedY[], GSVectorElement_t extrapolatedX[], GSInt extrapolatedSize,
                                                        GSVectorElement_t fitX[], GSVectorElement_t fitY[], GSVectorElement_t fitWeight[], GSInt fitSize, 
                                                        GSVectorElement_t fit_dX[], GSVectorElement_t fit_dY[], GSVectorElement_t fitWeight_d[], GSInt fitSize_d,
                                                        GSInt polyOrder);

/* Time series signal analysis */
GSInt GSOffsetBetweenSignalsD(GSDouble *firstSignal, 
                              GSInt firstSignalLength,
                              GSDouble *secondSignal,
                              GSInt secondSignalLength,
                              GSDouble *correlationCoefficient); /* The correlation coefficient defines a 'cosine' like value */
                                                                 /* that returns a number from -1 to 1 representing closeness */
                                                                 /* of fit                                                    */
GSInt GSOffsetBetweenSignalsUsingNormalizedCrossCorrelationD(GSDouble *firstSignal, 
                                                             GSInt firstSignalLength, 
                                                             GSDouble *secondSignal, 
                                                             GSInt secondSignalLength,
                                                             GSDouble *correlationCoefficient);



/* LAPACK wrappers */
GSErr GSLinearLeastSquares(GSDouble *x, GSDouble *A, GSDouble *b, GSInt rows, GSInt dof);
GSErr GSEigenvaluesAndEigenvectors(GSDouble eVal[], GSDouble eVec[], GSDouble mSymmetric[], GSInt order);

/* Root Finding */
GSFloat GSNewtonsMethod(void (*funcd)(GSFloat, GSFloat *, GSFloat *), GSFloat initialGuess, GSFloat leastBound, GSFloat maxBound, GSFloat xacc);

/* Gauss Jordan Elimination functions */
enum GJ_error_types_ {
	GJ_NO_ERROR = 0,
	GJ_A_SINGULAR,
};
GSInt GSGaussJordanEliminationSolverD(GSDouble A[], GSDouble x[], GSDouble b[], GSInt width);

/* Model-data fitting functions */
GSErr GSMrqMinD(GSDouble x[], GSInt xd, GSDouble y[], GSDouble sig[], GSInt ndata, 
                GSDouble a[], GSInt ia[], GSInt ma,
                GSDouble *covar, GSDouble *alpha, GSDouble *chisq,
                void (*funcs)(GSDouble [], GSInt, GSDouble [], GSDouble *, GSDouble [], GSInt), 
                GSInt *numIter);


/*** CUSTOM ***/
GSVectorElement_t yPointEstimatator_quadratic(GSVectorElement_t *threeXs, GSVectorElement_t *threeYs, GSVectorElement_t xOfInterest);

							
#endif //__GOLFSWING_MATH_H /* #ifndef __GOLFSWING_MATH_H */∫