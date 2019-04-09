//
//  ImageHelper.h
//  voiceXplorer
//
//  Created by asd on 19/10/2018.
//  Copyright © 2018 voicesync. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

/*
 * The MIT License
 *
 * Copyright (c) 2011 Paul Solt, PaulSolt@gmail.com
 *
 * https://github.com/PaulSolt/NSImage-Conversion/blob/master/MITLicense.txt
 *
 */

NS_ASSUME_NONNULL_BEGIN
@interface ImageHelper : NSObject

/** Converts a NSImage to RGBA8 bitmap.
 @param image - a NSImage to be converted
 @return a RGBA8 bitmap, or NULL if any memory allocation issues. Cleanup memory with free() when done.
 */
+ (unsigned char *) convertNSImageToBitmapRGBA8:(NSImage *)image;

/** A helper routine used to convert a RGBA8 to NSImage
 @return a new context that is owned by the caller
 */
+ (CGContextRef) newBitmapRGBA8ContextFromImage:(CGImageRef)image;


/** Converts a RGBA8 bitmap to a NSImage.
 @param buffer - the RGBA8 unsigned char * bitmap
 @param width - the number of pixels wide
 @param height - the number of pixels tall
 @return a NSImage that is autoreleased or nil if memory allocation issues
 */
+ (NSImage *) convertBitmapRGBA8ToNSImage:(unsigned char *) buffer
                                withWidth:(NSUInteger) width
                               withHeight:(NSUInteger) height;

@end

@interface NSColor(Hex)
-(int32_t)toABGR;
@end



NS_ASSUME_NONNULL_END
