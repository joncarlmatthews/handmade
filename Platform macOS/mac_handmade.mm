#import "mac_handmade.h"

#include <CoreGraphics/CoreGraphics.h>
#include <mach/mach_time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <wchar.h>

#include "../Game/game.h"

extern "C" GAME_UPDATE(gameUpdate);
extern "C" GAME_INIT_FRAME_BUFFER(gameInitFrameBuffer);
extern "C" GAME_INIT_AUDIO_BUFFER(gameInitAudioBuffer);

#define MAC_HANDMADE_FRAME_BUFFER_WIDTH 1280
#define MAC_HANDMADE_FRAME_BUFFER_HEIGHT 720
#define MAC_HANDMADE_BYTES_PER_PIXEL 4
#define MAC_HANDMADE_TARGET_FPS 60.0

typedef struct MacFrameBuffer
{
    uint32 width;
    uint32 height;
    uint16 bytesPerPixel;
    uint32 byteWidthPerRow;
    void *memory;
} MacFrameBuffer;

typedef struct MacState
{
    wchar_t absPath[GAME_MAX_PATH];
    NSWindow *window;
    NSView *view;
    NSTimer *frameTimer;
    uint64 lastCounter;
    mach_timebase_info_data_t timebaseInfo;
    bool8 running;
} MacState;

global_var MacState macState = {};
global_var MacFrameBuffer macFrameBuffer = {};
global_var GameFrameBuffer gameFrameBuffer = {};
global_var GameAudioBuffer gameAudioBuffer = {};
global_var GameMemory gameMemory = {};
global_var GameInput gameInput = {};
global_var GameInput gameInputPrevState = {};

static NSString *MacNSStringFromWide(const wchar_t *wideString)
{
    if (!wideString) {
        return @"";
    }

    size_t utf8Size = wcstombs(NULL, wideString, 0);
    if (utf8Size == (size_t)-1) {
        return @"";
    }

    char *utf8 = (char *)malloc(utf8Size + 1);
    if (!utf8) {
        return @"";
    }

    wcstombs(utf8, wideString, utf8Size + 1);
    NSString *result = [NSString stringWithUTF8String:utf8];
    free(utf8);

    return result ? result : @"";
}

static void MacWriteWidePathFromUTF8(wchar_t *destination, size_t destinationCount, const char *source)
{
    if (!destination || destinationCount == 0) {
        return;
    }

    destination[0] = 0;

    if (!source) {
        return;
    }

    mbstowcs(destination, source, destinationCount - 1);
    destination[destinationCount - 1] = 0;
}

static NSString *MacPathFromWideParts(const wchar_t *basePath, const wchar_t *relativePath)
{
    NSString *base = MacNSStringFromWide(basePath);
    NSString *relative = MacNSStringFromWide(relativePath);
    relative = [relative stringByReplacingOccurrencesOfString:@"\\" withString:@"/"];

    return [base stringByAppendingPathComponent:relative];
}

static void MacSetDevelopmentRootPath(void)
{
    NSString *sourceFile = [NSString stringWithUTF8String:__FILE__];
    NSString *platformFolder = [sourceFile stringByDeletingLastPathComponent];
    NSString *repoRoot = [platformFolder stringByDeletingLastPathComponent];
    NSString *repoRootWithSlash = [repoRoot stringByAppendingString:@"/"];

    MacWriteWidePathFromUTF8(macState.absPath,
                             countArray(macState.absPath),
                             [repoRootWithSlash fileSystemRepresentation]);
}

static uint64 MacGetWallClock(void)
{
    return mach_absolute_time();
}

static float32 MacGetSecondsElapsed(uint64 start, uint64 end)
{
    uint64 elapsed = end - start;
    uint64 nanos = elapsed * macState.timebaseInfo.numer / macState.timebaseInfo.denom;
    return (float32)((double)nanos / 1000000000.0);
}

static void MacProcessButton(GameControllerBtnState *button, bool32 endedDown)
{
    button->wasDown = button->endedDown;
    button->endedDown = endedDown;
}

static void MacRefreshWasDown(GameControllerBtnState *button, GameControllerBtnState previous)
{
    button->wasDown = previous.endedDown;
}

