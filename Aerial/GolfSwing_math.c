/*
 *  GolfSwing_math.c
 *  Zelus
 *
 *  Created by Gord Parke on 10-08-11.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "GolfSwing_math.h"

#include <Accelerate/Accelerate.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#define USE_vDSP 

/********** SINGLE PRECISION ******************/
/* Single-Vector Integer to Floating-Point Conversion */
void GSVectorInt8ToFloat(GSFloat vOut[], GSInt8 vIn[], GSInt size){
	vDSP_vflt8((char*)vIn, 1, vOut, 1, size);
}
void GSVectorInt16ToFloat(GSFloat vOut[], GSInt16 vIn[], GSInt size){
	vDSP_vflt16((short*)vIn, 1, vOut, 1, size);
}
void GSVectorInt32ToFloat(GSFloat vOut[], GSInt vIn[], GSInt size){
	vDSP_vflt32((int*)vIn, 1, vOut, 1, size);
}

/* Vector output addition functions */
void GSVectorAdd(GSFloat vOut[], GSFloat vIn1[],GSFloat vIn2[], GSInt size){
	vDSP_vadd(vIn1, 1, vIn2, 1, vOut, 1, size);
}
void GSVectorSubtract(GSFloat vOut[], GSFloat vMinuend[],GSFloat vSubtrahend[], GSInt size){
	vDSP_vsub(vSubtrahend, 1, vMinuend, 1, vOut, 1, size);
}

/* Vector output multiplication functions */
void GSVectorMatrixMultiply(GSFloat vOut[], GSFloat vVect[], GSFloat mMatrix[], GSInt size){
	if (vOut == vVect){
		GSFloat *vTmp = malloc(sizeof(GSFloat)*size);
		vDSP_mmul(mMatrix, 1, vVect, 1, vTmp, 1, size, 1, size);
		memmove(vOut, vTmp, size * sizeof(GSFloat));
		free(vTmp);
	} else 
		vDSP_mmul(mMatrix, 1, vVect, 1, vOut, 1, size, 1, size);
}
void GSVectorMatrixTransposeMultiply(GSFloat vOut[], GSFloat vVect[], GSFloat mMatrix[], GSInt size){
	GSFloat *mTmp = malloc(sizeof(GSFloat)*size*size);
	GSMatrixTranspose(mTmp, mMatrix, size);
	GSVectorMatrixMultiply(vOut, vVect, mTmp, size);
	free(mTmp);
}
void GSVectorScalarMultiply(GSFloat vOut[], GSFloat vIn[], GSFloat scalar, GSInt size){
	vDSP_vsmul(vIn,1,&scalar,vOut,1,size);
}
void GSVectorScalarDivide(GSFloat vOut[], GSFloat vIn[], GSFloat scalar, GSInt size){
	vDSP_vsdiv(vIn, 1, &scalar, vOut, 1, size);
}
void GSVectorCrossProduct(GSFloat vOut[], GSFloat vIn1[], GSFloat vIn2[]){
	GSFloat tmpResult[3];
	tmpResult[0] = vIn1[1]*vIn2[2] - vIn1[2]*vIn2[1];
	tmpResult[1] = vIn1[2]*vIn2[0] - vIn1[0]*vIn2[2];
	tmpResult[2] = vIn1[0]*vIn2[1] - vIn1[1]*vIn2[0];
	GSVectorCopy(vOut, tmpResult, 3);
}

/* Vector output single-vector fucntions */
void GSVectorAbsoluteValue(GSFloat vOut[], GSFloat vIn[], GSInt size){
#ifdef USE_vDSP
    #if !(TARGET_IPHONE_SIMULATOR || TARGET_OS_MAC) 
        vDSP_vabs(vIn, 1, vOut, 1, size);
    #else
        for (int i = 0; i < size; ++i)
            vOut[i] = fabsf(vIn[i]);
    #endif
#else
        for (int i = 0; i < size; ++i)
            vOut[i] = fabsf(vIn[i]);
#endif
}
void GSVectorMeanVector(GSFloat vOut[], GSFloat vIn[], GSInt vectorSize, GSInt strideBetweenVectors, GSInt windowSize){
	for (int i = 0; i < vectorSize; ++i)
		vDSP_meanv(&(vIn[i]), strideBetweenVectors, &(vOut[i]), windowSize);
}
void GSVectorVectorWeightedMean(GSFloat vOut[], GSFloat vIn1[], GSFloat vIn2[], GSFloat weight1, GSFloat weight2, GSInt size){
	for (int i = 0; i < size; ++i)
		vOut[i] = 0.5 * (vIn1[i] * weight1 + vIn2[i] * weight2);
}
void GSVectorNormalize(GSFloat vOut[], GSFloat vIn[], GSInt size){
	GSFloat mag = GSVectorMagnitude(vIn, size);
	if (mag != 0)
		GSVectorScalarDivide(vOut, vIn, mag, size);
}

/* Vector output vector copying functions */
void GSVectorSubmatrixCopy(GSFloat vOut[], GSFloat vIn[], GSInt submatrixCols, GSInt submatrixRows, 
					 GSInt vOutSupermatrixCols, GSInt vInSupermatrixCols){
	vDSP_mmov(vIn, vOut, submatrixCols, submatrixRows, vInSupermatrixCols, vOutSupermatrixCols);
}
void GSVectorCopy(GSFloat vOut[], GSFloat vIn[], GSInt size){
	memmove(vOut, vIn, size * sizeof(GSFloat));
}

/*Vector output other functions */
void GSVectorMultiplyScalarAddVector(GSFloat vOut[], GSFloat vIn[], GSFloat scalar, GSFloat vAdd[], GSInt size){
	vDSP_vsma(vIn, 1, &scalar, vAdd, 1, vOut, 1, size);
}
void GSVectorLinearInterpolate(GSFloat vOut[], GSFloat vInLower[], GSFloat vInUpper[], GSFloat indexFraction, GSInt size){
	for (int i = 0; i < size; ++i) 
		vOut[i] = vInLower[i] + indexFraction*(vInUpper[i] - vInLower[i]);
}

/* Scalar output functions */
GSFloat GSVectorSumMags(GSFloat vIn[], GSInt size){
	GSFloat sOut;
	vDSP_svemg(vIn, 1, &sOut, size);
	return sOut;
}
GSFloat GSVectorSumElements(GSFloat vIn[], GSInt size){
	GSFloat sOut;
	vDSP_sve(vIn, 1, &sOut, size);
	return sOut;
}
GSFloat GSVectorMagnitude(GSFloat vIn[], GSInt size){
	GSFloat sOut;
	vDSP_svesq(vIn, 1, &sOut, size);
	return sqrtf(sOut);
}
GSFloat GSVectorAngleToVector(GSFloat vIn1[], GSFloat vIn2[]){
	GSFloat result = GSVectorDotProduct(vIn1, vIn2, 3);
	//vDSP_dotpr(vIn1, 1, vIn2, 1, &result, 3);
	GSFloat vectorMagnitudes = GSVectorMagnitude(vIn1, 3) * GSVectorMagnitude(vIn2, 3);
	if (vectorMagnitudes == 0.0f)
		return 0.0f;
	else{
		GSFloat normalized = result / vectorMagnitudes;
		if (normalized >= 1.0f)
			return 0.0f;
		if (normalized <= -1.0f)
			return 180.0f;
		return acos(result / vectorMagnitudes);
	}
}
GSFloat GSMatrixDeterminant(GSFloat mIn[]){
	GSFloat output = 0;
	output += mIn[0] * (mIn[4]*mIn[8] - mIn[5]*mIn[7]); 
	output -= mIn[1] * (mIn[3]*mIn[8] - mIn[5]*mIn[6]);
	output += mIn[2] * (mIn[3]*mIn[7] - mIn[4]*mIn[6]);
	return output;
}
GSFloat GSVectorDotProduct(GSFloat vIn1[], GSFloat vIn2[], GSInt size){
	GSFloat result;
	vDSP_dotpr(vIn1, 1, vIn2, 1, &result, size);
	return result;
}


/* Matrix output functions */
void GSMatrixRotationFromVector(GSFloat mOut[], GSFloat vIn[], GSFloat tolerance){
	/* Find the rotation in radians by looked at the rotation vector magnitude */
	GSFloat r = GSVectorMagnitude(vIn, 3);
	
	/* Check to see if the small-angle approximation can be used */
	if (fabsf(r - sinf(r)) < tolerance){ 
		mOut[0] = 1.000f;    mOut[1] = vIn[2];    mOut[2] = -vIn[1];
		mOut[3] = -vIn[2];   mOut[4] = 1.000f;    mOut[5] = vIn[0];
		mOut[6] = vIn[1];    mOut[7] = -vIn[0];   mOut[8] = 1.000f;		
	} else { 
		/* Otherwise, use the big guns */
		GSFloat n[3];
		GSVectorNormalize(n, vIn, 3);
		
		mOut[0]=n[0]*n[0]+cosf(r)*(1-n[0]*n[0]);
		mOut[1]=n[0]*n[1]*(1-cosf(r))+n[2]*sinf(r);
		mOut[2]=n[0]*n[2]*(1-cosf(r))-n[1]*sinf(r);
		mOut[3]=n[1]*n[0]*(1-cosf(r))-n[2]*sinf(r); 
		mOut[4]=n[1]*n[1]+cosf(r)*(1-n[1]*n[1]);
		mOut[5]=n[1]*n[2]*(1-cosf(r))+n[0]*sinf(r);
		mOut[6]=n[2]*n[0]*(1-cosf(r))+n[1]*sinf(r); 
		mOut[7]=n[2]*n[1]*(1-cosf(r))-n[0]*sinf(r);
		mOut[8]=n[2]*n[2]+cosf(r)*(1-n[2]*n[2]);
	}
}
void GSMatrixMatrixMultiply(GSFloat mOut[], GSFloat mIn1[], GSFloat mIn2[], GSInt size){
	vDSP_mmul(mIn1, 1, mIn2, 1, mOut, 1, size, size, size);
}
void GSMatrixTranspose(GSFloat mOut[], GSFloat mIn[], GSInt size){
	GSFloat *mTmp = malloc(sizeof(GSFloat)*size*size);
	vDSP_mtrans(mIn, 1, mTmp, 1, size, size);
	memmove(mOut, mTmp, size * size * sizeof(GSFloat));
	free(mTmp);
}
void GSMatrixOrthogonalityTest(GSFloat mOut[], GSFloat mIn[], GSInt size){
	GSFloat *transpose = malloc(size*size*sizeof(GSFloat));
	GSMatrixTranspose(transpose, mIn, size);
	GSMatrixMatrixMultiply(mOut, mIn, transpose, size);
	free(transpose);
}



