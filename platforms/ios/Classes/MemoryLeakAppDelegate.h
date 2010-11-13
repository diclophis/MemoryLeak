//
//  MemoryLeakAppDelegate.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import <UIKit/UIKit.h>

@class EAGLView;

@interface MemoryLeakAppDelegate : NSObject <UIApplicationDelegate, UIWebViewDelegate> {
    UIWindow *window;
    EAGLView *glView;
	UIWebView *webView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;
@property (nonatomic, retain) IBOutlet UIWebView *webView;

@end