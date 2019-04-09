//
//  ImageBuffer.h
//  Domain Coloring
//
//  Created by asd on 25/10/2018.
//  Copyright © 2018 voicesync. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN
@interface ImageBuffer : NSObject {
}
+(ImageBuffer*)initWithWidth:(float)Width Height:(float)Height;
+(ImageBuffer*)initWithSize:(CGSize)size;
-(NSImage*)clear;
-(uint32_t)maskAlpha:(uint32)pixel;
-(NSImage*)fillWithColor:(NSColor*)uicolor offset:(CGPoint)offset frame:(CGSize)frame;
-(NSImage*)fillWithBuffer: (CGPoint)offset imgBuff:(ImageBuffer*)imgBuff;
-(NSImage*)fillAllWithColor:(NSColor*)uicolor;
-(NSImage*)uiimage;

@property CGSize size;
@property float w,h, area;
@property UInt8 *imgBuff;
@property uint32 *imgBuff32; // argb version
@property NSImage*image;
@end
NS_ASSUME_NONNULL_END