/********** DOUBLE PRECISION ******************/
/* Single-Vector Integer to Floating-Point Conversion */
void GSVectorInt8ToDouble(GSDouble vOut[], GSInt8 vIn[], GSInt size){
	vDSP_vflt8D((char*)vIn, 1, vOut, 1, size);
}
void GSVectorInt16ToDouble(GSDouble vOut[], GSInt16 vIn[], GSInt size){
#ifdef USE_vDSP
	vDSP_vflt16D((short*)vIn, 1, vOut, 1, size);
#else
	for (int i = 0; i < size; ++i)
		vOut[i] = (GSDouble) vIn[i];
#endif
}
void GSVectorInt32ToDouble(GSDouble vOut[], GSInt vIn[], GSInt size){
	vDSP_vflt32D((int*)vIn, 1, vOut, 1, size);
}

/* Vector output addition functions */
void GSVectorAddD(GSDouble vOut[], GSDouble vIn1[], GSDouble vIn2[], GSInt size){
#ifdef USE_vDSP
	
#if !(TARGET_IPHONE_SIMULATOR || TARGET_OS_MAC) 
	vDSP_vaddD(vIn1, 1, vIn2, 1, vOut, 1, size);
#else  /* TARGET_IPHONE_SIMULATOR */
	for (int i = 0; i < size; ++i)
		vOut[i] = vIn1[i] + vIn2[i];
#endif /* TARGET_IPHONE_SIMULATOR */
	
#else /* USE_vDSP */
	for (int i = 0; i < size; ++i)
		vOut[i] = vIn1[i] + vIn2[i];
#endif /* USE_vDSP */
}
void GSVectorSubtractD(GSDouble vOut[], GSDouble vMinuend[],GSDouble vSubtrahend[], GSInt size){
#if 0//def USE_vDSP
	vDSP_vsubD(vSubtrahend, 1, vMinuend, 1, vOut, 1, size);
#else
	for (int i = 0; i < size; ++i)
		vOut[i] = vMinuend[i] - vSubtrahend[i];
#endif
}

/* Vector output multiplication functions */
void GSVectorMatrixMultiplyD(GSDouble vOut[], GSDouble vVect[], GSDouble mMatrix[], GSInt size){
#ifdef USE_vDSP
	if (vOut == vVect){
		GSDouble *vTmp = malloc(sizeof(GSDouble)*size);
		vDSP_mmulD(mMatrix, 1, vVect, 1, vTmp, 1, size, 1, size);
		memmove(vOut, vTmp, size * sizeof(GSDouble));
		free(vTmp);
	} else 
		vDSP_mmulD(mMatrix, 1, vVect, 1, vOut, 1, size, 1, size);
	
#else
	if (vOut == vVect){
		GSDouble *vTmp = malloc(sizeof(GSDouble)*size);
		//GSDouble vTmp[100];
		
		for (int i = 0; i < size; ++i){
			vTmp[i] = 0.0;
			for (int j = 0; j < size; ++j)
				vTmp[i] += mMatrix[size*i + j] * vVect[j];
		}
		
		memmove(vOut, vTmp, size * sizeof(GSDouble));
		free(vTmp);
	} else {
		for (int i = 0; i < size; ++i){
			vOut[i] = 0.0;
			for (int j = 0; j < size; ++j)
				vOut[i] += mMatrix[size*i + j] * vVect[j];
		}
	}
#endif
}
void GSVectorMatrixRectangularMultiplyD(GSDouble vOut[], GSDouble vVect[], GSDouble mMatrix[], GSInt rows, GSInt cols){
#ifdef USE_vDSP
	if (vOut == vVect){
		GSDouble *vTmp = malloc(sizeof(GSDouble)*cols);
		vDSP_mmulD(mMatrix, 1, vVect, 1, vTmp, 1, rows, 1, cols);
		memmove(vOut, vTmp, cols * sizeof(GSDouble));
		free(vTmp);
	} else 
		vDSP_mmulD(mMatrix, 1, vVect, 1, vOut, 1, rows, 1, cols);
#endif
}
void GSVectorMatrixTransposeMultiplyD(GSDouble vOut[], GSDouble vVect[], GSDouble mMatrix[], GSInt size){
	GSDouble *mTmp = malloc(sizeof(GSDouble)*size*size);
	GSMatrixTransposeD(mTmp, mMatrix, size);
	GSVectorMatrixMultiplyD(vOut, vVect, mTmp, size);
	free(mTmp);
}
void GSVectorScalarMultiplyD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSInt size){
#ifdef USE_vDSP
	vDSP_vsmulD(vIn,1,&scalar,vOut,1,size);
#else
	for (int i = 0; i < size; ++i)
		vOut[i] = vIn[i] * scalar;
#endif
}
void GSVectorScalarDivideD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSInt size){
#ifdef USE_vDSP
	vDSP_vsdivD(vIn, 1, &scalar, vOut, 1, size);
#else
	for (int i = 0; i < size; ++i)
		vOut[i] = vIn[i] / scalar;
#endif
}
void GSVectorScalarAddD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSInt size){
#ifdef USE_vDSP
    vDSP_vsaddD(vIn, 1, &scalar, vOut, 1, size);
#else
    for (int i = 0; i < size ++i)
        vOut[i] = vIn[i] + scalar;
#endif
}
void GSVectorScalarSubtractD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSInt size){
#ifdef USE_vDSP
    GSDouble scalarNeg = -scalar;
    vDSP_vsaddD(vIn, 1, &scalarNeg, vOut, 1, size);
#else
    for (int i = 0; i < size ++i)
        vOut[i] = vIn[i] - scalar;
#endif
}
void GSVectorCrossProductD(GSDouble vOut[], GSDouble vIn1[], GSDouble vIn2[]){
	GSDouble tmpResult[3];
	tmpResult[0] = vIn1[1]*vIn2[2] - vIn1[2]*vIn2[1];
	tmpResult[1] = vIn1[2]*vIn2[0] - vIn1[0]*vIn2[2];
	tmpResult[2] = vIn1[0]*vIn2[1] - vIn1[1]*vIn2[0];
	GSVectorCopyD(vOut, tmpResult, 3);
}


/* Vector output single-vector fucntions */
void GSVectorAbsoluteValueD(GSDouble vOut[], GSDouble vIn[], GSInt size){
#ifdef USE_vDSP
    #if !(TARGET_IPHONE_SIMULATOR || TARGET_OS_MAC) 
        vDSP_vabsD(vIn, 1, vOut, 1, size);
    #else
        for (int i = 0; i < size; ++i)
            vOut[i] = fabs(vIn[i]);
    #endif
#else
	for (int i = 0; i < size; ++i)
		vOut[i] = fabs(vIn[i]);
#endif
}
void GSVectorMeanVectorD(GSDouble vOut[], GSDouble vIn[], GSInt vectorSize, GSInt strideBetweenVectors, GSInt windowSize){
#ifdef USE_vDSP
	for (int i = 0; i < vectorSize; ++i)
		vDSP_meanvD(&(vIn[i]), strideBetweenVectors, &(vOut[i]), windowSize);
#else
	for (int i = 0; i < vectorSize; ++i){
		for (int j = 0; j < windowSize*strideBetweenVectors; j+=strideBetweenVectors)
			
	}
	
#endif
}
void GSVectorVectorWeightedMeanD(GSDouble vOut[], GSDouble vIn1[], GSDouble vIn2[], GSDouble weight1, GSDouble weight2, GSInt size){
	for (int i = 0; i < size; ++i)
		vOut[i] = 0.5 * (vIn1[i] * weight1 + vIn2[i] * weight2);
}
GSInt GSVectorNormalizeD(GSDouble vOut[], GSDouble vIn[], GSInt size){
	GSDouble mag = GSVectorMagnitudeD(vIn, size);
	if (mag != 0.0)
		GSVectorScalarDivideD(vOut, vIn, mag, size);
    else 
        return GSFail;
    return GSSuccess;
}
void GSVectorSortIndexesD(GSInt vIndicesOut[], GSDouble vIn[], GSInt n, GSInt bAsc){
    /* Initialize vIndices */
    unsigned long *ic = malloc(sizeof(unsigned long) * n);
    for (GSInt i = 0; i < n; ++i)
        ic[i] = i;
    
    /* Format asending flag */
    GSInt oFlag;
    if (bAsc)
        oFlag = 1;
    else
        oFlag = -1;
    
    /* Do sort */
    vDSP_vsortiD(vIn, ic, NULL, n, oFlag);
    
    /* Copy over indices */
    for (GSInt i = 0; i < n; ++i)
        vIndicesOut[i] = (GSInt) ic[i];
    
    /* Free memory */
    free(ic);
}

/* Vector output vector copying functions */
void GSVectorSubmatrixCopyD(GSDouble vOut[], GSDouble vIn[], GSInt submatrixCols, GSInt submatrixRows,
							GSInt vOutSupermatrixCols, GSInt vInSupermatrixCols){
	vDSP_mmovD(vIn, vOut, submatrixCols, submatrixRows, vInSupermatrixCols, vOutSupermatrixCols);
}
void GSVectorCopyD(GSDouble vOut[], GSDouble vIn[], GSInt size){
	memmove(vOut, vIn, size * sizeof(GSDouble));
}

