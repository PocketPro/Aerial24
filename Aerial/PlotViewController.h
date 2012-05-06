//
//  PlotViewController.h
//  Aerial
//
//  Created by Gord Parke on 5/5/12.
//  Copyright (c) 2012 PPG Technologies. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CorePlot-CocoaTouch.h"

@class MotionRecognizer;

@interface PlotViewController : UIViewController <CPTScatterPlotDataSource>
@property (weak, nonatomic) IBOutlet CPTGraphHostingView *hostingView;

-(void)setPlotDataSourceToMotionRecognizer:(MotionRecognizer *)recognizer;

@end
