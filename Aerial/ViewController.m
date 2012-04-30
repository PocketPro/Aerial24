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

#pragma mark - Throw Motion Recognizer
- (void)throwMotionRecognizerChangedState:(ThrowMotionRecognizer *)throwMotionRecognizer
{
    NSLog(@"Throw motion recognizer in view controller");
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
    ThrowMotionRecognizer *throwMotionRecognizer = [[ThrowMotionRecognizer alloc] initWithTarget:self action:@selector(throwMotionRecognizerChangedState:)];
    [motionTimeline addMotionRecognizer:throwMotionRecognizer];
    
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
