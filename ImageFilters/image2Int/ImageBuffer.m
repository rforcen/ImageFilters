//
//  ImageBuffer.m
//  Domain Coloring
//
//  Created by asd on 25/10/2018.
//  Copyright © 2018 voicesync. All rights reserved.
//

#import "ImageBuffer.h"
#import "ImageHelper.h"

@implementation ImageBuffer
+(ImageBuffer*)initWithWidth:(float)Width Height:(float)Height {
    ImageBuffer*ib=[[super alloc]init];
    
    ib.w=Width; ib.h=Height;
    ib->_area = Height * Width;
    ib->_size = CGSizeMake(Width, Height);
    ib->_imgBuff = calloc(ib->_area, sizeof(uint32));
    ib->_imgBuff32 = (uint32*)ib->_imgBuff;
    
    return ib;
}
+(ImageBuffer*)initWithSize:(CGSize)size {
    return [ImageBuffer initWithWidth:size.width Height:size.height];
}

-(void)dealloc {
    if(_imgBuff) free(_imgBuff);
}
-(NSImage*)completeImage {
    return _image=[ImageHelper convertBitmapRGBA8ToNSImage:_imgBuff withWidth:_w withHeight:_h];
}
-(NSImage*)clear {
    memset(_imgBuff, 0, _area*sizeof(uint32));
    return [self completeImage];
}
-(int) index:(int)x y:(int)y { // point in image index space
    return x + y * _w;
}
-(uint32)maskAlpha:(uint32)pixel { // make alpha = 255
    return pixel | 0xff000000;
}
-(NSImage*)fillWithColor:(NSColor*)uicolor offset:(CGPoint)offset frame:(CGSize)frame {
    int color=[uicolor toABGR];
    for (int y=0; y<frame.height; y++)
        for (int x=0, base=[self index:offset.x y:offset.y+y]; x<frame.width; x++, base++)
            _imgBuff32[base]=[self maskAlpha:color]; // 0xff0000=blue bgr

    return [self completeImage];
}
-(NSImage*)fillAllWithColor:(NSColor*)uicolor {
    int color=[uicolor toABGR];
    for (int y=0; y<_h; y++)
        for (int x=0, base=[self index:0 y:y]; x<_w; x++, base++)
            _imgBuff32[base]=[self maskAlpha:color]; // 0xff0000=blue bgr
    
    return [self completeImage];
}
-(NSImage*)fillWithBuffer: (CGPoint)offset imgBuff:(ImageBuffer*)imgBuff {
    CGSize frame=imgBuff.size;
    
    for (int y=0; y<frame.height; y++)
        for (int x=0, base=[self index:offset.x y:offset.y+y]; x<frame.width; x++, base++)
            _imgBuff32[base]=imgBuff.imgBuff32[[imgBuff index:x y:y]]; 

    return [self completeImage];
}
-(NSImage*)uiimage {
    return [self completeImage];
}
@end


