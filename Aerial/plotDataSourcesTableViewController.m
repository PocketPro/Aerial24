//
//  plotDataSourcesTableViewController.m
//  Aerial
//
//  Created by Gord Parke on 5/5/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "plotDataSourcesTableViewController.h"
#import "MotionRecognizer.h"
#import "SensingCore.h"
#import "MotionTimeline.h"
#import "PlotViewController.h"
#import "GolfSwing_math.h"

@interface plotDataSourcesTableViewController ()

@end

@implementation plotDataSourcesTableViewController


- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    // Get plot view controller (the destination VC) and the segue identifier
    UITableViewCell *cell = [self.tableView cellForRowAtIndexPath:self.tableView.indexPathForSelectedRow];
    NSString *identifier = cell.textLabel.text;
    PlotViewController *plotViewController = (PlotViewController *)[segue destinationViewController];
    
    if ([identifier isEqualToString:@"Past 3 Seconds"]) {
        plotViewController.delegate = self;
    } else {
        // Use identifier to construct class of motion recognizer.  This assumes
        // some kind of formatting:  <idnetifier>MotionRecognizer
        NSString *prefix = [identifier capitalizedString];
        NSString *classString = [prefix stringByAppendingString:@"MotionRecognizer"];
        Class class = NSClassFromString(classString);
        
        // Loop through all recognizers in default timeline until we find one of the right class
        NSSet *allRecognizers = [[[SensingCore sharedInstance] motionTimeline] allMotionRecognizers];
        for (MotionRecognizer *recognizer in allRecognizers){
            if ([recognizer isKindOfClass:class]){
                plotViewController.delegate = recognizer;
                break;
            }
        }
    }
}


// Delegate method that returns the number of points on the plot
static const NSInteger numSamples =  175;
-(NSUInteger)numberOfRecordsForPlot:(CPTPlot *)plot userInfo:(NSDictionary *)dictionary
{
    // Number of samples in three seconds
    // Get motion sample and index
    MotionSample_t *firstSample = [[[SensingCore sharedInstance] motionTimeline] sampleAtPastIndex:numSamples];
    if (firstSample == nil)
        return 0;
    
    if ( [plot.identifier isEqual:@"mainplot"]  )
    {
        return numSamples;
    }
    
    return 0;
}

// Delegate method that returns a single X or Y value for a given plot.
-(NSNumber *)numberForPlot:(CPTPlot *)plot field:(NSUInteger)fieldEnum recordIndex:(NSUInteger)index userInfo:(NSDictionary *)userInfo
{
    if ( [plot.identifier isEqual:@"mainplot"] )
    {        
        // Get motion sample and index
        MotionSample_t *firstSample = [[[SensingCore sharedInstance] motionTimeline] sampleAtPastIndex:numSamples];
        MotionSample_t *sample = [[[SensingCore sharedInstance] motionTimeline] 
                                  sampleAtPastIndex:numSamples - index];
        
        if (firstSample == nil || sample == nil)
            return nil;
        
        // FieldEnum determines if we return an X or Y value.
        if ( fieldEnum == CPTScatterPlotFieldX )
        {
            return [NSNumber numberWithDouble:(sample->timestamp - firstSample->timestamp) * 1E3];
        }
        else    // Y-Axis
        {
            NSString *title = [userInfo objectForKey:@"SelectedSegmentTitle"];
            GSFloat mag;
            if ([title isEqualToString:@"Mag Deriv"]){
                mag = sample->vbAccelerationMagDerivative;
            }  else if ([title isEqualToString:@"Acceleration"]) {
                mag = GSVectorMagnitudeD(sample->vbAcceleration, 3);
            } else if ([title isEqualToString:@"Deriv Mag"]){
                mag = GSVectorMagnitudeD(sample->vbAccelerationDerivative, 3);
            }
            
            return [NSNumber numberWithFloat:mag];
        }
    }
    
    return [NSNumber numberWithFloat:0];
}
-(NSArray *)titlesForSegmentedControl
{
    return [NSArray arrayWithObjects:@"Acceleration", @"Mag Deriv", @"Deriv Mag", nil];
}


- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


@end