static void MacBeginInputFrame(void)
{
    MacRefreshWasDown(&gameInput.keyboard.dPadUp, gameInputPrevState.keyboard.dPadUp);
    MacRefreshWasDown(&gameInput.keyboard.dPadDown, gameInputPrevState.keyboard.dPadDown);
    MacRefreshWasDown(&gameInput.keyboard.dPadLeft, gameInputPrevState.keyboard.dPadLeft);
    MacRefreshWasDown(&gameInput.keyboard.dPadRight, gameInputPrevState.keyboard.dPadRight);
    MacRefreshWasDown(&gameInput.keyboard.down, gameInputPrevState.keyboard.down);
    MacRefreshWasDown(&gameInput.keyboard.shoulderR1, gameInputPrevState.keyboard.shoulderR1);
    MacRefreshWasDown(&gameInput.mouse.leftButton, gameInputPrevState.mouse.leftButton);
    MacRefreshWasDown(&gameInput.mouse.rightButton, gameInputPrevState.mouse.rightButton);
}

PLATFORM_LOG(platformLog)
{
    va_list args;
    va_start(args, str);
    vfwprintf(stderr, str, args);
    va_end(args);
}

PLATFORM_ALLOCATE_MEMORY(platformAllocateMemory)
{
    (void)thread;

    int protection = PROT_READ | PROT_WRITE;
    int flags = MAP_PRIVATE | MAP_ANON;

    if (memoryStartAddress != 0) {
        flags |= MAP_FIXED;
    }

    void *baseAddress = (memoryStartAddress != 0) ? (void *)memoryStartAddress : NULL;
    void *result = mmap(baseAddress, memorySizeInBytes, protection, flags, -1, 0);
    if (result == MAP_FAILED) {
        result = NULL;
    }

    return result;
}

PLATFORM_FREE_MEMORY(platformFreeMemory)
{
    (void)thread;
    // The current game API does not pass the original allocation size back.
    // Small debug allocations currently leak until process exit on macOS.
    (void)address;
}

PLATFORM_CONTROLLER_VIBRATE(platformControllerVibrate)
{
    (void)thread;
    (void)controllerIndex;
    (void)motor1Speed;
    (void)motor2Speed;
}

#ifdef HANDMADE_LOCAL_BUILD
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_platformReadEntireFile)
{
    (void)thread;

    DEBUG_file result = {};
    NSString *path = MacPathFromWideParts(exeAbsPath, filename);
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data) {
        platformLog(L"macOS: failed to read file: %ls\n", filename);
        return result;
    }

    result.sizeinBytes = (uint32)[data length];
    result.memory = platformAllocateMemory(thread, 0, result.sizeinBytes);
    if (result.memory) {
        memcpy(result.memory, [data bytes], result.sizeinBytes);
    } else {
        result.sizeinBytes = 0;
    }

    return result;
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_platformFreeFileMemory)
{
    (void)thread;
    if (file) {
        file->memory = NULL;
        file->sizeinBytes = 0;
    }
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_platformWriteEntireFile)
{
    (void)thread;

    NSString *path = MacPathFromWideParts(exeAbsPath, filename);
    NSData *data = [NSData dataWithBytesNoCopy:memory
                                        length:memorySizeInBytes
                                  freeWhenDone:NO];

    return [data writeToFile:path atomically:YES] ? true : false;
}
#endif

@interface HandmadeView : NSView
@end

@implementation HandmadeView

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)isFlipped
{
    return FALSE;
}

- (void)drawRect:(NSRect)dirtyRect
{
    (void)dirtyRect;

    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    CGContextSetRGBFillColor(context, 0.0f, 0.0f, 0.0f, 1.0f);
    CGContextFillRect(context, NSRectToCGRect(self.bounds));

    if (!macFrameBuffer.memory) {
        return;
    }

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(NULL,
                                                                  macFrameBuffer.memory,
                                                                  macFrameBuffer.byteWidthPerRow * macFrameBuffer.height,
                                                                  NULL);
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst;
    CGImageRef image = CGImageCreate(macFrameBuffer.width,
                                     macFrameBuffer.height,
                                     8,
                                     32,
                                     macFrameBuffer.byteWidthPerRow,
                                     colorSpace,
                                     bitmapInfo,
                                     dataProvider,
                                     NULL,
                                     false,
                                     kCGRenderingIntentDefault);

    if (image) {
        CGFloat viewWidth = NSWidth(self.bounds);
        CGFloat viewHeight = NSHeight(self.bounds);
        CGFloat scale = MIN(viewWidth / (CGFloat)macFrameBuffer.width,
                            viewHeight / (CGFloat)macFrameBuffer.height);
        CGFloat displayWidth = (CGFloat)macFrameBuffer.width * scale;
        CGFloat displayHeight = (CGFloat)macFrameBuffer.height * scale;
        CGRect displayRect = CGRectMake((viewWidth - displayWidth) * 0.5,
                                        (viewHeight - displayHeight) * 0.5,
                                        displayWidth,
                                        displayHeight);

        CGContextSetInterpolationQuality(context, kCGInterpolationNone);
        CGContextDrawImage(context, displayRect, image);
        CGImageRelease(image);
    }

    CGDataProviderRelease(dataProvider);
    CGColorSpaceRelease(colorSpace);
}

