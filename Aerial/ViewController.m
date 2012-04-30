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

@interface ViewController ()

// IO elements
@property (weak, nonatomic) IBOutlet UILabel *lblStatus;
@property (weak, nonatomic) IBOutlet UIButton *btnJump;

@end

@implementation ViewController
@synthesize lblStatus = _lblStatus;
@synthesize btnJump = _btnJump;

#pragma mark - Button touch events
- (IBAction)jumpTouchedDown:(id)sender {
}
- (IBAction)jumpTouchCancelled:(id)sender {
}
- (IBAction)jumpTouchUpInside:(id)sender {
}

#pragma mark - Motion Recognizers
#pragma mark Throw 
- (void)throwMotionRecognizerChangedState:(ThrowMotionRecognizer *)throwMotionRecognizer
{
    NSLog(@"Throw motion recognizer in view controller");
}

#pragma mark Catch
- (void)catchMotionRecognizerChangedState:(CatchMotionRecognizer *)catchMotionRecognizer
{
    NSLog(@"Catch motion recognizer changed state");
}

#pragma mark - Lifecycle
- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
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
