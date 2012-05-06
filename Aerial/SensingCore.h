//
//  SensingCore.h
//  Aerial
//
//  Created by Gord Parke on 4/28/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import <UIKit/UIKit.h>

@class MotionTimeline;

@interface SensingCore : NSObject
@property (strong, nonatomic) MotionTimeline *motionTimeline;
@property (nonatomic, readonly, getter = isSensing) BOOL sensing;

- (void)startSensing;
- (void)stopSensing;

+ (SensingCore *)sharedInstance;
@end
