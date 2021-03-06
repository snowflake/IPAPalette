#include <Carbon/Carbon.h>

typedef struct
{
  CGFloat height;
  CGFloat width;
  CGFloat ascent;
  CGFloat descent;
  CGFloat baseline;
  Boolean fontSupported;
} TRInfo;

typedef enum
{
  TRSubstituteFallbackBehavior,
  TRLastResortFallbackBehavior,
  TRNoRenderFallbackBehavior
} TRFallbackBehavior;

OSStatus TRGetBestFontSize(CGContextRef ctx, CGRect r, CFStringRef string,
                           CFStringRef fontName,
                           TRFallbackBehavior fallbackBehavior,
                           CGFloat* oFontSize, CGFloat* oBaseline);
OSStatus TRGetTextInfo(CGContextRef ctx, CGRect r, CFStringRef string,
                       CFStringRef fontName, CGFloat fontSize,
                       TRFallbackBehavior fallbackBehavior, TRInfo* oInfo);
OSStatus TRRenderText(CGContextRef ctx, CGRect r, CFStringRef string,
                      CFStringRef fontName, CGFloat fontSize,
                      TRFallbackBehavior fallbackBehavior, CGFloat baseline);

/*
Copyright © 2005-2020 Brian S. Hall, BLUGS.COM LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
