//
//  CatchMotionRecognizer.m
//  Aerial
//
//  Created by Gord Parke on 4/30/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "CatchMotionRecognizer.h"
#import "MotionRecognizerSubclass.h"

#define kNotFreeFallDerivMag     20  /* g's */
#define kCatchDuration          0.5  /* Seconds */
#define kMinFumbleDuration      0.1  /* Second */
#define kMaxFumbleAfterImpact     4  /* g's */


@interface CatchMotionRecognizer () {
    // Times
    NSTimeInterval catchTime;
    
    // Position in timeline
    MotionSample_t *catchSampleStart, *catchSampleEnd;
}
// Redefinitions to read-writes
@property (nonatomic, readwrite, getter = isFumble) BOOL fumble;
@end


@implementation CatchMotionRecognizer
@synthesize fumble = _fumble;
@synthesize catchSample = catchSampleStart;

#pragma mark - Motion Analysis

- (void)analyzeNewMotionSample:(MotionSample_t *)newSample
{    
    // Get magnitude of acceleration in newSample
    GSFloat derviMag = GSVectorMagnitudeD(newSample->vbAccelerationDerivative, 3);
    
    // Get last sample
    MotionSample_t *lastSample = [self.motionTimeline sampleAtPastIndex:1];
    if (!lastSample) return;
    
    MotionRecognizerState state = self.state;
    switch (state) {
            
            // Record the possible start of a throw when acceleration falls below threshold
        case MotionRecognizerStatePossible:
            if (derviMag > kNotFreeFallDerivMag){
                catchTime = lastSample->timestamp;
                catchSampleStart = lastSample;
                self.state = MotionRecognizerStateBegan;
            }
            break;
            
            // End catch after catch interval
        case MotionRecognizerStateBegan:
            
            // Check if the catch is over
            if (newSample->timestamp - catchTime >= kCatchDuration){
                catchSampleEnd = newSample;
                self.state = MotionRecognizerStateEnded;
            }
            break;
            
        default:
            NSAssert(0,@"Invalid state in analyzeNewMotionSample in %@",NSStringFromClass([self class]));
    }
}

#ifdef INCLUDES_PLOTTING
#pragma mark - Plotting
// Delegate method that returns the number of points on the plot
static const NSInteger beforePaddingSamples =  50;
-(NSUInteger)numberOfRecordsForPlot:(CPTPlot *)plot userInfo:(NSDictionary *)dictionary
{
    if ( [plot.identifier isEqual:@"plot-1"]  )
    {
        if (catchSampleStart && catchSampleEnd)
            return [self.motionTimeline numberOfSamplesBetweenStart:catchSampleStart end:catchSampleEnd] + 
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
        MotionSample_t *sample = [self.motionTimeline sampleForNumber:index - beforePaddingSamples ofSamplesAfter:catchSampleStart];
        
        // FieldEnum determines if we return an X or Y value.
        if ( fieldEnum == CPTScatterPlotFieldX )
        {
            return [NSNumber numberWithDouble:(sample->timestamp - catchSampleStart->timestamp) * 1E3];
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
    catchTime = 0;
    catchSampleStart = nil;
    catchSampleEnd = nil;
    self.fumble = NO;
}

#pragma mark - State Exit

#pragma mark - Lifecycle

@end
