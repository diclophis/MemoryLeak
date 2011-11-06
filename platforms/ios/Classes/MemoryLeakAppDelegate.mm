//
//  MemoryLeakAppDelegate.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//


#import "MemoryLeak.h"
#import "MemoryLeakAppDelegate.h"
#import "EAGLView.h"


@implementation MemoryLeakAppDelegate


@synthesize window;
@synthesize glView;


-(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    return YES;
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return YES;
}


-(void)applicationWillResignActive:(UIApplication *)application {
    [glView stopAnimation];
}


-(void)applicationDidBecomeActive:(UIApplication *)application {
	if ([glView wasActive]) {
    NSLog(@"starting animation again!!!");
		[glView startAnimation];
	} else {
		[glView build];
		[glView startGame:[NSNumber numberWithInt:3]];
		[glView startAnimation];
	}
}


-(void)applicationWillTerminate:(UIApplication *)application {
	[glView stopAnimation];
}


-(void)dealloc {
    [window release];
    [glView release];	
    [super dealloc];
}


@end