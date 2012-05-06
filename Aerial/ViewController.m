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

@interface ViewController ()

// IO elements
@property (strong, nonatomic) A24VideoCapture *videoCapture;
@property (weak, nonatomic) IBOutlet UILabel *lblStatus;
@property (weak, nonatomic) IBOutlet UIButton *btnJump;

@end

@implementation ViewController
@synthesize videoCapture = _videoCapture;
@synthesize lblStatus = _lblStatus;
@synthesize btnJump = _btnJump;

#pragma mark - Button touch events
- (IBAction)jumpTouchedDown:(id)sender {
}
- (IBAction)jumpTouchCancelled:(id)sender {
}
- (IBAction)jumpTouchUpInside:(id)sender {
    [[SensingCore sharedInstance] startSensing];
    [[[SensingCore sharedInstance] motionTimeline] resetAllMotionRecognizers];
    self.lblStatus.text = @"Ready..."; 
}

#pragma mark - Motion Recognizers
#pragma mark Throw 
- (void)throwMotionRecognizerChangedState:(ThrowMotionRecognizer *)throwMotionRecognizer
{
    if (throwMotionRecognizer.state == MotionRecognizerStateEnded) {
        [self.videoCapture startCapture];
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
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}


@end
