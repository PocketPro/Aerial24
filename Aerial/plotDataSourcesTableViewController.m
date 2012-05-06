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

@interface plotDataSourcesTableViewController ()

@end

@implementation plotDataSourcesTableViewController

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    // Get plot view controller (the destination VC) and the segue identifier
    UITableViewCell *cell = [self.tableView cellForRowAtIndexPath:self.tableView.indexPathForSelectedRow];
    NSString *identifier = cell.textLabel.text;
    PlotViewController *plotViewController = (PlotViewController *)[segue destinationViewController];
    
    // Use identifier to construct class of motion recognizer.  This assumes
    // some kind of formatting:  <idnetifier>MotionRecognizer
    NSString *prefix = [identifier capitalizedString];
    NSString *classString = [prefix stringByAppendingString:@"MotionRecognizer"];
    Class class = NSClassFromString(classString);
    
    // Loop through all recognizers in default timeline until we find one of the right class
    NSSet *allRecognizers = [[[SensingCore sharedInstance] motionTimeline] allMotionRecognizers];
    for (MotionRecognizer *recognizer in allRecognizers){
        if ([recognizer isKindOfClass:class]){
            [plotViewController setPlotDataSourceToMotionRecognizer:recognizer];
            break;
        }
    }
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
