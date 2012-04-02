//
//  ViewController.m
//  Aerial
//
//  Created by Gord Parke on 4/1/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "ViewController.h"
#import <CoreMotion/CoreMotion.h>


// Motion value constants
#define kStillAccelSlope            0.005

#define kStillTimePeriod            0.5
#define kTimeStillToJump            2
#define kFreeFallMag                0.5
#define kPossibleJumpPeriod         0.25

// Motion setup defines
#define kAccelHistoryLength         256      /* Should be power of 2 for fourier transform purposes */
#define kAccelSamplingInterval      0.01     /* 100 Hz */
#define kMovingAverageWindow        0.1      /* Seconds */

// Physical constants
#define kGravity                    9.81     /* m/s^2 */

// Typedefs
typedef CMAcceleration CMVelocity;

// Helper functions
#define mag(a) (sqrt(pow(a.x, 2.0) + pow(a.y, 2.0) + pow(a.z, 2.0)))
#define accumOpVector(a, b, op) {a.x = a.x op b.x; a.y = a.y op b.y; a.z = a.z op b.z;} 
#define accumOpScalar(a, b, op) {a.x = a.x op (b); a.y = a.y op (b); a.z = a.z op (b);} 

// States
enum local_state_e{
    STATE_STARTUP = 0,
    STATE_WAITING,
    STATE_WAITING_FOR_STILL,
    STATE_POSSIBLE_STILL,
    STATE_READY_FOR_JUMP,
    STATE_READY_FOR_JUMP_TIMEOUT,
    STATE_POSSIBLE_JUMP,
    STATE_JUMPING,
    STATE_JUMP_LANDING,
    
    STATE_NUM
};

@interface ViewController (){
    CMAcceleration _accelHistory[kAccelHistoryLength];
    CMAcceleration *_curAccelHistoryPointer;
    int _accelHistoryFilledLen;
}

// IO elements
@property (weak, nonatomic) IBOutlet UILabel *lblStatus;
@property (weak, nonatomic) IBOutlet UIButton *btnJump;

// State
@property (nonatomic) enum local_state_e localState;

// Motion manager
@property (nonatomic, strong) CMMotionManager *motionManager;

// Timing
@property (nonatomic, strong)       NSDate *stateDate;
@property (nonatomic, strong)       NSDate *readyForJumpDate;
@property (nonatomic, strong)       NSDate *jumpDate;
@property (nonatomic, strong)       NSTimer *switchStateTimer;

// Motion properties
@property (nonatomic)               NSTimeInterval      lastAccelTime;
@property (nonatomic, readonly)     CMAcceleration      movingAverageAccel;
@property (nonatomic, readonly)     CMAcceleration      lastMovingAverageAccel;
@property (nonatomic)               CMVelocity          velocity;
@property (nonatomic)               CMVelocity          jumpVelocity;
@property (nonatomic)               CMAcceleration      stillGravity;
@property (nonatomic, strong)       CMAttitude          *stillAttitude;
@property (nonatomic)               int                 accelHistoryFilledLen;
@end

@implementation ViewController
@synthesize lblStatus = _lblStatus;
@synthesize btnJump = _btnJump;
@synthesize localState = _localState;
@synthesize motionManager = _motionManager;
@synthesize stateDate = _stateDate;
@synthesize readyForJumpDate = _readyForJumpDate;
@synthesize jumpDate = _jumpDate;
@synthesize switchStateTimer = _switchStateTimer;
@synthesize lastAccelTime = _lastAccelTime;
@synthesize stillGravity  = _gravity;
@synthesize velocity = _velocity;
@synthesize jumpVelocity = _velocityAtJump;
@synthesize stillAttitude = _stillAttitude;
@synthesize accelHistoryFilledLen = _accelHistoryFilledLen;


-(void)switchStateFromTimer:(NSTimer *)theTimer
{   
    int state = [[[theTimer userInfo] objectForKey:@"state"] intValue];
    self.localState = state;
}
-(void)cancelDelayedStateSwitch
{
    [self.switchStateTimer invalidate];
    self.switchStateTimer = nil;
}
-(void)switchToState:(enum local_state_e)state afterDelay:(NSTimeInterval)delay
{
    [self cancelDelayedStateSwitch];
    
    NSDictionary *userInfo = [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:state] forKey:@"state"];
    self.switchStateTimer = [NSTimer scheduledTimerWithTimeInterval:delay target:self selector:@selector(switchStateFromTimer:) userInfo:userInfo repeats:NO];
}

#pragma mark - Button touch events
- (IBAction)jumpTouchedDown:(id)sender {
}
- (IBAction)jumpTouchCancelled:(id)sender {
}
- (IBAction)jumpTouchUpInside:(id)sender {
    self.localState = STATE_WAITING_FOR_STILL;
}