/*Vector output other functions */
void GSVectorMultiplyScalarAddVectorD(GSDouble vOut[], GSDouble vIn[], GSDouble scalar, GSDouble vAdd[], GSInt size){
	vDSP_vsmaD(vIn, 1, &scalar, vAdd, 1, vOut, 1, size);
}

void GSVectorLinearInterpolateD(GSDouble vOut[], const GSDouble vInLower[], const GSDouble vInUpper[], GSDouble indexFraction, GSInt size){
	/**
	 Estimates the value of a vector between to sample points using linear interpolation.
	 @param vOut output vector
	 @param vInLower the starting sample point
	 @param vInUpper the end sample point
	 @param indexFraction the interpolation distance between sample points
	 @param size the number of elements in the input vectors
	 */
	for (int i = 0; i < size; ++i) 
		vOut[i] = vInLower[i] + indexFraction*(vInUpper[i] - vInLower[i]);
}
void GSVectorWindowSumD(GSDouble vOut[], GSDouble vIn[], GSDouble outStride, GSDouble inStride, GSDouble outputSize, GSDouble windowSize){
	vDSP_vswsumD(vIn, inStride, vOut, outStride, outputSize, windowSize);
}
				 
/* Scalar output functions */
GSDouble GSVectorSumMagsD(GSDouble vIn[], GSInt size){
	GSDouble sOut;
	vDSP_svemgD(vIn, 1, &sOut, size);
	return sOut;
}
GSDouble GSVectorSumElementsD(GSDouble vIn[], GSInt size){
	GSDouble sOut;
	vDSP_sveD(vIn, 1, &sOut, size);
	return sOut;
}

GSDouble GSVectorMagnitudeD(GSDouble vIn[], GSInt size)
{
    GSDouble sOut = 0;
#ifdef USE_vDSP
	vDSP_svesqD(vIn, 1, &sOut, size);
#else
    for (int i = 0; i < size; ++i)
    {
        sOut += vIn[i]*vIn[i];
    }
#endif
    return sqrt(sOut);
}

GSDouble GSVectorAngleToVectorD(GSDouble vIn1[], GSDouble vIn2[]){
/* This was my old method. Subject to error for angles near 0 and pi */
#if 0
	GSDouble result = GSVectorDotProductD(vIn1, vIn2, 3);
	GSDouble vectorMagnitudes = GSVectorMagnitudeD(vIn1, 3) * GSVectorMagnitudeD(vIn2, 3);
	if (vectorMagnitudes == 0)
		return 0;
	else{
        GSDouble ratio = result / vectorMagnitudes;
        if (ratio < -1)
            return M_PI;
        else if (ratio > 1)
            return 0;
        else
            return acos(ratio);
    }
#endif
    /* Normalize vectors first */
    GSVectorElement_t vIn1N[3], vIn2N[3];
    GSVectorElement_t vCrossProduct[3];
    GSVectorNormalizeD(vIn1N, vIn1, 3);
    GSVectorNormalizeD(vIn2N, vIn2, 3);
    GSVectorCrossProductD(vCrossProduct, vIn1N, vIn2N);
    return atan2(GSVectorMagnitudeD(vCrossProduct, 3), GSVectorDotProductD(vIn1N, vIn2N, 3));
}
GSDouble GSVectorAngleInRangeToVectorD(GSDouble vInFrom[], GSDouble vInTo[], GSDouble vSignDirection[], GSInt bSignDirectionPos,
                                       GSDouble minDomainBound, GSInt bMinDomainHard){
    /* Normalize vectors and find angle between vectors */
    GSVectorElement_t vIn1N[3], vIn2N[3];
    GSVectorElement_t vCrossProduct[3];
    GSVectorNormalizeD(vIn1N, vInFrom, 3);
    GSVectorNormalizeD(vIn2N, vInTo, 3);
    GSVectorCrossProductD(vCrossProduct, vIn1N, vIn2N);
    GSFloat angle = atan2(GSVectorMagnitudeD(vCrossProduct, 3), GSVectorDotProductD(vIn1N, vIn2N, 3));
    
    /* Unranged angle is in the range [0, 180].  Use axis and pos/neg information to map it to (-180, 180] */
    GSInt sign = (bSignDirectionPos ? 1 : -1);
    GSFloat signDot = GSVectorDotProductD(vSignDirection, vCrossProduct, 3);
    if (sign*signDot < 0 )
        angle = -angle;
    
    /* Now map it to the range specified by minDomainBound and bMinDomainBoundHard */
    GSFloat maxDomainBound = minDomainBound + 2*M_PI;
    if (bMinDomainHard){
        while (angle < minDomainBound)
            angle += 2*M_PI;
        while (angle >= maxDomainBound)
            angle -= 2*M_PI;
    } else {
        while (angle <= minDomainBound)
            angle += 2*M_PI;
        while (angle > maxDomainBound)
            angle -= 2*M_PI;
    }
    
    return angle;
}
GSDouble GSMatrixDeterminantD(GSDouble mIn[]){
	GSDouble output = 0;
	output += mIn[0] * (mIn[4]*mIn[8] - mIn[5]*mIn[7]); 
	output -= mIn[1] * (mIn[3]*mIn[8] - mIn[5]*mIn[6]);
	output += mIn[2] * (mIn[3]*mIn[7] - mIn[4]*mIn[6]);
	return output;
}
GSDouble GSVectorDotProductD(GSDouble vIn1[], GSDouble vIn2[], GSInt size){
	GSDouble result;
	vDSP_dotprD(vIn1, 1, vIn2, 1, &result, size);
	return result;
}


