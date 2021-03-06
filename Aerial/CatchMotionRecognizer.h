//
//  CatchMotionRecognizer.h
//  Aerial
//
//  Created by Gord Parke on 4/30/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "MotionRecognizer.h"

@interface CatchMotionRecognizer : MotionRecognizer
@property (nonatomic, readonly) MotionSample_t *catchSample;
@property (nonatomic, readonly, getter = isFumble) BOOL fumble;
@end
