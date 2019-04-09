//
//  Filter.h
//  ImageFilters
//
//  Created by asd on 07/04/2019.
//  Copyright © 2019 voicesync. All rights reserved.
//

#ifndef Filter_h
#define Filter_h

#include <functional>
#include <thread>
#include <vector>
#include <set>

using std::thread;
using std::vector;
using std::function;
using std::max;
using std::min;
using std::set;


class Thread {
public:
    typedef std::function<void(size_t from, size_t to)>const& LambdaFromTo;
    int nthreads=thread::hardware_concurrency();
    vector<thread>threads;
    size_t segSize=0, size=0;
    
    size_t from(int t)  { return t*segSize; }
    size_t to(int t)    { return (t==nthreads-1) ? size : (t+1)*segSize ;}
    
    Thread(size_t size) : size(size), segSize(size/nthreads) {}
    void threading(LambdaFromTo lambda) {
        threads.clear();
        for (auto t=0; t<nthreads; t++)
            threads.push_back(thread([this, lambda, t](){
                lambda(from(t), to(t));
            } ));
        joinAll();
    }
    void joinAll() { for (auto &t : threads) t.join();  }
};


class RGBA {
public:
    typedef function<double(double)>const& Lambda;
    typedef function<RGBA(RGBA&)>const& LambdaRGBA;
    typedef function<void(void)>const& Lambdavoid;
    
    uint32 *rgba=nullptr, rgb=0;
    uint8 alpha=0;
    
    RGBA() {}
    RGBA(uint32 &rgba):rgba(&rgba), alpha(getAlpha(rgba)), rgb(getRGB(rgba)) {}
    RGBA(uint32 *rgba):rgba(rgba), alpha(getAlpha(*rgba)), rgb(getRGB(*rgba)) {}
    ~RGBA() { if(rgba!=nullptr) build(); } // destructor builds and set current alpha | rgb
    
    RGBA set(uint32 &rgba){
        this->rgba=&rgba;
        this->alpha=getAlpha(rgba);
        this->rgb=getRGB(rgba);
        return *this;
    }
    inline uint8 getRed()   { return rgb; }
    inline uint8 getGreen() { return rgb>>8; }
    inline uint8 getBlue()  { return rgb >>16; }
    inline void setRed(uint8 red)       { rgb=(rgb & 0xffff00) | red; }
    inline void setGreen(uint8 green)   { rgb=(rgb & 0xff00ff) | (((uint32)green)<<8); }
    inline void setBlue(uint8 blue)     { rgb=(rgb & 0x00ffff) | (((uint32)blue)<<16); }
    inline void setrgb(uint8 red, uint8 green, uint8 blue) { rgb = red|(green<<8)|(blue<<16);}
    inline uint32 build() {  return *rgba = (((uint32)alpha)<<24) | rgb;   }
    inline double toDouble() { return (double)rgb / 0xffffff;}
    inline void apply(Lambda lambda) { rgb = (uint32) (fabs(lambda(toDouble())) * 0xffffff); }
    inline void applyRGBA(LambdaRGBA lambda) {
        *this = lambda(*this);
    }
    
    static inline uint8  getAlpha(uint32 argb) { return argb & 0xff; }
    static inline uint32 getRGB(uint32 argb)   { return argb & 0x00ffffff; }
    static inline void getARGB(uint32 argb, uint8&alpha, uint32&rgb) {
        alpha=argb&0xff;
        rgb=argb&0x00ffffff;
    }
    static inline uint32 buildARGB(uint8 alpha, uint32 rgb) { return (((uint32)alpha)<<24) | rgb;}
};

class RGBAFilters : public RGBA { //  https://github.com/kpbird/Android-Image-Filters/blob/master/ImageEffect/src/main/java/com/kpbird/imageeffect/ImageFilters.java
    
private: // aux funcs
    inline double sqr(double x) { return x*x; }
    uint8 rangeByte(int b) { return max<int>(min<int>(b,255),0);  }
    inline int _contrast(double x, double contrast) {
        return rangeByte( (int)(((((x / 255.0) - 0.5) * contrast) + 0.5) * 255.0) );
    }
    inline int _brightness(double x, double bright) {  return rangeByte( x+bright );   }
    