- (void)keyDown:(NSEvent *)event
{
    [self handleKey:event isDown:true];
}

- (void)keyUp:(NSEvent *)event
{
    [self handleKey:event isDown:false];
}

- (void)handleKey:(NSEvent *)event isDown:(bool32)isDown
{
    NSString *characters = [[event charactersIgnoringModifiers] lowercaseString];
    if ([characters length] == 0) {
        return;
    }

    unichar key = [characters characterAtIndex:0];
    switch (key) {
        case 27:
        case 'q': {
            if (!isDown) {
                [NSApp terminate:nil];
            }
        } break;

        case 'w': {
            MacProcessButton(&gameInput.keyboard.dPadUp, isDown);
        } break;

        case 'a': {
            MacProcessButton(&gameInput.keyboard.dPadLeft, isDown);
        } break;

        case 's': {
            MacProcessButton(&gameInput.keyboard.dPadDown, isDown);
        } break;

        case 'd': {
            MacProcessButton(&gameInput.keyboard.dPadRight, isDown);
        } break;

        case 'e': {
            MacProcessButton(&gameInput.keyboard.shoulderR1, isDown);
        } break;

        case ' ': {
            MacProcessButton(&gameInput.keyboard.down, isDown);
        } break;
    }
}

- (void)mouseDown:(NSEvent *)event
{
    (void)event;
    MacProcessButton(&gameInput.mouse.leftButton, true);
}

- (void)mouseUp:(NSEvent *)event
{
    (void)event;
    MacProcessButton(&gameInput.mouse.leftButton, false);
}

- (void)rightMouseDown:(NSEvent *)event
{
    (void)event;
    MacProcessButton(&gameInput.mouse.rightButton, true);
}

- (void)rightMouseUp:(NSEvent *)event
{
    (void)event;
    MacProcessButton(&gameInput.mouse.rightButton, false);
}

- (void)mouseMoved:(NSEvent *)event
{
    [self updateMousePosition:event];
}

- (void)mouseDragged:(NSEvent *)event
{
    [self updateMousePosition:event];
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self updateMousePosition:event];
}

- (void)updateMousePosition:(NSEvent *)event
{
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    gameInput.mouse.isConnected = true;
    gameInput.mouse.position.x = (int32)point.x;
    gameInput.mouse.position.y = (int32)(NSHeight(self.bounds) - point.y);
}

@end

static void MacUpdateAndRender(NSTimer *timer)
{
    (void)timer;

    if (!macState.running) {
        return;
    }

    uint64 currentCounter = MacGetWallClock();
    float32 deltaTime = MacGetSecondsElapsed(macState.lastCounter, currentCounter);
    macState.lastCounter = currentCounter;

    MacBeginInputFrame();
    gameInput.deltaTime = deltaTime;
    gameInput.keyboard.isConnected = true;
    gameInput.maxGamepads = GAME_MAX_GAMEPADS;

    PlatformThreadContext thread = {};
    gameUpdate(&thread,
               &gameMemory,
               &gameFrameBuffer,
               &gameAudioBuffer,
               &gameInput);

    gameInputPrevState = gameInput;
    [macState.view setNeedsDisplay:YES];
}

static void MacInitialiseFrameBuffer(void)
{
    macFrameBuffer.width = MAC_HANDMADE_FRAME_BUFFER_WIDTH;
    macFrameBuffer.height = MAC_HANDMADE_FRAME_BUFFER_HEIGHT;
    macFrameBuffer.bytesPerPixel = MAC_HANDMADE_BYTES_PER_PIXEL;
    macFrameBuffer.byteWidthPerRow = macFrameBuffer.width * macFrameBuffer.bytesPerPixel;

    uint64 frameBufferSize = macFrameBuffer.byteWidthPerRow * macFrameBuffer.height;
    macFrameBuffer.memory = platformAllocateMemory(NULL, 0, frameBufferSize);

    PlatformThreadContext thread = {};
    gameInitFrameBuffer(&thread,
                        &gameFrameBuffer,
                        macFrameBuffer.height,
                        macFrameBuffer.width,
                        macFrameBuffer.bytesPerPixel,
                        macFrameBuffer.byteWidthPerRow,
                        macFrameBuffer.memory);
}

