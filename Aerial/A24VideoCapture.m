//
//  A24VideoCapture.m
//  MyAVController
//
//  Created by Eytan Moudahi on 12-05-06.
//  Copyright (c) 2012 PPG Technologies Inc. All rights reserved.
//

#import "A24VideoCapture.h"

#define CM_TIMESCALE 600
#define MAX_DURATION_SECONDS 10
#define CM_DURATION CM_TIMESCALE*MAX_DURATION_SECONDS

@interface A24VideoCapture()
@end

@implementation A24VideoCapture
@synthesize delegate;
@synthesize captureOutput,captureSession;
@synthesize movieURL;

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
    captureOutput.maxRecordedDuration = CMTimeMake(CM_DURATION, CM_TIMESCALE); //Denotes a maximum recording duration of 10 seconds.
    
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
    if (captureOutput.isRecording) {
        NSLog(@"A24VideoCapture is currently recording. All calls to -startCapture are being ignored");
        return;
    }
    
    if (movieURL) {
        NSLog(@"A24VideoCapture has previously captured a video. The old video will now be deleted. If the video was valid, it should be saved in the savted photos album");
        // This method may delay the start of playback. Perhaps deleteMovieFile 
        // should take a reference to a URL so the prior movie can be deleted in 
        // the background.
        [self deleteMovieFile];
    }
    
    NSString *fileName = [NSString stringWithFormat:@"Aerial24_%.0f.mp4",[[NSDate date] timeIntervalSinceReferenceDate]];
    NSString *filePath = [[self applicationDocumentsDirectory] stringByAppendingPathComponent:fileName];
    movieURL = [NSURL fileURLWithPath:filePath];
    [captureOutput startRecordingToOutputFileURL:movieURL recordingDelegate:self];
}

- (void)stopCapture
{
    [captureOutput stopRecording];   
}


- (void)deleteMovieFile
{
    NSError *error = nil;
    if ([[NSFileManager defaultManager] removeItemAtURL:movieURL error:&error]){
        // File removed successfully
    } else {
        // File was not removed
        [NSException raise:@"A24VideoCaptureCouldNotRemoveFile" format:@"The video file at path %@ could not be removed."];
    }
    
    if ([self.delegate respondsToSelector:@selector(A24VideoCaptureDidRemoveMovieFile:)]){
        [self.delegate A24VideoCaptureDidRemoveMovieFile:self];
    }
}

/*
- (void)trimMovieAtURL:(NSURL*)movieFileURL
{
    if ([self.delegate respondsToSelector:@selector(cropTimeForA24VideoCapture:)] == NO){
        NSLog(@"A24VideoCaptureDelegate does not implement croptimeForA24VideoCapture:. This method must be implemented in order to crop video.");
        return;
    }
    
    AVAsset *anAsset = [AVAsset assetWithURL:movieFileURL];
    
    AVAssetExportSession *exportSession = [[AVAssetExportSession alloc]
                                           initWithAsset:anAsset presetName:AVAssetExportPresetHighestQuality];
    NSString *trimmedFileName = [[movieFilename stringByAppendingString:@"_trimmed"] stringByAppendingPathExtension:VIDEO_EXTENSION];
    NSURL *baseURL = [movieFileURL URLByDeletingLastPathComponent];
    trimmedMovieURL = [baseURL URLByAppendingPathComponent:trimmedFileName];
    exportSession.outputURL = trimmedMovieURL;
    
    NSLog(@"supportedFileTypes %@", exportSession.supportedFileTypes);
    
    exportSession.outputFileType = @"com.apple.quicktime-movie";
                               
    float cropTime = [self.delegate cropTimeForA24VideoCapture:self];
    float cropTimeWithBias = MAX(0, cropTime - 2.0); //add a two second bias.
    float durationTime = CMTimeGetSeconds(self.captureOutput.recordedDuration) - cropTimeWithBias;
    CMTime start = CMTimeMakeWithSeconds(cropTimeWithBias, CM_TIMESCALE);
    CMTime duration = CMTimeMakeWithSeconds(durationTime, CM_TIMESCALE);
    CMTimeRange range = CMTimeRangeMake(start, duration);
    exportSession.timeRange = range;
    
    [exportSession exportAsynchronouslyWithCompletionHandler:^{
        NSLog(@"Export session has completed trimming the video. Status: %d.", exportSession.status);
        NSLog(@"Trimmed video URL: %@", trimmedMovieURL);
        [self deleteMovieFileAtURL:movieURL];
        
        BOOL isCompatibly = UIVideoAtPathIsCompatibleWithSavedPhotosAlbum([trimmedMovieURL path]);
        if (isCompatibly) {
            if ([self.delegate respondsToSelector:@selector(A24VideoCaptureWillSaveVideoToPhotosAlbum:)]){
                [self.delegate A24VideoCaptureWillSaveVideoToPhotosAlbum:self];
            }
            UISaveVideoAtPathToSavedPhotosAlbum([trimmedMovieURL path], self, @selector(video:didFinishSavingWithError:contextInfo:), nil);
        }
        else {
            [NSException raise:@"PhotosAlbumVideoFormatNotSupported" format:@"The video format for a recorded video is not supported by the saved photos album. This recording can not be saved."];
        }
    }];
}
*/

#pragma mark - AVCaptureFileOutputDelegate
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didStartRecordingToOutputFileAtURL:(NSURL *)fileURL fromConnections:(NSArray *)connections
{
    NSLog(@"captureOutput:didStartRecordingToOutputFileAtURL:fromConnections:");
    if ([self.delegate respondsToSelector:@selector(A24VideoCaptureDidStartRecording:)]){
        [self.delegate A24VideoCaptureDidStartRecording:self];
    }
}

- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL fromConnections:(NSArray *)connections error:(NSError *)error
{    
    NSLog(@"captureOutput:didFinishRecordingToOutputFileAtURL:fromConnections:");
    
    if ([self.delegate respondsToSelector:@selector(A24VideoCaptureDidStopRecording:)]){
        [self.delegate A24VideoCaptureDidStopRecording:self];
    }
    
    if ([self.delegate respondsToSelector:@selector(A24VideoCaptureShouldSaveVideo:)]) {
        if ([self.delegate A24VideoCaptureShouldSaveVideo:self]){
            // Save Video
            
            BOOL isCompatibly = UIVideoAtPathIsCompatibleWithSavedPhotosAlbum([outputFileURL path]);
            if (isCompatibly) {
                if ([self.delegate respondsToSelector:@selector(A24VideoCaptureWillSaveVideoToPhotosAlbum:)]){
                    [self.delegate A24VideoCaptureWillSaveVideoToPhotosAlbum:self];
                }
                UISaveVideoAtPathToSavedPhotosAlbum([outputFileURL path], self, @selector(video:didFinishSavingWithError:contextInfo:), nil);
            }
            else {
                [NSException raise:@"PhotosAlbumVideoFormatNotSupported" format:@"The video format for a recorded video is not supported by the saved photos album. This recording can not be saved."];
            }
            
        } else {
            // Do no save video
            [self deleteMovieFile];
        }
    } else {
        // Default behavior
            [self deleteMovieFile];
    }
    
    

}

- (void)video:(NSString *)videoPath didFinishSavingWithError: (NSError *) error contextInfo: (void *) contextInfo;
{
    if ([self.delegate respondsToSelector:@selector(A24VideoCapture:didSaveVideoToPhotosAlbum:)]){
        [self.delegate A24VideoCapture:self didSaveVideoToPhotosAlbum:error];
    }
    NSLog(@"Destination path: %@", videoPath);
}

@end
