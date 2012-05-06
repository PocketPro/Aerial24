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

@protocol PlotViewControllerDelegate <NSObject>
-(NSUInteger)numberOfRecordsForPlot:(CPTPlot *)plot userInfo:(NSDictionary *)dictionary;
-(NSNumber *)numberForPlot:(CPTPlot *)plot field:(NSUInteger)fieldEnum recordIndex:(NSUInteger)index userInfo:(NSDictionary *)dictionary;

@optional
- (NSArray *)titlesForSegmentedControl;
@end

@interface PlotViewController : UIViewController <CPTScatterPlotDataSource>
@property (weak, nonatomic) IBOutlet CPTGraphHostingView *hostingView;
@property (weak, nonatomic) id <PlotViewControllerDelegate> delegate;
@property (weak, nonatomic) IBOutlet UISegmentedControl *segmentedControl;

@end
