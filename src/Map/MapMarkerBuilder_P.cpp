#include "MapMarkerBuilder_P.h"
#include "MapMarkerBuilder.h"

#include "QtCommon.h"

#include "MapMarker.h"
#include "MapMarker_P.h"
#include "MapMarkersCollection.h"
#include "MapMarkersCollection_P.h"
#include "Utilities.h"

OsmAnd::MapMarkerBuilder_P::MapMarkerBuilder_P(MapMarkerBuilder* const owner_)
    : _isHidden(false)
    , _markerId(0)
    , _baseOrder(std::numeric_limits<int>::min())
    , _isAccuracyCircleSupported(false)
    , _isAccuracyCircleVisible(false)
    , _accuracyCircleRadius(0.0)
    , _direction(0.0f)
    , _pinIconVerticalAlignment(MapMarker::PinIconVerticalAlignment::CenterVertical)
    , _pinIconHorisontalAlignment(MapMarker::PinIconHorisontalAlignment::CenterHorizontal)
    , owner(owner_)
{
}

OsmAnd::MapMarkerBuilder_P::~MapMarkerBuilder_P()
{
}

bool OsmAnd::MapMarkerBuilder_P::isHidden() const
{
    QReadLocker scopedLocker(&_lock);

    return _isHidden;
}

void OsmAnd::MapMarkerBuilder_P::setIsHidden(const bool hidden)
{
    QWriteLocker scopedLocker(&_lock);

    _isHidden = hidden;
}

int OsmAnd::MapMarkerBuilder_P::getMarkerId() const
{
    QReadLocker scopedLocker(&_lock);
    
    return _markerId;
}

void OsmAnd::MapMarkerBuilder_P::setMarkerId(const int markerId)
{
    QWriteLocker scopedLocker(&_lock);
    
    _markerId = markerId;
}

int OsmAnd::MapMarkerBuilder_P::getBaseOrder() const
{
    QReadLocker scopedLocker(&_lock);

    return _baseOrder;
}

void OsmAnd::MapMarkerBuilder_P::setBaseOrder(const int baseOrder)
{
    QWriteLocker scopedLocker(&_lock);

    _baseOrder = baseOrder;
}

bool OsmAnd::MapMarkerBuilder_P::isAccuracyCircleSupported() const
{
    QReadLocker scopedLocker(&_lock);

    return _isAccuracyCircleSupported;
}

void OsmAnd::MapMarkerBuilder_P::setIsAccuracyCircleSupported(const bool supported)
{
    QWriteLocker scopedLocker(&_lock);

    _isAccuracyCircleSupported = supported;
}

bool OsmAnd::MapMarkerBuilder_P::isAccuracyCircleVisible() const
{
    QReadLocker scopedLocker(&_lock);

    return _isAccuracyCircleVisible;
}

void OsmAnd::MapMarkerBuilder_P::setIsAccuracyCircleVisible(const bool visible)
{
    QWriteLocker scopedLocker(&_lock);

    _isAccuracyCircleVisible = visible;
}

double OsmAnd::MapMarkerBuilder_P::getAccuracyCircleRadius() const
{
    QReadLocker scopedLocker(&_lock);

    return _accuracyCircleRadius;
}

void OsmAnd::MapMarkerBuilder_P::setAccuracyCircleRadius(const double radius)
{
    QWriteLocker scopedLocker(&_lock);

    _accuracyCircleRadius = radius;
}

OsmAnd::FColorRGB OsmAnd::MapMarkerBuilder_P::getAccuracyCircleBaseColor() const
{
    QReadLocker scopedLocker(&_lock);

    return _accuracyCircleBaseColor;
}

void OsmAnd::MapMarkerBuilder_P::setAccuracyCircleBaseColor(const FColorRGB baseColor)
{
    QWriteLocker scopedLocker(&_lock);

    _accuracyCircleBaseColor = baseColor;
}

OsmAnd::PointI OsmAnd::MapMarkerBuilder_P::getPosition() const
{
    QReadLocker scopedLocker(&_lock);

    return _position;
}

void OsmAnd::MapMarkerBuilder_P::setPosition(const PointI position)
{
    QWriteLocker scopedLocker(&_lock);

    _position = position;
}

sk_sp<const SkImage> OsmAnd::MapMarkerBuilder_P::getPinIcon() const
{
    QReadLocker scopedLocker(&_lock);

    return _pinIcon;
}

void OsmAnd::MapMarkerBuilder_P::setPinIcon(const sk_sp<const SkImage>& image)
{
    QWriteLocker scopedLocker(&_lock);

    _pinIcon = image;
}

OsmAnd::MapMarker::PinIconVerticalAlignment OsmAnd::MapMarkerBuilder_P::getPinIconVerticalAlignment() const
{
    QReadLocker scopedLocker(&_lock);

    return _pinIconVerticalAlignment;
}

