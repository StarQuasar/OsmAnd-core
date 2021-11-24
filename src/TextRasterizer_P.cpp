#include "TextRasterizer_P.h"
#include "TextRasterizer.h"

#include "ignore_warnings_on_external_includes.h"
#include <SkBitmap.h>
#include <SkImage.h>
#include <SkTypeface.h>
#include <SkUtils.h>
#include <SkFontMetrics.h>
#include "restore_internal_warnings.h"

#include "ICU.h"
#include "CoreResourcesEmbeddedBundle.h"

//#define OSMAND_LOG_CHARACTERS_WITHOUT_FONT 1
#ifndef OSMAND_LOG_CHARACTERS_WITHOUT_FONT
#   define OSMAND_LOG_CHARACTERS_WITHOUT_FONT 0
#endif // !defined(OSMAND_LOG_CHARACTERS_WITHOUT_FONT)

//#define OSMAND_LOG_CHARACTERS_FONT 1
#ifndef OSMAND_LOG_CHARACTERS_FONT
#   define OSMAND_LOG_CHARACTERS_FONT 0
#endif // !defined(OSMAND_LOG_CHARACTERS_FONT)

// TO the separate file
const double HARFBUZZ_FONT_SIZE_SCALE = 64.0f;
namespace OsmAnd {
    void trimspec(std::string& text)
    {
        // unicode symbols \u200e \u200f \u202a \u202c \u202b
        const char *symbols[] = { "\xE2\x80\x8E", "\xE2\x80\x8F", "\xE2\x80\xAA", "\xE2\x80\xAC", "\xE2\x80\xAB"};
        int length = text.length();
        for (auto t : symbols) {
            if (length >= 3 && t[0] == text.at(0) && t[1] == text.at(1) && t[2] == text.at(2)) {
                text.erase(0, 3);
                length = text.length();
            }
            if (length >= 3 && t[0] == text.at(length - 3) && t[1] == text.at(length - 2) && t[2] == text.at(length - 1)) {
                text.erase(length - 3);
                length = text.length();
            }
            if (length < 3) {
                return;
            }
        }
    }

    // Check if all of the specified text has a corresponding non-zero glyph ID
    bool containsText(const SkTypeface* typeface, const std::string& textString) {
        const char *text = textString.c_str();
        size_t byteLength = textString.length();
        const char *stop = text + byteLength;
        while (text < stop) {
            if (0 == typeface->unicharToGlyph(SkUTF8_NextUnichar(&text))) {
                return false;
            }
        }
        return true;
    }

    void FontRegistry::registerFonts(const char* pathToFont,
                                     const std::string& fontName,
                                     bool bold/* = false*/,
                                     bool italic/* = false*/)
    {
        OsmAnd::LogPrintf(OsmAnd::LogSeverityLevel::Info, "Font path %s index %d", pathToFont, index);
        auto entry = new FontEntry(pathToFont, index);
        index++;
        if (!entry->fSkiaTypeface)
        {
            return;
        }
        entry->bold = bold;
        entry->italic = italic;
        entry->fontName = fontName;	
        cache.push_back(entry);
    }


    FontEntry* FontRegistry::updateFontEntry(const std::string& text, bool bold/* = false*/, bool italic/* = false*/) {
        if (cache.size() == 0)
        {
            OsmAnd::LogPrintf(OsmAnd::LogSeverityLevel::Error, "Fonts are not registered. Set fonts by FontRegistry::registerFonts");
            return nullptr;
        }
        FontEntry* fontEntry = nullptr;
        for (const auto& cacheItem : cache)
        {
            if (fontEntry != nullptr && (bold != cacheItem->bold || italic != cacheItem->italic)) {
                continue;
            }
            if (!containsText(cacheItem->fSkiaTypeface.get(), text)) {
                continue;
            }
            fontEntry = cacheItem;
            // If this entry fully matches the request, stop search
            if (cacheItem->bold == bold && cacheItem->italic == italic) {
                break;
            }
        }

        if (fontEntry == nullptr) {
            fontEntry = cache[0];
        }
        return fontEntry;
    }