/* Matrix output functions */
void GSMatrixRotationFromVectorD(GSDouble mOut[], GSDouble vIn[], GSDouble tolerance){
	/* Find the rotation in radians by looked at the rotation vector magnitude */
	GSDouble r = GSVectorMagnitudeD(vIn, 3);
	
	/* Check to see if the small-angle approximation can be used */
	if (fabs(r - sinf(r)) < tolerance && 0){ 
		mOut[0] = 1.000;    mOut[1] = vIn[2];   mOut[2] = -vIn[1];
		mOut[3] = -vIn[2];  mOut[4] = 1.000;    mOut[5] = vIn[0];
		mOut[6] = vIn[1];   mOut[7] = -vIn[0];  mOut[8] = 1.000;		
	} else { 
		/* Otherwise, use the big guns */
		GSDouble n[3];
		GSVectorNormalizeD(n, vIn, 3);
		
		mOut[0]=n[0]*n[0]+cos(r)*(1-n[0]*n[0]);
		mOut[1]=n[0]*n[1]*(1-cos(r))+n[2]*sin(r);
		mOut[2]=n[0]*n[2]*(1-cos(r))-n[1]*sin(r);
		mOut[3]=n[1]*n[0]*(1-cos(r))-n[2]*sin(r); 
		mOut[4]=n[1]*n[1]+cos(r)*(1-n[1]*n[1]);
		mOut[5]=n[1]*n[2]*(1-cos(r))+n[0]*sin(r);
		mOut[6]=n[2]*n[0]*(1-cos(r))+n[1]*sin(r); 
		mOut[7]=n[2]*n[1]*(1-cos(r))-n[0]*sin(r);
		mOut[8]=n[2]*n[2]+cos(r)*(1-n[2]*n[2]);
	}
}
void GSMatrixMatrixMultiplyD(GSDouble mOut[], GSDouble mIn1[], GSDouble mIn2[], GSInt size){
#ifdef USE_vDSP
	if (mOut == mIn1 || mOut == mIn2){
		GSDouble *mTmp = malloc(size*size*sizeof(GSDouble));
		vDSP_mmulD(mIn1, 1, mIn2, 1, mTmp, 1, size, size, size);
		memmove(mOut, mTmp, size*size*sizeof(GSDouble));
		free(mTmp);
	} else 
		vDSP_mmulD(mIn1, 1, mIn2, 1, mOut, 1, size, size, size);
#else
	GSDouble *mTmp = malloc(size*size*sizeof(GSDouble));
	for (int i = 0; i < size; ++i)
		for (int j = 0; j < size; ++j){
			mTmp[size * i + j] = 0;
			for (int k = 0; k < size; ++k)
				mTmp[size * i + j] += mIn1[size * i + k] * mIn2[size * k + j];
		}
	memmove(mOut, mTmp, size*size*sizeof(GSDouble));
	free(mTmp);
#endif		
}
void GSMatrixMatrixRectangularMultiplyD(GSDouble mOut[], GSDouble mIn1[], GSDouble mIn2[],
										GSInt mIn1Rows, GSInt mIn2Cols, GSInt mIn1Cols){
	if (mOut == mIn1 || mOut == mIn2){
		GSDouble *mTmp = malloc(mIn1Rows*mIn2Cols*sizeof(GSDouble));
		vDSP_mmulD(mIn1, 1, mIn2, 1, mTmp, 1, mIn1Rows, mIn2Cols, mIn1Cols);
		memmove(mOut, mTmp, mIn1Rows*mIn2Cols*sizeof(GSDouble));
		free(mTmp);
	} else 
		vDSP_mmulD(mIn1, 1, mIn2, 1, mOut, 1, mIn1Rows, mIn2Cols, mIn1Cols);	
}
void GSMatrixTransposeD(GSDouble mOut[], GSDouble mIn[], GSInt size){
	GSDouble *mTmp = malloc(sizeof(GSDouble)*size*size);
	vDSP_mtransD(mIn, 1, mTmp, 1, size, size);
	memmove(mOut, mTmp, size * size * sizeof(GSDouble));
	free(mTmp);

}
void GSMatrixTransposeOutOfPlaceD(GSDouble mOut[], GSDouble mIn[], GSInt mInRows, GSInt mInCols){
	vDSP_mtransD(mIn, 1, mOut, 1, mInCols, mInRows);
}
void GSMatrixOrthogonalityTestD(GSDouble mOut[], GSDouble mIn[], GSInt size){
	GSDouble *transpose = malloc(size*size*sizeof(GSDouble));
	GSMatrixTransposeD(transpose, mIn, size);
	GSMatrixMatrixMultiplyD(mOut, mIn, transpose, size);
	free(transpose);
}
void GSMatrixFromOuterProductD(GSDouble mOut[], GSDouble vIn1[], GSDouble vIn2[], GSInt size){
    for (GSInt i = 0; i < size; ++i)
        for (GSInt j = 0; j < size; ++j){
            mOut[size*i + j] = vIn1[i]*vIn2[j];
        }
}
void GSMatrixOrthogonalityCharacterizationD(GSDouble *determinant, GSDouble *angleMag, GSDouble vAngle[3], GSDouble mIn[]){
    if (determinant){
        *determinant = GSMatrixDeterminantD(mIn);
    }
    if (angleMag){
        GSDouble trace = mIn[0] + mIn[4] + mIn[8];
        *angleMag = acos((trace-1.0)/2.0);
    }
    if (vAngle){
        printf("WARNING: vAngle not implemented\n");
    }
}
void GSMatrixMakeIdentity(GSDouble mOut[], GSInt size){
    GSInt size2 = size*size;
    GSInt i, j;
    for (i = 0, j = 0; i < size2; ++i){
        if (j-- == 0){
            mOut[i] = 1.0;
            j = size;
        } else {
            mOut[i] = 0.0;
        }
    }
}
GSErr GSMatrixFrameTransformationFromEquivalentWorldVectors(GSVectorElement_t mTransformation[], 
                                                           const GSVectorElement_t *vS,
                                                           const GSVectorElement_t *vD,
                                                           GSInt n, GSInt stride){
    /* INPUTS: This function expects mTransformation to be of length 9 elements (3x3).  vS and vD should */
    /*         be arrays of size vS[n][3] and vD[n][3], and n should be an integer greater than 0        */
    /* This function finds the best-fit transformation matrix between two coordinate frames using two    */
    /* arrays of vectors.  Corresponding vectors in each array are measurements of the same world        */
    /* vector, but represented in the two different coordinate frames.                                   */
    /* The resulting matrix will translate vectors in the vS coordinate frame to vectors in the vD       */
    /* coordinate frame                                                                                  */
    
    /* In more detail, this function follows the general strategy outlined here:                         */
    /* http://www.cs.iastate.edu/~cs577/handouts/quaternion.pdf                                          */
    
    /* The rotation we want can be represented by a unit quaterion q. We want the q that maximizes:      */
    /*                q(vS[i])q' • vD[i]   
                     =  q(vS[i]) • (vD[i])q                                                               */
    /* Where vS[i] and vD[i] are purely imaginary quaterions with imaginary components equal to their R3  */
    /* reprentation.  We can represent these quaterion products as matrix multiplication as follows:      */
    /*           (mS[i]_)q • (mD[i])q       where mS_[i] is the barred quaterion product matrix for vS[i] */
    /*                                        and mD[i] is the quaterion prodcut matrix for vD[i]         */
    /* Now that we have the problem in a matrix vector form, we can continue without explicitly worrying  */
    /* about quaterions.  Thus the previous equation can be written as q'(mS[i]_)'(mD[i])q.               */
    /* To find the best fit matrix, we consider each vector:                                              */
    /*     ∑ q'(mS[i]_)'(mD[i])q     for all i                                                            */
    /*     q' ( ∑ (mS[i]_)'(mD[i]) ) q    for all i, the q moved outside the sum because it's constant.   */
    /* The internal sum is a symmetric matrix.  To find the maximum q, we find the biggest eigenvalue of  */
    /* this sum (gauranteed to be real from symmetry); the corresponding eigenvalue, when normalized, is  */
    /* the unit quaterion we're looking for.                                                              */
    GSErr outputErr = GSSuccess;

    /* Find sum of product matrices ∑ (mS[i]_)'(mD[i]) */
    /* This can be  optimized by cutting out the quaterion middle-man calls */
    GSVectorElement_t M[16] = {0};
    GSVectorElement_t qS[4], qD[4];
    GSVectorElement_t mS[16], mD[16], mSt[16];
    GSVectorElement_t mAddition[16];
    for (GSInt i = 0; i < n; i += (stride + 1)){
        /* Get quaterions first */
        GSQuaternionFromVector(qS, &(vS[3*i]));
        GSQuaternionFromVector(qD, &(vD[3*i]));

        /* Now find the product matrices corresponding to these vectors */
        GSQuaternionProductMatrixBar(mS, qS);
        GSQuaternionProductMatrix(mD, qD);

        /* Calculate the addition to our M matrix */
        GSMatrixTransposeD(mSt, mS, 4);
        GSMatrixMatrixMultiplyD(mAddition, mSt, mD, 4);
        
        /* And add this addition to the M matrix */
        GSVectorAddD(M, M, mAddition, 16);
    }
     
    /* Eigenvalues and eigenvectors */
    GSVectorElement_t eigVal[4], eigVec[4][4];
    outputErr = GSEigenvaluesAndEigenvectors(eigVal, eigVec[0], M, 4);
    
    /* Eigenvalues and eigenVectors are sorted in ascending order. The biggest is our unit quaterion */
    GSVectorElement_t qr[4];
    GSVectorNormalizeD(qr, eigVec[3], 4);
    
    /* We now have our unit quaterion specified to within a sign.  To remove this degeneracy, we check */
    /* the first (real) element of the quaterion.  This element is equal to cosine(ø/2) where ø is the */
    /* magnitude of the rotation represented by the quaterion. Because ø should be in the domain [0 π] */
    /* cos(ø/2) should always be positive.                                                             */
    if (qr[0] < 0.0)
        GSVectorScalarMultiplyD(qr, qr, -1.0, 4);
    
    /* Finally, generate a rotation matrix from this rotation quaterion. */
    GSQuaterionToRotationMatrix(mTransformation, qr);
    
    return outputErr;
}

/* Random numbers */
#define IA 16807
#define IM 2147483647 
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB) 
#define EPS 1.2e-7
#define RNMX (1.0-EPS)
GSFloat GSRandomNumberUniform(GSLong *idum){    /*  ran1 */
    /*“Minimal” random number generator of Park and Miller with Bays-Durham shuffle and added safeguards. Returns a uniform random deviate between 0.0 and 1.0 
     (exclusive of the endpoint values). Call with idum a negative integer to initialize; thereafter, do not alter idum between successive deviates in a sequence. 
     RNMX should approximate the largest floating value that is less than 1.*/
    
    int j;
    long k; 
    static long iy = 0;
    static long iv[NTAB];
    float temp;
    
    if (*idum <= 0 || !iy) {            //  Initialize.
        if (-(*idum) < 1)
            *idum=1;      // Be sure to prevent idum = 0.
        else 
            *idum = -(*idum);
        
        for (j = NTAB + 7; j >= 0; j--){  // Load the shuffle table (after 8 warm-ups).
            k=(*idum)/IQ; 
            *idum=IA*(*idum-k*IQ)-IR*k; 
            if (*idum < 0) 
                *idum += IM; 
            if (j < NTAB) 
                iv[j] = *idum;
        }
        iy=iv[0]; 
    }
    
    k=(*idum)/IQ;   //  Start here when not initializing.
    *idum = IA*(*idum - k*IQ)-IR*k; // Compute idum=(IA*idum) % IM without over-
    if (*idum < 0) *idum += IM; // flows by Schrage’s method.
    j = (int) iy/NDIV; // Will be in the range 0..NTAB-1.
    iy = iv[j]; // Output previously stored value and refill the
    iv[j] = *idum; // shuffle table.
    
    if ((temp=AM*iy) > RNMX) //Because users don’t expect endpoint values.
        return RNMX; 
    else 
        return temp;       
}

GSFloat GSRandomNumberGaussian(GSLong *idum){
    /* Returns a normally distributed deviate with zero mean and unit variance, using ran1(idum) */
    /* as the source of uniform deviates.                                                        */
    static int iset = 0; 
    static float gset;
    float fac,rsq,v1,v2;
    
    if (*idum < 0) iset = 0; /* Reinitialize. */
    if (iset == 0) {         /* We don’t have an extra deviate handy, so */
        do {
            v1 = 2.0*GSRandomNumberUniform(idum) - 1.0;  /* pick two uniform numbers in the square ex- tending from -1 to +1 */
            v2 = 2.0*GSRandomNumberUniform(idum) - 1.0;  /* in each direction, see if they are in the unit circle, */

            rsq = v1*v1 + v2*v2;
        } while (rsq >= 1.0 || rsq == 0.0); /* and if they are not, try again. */
        
        fac=sqrt(-2.0*log(rsq)/rsq);
        
        // Now make the Box-Muller transformation to get two normal deviates. Return one and save the other for next time.
        gset=v1*fac;
        iset=1;
        return v2*fac;
    } else {                /* We have an extra deviate handy, */
        iset = 0;           /* so unset the flag,              */
        return gset;        /* and return it.                  */
    }
}
#undef IA  
#undef IM   
#undef AM  
#undef IQ  
#undef IR  
#undef NTAB
#undef NDIV
#undef EPS 
#undef RNMX


/* Quaternion Functions */
void GSQuaternionFromVector(GSVectorElement_t qQuaternion[], const GSVectorElement_t vVector[]){
    /* INPUTS: This function expects qQuaterion to be 4 elements long, and vVector to be 3 */
    /* This method is very simple, it just vVector - a vector in R3 - and uses it as the   */
    /* imaginary components of a new quaterion, which is represented by a vector in R4.    */
    qQuaternion[0] = 0.0;
    qQuaternion[1] = vVector[0];
    qQuaternion[2] = vVector[1];
    qQuaternion[3] = vVector[2];
}
void GSQuaternionProductMatrix(GSVectorElement_t mQuaterionProduct[], GSVectorElement_t qQuaterion[]){
    /* INPUTS: This function expects mQuaterionProduct to be 16 elements long (4x4), and qQuaterion to be 4  */
    /* Returns in 'mQuaterionProduct' a 4x4 matrix from 'qQuaterion' which, when multiplied with a different */
    /* quaterion, represents their quaterion product.                                                        */
    /* The returned matrix represents qQuaterion multiplied in from the left.  For the opposite case see     */
    /* GSQuaterionProductMatrixBar.                                                                          */
    
    /* Covenience variables */
    GSVectorElement_t *q = qQuaterion;
    GSVectorElement_t m[16] = {
        q[0], -q[1], -q[2], -q[3],
        q[1],  q[0], -q[3],  q[2],
        q[2],  q[3],  q[0], -q[1],
        q[3], -q[2],  q[1],  q[0]
    };
 
    /* Copy over to output */
    GSVectorCopyD(mQuaterionProduct, m, 16);
}

