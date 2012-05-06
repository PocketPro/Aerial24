//
//  MotionTimeline.m
//  Aerial 24
//
//  Created by Gord Parke on 4/19/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "MotionTimeline.h"
#import "GolfSwing_math.h"
#import "MotionRecognizer.h"

// Constants
#define kTimelineCapacity  4096   /* Preferably a power of 2 for fourier transform purposes */

// Class extension
@interface MotionTimeline () {
    MotionSample_t _samples[kTimelineCapacity];
    MotionSample_t *_nextSample;
}
@property (strong, nonatomic) NSMutableArray *motionRecognizers;
@end

// Class category to expose motion timeline property on motion recognizer as readwrite
@interface MotionRecognizer (MotionTimelineReadWrite)
@property (nonatomic, readwrite, weak) MotionTimeline *motionTimeline;
@end

@implementation MotionTimeline
@synthesize capacity = _capacity;
@synthesize count = _count;
@synthesize motionRecognizers = _motionRecognizers;  // Used internally
@synthesize allMotionRecognizers = _allMotionRecognizers; // External property

#pragma mark - Motion Recognizers
-(void)addMotionRecognizer:(MotionRecognizer *)recognizer
{
    if ([self.motionRecognizers indexOfObject:recognizer] == NSNotFound){
        [self.motionRecognizers addObject:recognizer];
        recognizer.motionTimeline = self;
    }
}
-(void)removeMotionRecognizer:(MotionRecognizer *)recognizer
{
    // Remove all recognizers that respond true to isEqual:recognizer
    NSUInteger index;
    while ((index = [self.motionRecognizers indexOfObject:recognizer]) != NSNotFound){
        MotionRecognizer *recognizerToRemove = [self.motionRecognizers objectAtIndex:index];
        recognizerToRemove.motionTimeline = nil;
        [self.motionRecognizers removeObjectAtIndex:index];
    }
}
- (void)resetAllMotionRecognizers
{
    [self.motionRecognizers makeObjectsPerformSelector:@selector(reset)];
}

#pragma mark - Indexing
-(MotionSample_t *)sampleForNumber:(NSInteger)n ofSamplesAfter:(MotionSample_t *)sample
{
    NSUInteger capacity = self.capacity;
    MotionSample_t *endSample = _samples + capacity;
    
    // Check validity of input sample pointer
    if (sample < _samples || sample >= endSample)
        return nil;
    
    // Reduce n to valid domain (if it's really big for some reason). Inc should have sign of dividend
    NSInteger inc = n % capacity;
    
    // Decrement sample and loop if necessary.  We shouldn't roll over more than once becuase of previous mod
    MotionSample_t *newSample = sample + inc;
    if (newSample >= endSample)
        newSample -= capacity;
    else if (newSample < _samples)
        newSample += capacity;
    
    NSAssert(newSample >= _samples && newSample < endSample, @"sampleForNumber:ofSamplesBefore: Calculated invalid number.");
    
    return newSample;
}
// Returns the number of samples from a start to and end point, includsive
-(NSUInteger)numberOfSamplesBetweenStart:(MotionSample_t *)start end:(MotionSample_t *)end
{
    if (end > start) {
        return  end + 1 - start;
    } else {
        return end + self.capacity + 1 - start;
    }
}

#pragma mark - Sample Input & Retrieval
-(MotionSample_t *)sampleAtPastIndex:(NSInteger)samplesAgo
{
    // Validity checks
    if (samplesAgo < 0) return nil;
    if (samplesAgo >= [self count]) return nil;
    
    return [self sampleForNumber:-(samplesAgo+1) ofSamplesAfter:_nextSample];
}
-(MotionSample_t *)newestSample
{
    return [self sampleAtPastIndex:0];
}
-(MotionSample_t *)oldestSample
{
    return [self sampleAtPastIndex:([self count] - 1)];
}

-(void)addSample:(MotionSample_t *)newSample
{
    // Get sample in timeline we should set
    MotionSample_t *newTimelineSample = _nextSample;
    
    // Set values
    *newTimelineSample = *newSample;
    
    // Increment next sample
    _nextSample = [self sampleForNumber:1 ofSamplesAfter:_nextSample];
    
    // Increment size
    self.count++; // Capped in setter method
    
    // Inform motion recognizers
    [self.motionRecognizers enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        MotionRecognizer *recognizer = (MotionRecognizer *)obj;
        [recognizer handleNewMotionSample:newTimelineSample];
    }];
}


#pragma mark - Sample Statistics
-(MotionSample_t)averageOfSamplesInRange:(NSRange)range
{
    MotionSample_t averagedSample = {0};
    MotionSample_t *startSample = [self sampleAtPastIndex:range.location];
    
    // Validity
    NSAssert(startSample, @"Invalid range location passed into averageOfSamplesInRange:");
    NSAssert(range.length <= self.count,@"Invalid range length passed into averageOfSamplesInRange:");

    
    // Do summing
    for (NSInteger i = 0; i < range.length; ++i)
    {
        MotionSample_t *curSample = [self sampleForNumber:i ofSamplesAfter:startSample];
        
        // Acceleration
        GSVectorAddD(averagedSample.vbAcceleration, 
                    averagedSample.vbAcceleration, 
                    curSample->vbAcceleration, 
                    3);
        
        // Angular velocity
        GSVectorAddD(averagedSample.vbAngularVelocity,
                    averagedSample.vbAngularVelocity,
                    curSample->vbAngularVelocity, 
                    3);
    }
    
    // Divide by length of range to get average
    GSVectorScalarDivideD(averagedSample.vbAcceleration, averagedSample.vbAcceleration, range.length, 3);
    GSVectorScalarDivideD(averagedSample.vbAngularVelocity, averagedSample.vbAngularVelocity, range.length, 3);
    
    return averagedSample;
} 

#pragma mark - Custom Setters & Getters
-(NSUInteger)capacity
{
    return kTimelineCapacity;
}
-(void)setCount:(NSUInteger)count
{
    // Cap to capacity
    NSUInteger capacity = self.capacity;
    if (count < capacity) 
        _count = count;
    else 
        _count = _capacity;
}
-(NSSet *)allMotionRecognizers
{
    // Currently we store all motion recognizers in a mutable array.  In the future, if there are special
    // ones, they should also be added to the set.
    return [NSSet setWithArray:self.motionRecognizers];
}

#pragma mark - Lifecycle
-(id)init
{
    if (self = [super init]){
        // Initialize circ buf
        _nextSample = _samples;
    
        // Allocate arrays
        self.motionRecognizers = [NSMutableArray array];
    }
    return self;
}
@end
