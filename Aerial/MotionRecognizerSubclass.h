//
//  MotionRecognizerSubclass.h
//  Aerial
//
//  Created by Gord Parke on 4/29/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "GolfSwing_math.h"

@interface MotionRecognizer (MotionRecognizerSubclass)
// Reveal some properites as readwrite in subclasses
@property (nonatomic, readwrite) MotionRecognizerState state;

// Called to analyze new motion data
- (void)analyzeNewMotionSample:(MotionSample_t *)newSample;

// This is one of many state change handling methods.  However
// Reset is usually required to initialize/reset all analysis variables
- (void)enteringStateReset;

@end