void GSQuaternionProductMatrixBar(GSVectorElement_t mQuaterionProductBar[], GSVectorElement_t qQuaterion[]){
    /* INPUTS: This function expects mQuaterionProduct to be 16 elements long (4x4), and qQuaterion to be 4  */
    /* Returns in 'mQuaterionProduct' a 4x4 matrix from 'qQuaterion' which, when multiplied with a different */
    /* quaterion, represents their quaterion product.                                                        */
    /* The returned matrix represents qQuaterion multiplied in from the right.  For the opposite case see    */
    /* GSQuaterionProductMatrix.                                                                             */
    
    /* Covenience variables */
    GSVectorElement_t *q = qQuaterion;
    GSVectorElement_t m[16] = {
        q[0], -q[1], -q[2], -q[3],
        q[1],  q[0],  q[3], -q[2],
        q[2], -q[3],  q[0],  q[1],
        q[3],  q[2], -q[1],  q[0]
    };
    
    /* Copy over to output */
    GSVectorCopyD(mQuaterionProductBar, m, 16);
}
void GSQuaterionToRotationMatrix(GSVectorElement_t mRotation[], GSVectorElement_t qQuaterion[]){
    /* INPUTS: This function expexts mRotation to be 9 elements long (3x3) and qQuaterion to be 4          */
    /* A unit quaterion represents a rotation.  This function calculates the corresponding rotation matrix */
    /* representation for that rotation.  The quaterion is normalized to ensure it's a unit vector.        */
    
    /* Normalize quaterion */
    GSVectorElement_t q[4];
    GSVectorNormalizeD(q, qQuaterion, 4);
    
    /* Calcuate rotation matrix */
    GSVectorElement_t m[9] = {
      q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3],    2.0*(q[1]*q[2] - q[0]*q[3]),     2.0*(q[1]*q[3]+q[0]*q[2]),
      2.0*(q[1]*q[2] + q[0]*q[3]),   q[0]*q[0] - q[1]*q[1] + q[2]*q[2] - q[3]*q[3],   2.0*(q[2]*q[3] - q[0]*q[1]),
      2.0*(q[1]*q[3]-q[0]*q[2]),   2.0*(q[2]*q[3] + q[0]*q[1]),   q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3]    
    };
    
    /* Copy over to output */
    GSVectorCopyD(mRotation, m, 9);
}


/* Statistical functions */
GSDouble GSVectorVarianceD(GSDouble v1[], GSInt size){
    GSDouble sumSquared;
    GSDouble mean;
    
    vDSP_dotprD(v1, 1, v1, 1, &sumSquared, size); /* Sum of Squares */
    vDSP_meanvD(v1, 1, &mean, size);              /* Sum */
    
    return sumSquared/size - pow(mean, 2);
}
GSDouble GSVectorMeanD(GSDouble v1[], GSInt size){
    GSDouble result;
    
    vDSP_meanvD(v1, 1, &result, size);

    return result;
}

/* Extrapolation functions */
void GSVectorPolynomialEvaluation(GSVectorElement_t y[], GSVectorElement_t x[], GSInt xNum, GSVectorElement_t polyCoeff[], GSInt polyOrder){
    vDSP_vpolyD(polyCoeff, 1, x, 1, y, 1, xNum, polyOrder);
}
GSErr GSVectorPolynomialExtrapolateD(GSVectorElement_t extrapolatedY[], GSVectorElement_t extrapolatedX[], GSInt extrapolatedSize,
                                              GSVectorElement_t fitX[], GSVectorElement_t fitY[], GSInt fitSize,
                                              GSInt polyOrder)
{
    return GSVectorPolynomialExtrapolate_withWeightAndDerivD(extrapolatedY, extrapolatedX, extrapolatedSize,
                                                             fitX, fitY, NULL, fitSize, NULL, NULL, NULL, 0, polyOrder);
}
GSErr GSVectorPolynomialExtrapolate_withWeightAndDerivD(GSVectorElement_t extrapolatedY[], GSVectorElement_t extrapolatedX[], GSInt extrapolatedSize,
                                     GSVectorElement_t fitX[], GSVectorElement_t fitY[], GSVectorElement_t fitWeight[], GSInt fitSize, 
                                     GSVectorElement_t fit_dX[], GSVectorElement_t fit_dY[], GSVectorElement_t fitWeight_d[], GSInt fitSize_d,
                                     GSInt polyOrder)
{

    /* This function uses fitSize (x,y) points in the fitX and fitY vectors to calculate a linear least squares best fit polynomial of order polyOrder */
    /* This polynomial is then used to calculate y values for the x values in the extrapolatedX vector, which are then placed in extrapolatedY.        */
    /* polyOrder: 1  ---> linear                                                                                                                       */
    /* polyOrder: 2  ---> quadratic                                                                                                                    */
    /* polyOrder: 3  ---> cubic                                                                                                                        */
    /*                                                                                                                                                 */
    /* Addition: This function also now allows weighting and derivative fitting.  Originally, this function found the set of j that minimized          */
    /*                     ∑ Yi - P(aj,xi)                                                                                                             */
    /* but now it minimizes:                                                                                                                           */
    /*                     ∑ Wi(Yi - P(aj,xi)) + ∑ Wi(Y'i - P'(aj,xi))                                                                                 */
    const GSInt minSupportedPolyOrder = 2;
    const GSInt maxSupportedPolyOrder = 20;  /* This number was picked without any real reason.  If you need bigger, try it out */
    
    /* Check to make sure we have all our input parameters */
    if (!extrapolatedY || !extrapolatedX || !fitX || !fitY || polyOrder < minSupportedPolyOrder
        || polyOrder > maxSupportedPolyOrder || fitSize < polyOrder) 
        return GSParameterMissing;
    
    /* Convenience variables */
    GSVectorElement_t *t = fitX;
    GSInt dof = polyOrder + 1;
    
    /* Get center and scaling factors for the x (aka t) variables */
    GSDouble xStd  = sqrt(GSVectorVarianceD(t, fitSize));
    GSDouble xMean = GSVectorMeanD(t, fitSize);
    
    /* Create polynomial coefficient matrix with the set of yi*/
    GSDouble *A = malloc(sizeof(GSVectorElement_t)*(fitSize + fitSize_d)*dof);
    if (!A) return GSMallocFailed;
    for (GSInt i = 0; i < fitSize; ++i){
        GSVectorElement_t xCenteredAndScaled_i = (t[i] - xMean)/xStd;
        for (GSInt j = 0; j < dof; ++j)
            if (fitWeight)
                A[dof*i + j] = fitWeight[i] * pow(xCenteredAndScaled_i,polyOrder - j);
            else 
                A[dof*i + j] = pow(xCenteredAndScaled_i,polyOrder - j);
    }
    
    /* Add to the polynomial coefficient matrix with the y'i */
    for (GSInt i = 0; i < fitSize_d; ++i){
        GSVectorElement_t xCenteredAndScaled_i = (fit_dX[i] - xMean)/xStd;
        for (GSInt j = 0; j < dof-1; ++j){
            if (fitWeight_d)
                A[dof*fitSize + dof*i + j] = fitWeight_d[i] * (polyOrder - j) * pow(xCenteredAndScaled_i, polyOrder - j - 1);
            else 
                A[dof*fitSize + dof*i + j] = (polyOrder - j) * pow(xCenteredAndScaled_i, polyOrder - j - 1);
        }
        A[dof*fitSize + dof*i + (dof-1)] = 0;
    }
    
    /* Create target vector */
    GSVectorElement_t *b = malloc(sizeof(GSVectorElement_t)*(fitSize + fitSize_d));
    if (!b) return GSMallocFailed;
    GSVectorCopyD(b, fitY, fitSize);
    GSVectorCopyD(b+fitSize, fit_dY, fitSize_d);
    
    /* Get poly coefficients */
    GSVectorElement_t *c = malloc(sizeof(GSVectorElement_t)*dof);
    if (!c) return GSMallocFailed;
    GSErr leastSquaresError;
    if ((leastSquaresError = GSLinearLeastSquares(c, A, b, fitSize + fitSize_d, dof)) == GSSuccess){
        /* Create space for scaled extrapolated x array */
        GSVectorElement_t *ex = malloc(sizeof(GSVectorElement_t)*extrapolatedSize);
        if (!ex) return GSMallocFailed;
        
        /* Center and scale */
        GSVectorScalarSubtractD(ex, extrapolatedX, xMean, extrapolatedSize);
        GSVectorScalarDivideD(ex, ex, xStd, extrapolatedSize);
        
        /* Do polynomial evaluation to get output */
        GSVectorPolynomialEvaluation(extrapolatedY, ex, extrapolatedSize, c, polyOrder);
        
        /* Clean up and return */
        free(c);
        free(b);
        free(A);
        free(ex);
        return GSSuccess;
    } else  {
        free(c);
        free(b);
        free(A);
        return leastSquaresError;
    }
}