    void FontRegistry::drawHbText(SkCanvas* cv,
                                  std::string textS,
                                  const FontEntry* fontEntry,
                                  const SkPaint& paint,
                                  SkFont& font,
                                  float centerX,
                                  float centerY)
    {
        font.setTypeface(fontEntry->fSkiaTypeface);
        trimspec(textS);
        const char *text = textS.c_str();
        
        hb_font_t *hb_font = hb_font_create(fontEntry->fHarfBuzzFace.get());
        hb_font_set_scale(hb_font,
                        HARFBUZZ_FONT_SIZE_SCALE * font.getSize(),
                        HARFBUZZ_FONT_SIZE_SCALE * font.getSize());
        hb_ot_font_set_funcs(hb_font);

        hb_buffer_t *hb_buffer = hb_buffer_create();
        hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1);
        hb_buffer_guess_segment_properties(hb_buffer);

        hb_shape(hb_font, hb_buffer, NULL, 0);

        unsigned int length = hb_buffer_get_length(hb_buffer);
        if (length == 0) {
            return;
        }	
        hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
        hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

        // SkTextBlobBuilder textBlobBuilder;
        // auto runBuffer = textBlobBuilder.allocRunPos(font, SkToInt(length));

        // double x = 0;
        // double y = 0;
        // for (unsigned int i = 0; i < length; i++)
        // {
        //     if (fontEntry->delCodePoints.count(info[i].codepoint))
        //     {
        //         runBuffer.glyphs[i] = fontEntry->repCodePoint;
        //     } else {
        //         runBuffer.glyphs[i] = info[i].codepoint;
        //     }
        //     reinterpret_cast<SkPoint *>(runBuffer.pos)[i] =
        //         SkPoint::Make(SkDoubleToScalar(x + pos[i].x_offset / HARFBUZZ_FONT_SIZE_SCALE),
        //                     SkDoubleToScalar(y - pos[i].y_offset / HARFBUZZ_FONT_SIZE_SCALE));
        //     x += pos[i].x_advance / HARFBUZZ_FONT_SIZE_SCALE;
        //     y += pos[i].y_advance / HARFBUZZ_FONT_SIZE_SCALE;
        // }
        // cv->drawTextBlob(textBlobBuilder.make(), centerX - x/2, centerY, paint);
        cv->drawSimpleText(text, textS.length(), SkTextEncoding::kUTF8, centerX, centerY, font, paint);	
        
        hb_buffer_destroy(hb_buffer);
        hb_font_destroy(hb_font);
    }
    void FontRegistry::drawHbTextOnPath(SkCanvas* canvas,
                                        std::string textS,
                                        const SkPath& path,
                                        const FontEntry* fontEntry, 
                                        const SkFont& font,
                                        const SkPaint& paint,
                                        float h_offset,
                                        float v_offset)
    {

    }

    void FontRegistry::drawSkiaTextOnPath(SkCanvas* canvas,
                                         std::string textS,
                                         const SkPath& path, 
                                         const FontEntry* fontEntry,
                                         const SkFont& font,
                                         const SkPaint& paint,
                                         float h_offset, 
                                         float v_offset)
    {

    }
}
OsmAnd::FontRegistry gFontRegistry;
////////////////////////////////////////////////////////////////////////////////////

OsmAnd::TextRasterizer_P::TextRasterizer_P(TextRasterizer* const owner_)
    : owner(owner_)
{
    _defaultPaint.setAntiAlias(true);
}

OsmAnd::TextRasterizer_P::~TextRasterizer_P()
{
}

