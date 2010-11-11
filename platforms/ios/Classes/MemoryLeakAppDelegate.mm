//
//  MemoryLeakAppDelegate.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//


#import "MemoryLeakAppDelegate.h"
#import "EAGLView.h"

@implementation MemoryLeakAppDelegate


@synthesize window;
@synthesize glView;
@synthesize webView;


-(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	[webView setBackgroundColor:[UIColor clearColor]];
	[webView loadRequest:[NSURLRequest requestWithURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"assets"] isDirectory:NO]]];
	
    return YES;
}


-(BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
	//NSLog(@"the fuck: %@", [[request mainDocumentURL] scheme]);
	//NSLog(@"the fuck: %@", [[request mainDocumentURL] path]);
	NSLog(@"the fuck: %@", [[request mainDocumentURL] fragment]);
	if ([[[request mainDocumentURL] scheme] isEqualToString:@"file"]) {
		return YES;
	} else {
		return NO;
	}
}


-(void)applicationWillResignActive:(UIApplication *)application {
    [glView stopAnimation];
}


-(void)applicationDidBecomeActive:(UIApplication *)application {
    [glView startAnimation];
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