/* Time series signal analysis */
GSInt GSOffsetBetweenSignalsD(GSDouble *firstSignal, 
                              GSInt firstSignalLength,
                              GSDouble *secondSignal,
                              GSInt secondSignalLength,
                              GSDouble *correlationCoefficient)
{
/* The correlation coefficient defines a 'cosine' like value */
/* that returns a number from -1 to 1 representing closeness */
/* of fit                                                    */

    return GSOffsetBetweenSignalsUsingNormalizedCrossCorrelationD(firstSignal, 
                                                                  firstSignalLength, 
                                                                  secondSignal, 
                                                                  secondSignalLength,
                                                                  correlationCoefficient);
}
GSInt GSOffsetBetweenSignalsUsingNormalizedCrossCorrelationD(GSDouble *firstSignal, 
                                                             GSInt firstSignalLength, 
                                                             GSDouble *secondSignal, 
                                                             GSInt secondSignalLength,
                                                             GSDouble *correlationCoefficient)
{
    GSDouble *shortSignal;
    GSInt shortSignalLength;
    GSDouble *longSignal;
    GSInt longSignalLength;
    if (firstSignalLength > secondSignalLength){
        longSignal = firstSignal;
        longSignalLength = firstSignalLength;
        shortSignal = secondSignal;
        shortSignalLength = secondSignalLength;
    } else if (firstSignalLength < secondSignalLength) {
        longSignal = secondSignal;
        longSignalLength = secondSignalLength;
        shortSignal = firstSignal;
        shortSignalLength = firstSignalLength;
    } else {
        return 0;
    }
    
    /* 
     the normalized cross correlation is performed by first subtracting the mean 
     and dividing by the standard deviation of each signal and then performing a 
     standard correlation.
     **/
    
    GSDouble shortSignalMean;
    GSDouble shortSignalDeviation;
    GSDouble longSignalMeanOnShortSignalDomain;
    GSDouble longSignalDeviationOnShortSignalDomain;
    GSDouble longSignalSquaredMeanOnShortSignalDomain;
    GSInt offset;
    GSDouble correlation, _correlation;
    
    /* calculate the mean of the short signal */
    GSDouble cumulativeSum = 0;
    for (GSInt i = 0; i < shortSignalLength; i++){
        cumulativeSum += shortSignal[i];
    }
    shortSignalMean = cumulativeSum / shortSignalLength;
    
    /* calculate the standard deviation of the short signal */
    GSDouble cumulativeDifference = 0;
    for (GSInt i = 0; i < shortSignalLength; i++) {
        cumulativeDifference += pow((shortSignal[i] - shortSignalMean),2);
    }
    shortSignalDeviation = sqrt(cumulativeDifference / shortSignalLength);
    
    /* 
     calculate the initial mean and standard deviation of the longer signal. The
     initial mean and deviation are defined over the length of the smaller
     signal.
     **/
    cumulativeSum = 0;
    GSDouble cumulativeSquaredSum = 0;
    for (int i = 0; i < shortSignalLength; i++){
        cumulativeSum += longSignal[i];
        cumulativeSquaredSum += pow(longSignal[i],2);
    }
    longSignalMeanOnShortSignalDomain = cumulativeSum / shortSignalLength;
    longSignalSquaredMeanOnShortSignalDomain = cumulativeSquaredSum / shortSignalLength;
    longSignalDeviationOnShortSignalDomain = sqrt(longSignalSquaredMeanOnShortSignalDomain - pow(longSignalMeanOnShortSignalDomain,2));
    
    /* calculate the initial correlation and offset values*/
    GSInt maximumOffset = longSignalLength - shortSignalLength;
    cumulativeSum = 0;
    for (GSInt i = 0; i < shortSignalLength; i++){
        cumulativeSum += (shortSignal[i] - shortSignalMean)*(longSignal[i] - longSignalMeanOnShortSignalDomain);
    }
    correlation = (cumulativeSum/(shortSignalDeviation*longSignalDeviationOnShortSignalDomain))/shortSignalLength;
    offset = 0;
    
    /*
     iterate over all possibles values of the offset to determine the best
     correlation value.
     **/
    for (GSInt _offset = 1; _offset < maximumOffset; _offset++){
        /* 
         subtract the contribution of the element dropped off and add the
         contribution from the element added on the mean and standard deviation
         of the long signal.
         */
        GSDouble oldContribution = longSignal[_offset - 1];
        GSDouble newContribution = longSignal[shortSignalLength + _offset - 1];
        longSignalMeanOnShortSignalDomain += (newContribution - oldContribution)/shortSignalLength;
        longSignalSquaredMeanOnShortSignalDomain += (pow(newContribution,2) - pow(oldContribution,2))/shortSignalLength;
        longSignalDeviationOnShortSignalDomain = sqrt(longSignalSquaredMeanOnShortSignalDomain - pow(longSignalMeanOnShortSignalDomain,2));
        
        /* calculate the new correlation */
        cumulativeSum = 0;
        for (GSInt j = 0; j < shortSignalLength; j++){
            cumulativeSum += (shortSignal[j] - shortSignalMean)*(longSignal[j + _offset] - longSignalMeanOnShortSignalDomain);
        }
        _correlation = (cumulativeSum/(shortSignalDeviation*longSignalDeviationOnShortSignalDomain))/shortSignalLength;
        
        /* update the correlation if possible */
        if (_correlation > correlation){
            correlation = _correlation;
            offset = _offset;
        }
    }
    
    /* If correlation coefficient was asked for, return it */
    if (correlationCoefficient)
        *correlationCoefficient = correlation;
    
    return offset;    
}


/* LAPACK Wrappers */
GSErr GSLinearLeastSquares(GSDouble *x, GSDouble *A, GSDouble *b, GSInt rows, GSInt dof )
{
    /* Check we don't have an underdetermined system */
    if (dof > rows) return GSUnderDeterminedSystem;
    
    /* Convert A to be column major and allocate space for output */
    /* Output is passed back in the same column vector that was passed in as input */
    /* Since we want this to be out of place, we must first copy over our input vector */
    __CLPK_doublereal *cmA = malloc(sizeof(__CLPK_doublereal)*rows*dof);
    __CLPK_doublereal *_b  = calloc(sizeof(__CLPK_doublereal),rows);
    if (!cmA || !_b) return GSMallocFailed;
    for (GSInt i = 0; i < rows; ++i){
        for (GSInt j = 0; j < dof; ++j)
            cmA[rows*j + i] = A[dof*i + j];
        _b[i] = b[i];
    }
    
    /* Allocate workspace. It might be possible to optimize performance by looking into block sizes */
    __CLPK_integer lwork = dof + rows /* times block size */ ;
    __CLPK_doublereal *work = malloc(sizeof(__CLPK_doublereal)*lwork);
    if (!work) return GSMallocFailed;
    
    /* Create other variables (fortran params must be passed by reference) */
    char trans = 'N';
    __CLPK_integer nrhs = 1;
    __CLPK_integer info;
    __CLPK_integer _rows = rows;
    __CLPK_integer _dof = dof;
    
    /* Make call to LAPACK */
    dgels_(&trans, &_rows, &_dof, &nrhs, cmA, &_rows, _b, &_rows, work, &lwork, &info);
    
    /* Check LAPACK return parameter */
    if (info < 0) {
        free(cmA); free(_b); free(work);
        return GSLapackParameterError;
    }
    else if (info > 0) {
        free(cmA); free(_b); free(work);
        return GSNotFullRank;
    }
    
    /* Copy output to our output vector */
    for (GSInt i = 0; i < dof; ++i)
        x[i] = _b[i];
    
    free(cmA); free(_b); free(work);
    return GSSuccess;
}
GSErr GSEigenvaluesAndEigenvectors(GSDouble eVal[], GSDouble eVec[], GSDouble mSymmetric[], GSInt order){
    /* This function returns the eigenvalues and corresponding eigenvectors of the symmetric matrix 'mSymmetric' ordered from smallest */
    /* To largest.                                                                                                                     */
    /* INPUTS: eVal must have space allocated for order elments, for eVec it must be order*order.                                      */
    GSErr outputErr;
    
    /* Check inputs */
    if (order <= 1)
        return GSInvalidParameter;
    
    /* Start creating inputs to lapack function */
    char jobz = 'V';
    char uplo = 'U';
    __CLPK_integer n = order;
    __CLPK_integer ldz = n;
    __CLPK_integer lwork = 1 + 6*n + n*n;
    __CLPK_integer liwork = 3 + 5*n;
    __CLPK_integer info;
    
    /* Create upper triangular matrix with column major representation */
    __CLPK_integer ap_size = (n*(n+1))>>1;
    __CLPK_doublereal *ap = malloc(sizeof(__CLPK_doublereal)*ap_size);
    for (__CLPK_integer j = 0; j < n; ++j)
        for (__CLPK_integer i = 0; i <= j; ++i)
            ap[i + ((j*(j+1))>>1)] = mSymmetric[order*i + j];
    
    /* Eigenvalue and eigenvector out arrays */
    __CLPK_doublereal *w = malloc(sizeof(__CLPK_doublereal)*n);
    __CLPK_doublereal *z = malloc(sizeof(__CLPK_doublereal)*ldz*n);
    __CLPK_doublereal *work = malloc(sizeof(__CLPK_doublereal)*lwork);
    __CLPK_integer *iwork = malloc(sizeof(__CLPK_integer)*liwork);
    

    /* Make call to lapack */
    dspevd_(&jobz, &uplo, &n, ap, w, z, &ldz, work, &lwork, iwork, &liwork, &info);
    
    /* Check lapack output code */
    if (info < 0){
        /* if INFO = -i, the i-th argument had an illegal value. */
        outputErr = GSLapackParameterError;
    } else if (info > 0){
        /* if INFO = i, the algorithm failed to converge; i
         off-diagonal elements of an intermediate tridiagonal
         form did not converge to zero. */
        outputErr = GSLapackDidNotConverge;
    } else {
        outputErr = GSSuccess;
        
        /* Copy to output */
        for (GSInt i = 0; i < order; ++i){
            /* Eigenvalue */
            eVal[i] = w[i];
            
            /* Eigenvector. Here we do not need to worry about row-major or column-major  */
            /* ordering. LAPACK will return an array with the i'th consequtive set of 'n' */ 
            /* samples as the eigenvector corresponding to the i'th eigenvalue. This will */
            /* work with c - internally the LAPACK thinks of this array as a matrix with  */
            /* eigenvectors as columns, whereas c will think of the eigenvectors arranged */
            /* in rows.  Practically this makes no difference to us, since to get the ith */
            /* eigenvector we'll pass in something like GSDouble eVecIN[i][3], and so     */
            /* we'll be able to access the i'th eigenvector as eVecIN[i][0], eVecIN[i][1] */
            /* etc. (here 3 assumes an order of 3                                         */
             
            for (GSInt j = 0; j < order; ++j)
                eVec[i*order + j] = z[i*order + j];
        }
        
    }
    
    /* Free storage we malloc'ed in this function */
    free(ap);
    free(w);
    free(z);
    free(work);
    free(iwork);
    
    return outputErr;
}

