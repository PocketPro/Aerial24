//
//  CatchMotionRecognizer.m
//  Aerial
//
//  Created by Gord Parke on 4/30/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "CatchMotionRecognizer.h"
#import "MotionRecognizerSubclass.h"

#define kNotFreeFallMag         0.5  /* g's */
#define kCatchDuration          0.5  /* Seconds */
#define kMinFumbleDuration      0.1  /* Second */
#define kMaxFumbleAfterImpact     4  /* g's */


@interface CatchMotionRecognizer () {
    // Times
    NSTimeInterval catchTime;
    NSTimeInterval possibleFumbleStartTime;
    
    // Position in timeline
    MotionSample_t *catchSampleStart, *catchSampleEnd;
}
// Redefinitions to read-writes
@property (nonatomic, readwrite, getter = isFumble) BOOL fumble;
@end


@implementation CatchMotionRecognizer
@synthesize fumble = _fumble;

#pragma mark - Motion Analysis

- (void)analyzeNewMotionSample:(MotionSample_t *)newSample
{    
    // Get magnitude of acceleration in newSample
    GSFloat mag = GSVectorMagnitudeD(newSample->vbAcceleration, 3);
    
    MotionRecognizerState state = self.state;
    switch (state) {
            
            // Record the possible start of a throw when acceleration falls below threshold
        case MotionRecognizerStatePossible:
            if (mag > kNotFreeFallMag){
                catchTime = newSample->timestamp;
                catchSampleStart = newSample;
                self.state = MotionRecognizerStateBegan;
            }
            break;
            
            // End catch after catch interval
        case MotionRecognizerStateBegan:
            // If the acceleration drops back into the free fall zone it's a fumble
            if (mag < 0.8*kNotFreeFallMag && possibleFumbleStartTime == 0){
                possibleFumbleStartTime = newSample->timestamp;
            } else if (mag > kMaxFumbleAfterImpact && possibleFumbleStartTime != 0){
                if (newSample->timestamp - possibleFumbleStartTime >= kMinFumbleDuration)
                    self.fumble = YES;
            }
            
            
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
-(NSUInteger)numberOfRecordsForPlot:(CPTPlot *)plot
{
    if ( [plot.identifier isEqual:@"mainplot"]  )
    {
        if (catchSampleStart && catchSampleEnd)
            return [self.motionTimeline numberOfSamplesBetweenStart:catchSampleStart end:catchSampleEnd];
        else 
            return 0;
    }
    
    return 0;
}

// Delegate method that returns a single X or Y value for a given plot.
-(NSNumber *)numberForPlot:(CPTPlot *)plot field:(NSUInteger)fieldEnum recordIndex:(NSUInteger)index
{
    if ( [plot.identifier isEqual:@"mainplot"] )
    {        
        // Get motion sample and index
        MotionSample_t *sample = [self.motionTimeline sampleForNumber:index ofSamplesAfter:catchSampleStart];
        
        // FieldEnum determines if we return an X or Y value.
        if ( fieldEnum == CPTScatterPlotFieldX )
        {
            return [NSNumber numberWithDouble:(sample->timestamp - catchSampleStart->timestamp) * 1E3];
        }
        else    // Y-Axis
        {
            GSFloat mag = GSVectorMagnitudeD(sample->vbAcceleration, 3);
            return [NSNumber numberWithFloat:mag];
        }
    }
    
    return [NSNumber numberWithFloat:0];
}
#endif

#pragma mark - State Entry
- (void)didEnterStateReset
{    
    catchTime = 0;
    possibleFumbleStartTime = 0;
    catchSampleStart = nil;
    catchSampleEnd = nil;
    self.fumble = NO;
}

#pragma mark - State Exit

#pragma mark - Lifecycle

@end
