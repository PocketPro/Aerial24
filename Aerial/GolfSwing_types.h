//
//  GolfSwing_types.h
//  GolfSwingKit
//
//  Created by PPG Technologies on 7/17/11.
//  Copyright 2011 PPG Technologies. All rights reserved.
//


#ifndef GOLFSWING_TYPES_H
#define GOLFSWING_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include "GolfSwing_errors.h"

#define true 1
#define false 0

/* Primitive Types */
#pragma mark Primitive Types
typedef float		GSFloat;
typedef int			GSInt;
typedef int16_t	    GSInt16;
typedef uint16_t    GSUInt16;
typedef int8_t		GSInt8;
typedef uint8_t		GSUInt8;
typedef uint32_t    GSUInt32;
typedef int32_t     GSInt32;
typedef double		GSDouble;
typedef long        GSLong;

typedef GSInt       GSBool;
typedef GSInt		GSTimestamp_t;
typedef GSDouble	GSVectorElement_t;
typedef GSInt16     GSIMUSampleType_t;

/* Calibration */
enum hardware_major_versions_e{
    MAJOR_VERSION_1 = 1,
    MAJOR_VERSION_2,
    MAJOR_VERSION_NUM
};

#pragma mark Calibration
typedef struct GSCalibrationParameters_t_{
	GSVectorElement_t mScale[9];
	GSVectorElement_t vOffset[3];
} GSCalibrationParameters_t;

/* Struct used in calibrations */
typedef struct __GSOrthorotationalCalibration_t GSOrthorotationalCalibration_t;

/* Often used vectors. Defined in GolfSwingKit.c */
extern GSVectorElement_t GSXAxis[3]; 
extern GSVectorElement_t GSYAxis[3]; 
extern GSVectorElement_t GSZAxis[3];
extern GSVectorElement_t GSXAxisNeg[3];
extern GSVectorElement_t GSYAxisNeg[3];
extern GSVectorElement_t GSZAxisNeg[3];

/* Raw Data */
#pragma mark Raw Data

enum raw_data_codes_e {
    RECORDS_DATA_TRANSFER                      = 0XFF,
    DEVICE_ID                                  = 0X01,
    SWING_UTC                                  = 0X02,
    IMPACT_TIME                                = 0x03,
    ACCEL_CALIBRATION                          = 0X04,
    GYRO_CALIBRATION                           = 0X05,
    IMU_DATA                                   = 0X06,
    MAG_CALIBRATION                            = 0X07,
    MAG_DATA                                   = 0X08,
    SENSOR_TEMPERATURE                         = 0x09,
    TARGET_LINE_AIM                            = 0x0A,
    SWING_SETUP                                = 0x0B,
    CLUB_SETTINGS                              = 0x0C,
};

/*** Alter these for different IMU data formats ***/
#pragma pack(push,1)
typedef struct GSIMUDataSample_t_{
	GSUInt8 timeStampFields[3];
	GSIMUSampleType_t accel_x;
	GSIMUSampleType_t accel_y;
	GSIMUSampleType_t accel_z;
	GSIMUSampleType_t gyro_x;
	GSIMUSampleType_t gyro_y;
	GSIMUSampleType_t gyro_z;
}GSIMUDataSample_t;

typedef struct GSMagDataSample_t_{
    GSUInt8 timeStampFields[3];
    GSIMUSampleType_t mag_x;
    GSIMUSampleType_t mag_y;
    GSIMUSampleType_t mag_z;
}GSMagDataSample_t;