GSFloat GSNewtonsMethod(void (*funcd)(GSFloat, GSFloat *, GSFloat *), GSFloat initialGuess, GSFloat leastBound, GSFloat maxBound, GSFloat xacc)
{
  //Using the Newton-Raphson method, find the root of a function known to lie in the interval [x1, x2]. The root rtnewt will be refined until its accuracy is known within ±xacc. funcd is a user-supplied routine that returns both the function value and the first derivative of the function at the point x.  
    GSInt j;
    GSFloat df,dx,f,rtn;
    GSFloat x1 = leastBound;
    GSFloat x2 = maxBound;
    
    rtn = initialGuess;            //  Initial guess.
    for (j = 1; j <= 20; j++) {
        (*funcd)(rtn,&f,&df); 
        dx = f/df;
        rtn -= dx;
        if ((x1-rtn)*(rtn-x2) < 0.0)
            return 0.0;
        if (fabsf(dx) < xacc)
                return rtn; //Convergence. 
    }
    
    return 0.0; //Never get here. 
}

/* Gauss Jordan Elimination functions */
#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}
GSInt GSGaussJordanEliminationSolverD(GSDouble A[], GSDouble x[], GSDouble b[], GSInt n){
/*Linear equation solution by Gauss-Jordan elimination a[1..n][1..n] is the input matrix. 
 b[1..n][ is input containing the m right-hand side vector. On output, a is replaced by its 
 matrix inverse, and x and b ire replaced by the corresponding set of solution vectors.*/
    GSInt *indxc,*indxr,*ipiv; 
    GSInt i,icol,irow,j,k,l,ll; 
    GSDouble big,dum,pivinv,temp;

    /* The integer arrays ipiv, indxr, and indxc are used for bookkeeping on the pivoting. */
    indxc = malloc(sizeof(GSInt)*n);
    indxr = malloc(sizeof(GSInt)*n);
    ipiv = malloc(sizeof(GSInt)*n);

    /* Initialize */
    for (j = 0; j < n; j++) 
        ipiv[j]=0;
    
    /* This is the main loop over the columns to be reduced. */
    for (i = 0; i < n; i++) {
        
        /* This is the outer loop of the search for a pivot element. */
        big = 0.0;
        for (j = 0; j < n; j++){
            if (ipiv[j] != 1)
                for (k = 0; k < n; k++) {
                    if (ipiv[k] == 0) {
                        if (fabs(A[n*j + k]) >= big) {
                            big = fabs(A[n*j + k]); 
                            irow=j;
                            icol=k;
                        }
                    } else if (ipiv[k] > 1) 
                        return GJ_A_SINGULAR;
                }
        }
        ++(ipiv[icol]);
        
        
        /*
         We now have the pivot element, so we interchange rows, if needed, to put the
         pivot element on the diagonal. The columns are not physically interchanged, only
         relabeled: indxc[i], the column of the ith pivot element, is the ith column that
         is reduced, while indxr[i] is the row in which that pivot element was originally
         located. If indxr[i] != indxc[i] there is an implied column interchange. With
         this form of bookkeeping, the solution b’s will end up in the correct order, and
         the inverse matrix will be scrambled by columns.
         */
        
        if (irow != icol) {
            for (l = 0; l < n; l++) 
                SWAP(A[n*irow + l],A[n*icol + l]); 
            SWAP(b[irow],b[icol]);
        }
        indxr[i] = irow;
        indxc[i] = icol;
        
        /* Verify */
        if (A[n*icol + icol] == 0.0) 
            return GJ_A_SINGULAR;
        
        /* We are now ready to divide the pivot row by the indxc[i]=icol; pivot element, located at irow and icol. */
        pivinv = 1.0/A[n*icol + icol];
        A[n*icol + icol] = 1.0;
        for (l = 0; l < n; l++) 
            A[n*icol + l] *= pivinv;
        b[icol] *= pivinv;
        
        /*  Next, we reduce the rows... */
        for (ll = 0; ll < n; ll++){ 
            if (ll != icol) { /* ..except for the pivot one, of course. */
                dum = A[n*ll + icol];
                A[n*ll + icol] = 0.0;
                for (l = 0; l < n; l++) 
                    A[n*ll + l] -= A[n*icol + l]*dum;
                b[ll] -= b[icol]*dum;
            }
        }
    }
    
    /* This is the end of the main loop over columns of the reduction. It only remains to unscram-
      ble the solution in view of the column interchanges. We do this by interchanging pairs of 
     columns in the reverse order that the permutation was built up. */
        for (l = n - 1; l >= 0;l--) {
            if (indxr[l] != indxc[l]) 
                for (k = 0; k < n; k++)
                    SWAP(A[n*k + indxr[l]], A[n*k + indxc[l]]);
        } 
    
    /* And we are done. */
    free(ipiv);
    free(indxr);
    free(indxc);
    
    /* Copy output vector */
    if (x)
        memmove(x, b, n*sizeof(GSDouble));    
    
    return GJ_NO_ERROR;
}
#undef SWAP
    


GSVectorElement_t yPointEstimatator_quadratic(GSVectorElement_t *threeXs, GSVectorElement_t *threeYs, GSVectorElement_t xOfInterest){
    /* This method applies a quadratic fit to the previous three points, and then uses the resulting polynomial */
    /* to estimate the y value for 'xOfInterest'.  This function should be easily expandible to a least squares */
    /* fit (with more than just 3 points) by modifying the precalculated fit matrix.                            */
    
    /* !! NB: THis function assumes the three points are linearly spaced so that it can use a precacluated mFit */
    
    /* Normalized x value  and associated x quadratic vector */
    GSVectorElement_t x;
   if (threeXs[1] - threeXs[0] >= threeXs[2] - threeXs[1]){
       x = (xOfInterest - threeXs[0])/(threeXs[1] - threeXs[0]) + 1.0;
   } else {
       x = (xOfInterest - threeXs[1])/(threeXs[2] - threeXs[1]) + 2.0;
   }
                                           
                                           
     //                                      
    GSVectorElement_t vX[3] = {1, x, x*x};
    
//    p/rintf("%f, %f, %f\n",threeYs[0],threeYs[1],threeYs[2]);
    
    /* Pre-calculated quadratic fit matrix. */
    GSVectorElement_t mFit[] = {  3.0,  -3.0,  1.0,
                                 -2.5,   4.0, -1.5,
                                  0.5,  -1.0,  0.5  };
    
    
    /* Get vector of polynomial coefficients */
    GSVectorElement_t a[3];
    GSVectorMatrixMultiplyD(a, threeYs, mFit, 3);
    
    /* Dot product with our x vector to get the expected next point */
    return GSVectorDotProductD(a, vX, 3);
}
















/*** Numerical recepies ****/
GSFloat gammln(GSFloat xx)
//Returns the value ln[Γ(xx)] for xx > 0. 
{
    GSDouble x,y,tmp,ser;
    static GSDouble cof[6]={76.18009172947146,-86.50532032941677,
        24.01409824083091,-1.231739572450155,
        0.1208650973866179e-2,-0.5395239384953e-5}; 
    GSInt j;
    y=x=xx;
    tmp=x+5.5;
    tmp -= (x+0.5)*log(tmp); ser=1.000000000190015;
    for (j=0;j<=5;j++) 
        ser += cof[j]/++y; 
    return -tmp+log(2.5066282746310005*ser/x);
}

#define ITMAX 1000
#define EPS 3.0e-7
GSErr gser(GSFloat *gamser, GSFloat a, GSFloat x, GSFloat *gln)
{
    //Returns the incomplete gamma function P (a, x) evaluated by its series 
    //representation as gamser. Also returns ln Γ(a) as gln.
    GSFloat sum,del,ap;
    GSInt n;
    *gln=gammln(a); 
    
    if (x == 0.0){
        *gamser = 0;
        return GSSuccess;
    }
    
    if (x < 0.0) {
        *gamser=0.0;
        return GSInvalidParameter;
    }
    
    
    ap=a;
    del=sum=1.0/a;
    for (n = 1; n <= ITMAX; n++) {
        ++ap;
        del *= x/ap;
        sum += del;
        if (fabs(del) < fabs(sum)*EPS) {
            *gamser=sum*exp(-x+a*log(x)-(*gln));
            return GSSuccess; 
        }
    }
    printf("a too large, ITMAX too small in routine gser"); 
    return GSFail;
}

#define FPMIN 1.0e-30
GSErr gcf(GSFloat *gammcf, GSFloat a, GSFloat x, GSFloat *gln)
//Returns the incomplete gamma function Q(a,x) evaluated by its continued fraction representation as gammcf. Also returns ln Γ(a) as gln.
{
    GSFloat an,b,c,d,del,h;
    GSInt i;
    *gln=gammln(a); b=x+1.0-a; c=1.0/FPMIN; d=1.0/b;
    h=d;
    for (i=1;i<=ITMAX;i++) {
        an = -i*(i-a);
        b += 2.0;
        d=an*d+b;
        if (fabs(d) < FPMIN) d=FPMIN; 
        c=b+an/c;
        if (fabs(c) < FPMIN) c=FPMIN; 
        d=1.0/d;
        del=d*c;
        h *= del;
        if (fabs(del-1.0) < EPS)
            break; 
    }
    if (i > ITMAX) 
        printf("a too large, ITMAX too small in gcf");
    *gammcf=exp(-x+a*log(x)-(*gln))*h; 
    return GSSuccess;
}