QVector<OsmAnd::TextRasterizer_P::LinePaint> OsmAnd::TextRasterizer_P::evaluatePaints(
    const QVector<QStringRef>& lineRefs,
    const Style& style) const
{
    // Prepare default paint
    SkPaint paint = _defaultPaint;
    paint.setColor(style.color.toSkColor());

    // Prepare default font
    SkFont font = _defaultFont;
    font.setSize(style.size);

    // Transform text style to font style
    const SkFontStyle fontStyle(
        style.bold ? SkFontStyle::kBold_Weight : SkFontStyle::kNormal_Weight,
        SkFontStyle::kNormal_Width,
        style.italic ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant);

    const auto pText = lineRefs.first().string()->constData();

    QVector<LinePaint> linePaints;
    linePaints.reserve(lineRefs.count());
    for (const auto& lineRef : constOf(lineRefs))
    {
        LinePaint linePaint;
        linePaint.line = lineRef;

        TextPaint* pTextPaint = nullptr;
        const auto pLine = lineRef.constData();
        const auto pEnd = pLine + lineRef.size();
        auto pNextCharacter = pLine;
        while (pNextCharacter != pEnd)
        {
            const auto pCharacter = pNextCharacter;
            const auto position = pNextCharacter - pText;
            const auto characterUCS4 = SkUTF16_NextUnichar(reinterpret_cast<const uint16_t**>(&pNextCharacter));
            
            // First of all check previous typeface if it contains this character
            auto typeface = pTextPaint ? pTextPaint->font.refTypeface() : nullptr;
            if (typeface)
            {
                if (typeface->unicharToGlyph(characterUCS4))
                    typeface = nullptr;
#if OSMAND_LOG_CHARACTERS_FONT
                else
                {
                    SkString typefaceName;
                    typeface->getFamilyName(&fontName);

                    LogPrintf(LogSeverityLevel::Warning,
                        "UCS4 character 0x%08x (%u) has been found in '%s' font (reused)",
                        characterUCS4,
                        characterUCS4,
                        typefaceName.c_str());
                }
#endif // OSMAND_LOG_CHARACTERS_FONT
            }
            if (!typeface)
            {
                typeface = owner->fontFinder->findFontForCharacterUCS4(characterUCS4, fontStyle);

#if OSMAND_LOG_CHARACTERS_WITHOUT_FONT
                if (!typeface)
                {
                    LogPrintf(LogSeverityLevel::Warning,
                        "UCS4 character 0x%08x (%u) has not been found in any font",
                        characterUCS4,
                        characterUCS4);
                }
#endif // OSMAND_LOG_CHARACTERS_WITHOUT_FONT

#if OSMAND_LOG_CHARACTERS_FONT
                if (typeface)
                {
                    SkString typefaceName;
                    typeface->getFamilyName(&typefaceName);

                    LogPrintf(LogSeverityLevel::Warning,
                        "UCS4 character 0x%08x (%u) has been found in '%s' font",
                        characterUCS4,
                        characterUCS4,
                        typefaceName.c_str());
                }
#endif // OSMAND_LOG_CHARACTERS_FONT
            }

            if (pTextPaint == nullptr || pTextPaint->font.refTypeface() != typeface)
            {
                linePaint.textPaints.push_back(qMove(TextPaint()));
                pTextPaint = &linePaint.textPaints.last();

                pTextPaint->text = QStringRef(lineRef.string(), position, 1);
                pTextPaint->paint = paint;
                pTextPaint->font = font;
                pTextPaint->font.setTypeface(typeface);

                SkFontMetrics metrics;
                pTextPaint->height = pTextPaint->font.getMetrics(&metrics) + 2.0f;
                linePaint.maxFontHeight = qMax(linePaint.maxFontHeight, pTextPaint->height);
                linePaint.minFontHeight = qMin(linePaint.minFontHeight, pTextPaint->height);
                linePaint.maxFontLineSpacing = qMax(linePaint.maxFontLineSpacing, metrics.fLeading);
                linePaint.minFontLineSpacing = qMin(linePaint.minFontLineSpacing, metrics.fLeading);
                linePaint.maxFontTop = qMax(linePaint.maxFontTop, -metrics.fTop);
                linePaint.minFontTop = qMin(linePaint.minFontTop, -metrics.fTop);
                linePaint.maxFontBottom = qMax(linePaint.maxFontBottom, metrics.fBottom);
                linePaint.minFontBottom = qMin(linePaint.minFontBottom, metrics.fBottom);
                linePaint.fontAscent = metrics.fAscent;

                if (style.bold && (!typeface || (typeface && typeface->fontStyle().weight() <= SkFontStyle::kNormal_Weight)))
                    pTextPaint->font.setEmbolden(true);
            }
            else
            {
                pTextPaint->text = QStringRef(lineRef.string(), pTextPaint->text.position(), pTextPaint->text.size() + 1);
            }
        }

        linePaints.push_back(qMove(linePaint));
    }

    return linePaints;
}

