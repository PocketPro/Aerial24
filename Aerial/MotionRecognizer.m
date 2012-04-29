//
//  MotionRecognizer.m
//  Aerial
//
//  Created by Gord Parke on 4/28/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "MotionRecognizer.h"

/**** Methods to subclass:
 @required
 - (void)analyzeNewMotionSample:(MotionSample_t *)newSample;
 
 @optional
 - (void)exitingState___       where ___ is the name of a state as defined in MotionRecognizerStateNames
 - (void)enteringState___         ex: (void)enteringStatePossible
 *************************/


@interface MotionRecognizer ()
// Redefinition of properties from readonly to rewrite
@property (nonatomic, readwrite) MotionRecognizerState state;
@property (nonatomic, readwrite) MotionTimeline *motionTimeline;

// Private properties
@property (strong, nonatomic) NSMutableArray *targetsAndActions;
@property (strong, nonatomic) NSMutableSet *motionRecognizersRequiredToBegin;
@end


@implementation MotionRecognizer
@synthesize targetsAndActions = _targetsAndActions;
@synthesize state = _state;
@synthesize enabled = _enabled;
@synthesize motionRecognizersRequiredToBegin = _gestureRecognizersRequiredToBegin;
@synthesize motionTimeline = _motionTimeline;

#pragma mark - Relationships
- (BOOL)shouldWaitToBegin
{
    // This method goes through all the gesture recognizers that are required to have
    // recognized their motions before we should begin this one
    BOOL shouldWait = NO;
    
    for (MotionRecognizer *requiredRecognizer in self.motionRecognizersRequiredToBegin){
        if (requiredRecognizer.state < MotionRecognizerStateBegan)
            shouldWait = YES;
    }
    
    return shouldWait;
}
- (void)requireMotionRecognizerToBegin:(MotionRecognizer *)recognizer
{
    // Add this reqiured recognizer to our own interal list
    [self.motionRecognizersRequiredToBegin addObject:recognizer];
    
    // Make sure the other gesture recognizer updates us of its state changes
    [recognizer addTarget:self action:@selector(stateChangedInMotionRecognizerRequiredToBegin:)];
}
-(void)stateChangedInMotionRecognizerRequiredToBegin:(MotionRecognizer *)recognizer
{
    if (self.state == MotionRecognizerStateWaiting){
        if ([self shouldWaitToBegin] == NO)
            self.state = MotionRecognizerStatePossible;
    }
}


#pragma mark - Motion Analysis
- (void)analyzeNewMotionSample:(MotionSample_t *)newSample
{
    // This is where the acutal analysis of the samples is done.  It should be sublcassed.
    // It differs from handleNewMotionSample in that the latter is responsible for doing
    // 'adminitstrative' work with the new sample, inlcuding deciding if it should be analyzed
    // or not.  Conversely, this method deals strictly with the motion analysis
    
    [[NSException exceptionWithName:@"Error"
                             reason:@"analyzeNewMotionSample method must be subclassed" 
                           userInfo:nil] raise];
}
- (void)handleNewMotionSample:(MotionSample_t *)newSample
{
    // First check that we're in a state where motion should be analyzed
    if (self.state >= MotionRecognizerStatePossible && self.state < MotionRecognizerStateEnded){
        // Then check to ensure we're enabled
        if (self.enabled){
            // Finally acutally do the motion analysis
            [self analyzeNewMotionSample:newSample];
        }
    }
}
- (void)reset
{
    self.state = MotionRecognizerStateReset;
}


#pragma mark - Targets & Actions
-(NSInvocation *)invocationForTarget:(id)target action:(SEL)action
{
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:
                                [target methodSignatureForSelector:action]];
    [invocation setArgument:(__bridge void *) self atIndex:2];    
                                
    return invocation;
}
- (void)removeTarget:(id)target action:(SEL)action
{
    [self.targetsAndActions removeObject:[self invocationForTarget:target action:action]];
}
- (void)addTarget:(id)target action:(SEL)action
{
    [self.targetsAndActions addObject:[self invocationForTarget:target action:action]];
}

#pragma mark - Custom Setters & Getters
- (void)enteringState:(MotionRecognizerState)state
{
    // The following code allows our subclasses to just implement the enter and exit
    // state functions that they care about according to formatting in the example:
    //      -(void)enteringStatePossible;
    // Any functions that are not implemented are ignored.  I'm trying this as an alternative
    // to the cluttering case statements that are ubiquitious in c state machines.
    // I need to do the weird following #pragma clang stuff to prevent ARC from warning me about
    // potential selector leaks.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
    SEL enteringStateSelector = NSSelectorFromString([NSString stringWithFormat:@"enteringState%c",
                                                      MotionRecognizerStateNames[state]]);
    if ([self respondsToSelector:enteringStateSelector])
        [self performSelector:enteringStateSelector];
#pragma clang diagnostic pop
    
    
    // Do super class stuff that subclasses don't need to worry about
    switch (state) {
        case MotionRecognizerStateReset:
            // Move to next state
            if ([self shouldWaitToBegin])
                self.state = MotionRecognizerStateWaiting;
            else
                self.state = MotionRecognizerStatePossible;
            break;
            
        default:
            break;
    }
}
- (void)exitingState:(MotionRecognizerState)state
{
    // See description for this code in the "enteringState:" method
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
    SEL enteringStateSelector = NSSelectorFromString([NSString stringWithFormat:@"exitingState%c",
                                                      MotionRecognizerStateNames[state]]);
    if ([self respondsToSelector:enteringStateSelector])
        [self performSelector:enteringStateSelector];
#pragma clang diagnostic pop
}
- (void)setState:(MotionRecognizerState)state
{
    MotionRecognizerState oldState = _state;
    
    // Switch state if it's different than the one before
    if (state != oldState){
        [self exitingState:_state];
        _state = state;
        [self enteringState:_state];
    }
    
    // Message targets
    for (NSInvocation *invocation in self.targetsAndActions)
        [invocation invoke];
    
    // Optionally log state change information
    NSLog(@"%@ changed state: [%c] -> [%c]",NSStringFromClass([self class]), oldState, state);
}

#pragma mark - Lifecycle
-(id)init{
    if (self = [super init]){
        // Create targets and actions array
        self.targetsAndActions = [NSMutableArray array];
        
        // Create relationship arrays
        self.motionRecognizersRequiredToBegin = [NSMutableSet set];
    }
    return self;
}
-(id)initWithTarget:(id)target action:(SEL)action
{
    if (self = [self init]){
            
    }
    return self;
}

@end
