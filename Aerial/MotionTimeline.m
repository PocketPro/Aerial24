//
//  MotionTimeline.m
//  Aerial 24
//
//  Created by Gord Parke on 4/19/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "MotionTimeline.h"
#import "GolfSwing_math.h"


// Constants
#define kTimelineCapacity  4096   /* Preferably a power of 2 for fourier transform purposes */


@interface MotionTimeline () {
    MotionSample_t _samples[kTimelineCapacity];
    MotionSample_t *_nextSample;
}
@end


@implementation MotionTimeline
@synthesize capacity = _capacity;
@synthesize count = _count;

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
    // Set values
    *_nextSample = *newSample;
    
    // Increment next sample
    _nextSample = [self sampleForNumber:1 ofSamplesAfter:_nextSample];
    
    // Increment size
    self.count++; // Capped in setter method
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

#pragma mark - Lifecycle
-(id)init
{
    if (self = [super init]){
        _nextSample = _samples;
    }
    return self;
}
@end