void OsmAnd::TextRasterizer_P::measureText(QVector<LinePaint>& paints, SkScalar& outMaxLineWidth) const
{
    outMaxLineWidth = 0;

    for (auto& linePaint : paints)
    {
        linePaint.maxBoundsTop = 0;
        linePaint.minBoundsTop = std::numeric_limits<SkScalar>::max();
        linePaint.width = 0;

        for (auto& textPaint : linePaint.textPaints)
        {
            textPaint.font.measureText(
                textPaint.text.constData(), textPaint.text.length()*sizeof(QChar), SkTextEncoding::kUTF16,
                &textPaint.bounds);

            textPaint.width = textPaint.bounds.width();

            linePaint.maxBoundsTop = qMax(linePaint.maxBoundsTop, -textPaint.bounds.top());
            linePaint.minBoundsTop = qMin(linePaint.minBoundsTop, -textPaint.bounds.top());
            linePaint.width += textPaint.width;
        }

        outMaxLineWidth = qMax(outMaxLineWidth, linePaint.width);
    }
}

void OsmAnd::TextRasterizer_P::measureGlyphs(const QVector<LinePaint>& paints, QVector<SkScalar>& outGlyphWidths) const
{
    for (const auto& linePaint : constOf(paints))
    {
        for (const auto& textPaint : constOf(linePaint.textPaints))
        {
            const auto glyphsCount = textPaint.font.countText(
                textPaint.text.constData(), textPaint.text.length()*sizeof(QChar), SkTextEncoding::kUTF16
            );

            QVector<SkGlyphID> glyphs(glyphsCount);
            textPaint.font.textToGlyphs(
                textPaint.text.constData(), textPaint.text.length()*sizeof(QChar), SkTextEncoding::kUTF16,
                glyphs.data(), glyphsCount
            );

            const auto previousSize = outGlyphWidths.size();
            outGlyphWidths.resize(previousSize + glyphsCount);
            const auto pWidth = outGlyphWidths.data() + previousSize;
            textPaint.font.getWidths(
                glyphs.constData(), glyphsCount,
                pWidth);

            *pWidth += -textPaint.bounds.left();

            ///////
            /*
            const float totalWidth = textPaint.paint.measureText(
                textPaint.text.constData(),
                textPaint.text.length()*sizeof(QChar));

            auto widthsSum = 0.0f;
            for (int idx = 0; idx < glyphsCount; idx++)
                widthsSum += pWidth[idx];

            if (widthsSum > totalWidth)
            {
                LogPrintf(LogSeverityLevel::Error, "totalWidth = %f, widthsSum = %f", totalWidth, widthsSum);
                int i = 5;
            }
            */
            ////////
        }
    }
}

SkPaint OsmAnd::TextRasterizer_P::getHaloPaint(const SkPaint& paint, const Style& style) const
{
    auto haloPaint = paint;
    haloPaint.setStyle(SkPaint::kStroke_Style);
    haloPaint.setColor(style.haloColor.toSkColor());
    haloPaint.setStrokeWidth(style.haloRadius);

    return haloPaint;
}

SkFont OsmAnd::TextRasterizer_P::getHaloFont(const SkFont& font, const Style& style) const
{
    auto haloFont = font;

    return haloFont;
}

void OsmAnd::TextRasterizer_P::measureHalo(const Style& style, QVector<LinePaint>& paints) const
{
    for (auto& linePaint : paints)
    {
        linePaint.maxBoundsTop = 0;
        linePaint.minBoundsTop = std::numeric_limits<SkScalar>::max();

        for (auto& textPaint : linePaint.textPaints)
        {
            const auto haloFont = getHaloFont(textPaint.font, style);
            /*
            SkPaint::FontMetrics metrics;
            textPaint.height = haloPaint.getFontMetrics(&metrics);
            linePaint.maxFontHeight = qMax(linePaint.maxFontHeight, textPaint.height);
            linePaint.minFontHeight = qMin(linePaint.minFontHeight, textPaint.height);
            linePaint.maxFontLineSpacing = qMax(linePaint.maxFontLineSpacing, metrics.fLeading);
            linePaint.minFontLineSpacing = qMin(linePaint.minFontLineSpacing, metrics.fLeading);
            linePaint.maxFontTop = qMax(linePaint.maxFontTop, -metrics.fTop);
            linePaint.minFontTop = qMin(linePaint.minFontTop, -metrics.fTop);
            linePaint.maxFontBottom = qMax(linePaint.maxFontBottom, metrics.fBottom);
            linePaint.minFontBottom = qMin(linePaint.minFontBottom, metrics.fBottom);
            */
            SkRect haloBounds;
            haloFont.measureText(
                textPaint.text.constData(), textPaint.text.length()*sizeof(QChar), SkTextEncoding::kUTF16,
                &haloBounds);
            textPaint.bounds.join(haloBounds);

            linePaint.maxBoundsTop = qMax(linePaint.maxBoundsTop, -textPaint.bounds.top());
            linePaint.minBoundsTop = qMin(linePaint.minBoundsTop, -textPaint.bounds.top());
        }
    }
}

