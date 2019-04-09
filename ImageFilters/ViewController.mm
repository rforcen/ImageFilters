//
//  ViewController.m
//  ImageFilters
//
//  Created by asd on 06/04/2019.
//  Copyright © 2019 voicesync. All rights reserved.
//
// /Users/asd/Pictures/people/p01.jpg

#import "ViewController.h"
#import "ImageHelper.h"
#import "Filter.h"

@implementation ViewController {
    NSString*imageName;
}

-(NSImage*)loadImage: (NSString*)filename { // must disable sandbox in config!!
    return [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath: filename]];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    imageName=@"/Users/asd/Pictures/people/p10.jpg";
    
    _image.image = [self loadImage:imageName];
    _imageFiltered.image = Filter(_image.image).filter(Filter::ftMeanRemoval, 1);
}

@end
