#include "TextRenderer.h"

/*#if !USE_CT
static ATSUFontID* local_FallbacksWithCount(unsigned* oCount);
#endif*/

static OSStatus local_CoreTR(CGContextRef ctx, CGRect r, CFStringRef string,
                             CFStringRef fontName, CGFloat fontSize,
                             TRFallbackBehavior fallbackBehavior,
                             Boolean render, CGFloat baseline, TRInfo* oInfo);

#ifdef __LP64__
#define __fpmin  fmin
#else
#define __fpmin  fminf
#endif

OSStatus TRGetBestFontSize(CGContextRef ctx, CGRect r, CFStringRef string,
                           CFStringRef fontName,
                           TRFallbackBehavior fallbackBehavior,
                           CGFloat* oFontSize, CGFloat* oBaseline)
{
  CGFloat fontSize = 0.0f;
  CGFloat baseline = 0.0f;
  if (fontName)
  {
    r = CGRectInset(r, 0.1f*r.size.width, 0.1f*r.size.height);
    fontSize = 100.0f;
    NSUInteger i = 0;
    while (fontSize > 0.0f)
    {
      TRInfo info;
      OSStatus err = TRGetTextInfo(ctx, r, string, fontName, fontSize,
                                   fallbackBehavior, &info);
      if (err == paramErr) return err;
      baseline = info.baseline;
      /*printf("TRGetBestFontSize:{%.2f %.2f} size=%.2fpt w=%.2f h=%.2f asc=%f desc=%f bl=%f\n",
              r.size.width, r.size.height, fontSize, info.width, info.height,
              info.ascent, info.descent, baseline);*/
      if (info.height < r.size.height && info.width < r.size.width) break;
      CGFloat pct = __fpmin((r.size.height/info.height), (r.size.width/info.width));
      //printf("pct %f\n", pct);
      if (pct > .99f && pct < 1.01f) break;
      fontSize *= pct;
      i++;
      if (i > 10) break;
    }
  }
  *oFontSize = fontSize;
  *oBaseline = baseline;
  return noErr;
}

OSStatus TRGetTextInfo(CGContextRef ctx, CGRect r, CFStringRef string,
                       CFStringRef fontName, CGFloat fontSize,
                       TRFallbackBehavior fallbackBehavior, TRInfo* oInfo)
{
  return local_CoreTR(ctx, r, string, fontName, fontSize, fallbackBehavior,
                      false, -1.0L, oInfo);
}

OSStatus TRRenderText(CGContextRef ctx, CGRect r, CFStringRef string,
                      CFStringRef fontName, CGFloat fontSize,
                      TRFallbackBehavior fallbackBehavior, CGFloat baseline)
{
  return local_CoreTR(ctx, r, string, fontName, fontSize, fallbackBehavior,
                      true, baseline, NULL);
}


static OSStatus local_CoreTR(CGContextRef ctx, CGRect r, CFStringRef string,
                             CFStringRef fontName, CGFloat fontSize,
                             TRFallbackBehavior fallbackBehavior,
                             Boolean render, CGFloat baseline, TRInfo* oInfo)
{
  if (!ctx || !string || !fontName) return paramErr;
  CGContextSaveGState(ctx);
  CGContextSetTextMatrix(ctx, CGAffineTransformIdentity);
  CTFontDescriptorRef fdesc = CTFontDescriptorCreateWithNameAndSize(fontName, fontSize);
  CTFontRef font = CTFontCreateWithFontDescriptor(fdesc, fontSize, NULL);
  CFIndex slen = CFStringGetLength(string);
  CFRange range = CFRangeMake(0L,slen);
  UniChar* buff = calloc(slen, sizeof(UniChar));
  CFStringGetCharacters(string, range, buff);
  CGGlyph* glyphs = calloc(slen, sizeof(CGGlyph));
  Boolean supported = CTFontGetGlyphsForCharacters(font, buff, glyphs, slen);
  //NSLog(@"%@ supported for '%@'? %d", fontName, string, supported);
  CFRelease(fdesc);
  if (!supported)
  {
    if (fallbackBehavior == TRLastResortFallbackBehavior)
    {
      CFRelease(font);
      fdesc = CTFontDescriptorCreateWithNameAndSize(CFSTR("LastResort"), fontSize);
      font = CTFontCreateWithFontDescriptor(fdesc, fontSize, NULL);
      CFRelease(fdesc);
      supported = true;
    }
    else
    {
      CFRange rng = CFRangeMake(0L, CFStringGetLength(string));
      CTFontRef font2 = CTFontCreateForString(font, string, rng);
      CFRelease(font);
      CFStringRef fontName2 = CTFontCopyName(font2, kCTFontFullNameKey);
      CFRelease(font2);
      fdesc = CTFontDescriptorCreateWithNameAndSize(fontName2, fontSize);
      //NSLog(@"falling back to %@ from %@ : %@", fontName2, fontName, fdesc);
      CFRelease(fontName2);
      font = CTFontCreateWithFontDescriptor(fdesc, fontSize, NULL);
      CFRelease(fdesc);
      supported = CTFontGetGlyphsForCharacters(font, buff, glyphs, slen);
    }
  }
  free(buff);
  free(glyphs);
  CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                   1L, &kCFTypeDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(attrs, kCTFontAttributeName, font);
  CFDictionarySetValue(attrs, kCTForegroundColorFromContextAttributeName, kCFBooleanTrue);
  CFAttributedStringRef attrStr = CFAttributedStringCreate(kCFAllocatorDefault,
                                                           string, attrs);
  CFRelease(attrs);
  CTLineRef line = CTLineCreateWithAttributedString(attrStr);
  CFRelease(attrStr);
  CGRect bounds = CTLineGetImageBounds(line, ctx);
  CGFloat descent = CTFontGetDescent(font);
  CGFloat ascent = CTFontGetAscent(font);
  CFRelease(font);
  if (baseline < 0.0L)
    baseline = ((r.size.height - bounds.size.height + descent)/2.0L);
  if (render && (supported || fallbackBehavior != TRNoRenderFallbackBehavior))
  {
    CGFloat x = r.origin.x + (r.size.width/2.0L) - (bounds.size.width/2.0L) - bounds.origin.x;
    CGFloat y = r.origin.y + baseline;
    CGPoint where = CGPointMake(x, y);
    CGContextSetTextPosition(ctx, where.x, where.y);
    CTLineDraw(line, ctx);
  }
  CFRelease(line);
  if (oInfo)
  {
    oInfo->height = bounds.size.height + descent;
    oInfo->width = bounds.size.width;
    oInfo->ascent = ascent;
    oInfo->descent = descent;
    oInfo->baseline = baseline;
    oInfo->fontSupported = supported;
  }
  CGContextRestoreGState(ctx);
  return (supported)? noErr:kATSUFontsNotMatched;
}

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
