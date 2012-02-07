//
//  main.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright GPL
//

#import <UIKit/UIKit.h>

int main(int argc, char *argv[]) {		
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    [pool release];
    return retVal;
}
