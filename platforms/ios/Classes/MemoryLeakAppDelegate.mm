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
@synthesize webView;


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
		[[webView.subviews objectAtIndex:0] setScrollEnabled:NO];  //to stop scrolling completely
		[[webView.subviews objectAtIndex:0] setBounces:NO]; //to stop bouncing
		[webView setScalesPageToFit:NO];
		[webView setBackgroundColor:[UIColor clearColor]];
		[webView setAllowsInlineMediaPlayback:YES];
		[webView setMediaPlaybackRequiresUserAction:NO];
    //[webView setBaseUrl:@"wtf"];
    NSURL *url = [[NSBundle mainBundle] URLForResource:@"index" withExtension:@"html" subdirectory:@"assets/offline"];
    NSString *guts = [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:nil];
    [webView loadHTMLString:guts baseURL:[[NSBundle mainBundle] bundleURL]];

		[glView build:webView];
		[glView startGame:[NSNumber numberWithInt:0]];
		[glView startAnimation];
	}
}


-(BOOL)webView:(UIWebView *)theWebView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
  NSLog(@"wtf load? %@", [request URL]);
	return YES;
}


- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error {
  NSLog(@"FAIL!!!!!! %@", error);
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
