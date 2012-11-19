#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <QuartzCore/QuartzCore.h>
#import <Cocoa/Cocoa.h>

#import "GLView.h"

@implementation GLView

-(void)prepareOpenGL {
  NSLog(@"prepare");
}

-(void)reshape {
  NSLog(@"reshape");
}

-(void)drawRect:(NSRect)rect {
  NSLog(@"draw");
}

@end