static void MacInitialiseGameMemory(void)
{
    PlatformThreadContext thread = {};

    uint64 permanentStorageSizeInBytes = utilGibibytesToBytes(1);
    uint64 transientStorageSizeInBytes = utilMebibytesToBytes(64);
    uint64 memoryTotalSize = permanentStorageSizeInBytes + transientStorageSizeInBytes;

    void *platformMemory = platformAllocateMemory(&thread, 0, memoryTotalSize);
    if (!platformMemory) {
        platformLog(L"macOS: unable to allocate game memory\n");
        [NSApp terminate:nil];
        return;
    }

    gameMemory.platformLog = &platformLog;

    gameMemory.permanentStorage.bytes = platformMemory;
    gameMemory.permanentStorage.sizeInBytes = permanentStorageSizeInBytes;
    gameMemory.permanentStorage.bytesUsed = 0;
    gameMemory.permanentStorage.bytesFree = permanentStorageSizeInBytes;

    gameMemory.transientStorage.bytes = ((uint8 *)platformMemory + permanentStorageSizeInBytes);
    gameMemory.transientStorage.sizeInBytes = transientStorageSizeInBytes;
    gameMemory.transientStorage.bytesUsed = 0;
    gameMemory.transientStorage.bytesFree = transientStorageSizeInBytes;

    gameMemory.platformAllocateMemory = &platformAllocateMemory;
    gameMemory.platformFreeMemory = &platformFreeMemory;
    gameMemory.platformControllerVibrate = &platformControllerVibrate;

    wcsncpy(gameMemory.platformAbsPath, macState.absPath, countArray(gameMemory.platformAbsPath) - 1);
    gameMemory.platformAbsPath[countArray(gameMemory.platformAbsPath) - 1] = 0;

#ifdef HANDMADE_LOCAL_BUILD
    gameMemory.DEBUG_platformReadEntireFile = &DEBUG_platformReadEntireFile;
    gameMemory.DEBUG_platformWriteEntireFile = &DEBUG_platformWriteEntireFile;
    gameMemory.DEBUG_platformFreeFileMemory = &DEBUG_platformFreeFileMemory;
#endif

    gameInitAudioBuffer(&thread, &gameMemory, &gameAudioBuffer, 0, 0, 0);
}

void MacHandmadeApplicationDidFinishLaunching(NSWindow *window)
{
    macState.running = true;
    macState.window = window;

    mach_timebase_info(&macState.timebaseInfo);
    MacSetDevelopmentRootPath();
    MacInitialiseFrameBuffer();
    MacInitialiseGameMemory();

    NSRect frame = NSMakeRect(0, 0, MAC_HANDMADE_FRAME_BUFFER_WIDTH, MAC_HANDMADE_FRAME_BUFFER_HEIGHT);
    HandmadeView *view = [[HandmadeView alloc] initWithFrame:frame];
    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    view.wantsLayer = YES;
    macState.view = view;

    if (!macState.window) {
        NSRect windowRect = NSMakeRect(0, 0, MAC_HANDMADE_FRAME_BUFFER_WIDTH, MAC_HANDMADE_FRAME_BUFFER_HEIGHT);
        macState.window = [[NSWindow alloc] initWithContentRect:windowRect
                                                      styleMask:NSWindowStyleMaskTitled |
                                                                NSWindowStyleMaskClosable |
                                                                NSWindowStyleMaskMiniaturizable |
                                                                NSWindowStyleMaskResizable
                                                        backing:NSBackingStoreBuffered
                                                          defer:NO];
        macState.window.title = @"Handmade Hero";
        [macState.window center];
    }

    [macState.window setContentView:view];
    [macState.window makeFirstResponder:view];
    [macState.window makeKeyAndOrderFront:nil];
    [macState.window setAcceptsMouseMovedEvents:YES];

    macState.lastCounter = MacGetWallClock();
    macState.frameTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / MAC_HANDMADE_TARGET_FPS)
                                                          repeats:YES
                                                            block:^(NSTimer *timer) {
                                                                MacUpdateAndRender(timer);
                                                            }];
}

void MacHandmadeApplicationWillTerminate(void)
{
    macState.running = false;
    [macState.frameTimer invalidate];
    macState.frameTimer = nil;
}
