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
	[NSTimer scheduledTimerWithTimeInterval:5.0 target:self selector:@selector(toggleWebView:) userInfo:nil repeats:NO];
	//[[webView.subviews objectAtIndex:0] setScrollEnabled:NO];  //to stop scrolling completely
	[[webView.subviews objectAtIndex:0] setBounces:NO]; //to stop bouncing
	[webView setScalesPageToFit:NO];
	[webView setBackgroundColor:[UIColor clearColor]];
	[webView loadHTMLString:[NSString stringWithContentsOfURL:[NSURL URLWithString:@"http://192.168.1.144:3000/OFConnectJavascript/index.html"] encoding:NSUTF8StringEncoding error:nil] baseURL:[NSURL URLWithString:@"https://api.openfeint.com/"]];
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
		[webView setFrame:CGRectMake(0.0, 0.0, window.frame.size.width, window.frame.size.height * 0.3)];
	} else {
		//[webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"javascript:alert('foo');"]]];
		
		[webView stringByEvaluatingJavaScriptFromString:@"javascript:(function() { alert('wang'); })()"];
		//[glView setFrame:CGRectMake(0.0, 0.0, window.frame.size.width, window.frame.size.height)];
		//[webView setFrame:CGRectMake(0.0, window.frame.size.height, window.frame.size.width, window.frame.size.height * 0.3)];
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
