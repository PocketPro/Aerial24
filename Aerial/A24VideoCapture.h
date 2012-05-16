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
@optional
- (void)A24VideoCaptureDidStartRecording:(A24VideoCapture*)videoCapture;
- (void)A24VideoCaptureDidStopRecording:(A24VideoCapture*)videoCapture;
- (BOOL)A24VideoCaptureShouldSaveVideo:(A24VideoCapture*)videoCapture;
- (void)A24VideoCaptureWillSaveVideoToPhotosAlbum:(A24VideoCapture*)videoCapture;
- (void)A24VideoCaptureDidRemoveMovieFile:(A24VideoCapture*)videoCapture;
- (void)A24VideoCapture:(A24VideoCapture*)videoCapture didSaveVideoToPhotosAlbum:(NSError*)error;
@end

@interface A24VideoCapture : NSObject <AVCaptureFileOutputRecordingDelegate>
{
    AVCaptureSession *captureSession;
    AVCaptureMovieFileOutput *captureOutput;
    NSURL *movieURL;
}
@property (weak) id <A24VideoCaptureDelegate> delegate;
@property (strong, readonly) NSURL *movieURL;

- (void)startCapture;
- (void)stopCapture;

@end
