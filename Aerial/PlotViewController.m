//
//  PlotViewController.m
//  Aerial
//
//  Created by Gord Parke on 5/5/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import "PlotViewController.h"

@interface PlotViewController ()
@property (nonatomic, strong) CPTXYGraph *graph;
@end

@implementation PlotViewController
@synthesize hostingView = _hostingView;
@synthesize graph = _graph;

-(void)setPlotDataSourceToMotionRecognizer:(MotionRecognizer *)recognizer
{
    NSArray *plots = [self.graph allPlots];
    
    for (CPTScatterPlot *plot in plots){   
        plot.dataSource = (id<CPTScatterPlotDataSource>) recognizer;
        
        // Scale to fit plots
        [self.graph reloadData];
        [self.graph.defaultPlotSpace scaleToFitPlots:[self.graph allPlots]];
    }
}

# pragma mark - Graphing
// This does the actual work of creating the plot if we don't already have a graph object.
-(void)initialisePlot
{
    // Start with some simple sanity checks before we kick off
    if ( (self.view == nil) ) {
        NSLog(@"TUTSimpleScatterPlot: Cannot initialise plot without hosting view or data.");
        return;
    }
    
    if ( self.graph != nil ) {
        NSLog(@"TUTSimpleScatterPlot: Graph object already exists.");
        return;
    }
    
    // Create a graph object which we will use to host just one scatter plot.
    CGRect frame = [self.hostingView bounds];
    self.graph = [[CPTXYGraph alloc] initWithFrame:frame];
    
    // Add some padding to the graph, with more at the bottom for axis labels.
    self.graph.plotAreaFrame.paddingTop = 10.0f;
    self.graph.plotAreaFrame.paddingRight = 10.0f;
    self.graph.plotAreaFrame.paddingBottom = 30.0f;
    self.graph.plotAreaFrame.paddingLeft = 30.0f;
    
    // Tie the graph we've created with the hosting view.
    self.hostingView.hostedGraph = self.graph;
    
    // If you want to use one of the default themes - apply that here.
    //[self.graph applyTheme:[CPTTheme themeNamed:kCPTDarkGradientTheme]];
    
    // Create a line style that we will apply to the axis and data line.
    CPTMutableLineStyle *lineStyle = [CPTMutableLineStyle lineStyle];
    lineStyle.lineColor = [CPTColor whiteColor];
    lineStyle.lineWidth = 2.0f;
    
    // Create a text style that we will use for the axis labels.
    CPTMutableTextStyle *textStyle = [CPTMutableTextStyle textStyle];
    textStyle.fontName = @"Helvetica";
    textStyle.fontSize = 14;
    textStyle.color = [CPTColor whiteColor];
    
    // Create the plot symbol we're going to use.
    CPTPlotSymbol *plotSymbol = [CPTPlotSymbol crossPlotSymbol];
    plotSymbol.lineStyle = lineStyle;
    plotSymbol.size = CGSizeMake(8.0, 8.0);
    
    // Setup some floats that represent the min/max values on our axis.
    /*float xAxisMin = -10;
    float xAxisMax = 10;
    float yAxisMin = 0;
    float yAxisMax = 100;
    
    // We modify the graph's plot space to setup the axis' min / max values.
    CPTXYPlotSpace *plotSpace = (CPTXYPlotSpace *)self.graph.defaultPlotSpace;
    plotSpace.xRange = [CPTPlotRange plotRangeWithLocation:CPTDecimalFromFloat(xAxisMin) length:CPTDecimalFromFloat(xAxisMax - xAxisMin)];
    plotSpace.yRange = [CPTPlotRange plotRangeWithLocation:CPTDecimalFromFloat(yAxisMin) length:CPTDecimalFromFloat(yAxisMax - yAxisMin)];
     */
    
    // Modify the graph's axis with a label, line style, etc.
    CPTXYAxisSet *axisSet = (CPTXYAxisSet *)self.graph.axisSet;
     
    //axisSet.xAxis.title = @"Data X";
    //axisSet.xAxis.titleTextStyle = textStyle;
    //axisSet.xAxis.titleOffset = 30.0f;
    axisSet.xAxis.axisLineStyle = lineStyle;
    axisSet.xAxis.majorTickLineStyle = lineStyle;
    axisSet.xAxis.minorTickLineStyle = lineStyle;
    axisSet.xAxis.labelTextStyle = textStyle;
    axisSet.xAxis.labelingPolicy = CPTAxisLabelingPolicyAutomatic;
    //axisSet.xAxis.labelOffset = 3.0f;
    //axisSet.xAxis.majorIntervalLength = CPTDecimalFromFloat(2.0f);
    //axisSet.xAxis.minorTicksPerInterval = 1;
    //axisSet.xAxis.minorTickLength = 5.0f;
    //axisSet.xAxis.majorTickLength = 7.0f;
    
    //axisSet.yAxis.title = @"Data Y";
    //axisSet.yAxis.titleTextStyle = textStyle;
    //axisSet.yAxis.titleOffset = 40.0f;
    axisSet.yAxis.axisLineStyle = lineStyle;
    axisSet.yAxis.majorTickLineStyle = lineStyle;
    axisSet.yAxis.minorTickLineStyle = lineStyle;
    axisSet.yAxis.labelTextStyle = textStyle;
    //axisSet.yAxis.labelOffset = 3.0f;
    //axisSet.yAxis.majorIntervalLength = CPTDecimalFromFloat(10.0f);
    //axisSet.yAxis.minorTicksPerInterval = 1;
    //axisSet.yAxis.minorTickLength = 5.0f;
    //axisSet.yAxis.majorTickLength = 7.0f;
    
    CPTMutableLineStyle *majorGridLineStyle = [CPTMutableLineStyle lineStyle];
    majorGridLineStyle.lineColor = [CPTColor lightGrayColor];
    majorGridLineStyle.lineWidth = 1.0f;
    
    CPTMutableLineStyle *minorGridLineStyle = [CPTMutableLineStyle lineStyle];
    minorGridLineStyle.lineColor = [CPTColor lightGrayColor];
    minorGridLineStyle.dashPattern = [NSArray arrayWithObjects:[NSNumber numberWithFloat:10], [NSNumber numberWithFloat:10], nil];
    minorGridLineStyle.lineWidth = 1.0f;
    
    axisSet.yAxis.majorGridLineStyle = majorGridLineStyle;
    axisSet.yAxis.minorGridLineStyle = minorGridLineStyle;
    
    // Add a plot to our graph and axis. We give it an identifier so that we
    // could add multiple plots (data lines) to the same graph if necessary.
    CPTScatterPlot *plot = [[CPTScatterPlot alloc] init];
    plot.dataSource = self;
    plot.identifier = @"mainplot";
    plot.dataLineStyle = lineStyle;
    //plot.plotSymbol = plotSymbol;
    [self.graph addPlot:plot];
    
    // Turn on interacivity
    [self.graph.defaultPlotSpace setAllowsUserInteraction:YES];
    
    // Scale to fit plots
    [self.graph reloadData];
    [self.graph.defaultPlotSpace scaleToFitPlots:[self.graph allPlots]];
}

// Delegate method that returns the number of points on the plot
-(NSUInteger)numberOfRecordsForPlot:(CPTPlot *)plot
{
    if ( [plot.identifier isEqual:@"mainplot"] )
    {
        return 5;
    }
    
    return 0;
}

// Delegate method that returns a single X or Y value for a given plot.
-(NSNumber *)numberForPlot:(CPTPlot *)plot field:(NSUInteger)fieldEnum recordIndex:(NSUInteger)index
{
    if ( [plot.identifier isEqual:@"mainplot"] )
    {        
        // FieldEnum determines if we return an X or Y value.
        if ( fieldEnum == CPTScatterPlotFieldX )
        {
            return [NSNumber numberWithFloat:index];
        }
        else    // Y-Axis
        {
            return [NSNumber numberWithFloat:index];
        }
    }
    
    return [NSNumber numberWithFloat:0];
}

# pragma mark - Lifecycle
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

-(void)awakeFromNib
{
    [self initialisePlot];

}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
}

- (void)viewDidUnload
{
    [self setHostingView:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

@end
