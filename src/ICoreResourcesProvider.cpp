#include "ICoreResourcesProvider.h"

#include "ignore_warnings_on_external_includes.h"
#include <SkBitmap.h>
#include <SkStream.h>
#include <SkImage.h>
#include "SkImageGeneratorCG.h"
#include "SkImageGeneratorWIC.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImageGenerator.h"
#include "restore_internal_warnings.h"

OsmAnd::ICoreResourcesProvider::ICoreResourcesProvider()
{
}

OsmAnd::ICoreResourcesProvider::~ICoreResourcesProvider()
{
}

std::shared_ptr<SkBitmap> OsmAnd::ICoreResourcesProvider::getResourceAsBitmap(
    const QString& name,
    const float displayDensityFactor) const
{
    bool ok = false;
    const auto data = getResource(name, displayDensityFactor, &ok);
    if (!ok)
        return nullptr;
    // hopefully it's redundant.
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    SkGraphics::SetImageGeneratorFromEncodedDataFactory(SkImageGeneratorCG::MakeFromEncodedCG);
#elif defined(SK_BUILD_FOR_WIN)
    SkGraphics::SetImageGeneratorFromEncodedDataFactory(SkImageGeneratorWIC::MakeFromEncodedWIC);
#endif
    const std::shared_ptr<SkBitmap> bitmap(new SkBitmap());
    SkMemoryStream dataStream(data.constData(), data.length(), false);
    sk_sp<SkData> skData = SkData::MakeFromStream(&dataStream, dataStream.getLength());
    sk_sp<SkImage> skImage = SkImage::MakeFromEncoded(skData);
    /*if (!SkImageDecoder::DecodeStream(
        &dataStream,
        bitmap.get(),
        SkColorType::kUnknown_SkColorType,
        SkImageDecoder::kDecodePixels_Mode))*/
    if (!skImage)
    {
        return nullptr;
    }
    //TODO check in runtime if bitmap is created
    if (skImage->asLegacyBitmap(bitmap.get())) 
    {
        return bitmap;
    }
    return nullptr;
}

std::shared_ptr<SkBitmap> OsmAnd::ICoreResourcesProvider::getResourceAsBitmap(const QString& name) const
{
    bool ok = false;
    const auto data = getResource(name, &ok);
    if (!ok)
        return nullptr;

    const std::shared_ptr<SkBitmap> bitmap(new SkBitmap());
    SkMemoryStream dataStream(data.constData(), data.length(), false);
    sk_sp<SkData> skData = SkData::MakeFromStream(&dataStream, dataStream.getLength());
    sk_sp<SkImage> skImage = SkImage::MakeFromEncoded(skData);
    /*if (!SkImageDecoder::DecodeStream(
        &dataStream,
        bitmap.get(),
        SkColorType::kUnknown_SkColorType,
        SkImageDecoder::kDecodePixels_Mode))*/
    if (!skImage)
    {
        return nullptr;
    }

    if (skImage->asLegacyBitmap(bitmap.get()))
    {
        return bitmap;
    }
    return nullptr;
}
