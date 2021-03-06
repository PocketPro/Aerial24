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
 - (void)willExitState___       where ___ is the name of a state as defined in MotionRecognizerStateNames
 - (void)didEnterState___         ex: (void)didEnterStatePossible
 *************************/


@interface MotionRecognizer ()
// Redefinition of properties from readonly to rewrite
@property (nonatomic, readwrite) MotionRecognizerState state;
@property (nonatomic, readwrite, weak) MotionTimeline *motionTimeline;

// Private properties
@property (strong, nonatomic) NSMutableArray *targetsAndActions;
@property (strong, nonatomic) NSMutableSet *motionRecognizersRequiredToAchieveState;
@end

// Required recognizer dictionary keys
static const NSString *MRRecognizerKey  = @"MRRecognizerKey";
static const NSString *MRStateKey       = @"MRStateKey";

// Target action dictionary keys
static const NSString *MRTargetKey      = @"MRTargetKey";
static const NSString *MRActionKey      = @"MRActionKey";

@implementation MotionRecognizer
@synthesize targetsAndActions = _targetsAndActions;
@synthesize state = _state;
@synthesize enabled = _enabled;
@synthesize motionRecognizersRequiredToAchieveState = _gestureRecognizersRequiredToAchieveState;
@synthesize motionTimeline = _motionTimeline;

#pragma mark - Relationships
- (BOOL)shouldWaitToBegin
{
    // This method goes through all the gesture recognizers that are required to have
    // recognized their motions before we should begin this one
    BOOL shouldWait = NO;
    
    for (NSDictionary *dict in self.motionRecognizersRequiredToAchieveState){
        MotionRecognizer *requiredRecognizer = [dict objectForKey:MRRecognizerKey];
        MotionRecognizerState minState = [[dict objectForKey:MRStateKey] intValue];
        
        if (requiredRecognizer.state < minState){
            shouldWait = YES;
            break;
        }
    }
    
    return shouldWait;
}
- (void)requireMotionRecognizer:(MotionRecognizer *)recognizer toAchieveState:(MotionRecognizerState)minState;
{
    // Create dictionary with our recognizer and its state
    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
                          recognizer, MRRecognizerKey,
                          [NSNumber numberWithInt:minState], MRStateKey,
                          nil];
    
    // Add this reqiured recognizer to our own interal list
    [self.motionRecognizersRequiredToAchieveState addObject:dict];
    
    // Make sure the other gesture recognizer updates us of its state changes
    [recognizer addTarget:self action:@selector(stateChangedInMotionRecognizerRequiredToAchiveState:)];
}
-(void)stateChangedInMotionRecognizerRequiredToAchiveState:(MotionRecognizer *)recognizer
{
    // Only look at this state change if we're in the waiting mode
    if (self.state == MotionRecognizerStateWaiting){

        // Switch state if all required recognizers have achieved their minimum state
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

#ifdef INCLUDES_PLOTTING
#pragma mark - Plotting

// Delegate method that returns the number of points on the plot
-(NSUInteger)numberOfRecordsForPlot:(CPTPlot *)plot userInfo:(NSDictionary *)dictionary;
{
    NSLog(@"Warning: plot data source set to a motion recognizer that doesn't subclass the plotting data source methods");
    return 0;
}

// Delegate method that returns a single X or Y value for a given plot.
-(NSNumber *)numberForPlot:(CPTPlot *)plot field:(NSUInteger)fieldEnum recordIndex:(NSUInteger)index userInfo:(NSDictionary *)dictionary
{
    return [NSNumber numberWithFloat:0];
}
#endif

#pragma mark - Targets & Actions
-(NSDictionary *)dictionaryForTarget:(id)target action:(SEL)action
{
    NSValue *selectorObject = [NSValue valueWithPointer:action];
    return [NSDictionary dictionaryWithObjectsAndKeys:target, MRTargetKey, selectorObject, MRActionKey, nil];
}
- (void)removeTarget:(id)target action:(SEL)action
{
    [self.targetsAndActions removeObject:[self dictionaryForTarget:target action:action]];
}
- (void)addTarget:(id)target action:(SEL)action
{
    [self.targetsAndActions addObject:[self dictionaryForTarget:target action:action]];
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
    NSString *selectorString = [NSString stringWithFormat:@"didEnterState%s",
                                MotionRecognizerStateNames[state]];
    SEL enteringStateSelector = NSSelectorFromString(selectorString);
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
    SEL enteringStateSelector = NSSelectorFromString([NSString stringWithFormat:@"willExitState%s",
                                                      MotionRecognizerStateNames[state]]);
    if ([self respondsToSelector:enteringStateSelector])
        [self performSelector:enteringStateSelector];
#pragma clang diagnostic pop
}
- (void)setState:(MotionRecognizerState)newState
{
    // Get old state and assert it's a valid state change
    MotionRecognizerState oldState = _state;
    NSAssert(newState >= oldState || newState == MotionRecognizerStateReset, 
             @"State error in %@: Attempted change from [%s] --> [%s].\n State must not decrease unless it is"
             "set to MotionRecognizerStateReset.",
             NSStringFromClass([self class]), 
             MotionRecognizerStateNames[oldState], 
             MotionRecognizerStateNames[newState]);
    
    // Call exit handlers
    if (newState != oldState)
        [self exitingState:_state];

    // Log state change information
    NSLog(@"%@ changed state: [%s] -> [%s]",NSStringFromClass([self class]), 
          MotionRecognizerStateNames[oldState],
          MotionRecognizerStateNames[newState]);
    
    // Set new state and message targets with new state
    _state = newState;
    for (NSDictionary *dict in self.targetsAndActions){
        id target = [dict objectForKey:MRTargetKey];
        SEL action = [[dict objectForKey:MRActionKey] pointerValue];
        // Need to do this clang stuff to avoid possible memory leak warning with ARC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
        [target performSelector:action withObject:self];
#pragma clang pop
    }
    
    // Call entering state handlers. This calls down to subclasses.  It's done last, becasue the subclasses
    // may change state in these handlers, and we don't want that to mess with messaging the targets for example.
    if (newState != oldState)
        [self enteringState:_state];
}

#pragma mark - Lifecycle
- (id)init{
    if (self = [super init]){
        // Create targets and actions array
        self.targetsAndActions = [NSMutableArray array];
        
        // Create relationship arrays
        self.motionRecognizersRequiredToAchieveState = [NSMutableSet set];
        
        // Set initial state and default state.
        // Default enabled state is enabled. Set this after a run loop cycle
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            self.state = MotionRecognizerStateReset;
            self.enabled = YES;
        }];
    }
    return self;
}
- (id)initWithTarget:(id)target action:(SEL)action
{
    if (self = [self init]){
        [self addTarget:target action:action];
    }
    return self;
}

@end
