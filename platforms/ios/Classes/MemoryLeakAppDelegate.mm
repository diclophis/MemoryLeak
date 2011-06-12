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
	[[webView.subviews objectAtIndex:0] setScrollEnabled:NO];  //to stop scrolling completely
	[[webView.subviews objectAtIndex:0] setBounces:NO]; //to stop bouncing
	[webView setScalesPageToFit:NO];
	[webView setBackgroundColor:[UIColor clearColor]];
	[webView setAllowsInlineMediaPlayback:YES];
	[webView setMediaPlaybackRequiresUserAction:NO];
	[webView loadHTMLString:[NSString stringWithContentsOfURL:[NSURL URLWithString:@"http://localhost:9292/"] encoding:NSUTF8StringEncoding error:nil] baseURL:[NSURL URLWithString:@"https://api.openfeint.com/?key=lxJAPbgkzhW91LqMeXEIg&secret=anQAUrXZTMfJxP8bLOMzmhfBlpuZMH9UPw45wCkGsQ"]];
	[glView build:webView];
	[glView startMemoryLeak];
	//[glView startAnimation];
}


-(BOOL)webView:(UIWebView *)theWebView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
	return YES;
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