    void applyConvolution(float matrix[3][3], size_t width, float factor, float offset) {
        int sumR=0, sumG=0, sumB=0;     // init color sum
        
        // get sum of RGB on matrix
        for (int i=0; i<3; ++i) {
            for (int j=0; j<3; j++) {
                RGBA pix( rgba + i*width + j );
                sumR += pix.getRed()    * matrix[i][j];
                sumG += pix.getGreen()  * matrix[i][j];
                sumB += pix.getBlue()   * matrix[i][j];
            }
        }
        uint8 R = rangeByte(sumR / factor + offset),
        G = rangeByte(sumG / factor + offset),
        B = rangeByte(sumB / factor + offset);
        
        RGBA pix_11(rgba + width + 1); // pixel(x+1, y+1)
        pix_11.setrgb(R, G, B);
    }
    
public:
    RGBAFilters(){}
    // filters
    RGBAFilters set(uint32 &rgba) { RGBA::set(rgba); return *this; }
    inline void invert() {
        setRed  ( 255 - getRed()   );
        setGreen( 255 - getGreen() );
        setBlue ( 255 - getBlue()  );
    }
    inline void greyScale() {
        double GS_RED = 0.299, GS_GREEN = 0.587, GS_BLUE = 0.114;
        uint8 c = (uint8)(GS_RED * getRed() + GS_GREEN * getGreen() + GS_BLUE * getBlue());
        setrgb(c,c,c);
    }
    inline void contrast(double value) {
        
        double contrast = sqr((100 + value) / 100);
        
        // apply filter contrast for every channel R, G, B
        setrgb(_contrast(getRed(),   contrast),
               _contrast(getGreen(), contrast),
               _contrast(getBlue(),  contrast) );
    }
    inline void brightness(double value) {
        // apply filter contrast for every channel R, G, B
        setrgb(_brightness(getRed(),   value),
               _brightness(getGreen(), value),
               _brightness(getBlue(),  value) );
    }
    void gaussianBlur(size_t width) {
        float kernel[3][3] = { { 1, 2, 1 }, { 2, 4, 2 },  { 1, 2, 1 } };
        applyConvolution(kernel, width, 16, 10);
    }
    void sharpen(size_t width, float weight) {
        float kernel[3][3] = {    { 0 , -2    , 0  },   { -2, weight, -2 },   { 0 , -2    , 0  }   };
        applyConvolution(kernel, width, weight-8, 0);
    }
    void meanRemoval(size_t width) {
        float kernel[3][3] = { { -1,-1,-1 },   { -1,9,-1 }, {-1,-1,-1 }   };
        applyConvolution(kernel, width, 1, 0);
    }
    void smooth(size_t width, float value) {
        float kernel[3][3] = { { 1,1,1 },  { 1,1,1 }, { 1,1,1 } };
        applyConvolution(kernel, width, value+8, 1);
    }
    void emboss(size_t width, float value) {
        float kernel[3][3] =  {{-1,0,-1},{0,4,0},{-1,0,-1}};
        applyConvolution(kernel, width, 1, value);
    }
   
};

class Image : public RGBAFilters , public Thread {
public:
    uint32 *data=nullptr;
    size_t nPixels=0, memSize=0;
    NSSize size;
    NSImage*image;
    enum FilterTypes {
        ftInvert, ftgreyScale, ftContrast, ftBrightness, ftGausianBlur,
        ftSharpen, ftMeanRemoval, ftSmooth, ftEmboss
    };
    
    Image(NSImage*image) : image(image),
    Thread(image.size.width*image.size.height),
    size(image.size),
    nPixels(image.size.width*image.size.height),
    memSize(image.size.width*image.size.height*sizeof(uint32)),
    data((uint32*)[ImageHelper convertNSImageToBitmapRGBA8:image]) {}
    
    NSImage*toImage() {
        return [ImageHelper convertBitmapRGBA8ToNSImage:(uint8*)data withWidth:size.width withHeight:size.height];
    }
    
    void zero() {  memset(data, 0, memSize);  }
};

class Filter : public Image {
    
public:
    Filter(NSImage*image) : Image(image) {}
    
    void toLimit(size_t &to) {
        if (to==nPixels)  to -= size.width*2+size.height*2;
    }
    
