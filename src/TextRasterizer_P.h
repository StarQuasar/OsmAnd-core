#ifndef _OSMAND_CORE_TEXT_RASTERIZER_P_H_
#define _OSMAND_CORE_TEXT_RASTERIZER_P_H_

#include "stdlib_common.h"
#include <functional>

#include "QtExtensions.h"
#include "ignore_warnings_on_external_includes.h"
#include <QList>
#include <QVector>
#include "restore_internal_warnings.h"

#include "ignore_warnings_on_external_includes.h"
#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkFont.h>
#include "restore_internal_warnings.h"

#include "OsmAndCore.h"
#include "CommonTypes.h"
#include "MapCommonTypes.h"
#include "TextRasterizer.h"

// To the separate file
#include <hb-ot.h>
#include <set>

#include <SkData.h>
// To the separate file
namespace OsmAnd
{
    // To the separate file
    using namespace std;

    void trimspec(std::string& text);
    struct FontEntry {
        bool bold;
        bool italic;
        struct HBFDel { void operator()(hb_face_t *f) { hb_face_destroy(f); }};
        sk_sp<SkTypeface> fSkiaTypeface = nullptr;
        std::unique_ptr<hb_face_t, HBFDel> fHarfBuzzFace;
        string fontName;
        string pathToFont;

        std::set<uint32_t> delCodePoints;//calculated deleting codepoint for current ttf
        uint32_t repCodePoint;//calculated replacement codepoint for current ttf
        
        //\xE2\x80\x8B (\u200b) ZERO WIDTH SPACE - used for replacement, must be always in 0 index!
        //\x41 - character A just use as divider
        //\xE2\x80\x8C (\u200c) ZERO WIDTH NON-JOINER for avoid create ligature in arabic text
        const char *repChars = "\xE2\x80\x8B\x41\xE2\x80\x8C";//just add code to the end with \x41 divider

        FontEntry(const char *path, int index) {
            pathToFont = path;
            // fairly portable mmap impl
            auto data = SkData::MakeFromFileName(path);
            assert(data);
            if (!data) {
                return;
            }
            fSkiaTypeface = SkTypeface::MakeFromFile(path);
            assert(fSkiaTypeface);
            if (!fSkiaTypeface) {
                return;
            }
            auto destroy = [](void *d) { static_cast<SkData *>(d)->unref(); };
            const char *bytes = (const char *)data->data();
            unsigned int size = (unsigned int)data->size();
            hb_blob_t *blob = hb_blob_create(bytes,
                                            size,
                                            HB_MEMORY_MODE_READONLY,
                                            data.release(),
                                            destroy);
            assert(blob);
            hb_blob_make_immutable(blob);
            hb_face_t *face = hb_face_create(blob, (unsigned)index);
            hb_blob_destroy(blob);
            assert(face);
            if (!face) {
                fSkiaTypeface.reset();
                return;
            }
            hb_face_set_index(face, (unsigned)index);
            hb_face_set_upem(face, fSkiaTypeface->getUnitsPerEm());
            fHarfBuzzFace.reset(face);

            //here calculating replacement symbols
            hb_font_t *hb_font = hb_font_create(fHarfBuzzFace.get());
            hb_ot_font_set_funcs(hb_font);
            hb_buffer_t *hb_buffer = hb_buffer_create();
            hb_buffer_add_utf8(hb_buffer, repChars, -1, 0, -1);
            hb_buffer_guess_segment_properties(hb_buffer);
            hb_shape(hb_font, hb_buffer, NULL, 0);
            unsigned int length = hb_buffer_get_length(hb_buffer);
            hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
            for (int i = 0; i < length; i++) {
                if (i == 0) {
                    repCodePoint = info[i].codepoint;
                } else if (i == 1) {
                    continue;
                } else {
                    delCodePoints.insert(info[i].codepoint);
                }
            }
            hb_buffer_destroy(hb_buffer);
            hb_font_destroy(hb_font);
        }
    };

    class FontRegistry {
        std::vector<FontEntry*> cache;

