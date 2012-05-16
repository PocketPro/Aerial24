//
//  ViewController.m
//  Aerial
//
//  Created by Gord Parke on 4/1/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "ViewController.h"
#import "MotionTimeline.h"
#import "SensingCore.h"
#import "ThrowMotionRecognizer.h"
#import "CatchMotionRecognizer.h"
#import "A24VideoCapture.h"
#import <MediaPlayer/MediaPlayer.h>

@interface ViewController ()

// IO elements
@property (strong, nonatomic) A24VideoCapture *videoCapture;
@property (weak, nonatomic) IBOutlet UILabel *lblStatus;
@property (weak, nonatomic) IBOutlet UIButton *btnJump;
@property (weak, nonatomic) IBOutlet UILabel *lblVideoStatus;

@end

@implementation ViewController
@synthesize videoCapture = _videoCapture;
@synthesize lblStatus = _lblStatus;
@synthesize btnJump = _btnJump;
@synthesize lblVideoStatus = _lblVideoStatus;

#pragma mark - Button touch events
- (IBAction)jumpTouchedDown:(id)sender {
}

- (IBAction)jumpTouchCancelled:(id)sender {
}

- (IBAction)jumpTouchUpInside:(id)sender {
    [[SensingCore sharedInstance] startSensing];
    [[[SensingCore sharedInstance] motionTimeline] resetAllMotionRecognizers];
    self.lblStatus.text = @"Ready..."; 
    [self.videoCapture startCapture];
}

- (IBAction)play:(id)sender {
    NSLog(@"Play button pressed. Play movie at URL: %@", self.videoCapture.movieURL);
    if ([self isMovieAvailableForPlayback]) {
        [self playbackMovie];
    } else {
        UIAlertView *av = [[UIAlertView alloc] initWithTitle:@"Movie Unavailable" message:@"No recording are currently available for playback. Please perform a throw and catch before trying again" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
        [av show];
    }
}

#pragma mark - Movie Methods

- (BOOL)isMovieAvailableForPlayback
{
    if (self.videoCapture.movieURL) {
        return YES;
    } else {
        return NO;
    }
}

- (void)playbackMovie
{
    MPMoviePlayerViewController *playerVC = [[MPMoviePlayerViewController alloc] initWithContentURL:self.videoCapture.movieURL];
    
    // Remove the movie player view controller from the "playback did finish" notification observers
    [[NSNotificationCenter defaultCenter] removeObserver:playerVC
                                                    name:MPMoviePlayerPlaybackDidFinishNotification
                                                  object:playerVC.moviePlayer];
    
    // Register this class as an observer instead
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(movieFinishedCallback:)
                                                 name:MPMoviePlayerPlaybackDidFinishNotification
                                               object:playerVC.moviePlayer];
    
    [self presentModalViewController:playerVC animated:YES];
    [playerVC.moviePlayer prepareToPlay];
    [playerVC.moviePlayer play];
}

- (void)movieFinishedCallback:(NSNotification*)aNotification
{
    // Obtain the reason why the movie playback finished
    NSNumber *finishReason = [[aNotification userInfo] objectForKey:MPMoviePlayerPlaybackDidFinishReasonUserInfoKey];
    
    // Dismiss the view controller ONLY when the reason is not "playback ended"
    if ([finishReason intValue] != MPMovieFinishReasonPlaybackEnded)
    {
        MPMoviePlayerController *moviePlayer = [aNotification object];
        
        // Remove this class from the observers
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:MPMoviePlayerPlaybackDidFinishNotification
                                                      object:moviePlayer];
        
        // Dismiss the view controller
        [self dismissModalViewControllerAnimated:YES];
    }
}


#pragma mark - Motion Recognizers
#pragma mark Throw 
- (void)throwMotionRecognizerChangedState:(ThrowMotionRecognizer *)throwMotionRecognizer
{
    if (throwMotionRecognizer.state == MotionRecognizerStateEnded) {
    }
}

#pragma mark Catch
- (void)catchMotionRecognizerChangedState:(CatchMotionRecognizer *)catchMotionRecognizer
{
    if (catchMotionRecognizer.state == MotionRecognizerStateEnded){
        self.lblStatus.text = (catchMotionRecognizer.isFumble ? @"FUMBLE!" : @"Catch" );
        [[SensingCore sharedInstance] stopSensing];
        [self.videoCapture stopCapture];
    }
}

#pragma mark - Lifecycle
- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    // Prepare the video catpure
    if (nil == self.videoCapture) {
        self.videoCapture = [[A24VideoCapture alloc] init];
        self.videoCapture.delegate = self;
    }
    
    // Create sensing core and get its timeline
    SensingCore *sensingCore = [SensingCore sharedInstance];
    MotionTimeline *motionTimeline = sensingCore.motionTimeline;
    
    // Create motion recognizers
    // Throw
    ThrowMotionRecognizer *throwMotionRecognizer = [[ThrowMotionRecognizer alloc] initWithTarget:self action:@selector(throwMotionRecognizerChangedState:)];
    [motionTimeline addMotionRecognizer:throwMotionRecognizer];
    
    // Catch
    CatchMotionRecognizer *catchMotionRecognizer = [[CatchMotionRecognizer alloc] initWithTarget:self action:@selector(catchMotionRecognizerChangedState:)];
    [catchMotionRecognizer requireMotionRecognizer:throwMotionRecognizer toAchieveState:MotionRecognizerStateEnded]; 
    [motionTimeline addMotionRecognizer:catchMotionRecognizer];
    
    // Start motion
    [sensingCore startSensing];
}
    

- (void)viewDidUnload
{
    [self setLblStatus:nil];
    [self setBtnJump:nil];
    [self setLblVideoStatus:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

#pragma mark - VideoCapture Delegate
- (void)A24VideoCaptureDidStartRecording:(A24VideoCapture*)videoCapture
{
    self.lblVideoStatus.text = @"recording";
}

- (void)A24VideoCaptureDidStopRecording:(A24VideoCapture*)videoCapture
{
    self.lblVideoStatus.text = @"not recording";
}

- (BOOL)A24VideoCaptureShouldSaveVideo:(A24VideoCapture*)videoCapture
{
    return YES;
}

- (void)A24VideoCaptureWillSaveVideoToPhotosAlbum:(A24VideoCapture*)videoCapture
{
    self.lblVideoStatus.text = @"saving";
}

- (void)A24VideoCapture:(A24VideoCapture*)videoCapture didSaveVideoToPhotosAlbum:(NSError*)error
{
    if (error) {
        self.lblVideoStatus.text = [error localizedDescription];
    } else {
        self.lblVideoStatus.text = @"saved";
    }
}

- (void)A24VideoCaptureDidRemoveMovieFile:(A24VideoCapture*)videoCapture
{
    self.lblVideoStatus.text = @"deleted";
}

@end
