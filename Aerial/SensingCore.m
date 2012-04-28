//
//  SensingCore.m
//  Aerial
//
//  Created by Gord Parke on 4/28/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "SensingCore.h"
#import "MotionTimeline.h"
#import <CoreMotion/CoreMotion.h>
#import "GolfSwing_math.h"


static SensingCore *sharedInstance;

@interface SensingCore ()
@property (strong, nonatomic) CMMotionManager *motionManager;

@property NSTimeInterval sensorUpdateInterval;
@end

@implementation SensingCore
@synthesize motionTimeline = _motionTimeline;
@synthesize motionManager = _motionManager;
@synthesize sensorUpdateInterval = _sensorUpdateInterval;

#pragma mark - Sensing
- (void)handleNewGyroSample:(CMGyroData *)gyroData
{
    // Get accelerometer data
    CMAccelerometerData *accelData = self.motionManager.accelerometerData;
    
    // Create new motion sample and set accel and gyro data
    MotionSample_t newMotionSample;
    
    newMotionSample.timestamp = gyroData.timestamp;
    newMotionSample.accelTimestamp = accelData.timestamp;
    
    newMotionSample.vbAngularVelocity[0] = gyroData.rotationRate.x;
    newMotionSample.vbAngularVelocity[1] = gyroData.rotationRate.y;
    newMotionSample.vbAngularVelocity[2] = gyroData.rotationRate.z;
    
    newMotionSample.vbAcceleration[0] = accelData.acceleration.x;
    newMotionSample.vbAcceleration[1] = accelData.acceleration.y;
    newMotionSample.vbAcceleration[2] = accelData.acceleration.z;
    
    // Now get the last sample, and set the delta variables
    MotionSample_t *lastMotionSample = [self.motionTimeline newestSample];
    if (lastMotionSample){
        // Calculate gyro dervative
        NSTimeInterval gyroInterval = newMotionSample.timestamp - lastMotionSample->timestamp;
        GSVectorSubtractD(newMotionSample.vbAngularVelocityDerivative, newMotionSample.vbAngularVelocity, lastMotionSample->vbAngularVelocity, 3);
        GSVectorScalarDivideD(newMotionSample.vbAngularVelocityDerivative, newMotionSample.vbAngularVelocityDerivative, gyroInterval, 3);
        
        // Calculate accel derivative, or set to 0 if there was no change
        NSTimeInterval accelInterval = newMotionSample.accelTimestamp - lastMotionSample->accelTimestamp;
        if (accelInterval <= 0){
            GSVectorElement_t vZero[3] = {0.0, 0.0, 0.0};
            GSVectorCopyD(newMotionSample.vbAccelerationDerivative, vZero, 3);
        } else {
            GSVectorSubtractD(newMotionSample.vbAccelerationDerivative, newMotionSample.vbAcceleration, lastMotionSample->vbAcceleration, 3);
            GSVectorScalarDivideD(newMotionSample.vbAccelerationDerivative, newMotionSample.vbAccelerationDerivative, accelInterval, 3);
        }
        
        // Calculate change in body to lab matrix 
        // Get average angular velcoity over interval
        GSVectorElement_t vbAngularVelocityAverage[3];
        GSVectorAddD(vbAngularVelocityAverage, newMotionSample.vbAngularVelocity, lastMotionSample->vbAngularVelocity, 3);
        GSVectorScalarDivideD(vbAngularVelocityAverage, vbAngularVelocityAverage, 2.0, 3);
        
        // Multiply this by interval to get rotation
        GSVectorElement_t vbRotationOverInterval[3];
        GSVectorScalarMultiplyD(vbRotationOverInterval, vbAngularVelocityAverage, gyroInterval, 3);
        
        // Turn this into a rotation matrix and multipy our last matrix
        GSVectorElement_t mAdd[9];
        GSMatrixRotationFromVectorD(mAdd, vbRotationOverInterval, 0);
        GSMatrixMatrixMultiplyD(newMotionSample.mBodyToInitial, mAdd, lastMotionSample->mBodyToInitial, 3);
        
    } else {
        // This is the first motion sample.  Initialize parameters
        GSMatrixMakeIdentity(newMotionSample.mBodyToInitial, 3);
    
        // Zero out derivatives
        GSVectorElement_t vZero[3] = {0.0, 0.0, 0.0};
        GSVectorCopyD(newMotionSample.vbAccelerationDerivative, vZero, 3);
        GSVectorCopyD(newMotionSample.vbAngularVelocityDerivative, vZero, 3);
    }
    
    // Add motion sample to timeline
    [self.motionTimeline addSample:&newMotionSample];
}
- (void)startSensing
{
    // Zero out motion timeline
    self.motionTimeline.count = 0;
    
    // Gyro-centric sampling
    [self.motionManager startAccelerometerUpdates];
    [self.motionManager startGyroUpdatesToQueue:[NSOperationQueue mainQueue] withHandler:^(CMGyroData *gyroData, NSError *error) {
        
        // Check error
        if (error){
            [[[UIAlertView alloc] initWithTitle:@"Error" message:[error localizedDescription] delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil] show];
            [self stopSensing];
        }
        
        // Send off new data to our handling function
        [self handleNewGyroSample:gyroData];
    }];
}
- (void)stopSensing
{   
    [self.motionManager stopAccelerometerUpdates];
    [self.motionManager stopGyroUpdates];
}

#pragma mark - Lifecycle
-(void)configureMotionManager
{
    // Check that we can access accel and gyroscope.  For now shut down the app if we can't.
    if (!self.motionManager.gyroAvailable){
        [[[UIAlertView alloc] initWithTitle:@"Gyroscope Unavailable" message:@"This app requires a gyroscope"delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil] show];
        abort();
    }
    if (!self.motionManager.accelerometerAvailable){
        [[[UIAlertView alloc] initWithTitle:@"Accelerometer Unavailable" message:@"This app requires an accelerometer"delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil] show];
        abort();
    }
    
    // Set sampling rates
    self.sensorUpdateInterval = 0.01;
    self.motionManager.gyroUpdateInterval = self.sensorUpdateInterval;
    self.motionManager.accelerometerUpdateInterval = self.sensorUpdateInterval;
}

+ (SensingCore *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[SensingCore alloc] init];
        
        // Create an initial timeline
        sharedInstance.motionTimeline = [[MotionTimeline alloc] init];
        
        // Create the CMMotionManager.  There should only be one CMMotionManager in the application
        sharedInstance.motionManager = [[CMMotionManager alloc] init];
        [sharedInstance configureMotionManager];
    });
    return sharedInstance;
}

@end