#pragma mark - Accel history helpers
-(CMAcceleration)accelAtPreviousSampleIndex:(NSUInteger)samplesAgo
{
    // Initialization check
    if (_curAccelHistoryPointer == NULL){
        CMAcceleration zeroAccel = {0};
        return zeroAccel;
    }
    
    // Get pointer to the requested acceleration
    CMAcceleration *requestedAccel = _curAccelHistoryPointer - (samplesAgo + 1);
    if (requestedAccel < _accelHistory)
        requestedAccel += kAccelHistoryLength;
    
    // Return this acceleration
    return *requestedAccel;
}
-(CMAcceleration)lastAccelInBuffer
{
    return [self accelAtPreviousSampleIndex:0];
}
-(CMAcceleration)firstAccelInBuffer
{
    return [self accelAtPreviousSampleIndex:kAccelHistoryLength-1];
}
-(void)addAccelToBuffer:(CMAcceleration)newAccel
{
    // Initalization
    if (_curAccelHistoryPointer == NULL) 
        _curAccelHistoryPointer = _accelHistory;
    
    // Set Value 
    (*_curAccelHistoryPointer) = newAccel;
    
    // Increment
    ++_curAccelHistoryPointer;
    if (_curAccelHistoryPointer - _accelHistory >= kAccelHistoryLength)
        _curAccelHistoryPointer = _accelHistory;
    if (_accelHistoryFilledLen < kAccelHistoryLength)
        _accelHistoryFilledLen++;
}
-(CMAcceleration)movingAverageAccelWithOffset:(NSUInteger)offset length:(NSUInteger)len
{
    CMAcceleration accum = {0};
    
    // Add up len samples starting from offset samples ago
    for (int i = 0; i < len; ++i) {
        CMAcceleration accel = [self accelAtPreviousSampleIndex:offset+i];
        accumOpVector(accum, accel, +);
    }
    
    // Divide by len
    accumOpScalar(accum, len, /);
    
    return accum;
}


#pragma mark - Motion Analysis
-(void)resetToStateWaiting
{
    self.localState = STATE_WAITING;
}
-(void)endJump
{
    NSDate *curDate = [NSDate date];
    NSTimeInterval hangtime = [curDate timeIntervalSinceDate:self.jumpDate];
    
    self.lblStatus.text = [NSString stringWithFormat:@"Hangtime: %0.2f "
                           "\tHeight: %0.2f \tInitial Velocity: %0.2f",
                           hangtime,
                           3.2808399*0.125*9.81*pow(hangtime, 2.0),
                           mag(self.jumpVelocity)*2.23
                           ];
}
-(void)switchStateFromMotion
{
    // Analyze filter results
    enum local_state_e state = self.localState;
    
    // Get slope and mag
    double slope = mag(self.movingAverageAccel) - mag(self.lastMovingAverageAccel);
    double mag = mag(self.movingAverageAccel);

    // Peform various actions based on state
    switch (state) {
        case STATE_WAITING_FOR_STILL:
            // Check if slope is below specified level
            if (fabs(slope) < kStillAccelSlope)
                self.localState = STATE_POSSIBLE_STILL;
            break;
            
        case STATE_POSSIBLE_STILL:
            // Check that slope is still below level
            if (fabsf(slope) > kStillAccelSlope)
                self.localState = STATE_WAITING_FOR_STILL;
            
            // See if we've been still long enough to switch to jump mode
            if ([[NSDate date] timeIntervalSinceDate:self.stateDate] > kStillTimePeriod)
                self.localState = STATE_READY_FOR_JUMP;
            break;
            
        case STATE_READY_FOR_JUMP:
            // We are possibly jumping when we fall below a certain acceleration magnitude
            if (mag < kFreeFallMag)
                self.localState = STATE_POSSIBLE_JUMP;
            
            // Check jump timeout
            if ([[NSDate date] timeIntervalSinceDate:self.readyForJumpDate] > kTimeStillToJump)
                self.localState = STATE_READY_FOR_JUMP_TIMEOUT;
            break;
            
        case STATE_POSSIBLE_JUMP:
            // Check this is still a possible jump
            if (mag > kFreeFallMag)
                self.localState = STATE_READY_FOR_JUMP;
            
            // Ok this is a jump.
            if ([[NSDate date] timeIntervalSinceDate:self.stateDate] > kPossibleJumpPeriod)
                self.localState = STATE_JUMPING;
            
            // Check jump timeout
            if ([[NSDate date] timeIntervalSinceDate:self.readyForJumpDate] > kTimeStillToJump)
                self.localState = STATE_READY_FOR_JUMP_TIMEOUT;
            break;
            
        case STATE_JUMPING:
            // See if we've landed yet
            if (mag > kFreeFallMag)
                self.localState = STATE_JUMP_LANDING;
            break;
            
            
        // Do nothing states
        case STATE_WAITING:
        case STATE_READY_FOR_JUMP_TIMEOUT: 
        case STATE_STARTUP:
        case STATE_JUMP_LANDING:
            break;
            
        case STATE_NUM:
            NSAssert(0, @"Unexpected state in acceleration block, state: %d",state);
            break;
    }
}

