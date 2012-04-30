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
#define kFreeFallMag            0.5     /* g's */
#define kThrowProbationPeriod   0.25    /* Seconds */

@interface ThrowMotionRecognizer () {
    NSTimeInterval possibleThrowTime, confirmedThrowTime;
}
@end


@implementation ThrowMotionRecognizer

#pragma mark - Motion Analysis
- (void)analyzeNewMotionSample:(MotionSample_t *)newSample
{    
    // Get magnitude of acceleration in newSample
    GSFloat mag = GSVectorMagnitudeD(newSample->vbAcceleration, 3);
    
    MotionRecognizerState state = self.state;
    switch (state) {
            
        // Record the possible start of a throw when acceleration falls below threshold
        case MotionRecognizerStatePossible:
            if (mag < kFreeFallMag){
                possibleThrowTime = newSample->timestamp;
                self.state = MotionRecognizerStateBegan;
            }
            break;
            
       // Cancel throw if the mag jumps back over the threshold within the probation period
       // otherwise, move to state ended
        case MotionRecognizerStateBegan:
            if (mag > kFreeFallMag){
                possibleThrowTime = 0;
                self.state = MotionRecognizerStateCancelled;
            } else if (newSample->timestamp - possibleThrowTime >= kThrowProbationPeriod) {
                confirmedThrowTime = possibleThrowTime;
                self.state = MotionRecognizerStateEnded;
            }
            break;
            
        default:
            break;
    }
}

#pragma mark - State Entry
- (void)didEnterStateReset
{    
    possibleThrowTime = 0;
    confirmedThrowTime = 0;
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