    public:
        int index = 0;
        void registerFonts(const char* pathToFont,
                           const std::string& fontName,
                           bool bold = false,
                           bool italic = false);
        FontEntry*  updateFontEntry(const std::string& text, bool bold = false, bool italic = false);
        void drawHbText(SkCanvas* cv,
                        std::string textS,
                        const FontEntry* fontEntry,
                        const SkPaint& paint,
                        SkFont& font,
                        float centerX,
                        float centerY);
        void drawHbTextOnPath(SkCanvas* canvas,
                              std::string textS,
                              const SkPath& path,
                              const FontEntry* fontEntry, 
                              const SkFont& font,
                              const SkPaint& paint,
                              float h_offset,
                              float v_offset);
        void drawSkiaTextOnPath(SkCanvas* canvas,
                                std::string textS,
                                const SkPath& path, 
                                const FontEntry* fontEntry,
                                const SkFont& font,
                                const SkPaint& paint,
                                float h_offset, 
                                float v_offset);
    };

    extern FontRegistry gFontRegistry;
    //

    class BinaryMapObject;
    class IQueryController;

    class TextRasterizer;
    class TextRasterizer_P Q_DECL_FINAL
    {
    public:
        typedef TextRasterizer::Style Style;

    private:
        SkPaint _defaultPaint;
        SkFont _defaultFont;

        struct TextPaint
        {
            inline TextPaint()
                : height(0)
                , width(0)
            {
            }

            QStringRef text;
            SkPaint paint;
            SkFont font;
            SkScalar height;
            SkScalar width;
            SkRect bounds;
            SkRect positionedBounds;
        };
        struct LinePaint
        {
            inline LinePaint()
                : maxFontHeight(0)
                , minFontHeight(std::numeric_limits<SkScalar>::max())
                , maxFontLineSpacing(0)
                , minFontLineSpacing(std::numeric_limits<SkScalar>::max())
                , maxFontTop(0)
                , minFontTop(std::numeric_limits<SkScalar>::max())
                , maxFontBottom(0)
                , minFontBottom(std::numeric_limits<SkScalar>::max())
                , fontAscent(0)
                , maxBoundsTop(0)
                , minBoundsTop(std::numeric_limits<SkScalar>::max())
                , width(0)
            {
            }

            QStringRef line;
            QVector<TextPaint> textPaints;
            SkScalar maxFontHeight;
            SkScalar minFontHeight;
            SkScalar maxFontLineSpacing;
            SkScalar minFontLineSpacing;
            SkScalar maxFontTop;
            SkScalar minFontTop;
            SkScalar maxFontBottom;
            SkScalar minFontBottom;
            SkScalar fontAscent;
            SkScalar maxBoundsTop;
            SkScalar minBoundsTop;
            SkScalar width;
        };
        QVector<LinePaint> evaluatePaints(const QVector<QStringRef>& lineRefs, const Style& style) const;
        void measureText(QVector<LinePaint>& paints, SkScalar& outMaxLineWidth) const;
        void measureGlyphs(const QVector<LinePaint>& paints, QVector<SkScalar>& outGlyphWidths) const;
        SkPaint getHaloPaint(const SkPaint& paint, const Style& style) const;
        SkFont getHaloFont(const SkFont& font, const Style& style) const;
        void measureHalo(const Style& style, QVector<LinePaint>& paints) const;
        void measureHaloGlyphs(const Style& style, const QVector<LinePaint>& paints, QVector<SkScalar>& outGlyphWidths) const;
        SkRect positionText(
            QVector<LinePaint>& paints,
            const SkScalar maxLineWidth,
            const Style::TextAlignment textAlignment) const;
    protected:
        TextRasterizer_P(TextRasterizer* const owner);
    public:
        ~TextRasterizer_P();

        ImplementationInterface<TextRasterizer> owner;

        std::shared_ptr<SkBitmap> rasterize(
            const QString& text,
            const Style& style,
            QVector<SkScalar>* const outGlyphWidths,
            float* const outExtraTopSpace,
            float* const outExtraBottomSpace,
            float* const outLineSpacing,
            float* const outFontAscent) const;

        bool rasterize(
            SkBitmap& targetBitmap,
            const QString& text,
            const Style& style,
            QVector<SkScalar>* const outGlyphWidths,
            float* const outExtraTopSpace,
            float* const outExtraBottomSpace,
            float* const outLineSpacing,
            float* const outFontAscent) const;

    friend class OsmAnd::TextRasterizer;
    };
}

#endif // !defined(_OSMAND_CORE_TEXT_RASTERIZER_P_H_)