#pragma mark - Synthesized setter and getter
-(void)setLocalState:(enum local_state_e)localState
{
    // Cancel any delayed state switch timers
    [self cancelDelayedStateSwitch];
    
    // Perform actions on leaving state
    CMAcceleration zero = {0};
    switch (_localState) {
        case STATE_POSSIBLE_STILL:
            self.velocity = zero;
            self.stillGravity = self.movingAverageAccel;
            break;
            
            // Do nothing states.
        case STATE_WAITING:
        case STATE_WAITING_FOR_STILL:
        case STATE_STARTUP:
        case STATE_READY_FOR_JUMP_TIMEOUT:
        case STATE_READY_FOR_JUMP:
        case STATE_NUM:
        case STATE_JUMP_LANDING:
        case STATE_JUMPING:
        case STATE_POSSIBLE_JUMP:
            break;
    }    
    
    // Set new state and start date
    _localState = localState;
    self.stateDate = [NSDate date];
    
    // Do stuff after entering state
    switch (_localState) {
        case STATE_STARTUP:
            self.lblStatus.text = @"Starting up...";
            break;
            
        case STATE_WAITING:
            self.lblStatus.text = @"Ready...";
            [self.btnJump setTitle:@"Press to Jump" forState:UIControlStateNormal];
            break;
            
        case STATE_WAITING_FOR_STILL:
            [self.lblStatus setText:@"Hold still..."];
            break;
            
        case STATE_POSSIBLE_STILL:
            [self.lblStatus setText:@"Hold still..."];
            break;
            
        case STATE_READY_FOR_JUMP:
            [self.lblStatus setText:@"Ok, Jump!"];
            self.readyForJumpDate = self.stateDate;
            break;    
            
        case STATE_READY_FOR_JUMP_TIMEOUT:
            [self.lblStatus setText:@"Still timeout"];
            [self switchToState:STATE_WAITING afterDelay:2];
            break;
            
        case STATE_POSSIBLE_JUMP:
            [self.lblStatus setText:@"Possible"];
            self.jumpDate = [NSDate date];
            self.jumpVelocity = self.velocity;
            break;
        
        case STATE_JUMPING:
            [self.lblStatus setText:@"Jumping..."];
            break;
        
        case STATE_JUMP_LANDING:
            [self endJump];
            break;

        case STATE_NUM:
            NSAssert(0, @"Unexpected state in state setting block, state: %d", _localState);
            break;
    }
}
-(CMAcceleration)movingAverageAccel
{
    return [self movingAverageAccelWithOffset:0 length:kMovingAverageWindow / kAccelSamplingInterval];
}
-(CMAcceleration)lastMovingAverageAccel
{
    return [self movingAverageAccelWithOffset:1 length:kMovingAverageWindow / kAccelSamplingInterval];
}

#pragma mark - Lifecycle
-(void)motionAnalysisSetup
{
    // Create motion manager
    self.motionManager = [[CMMotionManager alloc] init];
    self.motionManager.accelerometerUpdateInterval = kAccelSamplingInterval;
    
    // Get operation queue. Use main one
    NSOperationQueue *operationQueue = [NSOperationQueue mainQueue];
    
    [self.motionManager startAccelerometerUpdatesToQueue:operationQueue withHandler:^(CMAccelerometerData *accelerometerData, NSError *error) {
        
        // Get acceleration data and mag
        CMAcceleration newAccel = accelerometerData.acceleration;
        CMAcceleration lastAccel = [self lastAccelInBuffer];
        
        // Get current time interval and motion state
        NSTimeInterval curTimeInterval = accelerometerData.timestamp;
        int state = self.localState;
        
        // Update history
        [self addAccelToBuffer:newAccel];
        
        // Check our current state
        if (state == STATE_STARTUP){
            // Do we have enough points to move into waiting?
            if (self.accelHistoryFilledLen >= kAccelHistoryLength)
                self.localState = STATE_WAITING;
    
        } else {
            // Elapsed interval
            NSTimeInterval interval = curTimeInterval - self.lastAccelTime;
            
            // Calculate mean accel
            CMAcceleration meanAccel = newAccel;
            accumOpVector(meanAccel, lastAccel, +);
            accumOpScalar(meanAccel, 2.0, /);
            
            // Calculate velocity
            CMAcceleration velAdd = meanAccel;
            accumOpVector(velAdd, self.stillGravity, -);
            accumOpScalar(velAdd, interval*kGravity, *);
            accumOpVector(_velocity, velAdd, +);
            NSLog(@"Vel: %f", mag(self.velocity));    
            
            [self switchStateFromMotion];
        }   
        
        // Update last variables
        self.lastAccelTime = curTimeInterval;
    }];
 
    // Now startup device motion to get gyro rotation matrix
    self.motionManager.deviceMotionUpdateInterval = kAccelSamplingInterval;
    [self.motionManager startDeviceMotionUpdates];
    self.motionManager.deviceMotion.
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    // Set initial state
    self.localState = STATE_WAITING;
    
    // Start motoin
    [self motionAnalysisSetup];
}
    

- (void)viewDidUnload
{
    [self setLblStatus:nil];
    [self setBtnJump:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
}

@end