/*** Records ***/
/* Each record is encoded in the following format:     */
/*                                                     */
/* | record_code_byte | size_lsb | size_msb |          */
/* | data_byte_1      | data_b2  | data_b3  | ...      */
/*                                                     */
/* size_lsb/msb define the data length (not including  */
/* size or code. This must be a multiple of three, so  */
/* that each record, and the entire transfer is a      */
/* multiple of three (this is the kalimba word size    */
typedef struct GSRawDataRecords_t_{
    struct GSIMUDataRecord_t{
        GSUInt16 size;
        GSIMUDataSample_t samples[0];
    } *IMUSamples;
    
    struct GSMagDataRecord_t{
        GSUInt16 size;
        GSMagDataSample_t samples[0];
    } *magSamples;
    
    struct GSDeviceIDRecord_t{
        GSUInt16 size;
        GSUInt8 deviceIdData[6];
    } *deviceID;
    
    struct GSUTCDataRecord_t{
        GSUInt16 size;
        GSUInt32 seconds;
        GSUInt8 padding[2];
    } *UTC;
    
    struct GSImpactTimestampDataRecord_t{
        GSUInt16 size;
        GSUInt8  timeStampFields[3];
    } *impactTimestamp;
    
    struct GSAccelCalibrationRecord_t{
        GSUInt16 size;
        GSInt32 mScale[9]; /* This is acutally a float, but we have to dereference it as  */
        GSInt32 vOffset[3];/* an integer on the iphone to avoid a mis-alignment bus error */
    } *accelCalibration;   
    
    struct GSGyroCalibrationRecord_t{
        GSUInt16 size;
        GSInt32 mScale[9];  /* This is acutally a float, but we have to dereference it as  */
        GSInt32 vOffset[3]; /* an integer on the iphone to avoid a mis-alignment bus error */
    } *gyroCalibration;   
    
    struct GSMagCalibrationRecord_t{
        GSUInt16 size;
        GSInt32 mScale[9]; /* This is acutally a float, but we have to dereference it as  */
        GSInt32 vOffset[3];/* an integer on the iphone to avoid a mis-alignment bus error */
    } *magCalibration;   
    
    struct GSSensorTemperaturesRecord_t{
        GSUInt16 size;
        GSUInt8 accel;
        GSUInt8 gyro;
        GSUInt8 mag;
    } *sensorTemperatures;
    
    struct GSTargetLineAimRecord_t{
        GSUInt16 size;
        GSUInt8 secondsBeforeSwing[3];
        GSIMUDataSample_t IMUSample;
        GSMagDataSample_t magSample;
    } *targetLineAim;
    
    struct GSSwingSetupRecord_t{
        GSUInt16 size;
        
        GSUInt8 secondsAgo[3];
        
        GSUInt8 padding;
        GSUInt16 numIMUSamples; /* Transferred in little endian */
        GSIMUSampleType_t accel_mean_x;
        GSIMUSampleType_t accel_mean_y;
        GSIMUSampleType_t accel_mean_z;
        GSIMUSampleType_t gyro_mean_x;
        GSIMUSampleType_t gyro_mean_y;
        GSIMUSampleType_t gyro_mean_z;
        
        GSUInt8 padding1;
        GSUInt16 numMagSamples; /* Transferred in little endian */
        GSIMUSampleType_t mag_mean_x;
        GSIMUSampleType_t mag_mean_z;
        GSIMUSampleType_t mag_mean_y;
        
        GSIMUSampleType_t accel_varp_x; //Population variance (biased)
        GSIMUSampleType_t accel_varp_y;
        GSIMUSampleType_t accel_varp_z;
        GSIMUSampleType_t gyro_varp_x;
        GSIMUSampleType_t gyro_varp_y;
        GSIMUSampleType_t gyro_varp_z;
        
        GSIMUSampleType_t mag_varp_x;
        GSIMUSampleType_t mag_varp_z;
        GSIMUSampleType_t mag_varp_y;
        
    } *swingSetup;
    
    struct GSClubSettingsRecord_t{
        GSUInt16 size;

        GSInt16 ID;
        GSInt32 length; /* The mClipToBody matrix can have the face normal baked into it. If this */
        GSInt32 mClipToHead[9]; /* is the case, the bottom row will have magnitude of 2.0. */
    } *clubSettings;
}GSRawDataRecords_t;

#pragma pack(pop)


/* Helpers */
static inline GSTimestamp_t getTimeLong(const GSUInt8 timeStampFields[]){
	GSTimestamp_t acc = (GSInt)timeStampFields[0];
	acc = acc << 8;
	acc = acc | (GSInt)timeStampFields[1];
	acc = acc << 8;
	acc = acc | (GSInt)timeStampFields[2];
	return acc;
}


typedef struct GSIndices_t_{
	GSInt swingStart;
	GSInt topOfBackswing;
	GSInt impact;
	GSInt swingEnd;
    
    GSInt accelCappingStart_accel_y;
    GSInt accelCappingStart_accel_x;
    GSInt accelCappingStart;
} GSIndices_t;

typedef struct GSCalibratedIMUDataSample_t_{
    GSTimestamp_t timestamp;
    
    GSVectorElement_t vAccel[3];
    GSVectorElement_t vGyro[3];
}GSCalibratedIMUDataSample_t;


/* Club State */
#pragma mark Club State 
/* NB: window struct returns -1 as the index fraction if the requsted window is invalid */
typedef struct GSLinearInterpolationWindow_t_ {
	GSInt lowerIndex;
	GSInt upperIndex;
	GSFloat indexFraction;
} GSLinearInterpolationWindow_t_;