OsmAnd::MapMarker::PinIconHorisontalAlignment OsmAnd::MapMarkerBuilder_P::getPinIconHorisontalAlignment() const
{
    QReadLocker scopedLocker(&_lock);
    
    return _pinIconHorisontalAlignment;
}

void OsmAnd::MapMarkerBuilder_P::setPinIconVerticalAlignment(const MapMarker::PinIconVerticalAlignment value)
{
    QWriteLocker scopedLocker(&_lock);

    _pinIconVerticalAlignment = value;
}

void OsmAnd::MapMarkerBuilder_P::setPinIconHorisontalAlignment(const MapMarker::PinIconHorisontalAlignment value)
{
    QWriteLocker scopedLocker(&_lock);
    
    _pinIconHorisontalAlignment = value;
}

OsmAnd::ColorARGB OsmAnd::MapMarkerBuilder_P::getPinIconModulationColor() const
{
    QReadLocker scopedLocker(&_lock);

    return _pinIconModulationColor;
}

void OsmAnd::MapMarkerBuilder_P::setPinIconModulationColor(const ColorARGB colorValue)
{
    QWriteLocker scopedLocker(&_lock);

    _pinIconModulationColor = colorValue;
}

QString OsmAnd::MapMarkerBuilder_P::getCaption() const
{
    QReadLocker scopedLocker(&_lock);

    return _caption;
}

void OsmAnd::MapMarkerBuilder_P::setCaption(const QString& caption)
{
    QWriteLocker scopedLocker(&_lock);

    _caption = caption;
}

OsmAnd::TextRasterizer::Style OsmAnd::MapMarkerBuilder_P::getCaptionStyle() const
{
    QReadLocker scopedLocker(&_lock);

    return _captionStyle;
}

void OsmAnd::MapMarkerBuilder_P::setCaptionStyle(const TextRasterizer::Style captionStyle)
{
    QWriteLocker scopedLocker(&_lock);

    _captionStyle = captionStyle;
}

double OsmAnd::MapMarkerBuilder_P::getCaptionTopSpace() const
{
    QReadLocker scopedLocker(&_lock);

    return _captionTopSpace;
}

void OsmAnd::MapMarkerBuilder_P::setCaptionTopSpace(const double captionTopSpace)
{
    QWriteLocker scopedLocker(&_lock);

    _captionTopSpace = captionTopSpace;
}

QHash< OsmAnd::MapMarker::OnSurfaceIconKey, sk_sp<const SkImage> >
OsmAnd::MapMarkerBuilder_P::getOnMapSurfaceIcons() const
{
    QReadLocker scopedLocker(&_lock);

    return detachedOf(_onMapSurfaceIcons);
}

void OsmAnd::MapMarkerBuilder_P::addOnMapSurfaceIcon(
    const MapMarker::OnSurfaceIconKey key,
    const sk_sp<const SkImage>& image)
{
    QWriteLocker scopedLocker(&_lock);

    _onMapSurfaceIcons.insert(key, image);
}

void OsmAnd::MapMarkerBuilder_P::removeOnMapSurfaceIcon(const MapMarker::OnSurfaceIconKey key)
{
    QWriteLocker scopedLocker(&_lock);

    _onMapSurfaceIcons.remove(key);
}

void OsmAnd::MapMarkerBuilder_P::clearOnMapSurfaceIcons()
{
    QWriteLocker scopedLocker(&_lock);

    _onMapSurfaceIcons.clear();
}

std::shared_ptr<OsmAnd::MapMarker> OsmAnd::MapMarkerBuilder_P::buildAndAddToCollection(
    const std::shared_ptr<MapMarkersCollection>& collection)
{
    QReadLocker scopedLocker(&_lock);

    // Construct map symbols group for this marker
    const std::shared_ptr<MapMarker> marker(new MapMarker(
        _markerId,
        _baseOrder,
        _pinIcon,
        _pinIconVerticalAlignment,
        _pinIconHorisontalAlignment,
        _caption,
        _captionStyle,
        _captionTopSpace,
        detachedOf(_onMapSurfaceIcons),
        _isAccuracyCircleSupported,
        _accuracyCircleBaseColor));
    
    marker->setIsHidden(_isHidden);
    if (_isAccuracyCircleSupported)
    {
        marker->setIsAccuracyCircleVisible(_isAccuracyCircleVisible);
        marker->setAccuracyCircleRadius(_accuracyCircleRadius);
    }
    marker->setPosition(_position);
    marker->setPinIconModulationColor(_pinIconModulationColor);
    
    marker->applyChanges();

    // Add marker to collection and return it if adding was successful
    if (!collection->_p->addMarker(marker))
        return nullptr;
    return marker;
}
