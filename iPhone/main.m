//
//  main.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import <UIKit/UIKit.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    
	#define VALGRIND "/usr/local/bin/valgrind"
	
	if (YES) {
		/* Using the valgrind build config, rexec ourself
		 * in valgrind */
		if (argc < 2 || (argc >= 2 && strcmp(argv[1], "-valgrind") != 0)) {
			execl(VALGRIND, VALGRIND, "--dsymutil=yes", "--leak-check=full", argv[0], "-valgrind", NULL);
		}
	}
		
	
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    [pool release];
    return retVal;
}