typedef struct GSClubMeasurements_t_{
    GSFloat length;              /* Distance from accelerometer to leading edge */
    GSFloat manufacturedLoft;    
    GSFloat manufacturedLie;
    GSFloat centerFaceOffsetX;   /* Distance from hosel to center face along leading edge     */
    GSFloat centerFaceOffsetZ;   /* Vertical height of center face from leading edge.         */
                                 /* This is NOT the distance along the face fromt he leading  */
                                 /* edge to center face because of the loft.                  */
    GSFloat leadingEdgeOffsetY;  /* Perpendicular distance from the leading edge to the hosel */
} GSClubMeasurements_t;

static inline GSErr GSCheckClubMeasurements(GSClubMeasurements_t clubMeasurements){
    if (clubMeasurements.length > 2.5 || clubMeasurements.length < 0.2)
		return GSInvalidClubLength;
    if (clubMeasurements.manufacturedLoft < 0.0 || clubMeasurements.manufacturedLoft > 1.31) /* 75.0° */
        return GSInvalidClubLoft;
    if (clubMeasurements.manufacturedLie < 0.0 || clubMeasurements.manufacturedLie > 1.57) /* 90° */
        return GSInvalidClubLie;
    if (clubMeasurements.centerFaceOffsetX < 0.01 || clubMeasurements.centerFaceOffsetX > 0.10)
        return GSInvalidClubCenterFaceOffsetX;
    if (clubMeasurements.centerFaceOffsetZ < 0.0 || clubMeasurements.centerFaceOffsetZ > 0.10)
        return GSInvalidClubCenterFaceOffsetZ;
    if (clubMeasurements.leadingEdgeOffsetY < -0.05 || clubMeasurements.leadingEdgeOffsetY > 0.05)
        return GSInvalidClubLeadingEdgeOffsetY;
        
    return GSSuccess;
}

typedef struct GSClubStateElement_t_{
	GSTimestamp_t timestamp;
	
	/* Club Movement */
	GSVectorElement_t mBodyToLab[9];
	GSVectorElement_t vbRotationRate[3];
	GSVectorElement_t vlClipPosition[3];
	GSVectorElement_t vlClipPositionCorrected[3];
	GSVectorElement_t vlClipVelocity[3];  
	GSVectorElement_t vlClipVelocityCorrected[3];
	GSVectorElement_t vlClipAcceleration[3];
	
	/* Club Deflection */
	GSVectorElement_t mLabToDeflection[9];
	GSVectorElement_t vdDeflectionPosition[3];
	GSVectorElement_t vdDeflectionVelocity[3];
	GSVectorElement_t vdDeflectionAcceleration[3];
    
    /* Path Distance */
    GSVectorElement_t clubheadPathDistance;
} GSClubStateElement_t;


typedef struct GSSwingTimestamps_t_{
	GSTimestamp_t swingStart;
    GSTimestamp_t midBackswing;
	GSTimestamp_t topOfBackswing;
    GSTimestamp_t midForwardswing;
	GSTimestamp_t impact;
	GSTimestamp_t swingEnd;
    
    GSTimestamp_t accelYCappingStart;
}GSSwingTimestamps_t;


/* Calculation metrics */
#pragma mark Calculation Metrics

enum target_line_method_e{
    TARGET_LINE_UNKNOWN = 0,
    TARGET_LINE_GRAVITY_Y_CROSS,
    TARGET_LINE_CLUB_POINTING,
    TARGET_LINE_CLUB_SETUP_TAP,
    TARGET_LINE_FACE_NORMAL,
    TARGET_LINE_LEADING_EDGE_NORMAL
};

enum clip_to_body_method_e{
    CLIP_TO_BODY_UNKNOWN = 0,
    CLIP_TO_BODY_SETUP_TAP,
    CLIP_TO_BODY_GRAVITY_Y_CROSS,
    CLIP_TO_BODY_SWING_PLANE,
    CLIP_TO_BODY_CLIP_ON_TOP,
    CLIP_TO_BODY_CLIP_ON_BOTTOM,
    CLIP_TO_BODY_CLUB_SETTINGS_IN_TRANSFER
};

typedef struct GSSensorChannelsBitfield_t_ {
    unsigned int accel_x            : 1;
    unsigned int accel_y            : 1;
    unsigned int accel_z            : 1;
    unsigned int gyro_x             : 1;
    unsigned int gyro_y             : 1;
    unsigned int gyro_z             : 1;
    unsigned int mag_x              : 1;
    unsigned int mag_y              : 1;
    unsigned int mag_z              : 1; 
} GSSensorChannelsBitfield_t; 

