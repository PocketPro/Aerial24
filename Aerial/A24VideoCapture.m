//
//  A24VideoCapture.m
//  MyAVController
//
//  Created by Eytan Moudahi on 12-05-06.
//  Copyright (c) 2012 PPG Technologies Inc. All rights reserved.
//

#import "A24VideoCapture.h"

@implementation A24VideoCapture

- (id)init
{
    self = [super init];
    if (self) {
        [self initCaptureSession];
    }
    return self;
}

- (void)initCaptureSession
{
	AVCaptureDeviceInput *captureInput = [AVCaptureDeviceInput 
										  deviceInputWithDevice:[AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] 
										  error:nil];
    captureOutput = [[AVCaptureMovieFileOutput alloc] init];
    
    // The capture session coordinates the input and output devices.
	captureSession = [[AVCaptureSession alloc] init];
    
	[captureSession addInput:captureInput];
	[captureSession addOutput:captureOutput];
    
    // Set the capture quality. If we want to upload the video, we may want to be consciou about the setting we choose.
    [captureSession setSessionPreset:AVCaptureSessionPresetHigh];
	
	/*We start the capture*/
    [captureSession startRunning];
}

- (NSString *) applicationDocumentsDirectory 
{    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : nil;
    return basePath;
}

- (void)startCapture
{
    NSString *fileName = [NSString stringWithFormat:@"Aerial24_%.0f.mp4",[[NSDate date] timeIntervalSinceReferenceDate]];
    NSString *filePath = [[self applicationDocumentsDirectory] stringByAppendingPathComponent:fileName];
    movieURL = [NSURL fileURLWithPath:filePath];
    [captureOutput startRecordingToOutputFileURL:movieURL recordingDelegate:self];
}

- (void)stopCapture
{
    [captureOutput stopRecording];   
}

#pragma mark - AVCaptureFileOutputDelegate
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didStartRecordingToOutputFileAtURL:(NSURL *)fileURL fromConnections:(NSArray *)connections
{
    NSLog(@"Video capture did start");
}

- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL fromConnections:(NSArray *)connections error:(NSError *)error
{
    NSLog(@"Video capture did finish with errors: %@", [error description]);
    NSLog(@"Savings video to photo album...");
    BOOL isCompatibly = UIVideoAtPathIsCompatibleWithSavedPhotosAlbum([outputFileURL path]);
    if (isCompatibly) {
        UISaveVideoAtPathToSavedPhotosAlbum([outputFileURL path], self, @selector(video:didFinishSavingWithError:contextInfo:), nil);
    }
    else {
        NSLog(@"FAILED: Video is not compatible with saved photos album");
    }
}

- (void)video:(NSString *)videoPath didFinishSavingWithError: (NSError *) error contextInfo: (void *) contextInfo;
{
    NSLog(@"Finished writing file to saved photos album with errors %@", [error description]);
}

@end
