#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <QuartzCore/QuartzCore.h>
#import <Cocoa/Cocoa.h>

#import "GLView.h"

@implementation GLView
- (void)prepareOpenGL
{
//lock = [NSRecursiveLock new];
NSLog(@"prepare");
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //glClearColor(0.2f, 1.0f, 0.2f, 0.0f);
    GLfloat light_diffuse[] = { 1, 1, 1, 1 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    GLfloat light_position[] = { 0, 0, 5, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

- (void)reshape
{
NSLog(@"reshape");
//  [lock lock];
//          [[self openGLContext] makeCurrentContext];

    const NSSize size = self.bounds.size;
    glViewport(0,0,size.width,size.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,size.width/size.height,0.1f,100.0f);
//    [lock unlock];
}

- (void)drawRect:(NSRect)rect
{
NSLog(@"draw");
//  [lock lock];
//          [[self openGLContext] makeCurrentContext];

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glTranslatef(0.0f,0.0f,-6);
    //glRotatef(20,0,0,1);
    //glRotatef(20,1,0,0);
    //glColor3f(1,0,0);
    //glutSolidCube(2);
    [[self openGLContext] flushBuffer];
//    [lock unlock];
}
@end