typedef struct GSSensorBitfield_t_ {
    unsigned int accel            : 1;
    unsigned int gyro             : 1;
    unsigned int mag              : 1;
} GSSensorBitfield_t; 


typedef struct GSCalculationMetrics_t_ {
    GSSensorBitfield_t customScaleCalibrationChannels; 
    GSSensorBitfield_t customOffsetCalibrationChannels; 
    GSSensorChannelsBitfield_t cappingCorrectedChannels; 
    
    GSFloat setupGravityMagnitude;
    GSFloat setupMagnetometerMagnitude;
    GSFloat setupMagnetometerGravityAngle;
    
    GSFloat startGravityMagnitude;
    GSFloat startMagnetometerMagnitude;
    GSFloat startMagnetometerGravityAngle;
    
    GSFloat magnometerGravityAngleDifference;
    GSFloat magnometerCalibrationSquaredResidualsSum;
    
    GSFloat kAssumedValueOfGravity;
    
    GSFloat accelCorrectionConvergedRadius;
    GSFloat accelCorrectionConvergedRadiusLastDiff;
    
    enum target_line_method_e targetLineMethod;
    enum clip_to_body_method_e clipToBodyMethod;

    
    /* Misc flags */
    unsigned int impactTimeMismatch             :1;
    unsigned int bMeasuredFaceNormal            :1;
    unsigned int bForcedSwingStartTime          :1;
    unsigned int bForcedImpactTime              :1;
} GSCalculationMetrics_t;


#pragma mark Impact Quality
/* Impact location */
enum impact_location_e_{
	GSImpactUnknown,
	GSImpactHeelHeavy,
	GSImpactHeelLight,
	GSImpactNeutral,
	GSImpactToeLight,
	GSImpactToeHeavy
};

enum GSFat_e{
    GS_FAT_NOT          = 0,
    GS_FAT_SLIGHTLY     = 1,
    GS_FAT_VERY         = 2,
    GS_FAT_UNSURE       = 3,
};

typedef struct GSImpactQuality_t_{
    unsigned int fat                            :2; /* See enum fat_e_ */
}GSImpactQuality_t;

typedef struct GSWarnings_t_{
    unsigned int bDetectedCappingButCouldntFixWithHandFit :1; 
}GSWarnings_t;

/* Swing */
#pragma mark Swing 
typedef struct __GSSwingInternalStatistics GSSwingInternalStatistics_t;
typedef struct GSSwing_t_{
    GSRawDataRecords_t rawDataRecords;
	GSCalibrationParameters_t calibrationAccel;
	GSCalibrationParameters_t calibrationGyro;
    GSCalibrationParameters_t calibrationMag;
    GSClubMeasurements_t clubMeasurements;
	
	/* Preprocessing swing */
	GSInt numIMUDataSamples;  /* These two parameters are left in */
	GSIndices_t indices;      /* for backwards compatiblity       */
	GSInt bImpactFound;
	GSVectorElement_t mClipToBody[9];
    GSVectorElement_t mBodyToDeflection[9];
    GSVectorElement_t vbFaceNormal[3];
	GSVectorElement_t vbClub[3];
	GSVectorElement_t startAccelMean[3];
    GSVectorElement_t startGyroMean[3];
    GSVectorElement_t startMagMean[3];
    
	
	/* Calculated swing state data */
	GSClubStateElement_t *clubStateTimeline;
	GSInt numClubStateElements;
    GSInt clubStateTimelineImpactIndex;
	GSInt clubStateTimelinePeriod;
    
    /* Misc data */
	GSSwingTimestamps_t swingTimestamps;
    GSCalculationMetrics_t calculationMetrics;
    GSVectorElement_t vlNormalAddressSwingPlane[3];
    
    /* Impact quality */
    GSImpactQuality_t impactQuality;
    
    /* Warnings */
    GSWarnings_t warnings;
	
	/* Parameters */
    struct __GSParameterCache_t *paramCache; //pointer to a param cache struct
	
	GSFloat tempCL;
    GSTimestamp_t forcedStartTime;
    GSTimestamp_t forcedImpactTime;

}GSSwing_t;


enum GSSwingPart_e{
    SWING_PART_BACKSWING = 0,
    SWING_PART_DOWNSWING,
    SWING_PART_FOLLOW_THROUGH,
    SWING_PART_NUM
};

#endif //GOLFSWING_TYPES_H

