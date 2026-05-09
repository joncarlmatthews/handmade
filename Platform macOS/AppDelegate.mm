#import "AppDelegate.h"
#import "mac_handmade.h"

@interface AppDelegate ()

@property (strong) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    (void)aNotification;
    MacHandmadeApplicationDidFinishLaunching(self.window);
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    (void)aNotification;
    MacHandmadeApplicationWillTerminate();
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


@end
