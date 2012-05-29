//
//  ThrowMotionRecognizer.h
//  Aerial
//
//  Created by Gord Parke on 4/29/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "MotionRecognizer.h"

@interface ThrowMotionRecognizer : MotionRecognizer
@property (nonatomic, readonly) MotionSample_t *throwSample;

@end