void OsmAnd::TextRasterizer_P::measureHaloGlyphs(
    const Style& style,
    const QVector<LinePaint>& paints,
    QVector<SkScalar>& outGlyphWidths) const
{
    for (const auto& linePaint : constOf(paints))
    {
        for (const auto& textPaint : constOf(linePaint.textPaints))
        {
            const auto haloFont = getHaloFont(textPaint.font, style);

            const auto glyphsCount = haloFont.countText(
                textPaint.text.constData(), textPaint.text.length()*sizeof(QChar), SkTextEncoding::kUTF16
            );

            QVector<SkGlyphID> glyphs(glyphsCount);
            haloFont.textToGlyphs(
                textPaint.text.constData(), textPaint.text.length()*sizeof(QChar), SkTextEncoding::kUTF16,
                glyphs.data(), glyphsCount
            );

            const auto previousSize = outGlyphWidths.size();
            outGlyphWidths.resize(previousSize + glyphsCount);
            const auto pWidth = outGlyphWidths.data() + previousSize;
            haloFont.getWidths(
                glyphs.constData(), glyphsCount,
                pWidth);

            *pWidth += -textPaint.bounds.left();
        }
    }
}

SkRect OsmAnd::TextRasterizer_P::positionText(
    QVector<LinePaint>& paints,
    const SkScalar maxLineWidth,
    const Style::TextAlignment textAlignment) const
{
    auto textArea = SkRect::MakeEmpty();

    SkScalar verticalOffset = 0;
    for (auto& linePaint : paints)
    {
        SkScalar horizontalOffset = 0;
        const auto widthDelta = maxLineWidth - linePaint.width;
        switch (textAlignment)
        {
            case Style::TextAlignment::Center:
                horizontalOffset += widthDelta / 2.0f;
                break;

            case Style::TextAlignment::Right:
                horizontalOffset += widthDelta;
                break;

            case Style::TextAlignment::Left:
            default:
                // Do nothing here
                break;
        }

        for (auto& textPaint : linePaint.textPaints)
        {
            textPaint.positionedBounds = textPaint.bounds;

            // Position horizontally
            textPaint.positionedBounds.offset(-2.0f*textPaint.bounds.left(), 0);
            textPaint.positionedBounds.offset(horizontalOffset, 0);
            horizontalOffset += textPaint.width;

            // Position vertically
            textPaint.positionedBounds.offset(0, -2.0f*textPaint.bounds.top());
            textPaint.positionedBounds.offset(0, linePaint.maxBoundsTop + textPaint.bounds.top());
            textPaint.positionedBounds.offset(0, verticalOffset);

            // Include into text area
            textArea.join(textPaint.positionedBounds);
        }

        verticalOffset += linePaint.maxFontHeight;

        //// Calculate text area and move bounds vertically
        //auto textArea = linesNormalizedBounds.first();
        //auto linesHeightSum = textArea.height();
        //auto citPrevLineBounds = linesBounds.cbegin();
        //auto citLineBounds = citPrevLineBounds + 1;
        //for (auto itNormalizedLineBounds = linesNormalizedBounds.begin() + 1, itEnd = linesNormalizedBounds.end();
        //    itNormalizedLineBounds != itEnd;
        //    ++itNormalizedLineBounds, citPrevLineBounds = citLineBounds, ++citLineBounds)
        //{
        //    auto& lineNormalizedBounds = *itNormalizedLineBounds;
        //    const auto& prevLineBounds = *citPrevLineBounds;
        //    const auto& lineBounds = *citLineBounds;

        //    // Include gap between previous line and it's font-end
        //    const auto extraPrevGapHeight = qMax(0.0f, fontMaxBottom - prevLineBounds.fBottom);
        //    textArea.fBottom += extraPrevGapHeight;
        //    linesHeightSum += extraPrevGapHeight;

        //    // Include line spacing
        //    textArea.fBottom += lineSpacing;
        //    linesHeightSum += lineSpacing;

        //    // Include gap between current line and it's font-start
        //    const auto extraGapHeight = qMax(0.0f, fontMaxTop - (-lineBounds.fTop));
        //    textArea.fBottom += extraGapHeight;
        //    linesHeightSum += extraGapHeight;

        //    // Move current line baseline
        //    lineNormalizedBounds.offset(0.0f, linesHeightSum);

        //    // Include height of current line
        //    const auto& lineHeight = lineNormalizedBounds.height();
        //    textArea.fBottom += lineHeight;
        //    linesHeightSum += lineHeight;

        //    // This will expand left-right bounds to get proper area width
        //    textArea.fLeft = qMin(textArea.fLeft, lineNormalizedBounds.fLeft);
        //    textArea.fRight = qMax(textArea.fRight, lineNormalizedBounds.fRight);
        //}

    }

    return textArea;
}