    NSImage* filter(FilterTypes filterType, float value=0) {
        bool useCopy=false;
        Image *imgCopy=nullptr;
        // set of filters that require imageCopy
        std::set<FilterTypes>needCopy={ftGausianBlur, ftSharpen, ftMeanRemoval, ftSmooth, ftEmboss};
        
        if (needCopy.count(filterType)) {
            imgCopy = new Image(image);
            imgCopy->zero();
            useCopy=true;
        }
        
        threading([this, filterType, value, &imgCopy, &useCopy](size_t from, size_t to) {
            RGBAFilters filter; // each thread should have it's own RGBA copy
            switch (filterType) {
                case ftInvert:      for (auto i=from; i<to; i++) filter.set(data[i]).invert();          break;
                case ftgreyScale:   for (auto i=from; i<to; i++) filter.set(data[i]).greyScale();       break;
                case ftContrast:    for (auto i=from; i<to; i++) filter.set(data[i]).contrast(value);   break;
                case ftBrightness:  for (auto i=from; i<to; i++) filter.set(data[i]).brightness(value); break;
                case ftGausianBlur: toLimit(to);
                    for (auto i=from; i<to; i++) filter.set(imgCopy->data[i]).gaussianBlur(size.width); break;
                case ftSharpen:     toLimit(to);
                    for (auto i=from; i<to; i++) filter.set(imgCopy->data[i]).sharpen(size.width, value); break;
                case Image::ftMeanRemoval: toLimit(to);
                    for (auto i=from; i<to; i++) filter.set(imgCopy->data[i]).meanRemoval(size.width); break;
                    break;
                case Image::ftSmooth: toLimit(to);
                    for (auto i=from; i<to; i++) filter.set(imgCopy->data[i]).smooth(size.width, value); break;
                    break;
                case Image::ftEmboss: toLimit(to);
                    for (auto i=from; i<to; i++) filter.set(imgCopy->data[i]).emboss(size.width, value); break;
                    break;
            }
        });
        if (useCopy) memcpy(data, imgCopy->data, memSize);
        return toImage();
    }
    
    void apply(Lambda lambda) {  for (auto i=0; i<nPixels; i++)  set(data[i]).apply(lambda);    }
    void applyRGBA(LambdaRGBA lambda) {  for (auto i=0; i<nPixels; i++)  set(data[i]).applyRGBA(lambda);   }
};

class GaussianBlur  {
public:
    void gaussBlur_4 (uint32*scl, uint32*tcl, size_t w, size_t h, int r) {
        auto bxs = boxesForGauss(r, 3);
        boxBlur_4 (scl, tcl, w, h, (bxs[0]-1)/2);
        boxBlur_4 (tcl, scl, w, h, (bxs[1]-1)/2);
        boxBlur_4 (scl, tcl, w, h, (bxs[2]-1)/2);
    }
private:
    vector<float> boxesForGauss(float sigma, int n)  // standard deviation, number of boxes
    {
        auto wIdeal = sqrt((12*sigma*sigma/n)+1);  // Ideal averaging filter width
        auto wl = floor(wIdeal);  if(fmod(wl,2)==0) wl--;
        auto wu = wl+2;
        
        auto mIdeal = (12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n)/(-4*wl - 4);
        auto m = round(mIdeal);
        
        vector<float> sizes;
        for(auto i=0; i<n; i++) sizes.push_back(i<m ? wl : wu);
        return sizes;
    }
    
    void boxBlur_4 (uint32*scl, uint32*tcl, size_t w, size_t h, int r) {
        for(auto i=0; i<w*h; i++) tcl[i] = scl[i];
        boxBlurH_4(tcl, scl, w, h, r);
        boxBlurT_4(scl, tcl, w, h, r);
    }
    void boxBlurH_4 (uint32*scl, uint32*tcl, size_t w, size_t h, int r) {
        auto iarr = 1 / (r+r+1);
        for(auto i=0; i<h; i++) {
            auto ti = i*w, li = ti, ri = ti+r;
            uint32 fv = scl[ti], lv = scl[ti+w-1], val = (r+1)*fv;
            for(auto j=0; j<r; j++) val += scl[ti+j];
            for(auto j=0  ; j<=r ; j++) { val += scl[ri++] - fv       ;   tcl[ti++] = round(val*iarr); }
            for(auto j=r+1; j<w-r; j++) { val += scl[ri++] - scl[li++];   tcl[ti++] = round(val*iarr); }
            for(auto j=w-r; j<w  ; j++) { val += lv        - scl[li++];   tcl[ti++] = round(val*iarr); }
        }
    }
    void boxBlurT_4 (uint32*scl, uint32*tcl, size_t w, size_t h, int r) {
        auto iarr = 1 / (r+r+1);
        for(auto i=0; i<w; i++) {
            size_t ti = i, li = ti, ri = ti+r*w;
            size_t fv = scl[ti], lv = scl[ti+w*(h-1)], val = (r+1)*fv;
            for(auto j=0; j<r; j++) val += scl[ti+j*w];
            for(auto j=0  ; j<=r ; j++) { val += scl[ri] - fv     ;  tcl[ti] = round(val*iarr);  ri+=w; ti+=w; }
            for(auto j=r+1; j<h-r; j++) { val += scl[ri] - scl[li];  tcl[ti] = round(val*iarr);  li+=w; ri+=w; ti+=w; }
            for(auto j=h-r; j<h  ; j++) { val += lv      - scl[li];  tcl[ti] = round(val*iarr);  li+=w; ti+=w; }
        }
    }
};

#endif /* Filter_h */
