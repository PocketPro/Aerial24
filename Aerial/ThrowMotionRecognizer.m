//
//  ThrowMotionRecognizer.m
//  Aerial
//
//  Created by Gord Parke on 4/29/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "ThrowMotionRecognizer.h"
#import "MotionRecognizerSubclass.h"

// Physical constants
#define kThrowAccelDerivMin     40     /* g/s */
#define kFreeFallAccelDerivMin  20     /* g/s */
#define kThrowProbationPeriod   0.25    /* Seconds */

@interface ThrowMotionRecognizer () {
    GSFloat lastDerivMag, secondLastDerivMag, thirdLastDerivMag;
    
    NSTimeInterval possibleThrowTime, confirmedThrowTime;
    
    MotionSample_t *throwStartSample, *throwConfirmedSample;
}
@end


@implementation ThrowMotionRecognizer

#pragma mark - Motion Analysis
- (void)analyzeNewMotionSample:(MotionSample_t *)newSample
{    
    // Get magnitude of acceleration in newSample
    GSFloat derivMag = GSVectorMagnitudeD(newSample->vbAccelerationDerivative, 3);
    
    // Skip the rest if we dont' have enough samples yet
    if (lastDerivMag && secondLastDerivMag && thirdLastDerivMag){
        MotionRecognizerState state = self.state;
        switch (state) {
                
                // Record the possible start of a throw when acceleration falls below threshold
            case MotionRecognizerStatePossible:
                if (derivMag < kFreeFallAccelDerivMin && (
                    lastDerivMag > kThrowAccelDerivMin ||
                    secondLastDerivMag > kThrowAccelDerivMin ||
                    thirdLastDerivMag > kThrowAccelDerivMin)){
                    
                    possibleThrowTime = newSample->timestamp;
                    throwStartSample = newSample;
                    self.state = MotionRecognizerStateBegan;
                }
                break;
                
                // Cancel throw if the mag jumps back over the threshold within the probation period
                // otherwise, move to state ended
            case MotionRecognizerStateBegan:
                if (derivMag > kFreeFallAccelDerivMin){
                    possibleThrowTime = 0;
                    self.state = MotionRecognizerStateCancelled;
                } else if (newSample->timestamp - possibleThrowTime >= kThrowProbationPeriod) {
                    confirmedThrowTime = possibleThrowTime;
                    throwConfirmedSample = newSample;
                    self.state = MotionRecognizerStateEnded;
                }
                break;
                
            default:
                break;
        }
    }
    
    // Update previous ones
    thirdLastDerivMag = secondLastDerivMag;
    secondLastDerivMag = lastDerivMag;
    lastDerivMag = derivMag;
}

#ifdef INCLUDES_PLOTTING
#pragma mark - Plotting
// Delegate method that returns the number of points on the plot
static const NSInteger beforePaddingSamples =  50;
-(NSUInteger)numberOfRecordsForPlot:(CPTPlot *)plot userInfo:(NSDictionary *)dictionary
{
    if ( [plot.identifier isEqual:@"plot-1"]  )
    {
        if (throwConfirmedSample && throwStartSample)
            return [self.motionTimeline numberOfSamplesBetweenStart:throwStartSample end:throwConfirmedSample] + 
            beforePaddingSamples;
        else 
            return 0;
    }
    
    return 0;
}

// Delegate method that returns a single X or Y value for a given plot.
-(NSNumber *)numberForPlot:(CPTPlot *)plot field:(NSUInteger)fieldEnum recordIndex:(NSUInteger)index userInfo:(NSDictionary *)userInfo
{
    if ( [plot.identifier isEqual:@"plot-1"] )
    {        
        // Get motion sample and index
        MotionSample_t *sample = [self.motionTimeline sampleForNumber:index - beforePaddingSamples ofSamplesAfter:throwStartSample];
        
        // FieldEnum determines if we return an X or Y value.
        if ( fieldEnum == CPTScatterPlotFieldX )
        {
            return [NSNumber numberWithDouble:(sample->timestamp - throwStartSample->timestamp) * 1E3];
        }
        else    // Y-Axis
        {
            NSString *title = [userInfo objectForKey:@"SelectedSegmentTitle"];
            GSFloat mag;
            if ([title isEqualToString:@"Mag Deriv"]){
                mag = sample->vbAccelerationMagDerivative;
            }  else if ([title isEqualToString:@"Acceleration"]) {
                mag = GSVectorMagnitudeD(sample->vbAcceleration, 3);
            } else if ([title isEqualToString:@"Deriv Mag"]){
                mag = GSVectorMagnitudeD(sample->vbAccelerationDerivative, 3);
            }
            
            return [NSNumber numberWithFloat:mag];
        }
    }
    
    return [NSNumber numberWithFloat:0];
}
-(NSArray *)titlesForSegmentedControl
{
    return [NSArray arrayWithObjects:@"Acceleration", @"Mag Deriv", @"Deriv Mag", nil];
}
#endif

#pragma mark - State Entry
- (void)didEnterStateReset
{    
    possibleThrowTime = 0;
    confirmedThrowTime = 0;
    throwStartSample = nil;
    throwConfirmedSample = nil;
    lastDerivMag = 0;
    secondLastDerivMag = 0;
    thirdLastDerivMag = 0;
}

-(void)didEnterStateCancelled
{
    // Switch state to reset on the next run loop cycle
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        self.state = MotionRecognizerStateReset;
    }];
}

#pragma mark - State Exit

#pragma mark - Lifecycle

@end