std::shared_ptr<SkBitmap> OsmAnd::TextRasterizer_P::rasterize(
    const QString& text,
    const Style& style,
    QVector<SkScalar>* const outGlyphWidths,
    float* const outExtraTopSpace,
    float* const outExtraBottomSpace,
    float* const outLineSpacing,
    float* const outFontAscent) const
{
    std::shared_ptr<SkBitmap> bitmap(new SkBitmap());
    const bool ok = rasterize(
        *bitmap,
        text,
        style,
        outGlyphWidths,
        outExtraTopSpace,
        outExtraBottomSpace,
        outLineSpacing,
        outFontAscent);
    if (!ok)
        return nullptr;
    return bitmap;
}

bool OsmAnd::TextRasterizer_P::rasterize(
    SkBitmap& targetBitmap,
    const QString& text_,
    const Style& style,
    QVector<SkScalar>* const outGlyphWidths,
    float* const outExtraTopSpace,
    float* const outExtraBottomSpace,
    float* const outLineSpacing,
    float* const outFontAscent) const
{
    // Prepare text and break by lines
    const auto text = ICU::convertToVisualOrder(text_);
    const auto lineRefs = style.wrapWidth > 0
        ? ICU::getTextWrappingRefs(text, style.wrapWidth, style.maxLines)
        : (QVector<QStringRef>() << QStringRef(&text));

    // Obtain paints from lines and style
    auto paints = evaluatePaints(lineRefs, style);

    // Measure text
    SkScalar maxLineWidthInPixels = 0;
    measureText(paints, maxLineWidthInPixels);

    // Measure glyphs (if requested and there's no halo)
    if (outGlyphWidths && style.haloRadius == 0)
        measureGlyphs(paints, *outGlyphWidths);

    // Process halo if exists
    if (style.haloRadius > 0)
    {
        measureHalo(style, paints);

        if (outGlyphWidths)
            measureHaloGlyphs(style, paints, *outGlyphWidths);
    }

    // Set output font ascent
    if (outFontAscent)
    {
        float fontAscent = 0.0f;
        for (const auto& linePaint : constOf(paints))
            fontAscent = qMin(fontAscent, linePaint.fontAscent);

        *outFontAscent = fontAscent;
    }
    
    // Set output line spacing
    if (outLineSpacing)
    {
        float lineSpacing = 0.0f;
        for (const auto& linePaint : constOf(paints))
            lineSpacing = qMax(lineSpacing, linePaint.maxFontLineSpacing);

        *outLineSpacing = lineSpacing;
    }

    // Calculate extra top and bottom space
    if (outExtraTopSpace)
    {
        SkScalar maxTop = 0;
        for (const auto& linePaint : constOf(paints))
            maxTop = qMax(maxTop, linePaint.maxFontTop);

        *outExtraTopSpace = qMax(0.0f, maxTop - paints.first().maxFontTop);
    }
    if (outExtraBottomSpace)
    {
        SkScalar maxBottom = 0;
        for (const auto& linePaint : constOf(paints))
            maxBottom = qMax(maxBottom, linePaint.maxFontBottom);

        *outExtraBottomSpace = qMax(0.0f, maxBottom - paints.last().maxFontBottom);
    }

    // Position text horizontally and vertically
    const auto textArea = positionText(paints, maxLineWidthInPixels, style.textAlignment);
    
    // Calculate bitmap size
    auto bitmapWidth = qCeil(textArea.width());
    auto bitmapHeight = qCeil(textArea.height());
    if (style.backgroundBitmap)
    {
        // Clear extra spacing
        if (outExtraTopSpace)
            *outExtraTopSpace = 0.0f;
        if (outExtraBottomSpace)
            *outExtraBottomSpace = 0.0f;

        // Enlarge bitmap if shield is larger than text
        bitmapWidth = qMax(bitmapWidth, style.backgroundBitmap->width());
        bitmapHeight = qMax(bitmapHeight, style.backgroundBitmap->height());

        // Shift text area to proper position in a larger
        const auto offset = SkPoint::Make(
            (bitmapWidth - qCeil(textArea.width())) / 2.0f,
            (bitmapHeight - qCeil(textArea.height())) / 2.0f);
        for (auto& linePaint : paints)
        {
            for (auto& textPaint : linePaint.textPaints)
                textPaint.positionedBounds.offset(offset);
        }
    }

    // Check if bitmap size was successfully calculated
    if (bitmapWidth <= 0 || bitmapHeight <= 0)
    {
        LogPrintf(LogSeverityLevel::Error,
            "Failed to rasterize text '%s': resulting bitmap size %dx%d is invalid",
            qPrintable(text),
            bitmapWidth,
            bitmapHeight);
        return false;
    }

    // Create a bitmap that will be hold entire symbol (if target is empty)
    if (targetBitmap.isNull())
    {
        if (!targetBitmap.tryAllocPixels(SkImageInfo::MakeN32Premul(bitmapWidth, bitmapHeight)))
        {
            LogPrintf(LogSeverityLevel::Error,
                "Failed to allocate bitmap of size %dx%d",
                qPrintable(text),
                bitmapWidth,
                bitmapHeight);
            return false;
        }

        targetBitmap.eraseColor(SK_ColorTRANSPARENT);
    }
    SkCanvas canvas(targetBitmap);

    // If there is background this text, rasterize it also
    if (style.backgroundBitmap)
    {
        canvas.drawImage(style.backgroundBitmap->asImage(),
            (bitmapWidth - style.backgroundBitmap->width()) / 2.0f,
            (bitmapHeight - style.backgroundBitmap->height()) / 2.0f
        );
    }

    auto stdText = text.toUtf8().constData();
    auto fontEntry = gFontRegistry.updateFontEntry(stdText);
    // Rasterize text halo first (if enabled)
    if (style.haloRadius > 0)
    {
        for (const auto& linePaint : paints)
        {
            for (auto& textPaint : linePaint.textPaints)
            {
                auto haloPaint = getHaloPaint(textPaint.paint, style);
                auto haloFont = getHaloFont(textPaint.font, style);

                // canvas.drawSimpleText(
                //     textPaint.text.constData(), textPaint.text.length()*sizeof(QChar), SkTextEncoding::kUTF16,
                //     textPaint.positionedBounds.left(), textPaint.positionedBounds.top(),
                //     haloFont, haloPaint);
                gFontRegistry.drawHbText(&canvas,
                                         stdText,
                                         fontEntry,
                                         haloPaint,
                                         haloFont,
                                         textPaint.positionedBounds.left(),
                                         textPaint.positionedBounds.top());
            }
        }
    }

    // Rasterize text itself
    for (auto& linePaint : paints)
    {
        for (auto& textPaint : linePaint.textPaints)
        {
            // canvas.drawSimpleText(
            //     textPaint.text.constData(), textPaint.text.length()*sizeof(QChar), SkTextEncoding::kUTF16,
            //     textPaint.positionedBounds.left(), textPaint.positionedBounds.top(),
            //     textPaint.font, textPaint.paint);
            gFontRegistry.drawHbText(&canvas,
                                     stdText,
                                     fontEntry,
                                     textPaint.paint,
                                     textPaint.font,
                                     textPaint.positionedBounds.left(),
                                     textPaint.positionedBounds.top());
        }
    }

    canvas.flush();

    return true;
}