GSDouble GSGammaQ(GSFloat a, GSFloat x)
{
    //Returns the incomplete gamma function Q(a, x) ≡ 1 − P (a, x). {
    GSFloat gamser,gammcf,gln;
    if (x < 0.0 || a <= 0.0) 
        return GSInvalidParameter;
    
    if (x < a + 1.0){
        gser(&gamser, a, x, &gln); 
        return 1.0 - gamser;
    } else {
        gcf(&gammcf, a, x, &gln);
        return gammcf;
    } 
}


#define SWAP(a,b) {swap=(a);(a)=(b);(b)=swap;}
void covsrtD(GSDouble *covar, GSInt ma, GSInt ia[], GSInt mfit)
{
/* xpand in storage the covariance matrix covar, so as to take into account parameters that are being held fixed. (For the latter, return zero covariances.) */
    GSInt i,j,k; 
    GSDouble swap;
    
    /* First take covar, which is currently an mfit x mfit sized matrix, and space it out   */
    /* so that it's the top left corner of an ma x ma sized matrix. Zero all other elements */
    for (i = ma - 1; i >= 0; --i){
        for (j = ma - 1; j>= 0; --j){
            if (i >= mfit || j >= mfit){
                covar[i*ma + j] = 0.0;
            } else {
                covar[i*ma + j] = covar[i*mfit + j];
            }
        }
    }
    
    /* Then space out elements so that fixed parameters' rows and columns are all zero. */
    for (i = mfit; i < ma; i++){
        k = mfit - 1;
        for (j = ma - 1; j >= 0; j--) {
            if (ia[j]) {
                for (i = 0; i < ma; i++)
                    SWAP(covar[ma*i + k],covar[ma*i + j]) 
                    for (i = 0; i < ma; i++) 
                        SWAP(covar[ma*k + i],covar[ma*j + i]) 
                        k--;
            } 
        }
        
    }
}
#undef SWAP

void mrqcofD(GSDouble x[], GSInt xd, GSDouble y[], GSDouble sig[], GSInt ndata, GSDouble a[], GSInt ia[],
            GSInt ma, GSDouble *alpha, GSDouble beta[], GSDouble *chisq,
            void (*funcs)(GSDouble[], GSInt, GSDouble [], GSDouble *, GSDouble [], GSInt)
            )
{
    //Used by mrqmin to evaluate the linearized fitting matrix alpha, and vector beta as in (15.5.8), and calculate χ2.
    
    GSInt i,j,k,l,m,mfit=0;
    GSDouble ymod,wt,sig2i,dy,*dyda;
    
    //dyda=vector(1,ma); 
    dyda = malloc(sizeof(GSDouble)*ma);
    
    // Loop through parameter array and find number that we should fit
    for (j = 0;  j< ma; j++)
        if (ia[j]) 
            mfit++;
    
    // Initialize (symmetric) alpha, beta.
    for (j = 0; j < mfit; j++){ 
        for (k = 0; k <= j; k++) 
            alpha[mfit*j + k] = 0.0;
        beta[j] = 0.0;
    }
    
    // Initialize chi squared parameter
    *chisq = 0.0;
    
    // For debugging. Keep some record of dy's.
    GSDouble meandy, stddy, maxdy;
    meandy = stddy = maxdy = 0.0;
    
    /// Summation loop over all data
    for (i = 0; i < ndata; i++) { 
        (*funcs)(&(x[i*xd]), xd, a, &ymod, dyda, ma); 
        sig2i = 1.0/(sig[i]*sig[i]);
        dy=y[i]-ymod;
        for (j = 0,l = 0;l < ma; l++) {
            if (ia[l]) {
                wt = dyda[l]*sig2i;
                for (k = 0, m = 0; m <= l; m++)
                    if (ia[m]) {
                        alpha[mfit*j + k] += wt*dyda[m];
                        ++k;
                    }
                beta[j] += dy*wt;
                
                ++j;
            }
        }
        
        // Do debug stuff
        meandy += dy;
        stddy += pow(dy, 2.0);
        if (dy > maxdy) maxdy = dy;
        
        // And find chi squared
        *chisq += dy*dy*sig2i;
        if (isnan(*chisq)){
            printf("ERROR: Chisq became nan\n");
            break;
        }
    }
    
    // Calculate debug stuff
    meandy = meandy / ndata;
    stddy = sqrt(stddy/ndata - pow(meandy, 2.0));
    
    // Fill in the symmetric side
    for (j = 1; j < mfit; ++j)
        for (k = 0; k < j; k++) 
            alpha[mfit*k + j]=alpha[mfit*j + k]; 
    
    free(dyda);
}
    
GSErr GSMrqMinD(GSDouble x[], GSInt xd, GSDouble y[], GSDouble sig[], GSInt ndata, GSDouble a[], GSInt ia[], GSInt ma, GSDouble *covar, GSDouble *alpha, GSDouble *chisq,
              void (*funcs)(GSDouble[], GSInt, GSDouble [], GSDouble *, GSDouble [], GSInt), GSInt *numIterIN)
/*Levenberg-Marquardt method, attempting to reduce the value χ2 of a fit between a set of data points x[1..ndata][xd], y[1..ndata] with individual standard deviations sig[1..ndata], and a nonlinear function dependent on ma coefficients a[1..ma]. The input array ia[1..ma] indicates by nonzero entries those components of a that should be fitted for, and by zero entries those components that should be held fixed at their input values. The program re- turns current best-fit values for the parameters a[1..ma], and χ2 = chisq. The arrays covar[1..ma][1..ma], alpha[1..ma][1..ma] are used as working space during most iterations. Supply a routine funcs(x,a,yfit,dyda,ma) that evaluates the fitting function yfit, and its derivatives dyda[1..ma] with respect to the fitting parameters a at x. On the first call provide an initial guess for the parameters a, and set alamda<0 for initialization (which then sets alamda=.001). If a step succeeds chisq becomes smaller and alamda de- creases by a factor of 10. If a step fails alamda grows by a factor of 10. You must call this
    routine repeatedly until convergence is achieved. Then, make one final call with alamda=0,so that covar[1..ma][1..ma] returns the covariance matrix, and alpha the curvature matrix. 
    NB: The value in numIterIN will be taken as the maximum number of iterations to try.  Upon return this value will be replaced by the acutal number of iterations.
 (Parameters held fixed will return zero covariances.)
 */
{   
    GSFloat alamdaStr = -1;
    GSFloat *alamda = &alamdaStr; /* This is just so rest of code works without modification */
    
    
    GSInt j,k,l;
    static GSInt mfit;
    static GSDouble ochisq, *atry, *beta, *da, *oneda;
    
    /* Start iteratiom loop */
    static GSDouble lastChiSquared = -100;
    GSInt stopEvents = 0;
    for (GSInt numIter = 0; numIter < *numIterIN; ++numIter){
        
        /* Initialization */
        if (*alamda < 0.0) { 
            /* atry=vector(1,ma); beta=vector(1,ma); da=vector(1,ma); */
            atry = malloc(sizeof(GSDouble)*ma);
            beta = malloc(sizeof(GSDouble)*ma);
            da = malloc(sizeof(GSDouble)*ma);
            
            for (mfit = 0, j = 0; j < ma; j++)
                if (ia[j]) 
                    mfit++;
            
            /* oneda = matrix(1,mfit,1,1); */
            oneda = malloc(mfit*sizeof(GSDouble));
            *alamda=0.001;
            GSDouble *aForDebug;
            aForDebug = a;
            mrqcofD(x, xd, y,sig,ndata,a,ia,ma,alpha,beta,chisq,funcs); 
            ochisq=(*chisq);
            for (j = 0; j < ma; j++) 
                atry[j] = a[j]; 
        }
        
        /* Alter linearized fitting matrix, by augmenting diagonal elements */
        for (j = 0; j < mfit; j++) { 
            for (k = 0; k < mfit; k++) 
                covar[mfit*j + k] = alpha[mfit*j + k]; 
            covar[mfit*j + j] = alpha[mfit*j + j]*(1.0 + (*alamda)); 
            oneda[j] = beta[j];
        }
        
        // gaussj(covar,mfit,oneda,1); //Matrix solution.
        GSGaussJordanEliminationSolverD(covar, NULL, oneda, mfit);
        for (j = 0; j < mfit; j++) 
            da[j] = oneda[j];
        
        /* Once converged, evaluate covariance matrix. */
        if (*alamda == 0.0){ 
            if (numIterIN) *numIterIN = numIter - 1;

            covsrtD(covar,ma,ia,mfit); 
            covsrtD(alpha,ma,ia,mfit); 
            free(oneda);
            free(da);
            free(beta);
            free(atry);
            return GSSuccess;
        }
        
        /* Did the trial succeed? */
        for (j = 0, l = 0; l < ma; l++)
            if (ia[l]) 
                atry[l] = a[l] + da[j++]; 
        mrqcofD(x,xd,y,sig,ndata,atry,ia,ma,covar,da,chisq,funcs);
        GSDouble newChisq = *chisq;
        if (*chisq < ochisq) {
            /*Success, accept the new solution. */
            *alamda *= 0.1; 
            ochisq=(*chisq);
            for (j = 0; j < mfit; j++) {
                for (k = 0; k < mfit; k++) 
                    alpha[mfit*j + k] = covar[mfit*j + k];
                beta[j]=da[j]; 
            }
            for (l = 0; l < ma; l++) 
                a[l]=atry[l];
        } else { 
            /* Failure, increase alamda and return. */
            *alamda *= 10.0;
            *chisq=ochisq; 
        }
     
        
        /* Test if we should stop */
        if (newChisq <= lastChiSquared){ /* The weakest test for stopping */
            /* Get change in chi squared */
            GSDouble deltaChiSquared = lastChiSquared - newChisq;
            
            /* Check absolute value */
            if (deltaChiSquared < 0.01)
                ++stopEvents;
            
            /* Check fractional value */
            else if (deltaChiSquared / lastChiSquared < 0.001)
                ++stopEvents;
            
            /* Check number of stop events */
            if (stopEvents == 2){
                *alamda = 0.0; /* Signal stop */
            }
        }
        lastChiSquared = newChisq;
    }
    
    /* Should never get here. Likely means we failed to converge quickly enough. */
    return GSFail;
}
