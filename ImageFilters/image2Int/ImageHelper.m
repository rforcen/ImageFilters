//
//  ImageHelper.m
//  voiceXplorer
//
//  Created by asd on 19/10/2018.
//  Copyright © 2018 voicesync. All rights reserved.
//
/*
 * https://github.com/PaulSolt/NSImage-Conversion/blob/master/MITLicense.txt
 */

#import "ImageHelper.h"


@implementation ImageHelper

+ (CGImageRef)CGImage: (NSImage*)image {
    NSData *imageData = image.TIFFRepresentation;
    CGImageSourceRef source = CGImageSourceCreateWithData((__bridge CFDataRef)imageData, NULL);
    CGImageRef maskRef =  CGImageSourceCreateImageAtIndex(source, 0, NULL);
    return maskRef;
}

+ (unsigned char *) convertNSImageToBitmapRGBA8:(NSImage *) image {
    
    CGImageRef imageRef = [ImageHelper CGImage: image];
    
    // Create a bitmap context to draw the uiimage into
    CGContextRef context = [self newBitmapRGBA8ContextFromImage:imageRef];
    
    if(!context)  return NULL;
    
    size_t width = CGImageGetWidth(imageRef);
    size_t height = CGImageGetHeight(imageRef);
    
    CGRect rect = CGRectMake(0, 0, width, height);
    
    
    CGContextDrawImage(context, rect, imageRef); // Draw image into the context to get the raw image data
    unsigned char *bitmapData = (unsigned char *)CGBitmapContextGetData(context);  // Get a pointer to the data
    
    // Copy the data and release the memory (return memory allocated with new)
    size_t bytesPerRow = CGBitmapContextGetBytesPerRow(context);
    size_t bufferLength = bytesPerRow * height;
    
    unsigned char *newBitmap = NULL;
    
    if(bitmapData) {
        newBitmap = (unsigned char *)malloc(sizeof(unsigned char) * bufferLength);
        
        if(newBitmap) {    // Copy the data
            // for(int i = 0; i < bufferLength; ++i) newBitmap[i] = bitmapData[i];
            memcpy(newBitmap, bitmapData, bufferLength);
        }
        
        free(bitmapData);
        
    }
    
    CGContextRelease(context);
    
    return newBitmap;
}

+ (CGContextRef) newBitmapRGBA8ContextFromImage:(CGImageRef) image {
    CGContextRef context = NULL;
    CGColorSpaceRef colorSpace;
    uint32_t *bitmapData;
    
    size_t bitsPerPixel = 32;
    size_t bitsPerComponent = 8;
    size_t bytesPerPixel = bitsPerPixel / bitsPerComponent;
    
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    
    size_t bytesPerRow = width * bytesPerPixel;
    size_t bufferLength = bytesPerRow * height;
    
    colorSpace = CGColorSpaceCreateDeviceRGB();
    
    if(!colorSpace) {
        NSLog(@"Error allocating color space RGB\n");
        return NULL;
    }
    
    // Allocate memory for image data
    bitmapData = (uint32_t *)malloc(bufferLength);
    
    if(!bitmapData) {
        NSLog(@"Error allocating memory for bitmap\n");
        CGColorSpaceRelease(colorSpace);
        return NULL;
    }
    
    //Create bitmap context
    
    context = CGBitmapContextCreate(bitmapData,
                                    width,
                                    height,
                                    bitsPerComponent,
                                    bytesPerRow,
                                    colorSpace,
                                    kCGImageAlphaPremultipliedLast);    // RGBA
    if(!context) {
        free(bitmapData);
        NSLog(@"Bitmap context not created");
    }
    
    CGColorSpaceRelease(colorSpace);
    
    return context;
}

+ (NSImage *) convertBitmapRGBA8ToNSImage:(unsigned char *) buffer
                                withWidth:(NSUInteger) width
                               withHeight:(NSUInteger) height {
    
    
    size_t bufferLength = width * height * 4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, bufferLength, NULL);
    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = 32;
    size_t bytesPerRow = 4 * width;
    
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    if(colorSpaceRef == NULL) {
        NSLog(@"Error allocating color space");
        CGDataProviderRelease(provider);
        return nil;
    }
    
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGImageRef iref = CGImageCreate(width,
                                    height,
                                    bitsPerComponent,
                                    bitsPerPixel,
                                    bytesPerRow,
                                    colorSpaceRef,
                                    bitmapInfo,
                                    provider,    // data provider
                                    NULL,        // decode
                                    YES,            // should interpolate
                                    renderingIntent);
    
    uint32_t* pixels = (uint32_t*)malloc(bufferLength);
    
    if(pixels == NULL) {
        NSLog(@"Error: Memory not allocated for bitmap");
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpaceRef);
        CGImageRelease(iref);
        return nil;
    }
    
    CGContextRef context = CGBitmapContextCreate(pixels,
                                                 width,
                                                 height,
                                                 bitsPerComponent,
                                                 bytesPerRow,
                                                 colorSpaceRef,
                                                 bitmapInfo);
    
    if(context == NULL) {
        NSLog(@"Error context not created");
        free(pixels);
    }
    
    NSImage *image = nil;
    if(context) {
        
        CGContextDrawImage(context, CGRectMake(0.0f, 0.0f, width, height), iref);
        
        CGImageRef imageRef = CGBitmapContextCreateImage(context); 
        image = [[NSImage alloc] initWithCGImage: imageRef size:CGRectMake(0.0f, 0.0f, width, height).size];
        
        CGImageRelease(imageRef);
        CGContextRelease(context);
    }
    
    CGColorSpaceRelease(colorSpaceRef);
    CGImageRelease(iref);
    CGDataProviderRelease(provider);
    
    if(pixels) {
        free(pixels);
    }
    return image;
}

@end

@implementation NSColor(Hex)
- (int32_t)toABGR { // abgr in image context 0xff0000ff==red; // abgr
    CGFloat red, green, blue, alpha; // system colors [NSColor redColor] -> alpha==0
    [self getRed:&red green:&green blue:&blue alpha:&alpha];
    int32_t
    redInt      = (int32_t)(red     * 255 + 0.5),
    greenInt    = (int32_t)(green   * 255 + 0.5),
    blueInt     = (int32_t)(blue    * 255 + 0.5),
    alphaInt    = (int32_t)(alpha   * 255 + 0.5);
    
    return (alphaInt << 24) | (blueInt << 16) | (greenInt << 8) | redInt; //ABGR in images
}
@end
