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


-(void)applicationWillResignActive:(UIApplication *)application {
    [glView stopAnimation];
}


-(void)applicationDidBecomeActive:(UIApplication *)application {
	on = YES;
	[self toggleWebView:nil];
	//[NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(toggleWebView:) userInfo:nil repeats:NO];
	//[[webView.subviews objectAtIndex:0] setScrollEnabled:NO];  //to stop scrolling completely
	[[webView.subviews objectAtIndex:0] setBounces:NO]; //to stop bouncing
	[webView setScalesPageToFit:NO];
	
	[webView loadHTMLString:[NSString stringWithContentsOfURL:[NSURL URLWithString:@"http://localhost:3000/OFConnectJavascript/index.html"]] baseURL:[NSURL URLWithString:@"http://openfeint.com/"]];
	
	 //[webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"http://localhost:3000/OFConnectJavascript/index.html"]]];

	//[webView loadHTMLString:@"<html><body onclick=\"alert('foo');\"style=\"width: 5000px; height: 70px;\"><img style=\"width: 100%; height: 100%;\"src=\"http://upload.wikimedia.org/wikipedia/commons/thumb/4/43/Pont_de_Brooklyn_de_nuit_-_Octobre_2008.jpg/5000px-Pont_de_Brooklyn_de_nuit_-_Octobre_2008.jpg\"></body></html>" baseURL:nil];
    [glView build];
	[glView startAnimation];
}


-(BOOL)webView:(UIWebView *)theWebView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
	return YES;
}

-(void)toggleWebView:(id)sender {
	//webView.frame.size.height
	[UIView beginAnimations:@"toggleWebView" context:nil];
	[UIView setAnimationDuration:0.33];
	if (on) {
		[glView setFrame:CGRectMake(0.0, 0.0, window.frame.size.width, window.frame.size.height)];
		[webView setFrame:CGRectMake(0.0, window.frame.size.height * 0.7, window.frame.size.width, window.frame.size.height * 0.3)];
	} else {
		[glView setFrame:CGRectMake(0.0, 0.0, window.frame.size.width, window.frame.size.height)];
		[webView setFrame:CGRectMake(0.0, window.frame.size.height, window.frame.size.width, window.frame.size.height * 0.3)];
	}
	on = !on;
	[UIView commitAnimations];
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
