//
//  A24VideoCapture.h
//  MyAVController
//
//  Created by Eytan Moudahi on 12-05-06.
//  Copyright (c) 2012 PPG Technologies Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>

@class A24VideoCapture;

@protocol A24VideoCaptureDelegate <NSObject>
- (void)a24VideoCapture:(A24VideoCapture*)videoCapture;
@end

@interface A24VideoCapture : NSObject <AVCaptureFileOutputRecordingDelegate>
{
    AVCaptureSession *captureSession;
    AVCaptureMovieFileOutput *captureOutput;
    NSURL *movieURL;
}

- (void)startCapture;
- (void)stopCapture;

@end
