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
#import "GolfSwing_types.h"
#import "GolfSwing_math.h"

@interface ViewController ()

// IO elements
@property (weak, nonatomic) IBOutlet UILabel *lblStatus;
@property (weak, nonatomic) IBOutlet UILabel *lblDetails;
@property (weak, nonatomic) IBOutlet UIButton *btnJump;

// Motion recognizers
@property (weak, nonatomic) ThrowMotionRecognizer *throwMotionRecognizer;
@property (weak, nonatomic) CatchMotionRecognizer *catchMotionRecognizer;
@end

@implementation ViewController
@synthesize lblStatus = _lblStatus;
@synthesize lblDetails = _lblDetails;
@synthesize btnJump = _btnJump;
@synthesize throwMotionRecognizer = _throwMotionRecognizer;
@synthesize catchMotionRecognizer = _catchMotionRecognizer;

#pragma mark - Evaluation functions
typedef struct {
    GSDouble components[9];
} SampleSubmatrix_t;

- (void)solveForMomentOfInertia
{
    SensingCore *sensingCore = [SensingCore sharedInstance];
    MotionTimeline *motionTimeline = sensingCore.motionTimeline;
    MotionSample_t *catchSample = self.catchMotionRecognizer.catchSample;
    MotionSample_t *throwSample = self.throwMotionRecognizer.throwSample;
    NSUInteger freeFallSamplesCount = [motionTimeline numberOfSamplesBetweenStart:throwSample end:catchSample];
    
    NSLog(@"Free fall duration: %d --> %f",freeFallSamplesCount,catchSample->timestamp - throwSample->timestamp );
    
    // Now we do a minimum least squares fit on the equation Ax = 0 where A is composed of the angular velocity and
    // its derivatives, while x is a vector containting the principal moment of inertia axes: [I1 I2 I3]'
    
    // In this evaluation function, lets put a buffer of 3 samples on either side of the supposed 'throw' and 'catch'
    GSInt sampleCount = freeFallSamplesCount - 6;
    SampleSubmatrix_t *A = malloc(sizeof(SampleSubmatrix_t)*sampleCount);
    GSDouble *At = malloc(sizeof(SampleSubmatrix_t)*sampleCount); // Transpose of a
    
    // Fill out A
    GSDouble aveInterval = 0;
    for (GSInt i = 0; i < sampleCount; ++i){
        MotionSample_t *s = [motionTimeline sampleForNumber:i+3 ofSamplesAfter:throwSample];
        
        GSDouble submatrix[9] = {
            s->vbAngularVelocityDerivative[0], -(s->vbAngularVelocity[1]*s->vbAngularVelocity[2]), s->vbAngularVelocity[1]*s->vbAngularVelocity[2],
            s->vbAngularVelocity[0]*s->vbAngularVelocity[2], s->vbAngularVelocityDerivative[1], -(s->vbAngularVelocity[0]*s->vbAngularVelocity[2]),
            -(s->vbAngularVelocity[0]*s->vbAngularVelocity[1]), s->vbAngularVelocity[0]*s->vbAngularVelocity[1], s->vbAngularVelocityDerivative[2] 
        };
        
        GSVectorCopyD(A[i].components, submatrix, 9);
        
        // Sum time interval
        MotionSample_t *s_old = [motionTimeline sampleForNumber:i+2 ofSamplesAfter:throwSample];
        aveInterval += (s->timestamp - s_old->timestamp);
    }
    aveInterval /= sampleCount;
    
    // Create a symmetric, quare matrix by multiplying A from the left by its transpose
    GSDouble mSymmetric[9];
    GSMatrixTransposeOutOfPlaceD(At, (GSDouble *)A, sampleCount*3, 3);
    GSMatrixMatrixRectangularMultiplyD(mSymmetric, At, (GSDouble *)A, 3, 3, sampleCount*3);
    
    // Now find the eigenvalues and eigenvectors of this square matrix
    GSDouble eigVal[3], eigVec[3][3];
    GSErr err = GSEigenvaluesAndEigenvectors(eigVal, eigVec[0], mSymmetric, 3);
    assert(err == GSSuccess && "GSEigenvaluesAndEigenvectors in inertia solving routine exited with error");
    
    // We wanted the eigenvector associated with the minimum eigenvalue. Since the eigenvalues are sorted in ascending order, we want the first vector.
    GSDouble inertiaVector[3];
    GSVectorNormalizeD(inertiaVector, eigVec[0], 3);
    
    // We've found the normalized eigenvector to within a sign.  To fix the sign, we use the fact that all elements in the inertia vector should be +'ve
    if (inertiaVector[0] < 0 || inertiaVector[1] < 0 || inertiaVector[2] < 0)
        GSVectorScalarMultiplyD(inertiaVector, inertiaVector, -1.0, 3);
    
    // Calculate standard deviation of error in measurements (which we assume is dominated by the derivative term)
    const GSFloat gyroSTD = 0.005;
    GSFloat std = sqrt(2.0)/aveInterval * gyroSTD;
    
    // Calculate a chi-sq value
    GSDouble *vErrors = malloc(sizeof(GSDouble)*3*sampleCount);
    GSVectorMatrixRectangularMultiplyD(vErrors, inertiaVector, (GSDouble *) A, 3*sampleCount, 3);
    GSDouble chisq = GSVectorDotProductD(vErrors, vErrors, 3*sampleCount) / pow(std, 2.0);
    
    // For now, just print out our findings
    NSString *findings = [NSString stringWithFormat:@"Inertia vector: <%0.2f, %0.2f, %0.2f>\n"
                                                     "\t(we expect I3 > I1 > I2 > 0)\n"
                                                      "num: %d, chisq: %0.2f, Q: %f",
                          inertiaVector[0],inertiaVector[1],inertiaVector[2], 
                          sampleCount, chisq, GSGammaQ(0.5*(sampleCount*3-3), 0.5*chisq)];
    NSLog(@"%@",findings);
    self.lblDetails.text = findings;
    
    free(A);
    free(At);
}

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
}

#pragma mark Catch
- (void)catchMotionRecognizerChangedState:(CatchMotionRecognizer *)catchMotionRecognizer
{
    if (catchMotionRecognizer.state == MotionRecognizerStateEnded){
        self.lblStatus.text = (catchMotionRecognizer.isFumble ? @"FUMBLE!" : @"Catch" );
        [[SensingCore sharedInstance] stopSensing];
        [self solveForMomentOfInertia];
    }
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
    self.throwMotionRecognizer = throwMotionRecognizer;
    
    // Catch
    CatchMotionRecognizer *catchMotionRecognizer = [[CatchMotionRecognizer alloc] initWithTarget:self action:@selector(catchMotionRecognizerChangedState:)];
    [catchMotionRecognizer requireMotionRecognizer:throwMotionRecognizer toAchieveState:MotionRecognizerStateEnded]; 
    [motionTimeline addMotionRecognizer:catchMotionRecognizer];
    self.catchMotionRecognizer = catchMotionRecognizer;
    
    // Start motion
    [sensingCore startSensing];
}
    

- (void)viewDidUnload
{
    [self setLblStatus:nil];
    [self setBtnJump:nil];
    [self setLblDetails:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}


@end
