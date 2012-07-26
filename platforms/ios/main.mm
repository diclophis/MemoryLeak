//
//  main.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright GPL
//

#import <UIKit/UIKit.h>
#import "MemoryLeakAppDelegate.h"

extern "C" {
  #include "memmgr.h"
}

int main(int argc, char *argv[]) {
    memmgr_init();
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, NSStringFromClass([MemoryLeakAppDelegate class]));
    [pool release];
    return retVal;
}
