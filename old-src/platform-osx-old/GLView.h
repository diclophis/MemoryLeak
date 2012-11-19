#import <Cocoa/Cocoa.h>

@interface GLView : NSOpenGLView {
  NSLock *lock;
}
@end
