#ifndef _OSMAND_CORE_TILE_SQLITE_DATABASE_H_
#define _OSMAND_CORE_TILE_SQLITE_DATABASE_H_

#include <OsmAndCore/stdlib_common.h>
#include <array>

#include <OsmAndCore/QtExtensions.h>
#include <QString>
#include <QByteArray>
#include <QHash>
#include <QVariant>

#include <OsmAndCore.h>
#include <OsmAndCore/CommonTypes.h>
#include <OsmAndCore/PrivateImplementation.h>

namespace OsmAnd
{
    class TileSqliteDatabase_P;
    class OSMAND_CORE_API TileSqliteDatabase
    {
        Q_DISABLE_COPY_AND_MOVE(TileSqliteDatabase);
    public:
        struct OSMAND_CORE_API Meta Q_DECL_FINAL
        {
            Meta();
            ~Meta();

            QHash<QString, QVariant> values;

            static const QString TITLE;
            QString getTitle(bool* outOk = nullptr) const;
            void setTitle(QString title);

            static const QString RULE;
            QString getRule(bool* outOk = nullptr) const;
            void setRule(QString rule);

            static const QString REFERER;
            QString getReferer(bool* outOk = nullptr) const;
            void setReferer(QString referer);

            static const QString RANDOMS;
            QString getRandoms(bool* outOk = nullptr) const;
            void setRandoms(QString randoms);

            static const QString URL;
            QString getUrl(bool* outOk = nullptr) const;
            void setUrl(QString url);

            static const QString MIN_ZOOM;
            int64_t getMinZoom(bool* outOk = nullptr) const;
            void setMinZoom(int64_t minZoom);

            static const QString MAX_ZOOM;
            int64_t getMaxZoom(bool* outOk = nullptr) const;
            void setMaxZoom(int64_t maxZoom);

            static const QString ELLIPSOID;
            int64_t getEllipsoid(bool* outOk = nullptr) const;
            void setEllipsoid(int64_t ellipsoid);
            
            static const QString INVERTED_Y;
            int64_t getInvertedY(bool* outOk = nullptr) const;
            void setInvertedY(int64_t invertedY);

            static const QString TIME_COLUMN;
            QString getTimeColumn(bool* outOk = nullptr) const;
            void setTimeColumn(QString timeColumn);

            static const QString EXPIRE_MINUTES;
            int64_t getExpireMinutes(bool* outOk = nullptr) const;
            void setExpireMinutes(int64_t expireMinutes);

            static const QString TILE_NUMBERING;
            QString getTileNumbering(bool* outOk = nullptr) const;
            void setTileNumbering(QString tileNumbering);

            static const QString TILE_SIZE;
            int64_t getTileSize(bool* outOk = nullptr) const;
            void setTileSize(int64_t tileSize);
        };
    private:
        PrivateImplementation<TileSqliteDatabase_P> _p;
    protected:
    public:
        TileSqliteDatabase();
        TileSqliteDatabase(QString filename);
        virtual ~TileSqliteDatabase();

        const QString filename;

        bool isOpened() const;
        bool open();
        bool close(bool compact = true);

        bool isTileTimeSupported() const;
        bool hasTimeColumn() const;
        bool enableTileTimeSupport(bool force = false);

        ZoomLevel getMinZoom() const;
        ZoomLevel getMaxZoom() const;
        bool recomputeMinMaxZoom();

        AreaI getBBox31() const;
        bool recomputeBBox31(AreaI* pOutBBox31 = nullptr);

        AreaI getBBox31(ZoomLevel zoom) const;
        bool recomputeBBox31(ZoomLevel zoom, AreaI* pOutBBox31 = nullptr);

        std::array<AreaI, ZoomLevelsCount> getBBoxes31() const;
        bool recomputeBBoxes31(
            std::array<AreaI, ZoomLevelsCount>* pOutBBoxes31 = nullptr,
            AreaI* pOutBBox31 = nullptr
        );

        bool obtainMeta(Meta& outMeta) const;
        bool storeMeta(const Meta& meta);

        bool containsTileData(TileId tileId, ZoomLevel zoom) const;
        bool obtainTileTime(TileId tileId, ZoomLevel zoom, int64_t& outTime) const;
        bool obtainTileData(TileId tileId, ZoomLevel zoom, QByteArray& outData, int64_t* pOutTime = nullptr) const;
        bool storeTileData(TileId tileId, ZoomLevel zoom, const QByteArray& data, int64_t time = 0);
        bool removeTileData(TileId tileId, ZoomLevel zoom);
        bool removeTilesData();
        bool removeTilesData(ZoomLevel zoom);
        bool removeTilesData(AreaI bbox31, bool strict = true);
        bool removeTilesData(AreaI bbox31, ZoomLevel zoom, bool strict = true);

        bool compact();
    };

}

#endif // !defined(_OSMAND_CORE_TILE_SQLITE_DATABASE_H_)
