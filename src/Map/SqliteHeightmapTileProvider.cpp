#include "SqliteHeightmapTileProvider.h"
#include "SqliteHeightmapTileProvider_P.h"

#include "MapDataProviderHelpers.h"

OsmAnd::SqliteHeightmapTileProvider::SqliteHeightmapTileProvider(
    const std::shared_ptr<const ITileSqliteDatabasesCollection>& sourcesCollection_,
    uint32_t outputTileSize_)
    : _p(new SqliteHeightmapTileProvider_P(this))
    , sourcesCollection(sourcesCollection_)
    , outputTileSize(outputTileSize_)
{
}

OsmAnd::SqliteHeightmapTileProvider::~SqliteHeightmapTileProvider()
{
}

OsmAnd::ZoomLevel OsmAnd::SqliteHeightmapTileProvider::getMinZoom() const
{
    return _p->getMinZoom();
}

OsmAnd::ZoomLevel OsmAnd::SqliteHeightmapTileProvider::getMaxZoom() const
{
    return _p->getMaxZoom();
}

uint32_t OsmAnd::SqliteHeightmapTileProvider::getTileSize() const
{
    return outputTileSize;
}

bool OsmAnd::SqliteHeightmapTileProvider::supportsNaturalObtainData() const
{
    return true;
}

bool OsmAnd::SqliteHeightmapTileProvider::obtainData(
    const IMapDataProvider::Request& request,
    std::shared_ptr<IMapDataProvider::Data>& outData,
    std::shared_ptr<Metric>* const pOutMetric /*= nullptr*/)
{
    return _p->obtainData(request, outData, pOutMetric);
}

bool OsmAnd::SqliteHeightmapTileProvider::supportsNaturalObtainDataAsync() const
{
    return false;
}

void OsmAnd::SqliteHeightmapTileProvider::obtainDataAsync(
    const IMapDataProvider::Request& request,
    const IMapDataProvider::ObtainDataAsyncCallback callback,
    const bool collectMetric /*= false*/)
{
    MapDataProviderHelpers::nonNaturalObtainDataAsync(this, request, callback, collectMetric);
}
