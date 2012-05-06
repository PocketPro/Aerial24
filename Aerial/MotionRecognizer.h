//
//  MotionRecognizer.h
//  Aerial
//
//  Created by Gord Parke on 4/28/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MotionTimeline.h"

// Plotting (for debugging)
#define INCLUDES_PLOTTING
    #ifdef INCLUDES_PLOTTING
    #import "CorePlot-CocoaTouch.h"
    #endif

// State must only transition to a higher value (until it is reset) 
typedef enum {
    MotionRecognizerStateStartup = 0,
    
    // Reset.  Entering this state should reset the motion recognizer
    MotionRecognizerStateReset,
    
    // Recognizer is waiting for another gesture recognizer
    MotionRecognizerStateWaiting,
    
    // Recognizer is actively monitoring data and looking for its motion
    MotionRecognizerStatePossible,
    
    // Continous or multi-step recognizer has recognized its motion:
    MotionRecognizerStateBegan,         // Motion initially recognized
    MotionRecognizerStateChanged,       // Motion changes after being recognized
    MotionRecognizerStateCancelled,     // Recognizer was stopped prematured during motion analysis
    MotionRecognizerStateEnded,         // Recognizer has finished analyzing motion with success
    
    // Recognizer has rejected motion
    MotionRecognizerStateFailed,        
    
    MotionRecognizerStateCount
} MotionRecognizerState;


// State names
static const char* const MotionRecognizerStateNames[] = {
    [MotionRecognizerStateStartup]      = "Startup",
    [MotionRecognizerStateReset]        = "Reset",
    [MotionRecognizerStateWaiting]      = "Waiting",
    [MotionRecognizerStatePossible]     = "Possible",
    [MotionRecognizerStateBegan]        = "Began",
    [MotionRecognizerStateChanged]      = "Changed",
    [MotionRecognizerStateCancelled]    = "Cancelled",
    [MotionRecognizerStateEnded]        = "Ended",
    [MotionRecognizerStateFailed]       = "Failed"
};


@interface MotionRecognizer : NSObject
@property (nonatomic, readonly, weak) MotionTimeline *motionTimeline;
@property (nonatomic, readonly) MotionRecognizerState state;
@property (nonatomic, getter=isEnabled) BOOL enabled;

// Relationships
- (void)requireMotionRecognizer:(MotionRecognizer *)recognizer toAchieveState:(MotionRecognizerState)minState;

// Motion Analysis
- (void)handleNewMotionSample:(MotionSample_t *)newSample;
- (void)reset;

// Target & Actions
- (void)addTarget:(id)target action:(SEL)action;
- (void)removeTarget:(id)target action:(SEL)action;

// Life cycle
- (id)initWithTarget:(id)target action:(SEL)action;
@end



