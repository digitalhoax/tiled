/*
 * stampbrush.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010 Stefan Beller <stefanbeller@googlemail.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "stampbrush.h"

#include "brushitem.h"
#include "geometry.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "tilelayer.h"

#include <math.h>
#include <QVector>

using namespace Tiled;
using namespace Tiled::Internal;

StampBrush::StampBrush(QObject *parent)
    : AbstractTileTool(tr("Stamp Brush"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-clone.png")),
                       QKeySequence(tr("B")),
                       parent)
    , mStamp(0)
    , mStampX(0), mStampY(0)
    , mBrushBehavior(Free)
    , mStampReferenceX(0)
    , mStampReferenceY(0)
    , mIsRandom(false)
{
}

StampBrush::~StampBrush()
{
    delete mStamp;
}

void StampBrush::tilePositionChanged(const QPoint &)
{
    const int x = mStampX;
    const int y = mStampY;
    updatePosition();
    switch (mBrushBehavior) {
    case Paint:
        foreach (const QPoint &p, pointsOnLine(x, y, mStampX, mStampY))
            doPaint(true, p.x(), p.y());
        break;
    case LineStartSet:
        configureBrush(pointsOnLine(mStampReferenceX, mStampReferenceY,
                                    mStampX, mStampY));
        break;
    case CircleMidSet:
        configureBrush(pointsOnEllipse(mStampReferenceX, mStampReferenceY,
                                       mStampX, mStampY));
        break;
    case Capture:
        brushItem()->setTileRegion(capturedArea());
        break;
    case Line:
    case Circle:
        updatePosition();
        break;
    case Free:
        updatePosition();
        break;
    }
}

void StampBrush::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (!brushItem()->isVisible())
        return;

    if (event->button() == Qt::LeftButton) {
        switch (mBrushBehavior) {
        case Line:
            mStampReferenceX = mStampX;
            mStampReferenceY = mStampY;
            mBrushBehavior = LineStartSet;
            break;
        case Circle:
            mStampReferenceX = mStampX;
            mStampReferenceY = mStampY;
            mBrushBehavior = CircleMidSet;
            break;
        case LineStartSet:
            doPaint(false, 0, 0);
            mStampReferenceX = mStampX;
            mStampReferenceY = mStampY;
            break;
        case CircleMidSet:
            doPaint(false, 0, 0);
            break;
        case Paint:
            beginPaint();
            break;
        case Free:
            beginPaint();
            mBrushBehavior = Paint;
            break;
        case Capture:
            break;
        }
    } else {
        if (event->button() == Qt::RightButton)
            beginCapture();
    }
}

void StampBrush::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    switch (mBrushBehavior) {
    case Capture:
        if (event->button() == Qt::RightButton) {
            endCapture();
            mBrushBehavior = Free;
        }
        break;
    case Paint:
        if (event->button() == Qt::LeftButton)
            mBrushBehavior = Free;
    default:
        // do nothing?
        break;
    }
}

void StampBrush::configureBrush(const QVector<QPoint> &list)
{
    if (!mStamp)
        return;

    QRegion reg;
    QRegion stampRegion;

    if (mIsRandom)
        stampRegion = brushItem()->tileLayer()->region();
    else
        stampRegion = mStamp->region();

    Map *map = mapDocument()->map();

    // TODO Alex : check parallax value
    TileLayer *stamp = new TileLayer(QString(), 0, 0,
                                     map->width(), map->height(), 1.0f);

    foreach (const QPoint p, list) {
        const QRegion update = stampRegion.translated(p.x() - mStampX,
                                                      p.y() - mStampY);
        if (!reg.intersects(update)) {
            reg += update;

            if (mIsRandom) {
                TileLayer *newStamp = getRandomTileLayer();
                stamp->merge(p, newStamp);
                delete newStamp;
            } else {
                stamp->merge(p, mStamp);
            }

        }
    }

    brushItem()->setTileLayer(stamp);
    delete stamp;
}

void StampBrush::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if (!mStamp)
        return;

    if (modifiers & Qt::ShiftModifier) {
        mBrushBehavior = Line;
        if (modifiers & Qt::ControlModifier) {
            mBrushBehavior = Circle;
            // while finding the mid point, there is no need to show
            // the (maybe bigger than 1x1) stamp
            brushItem()->setTileLayer(0);
            brushItem()->setTileRegion(QRect(tilePosition(), QSize(1, 1)));
        }
    } else {
        mBrushBehavior = Free;
    }

    switch (mBrushBehavior) {
    case Circle:
        // do not update brushItems tilelayer by setStamp
        break;
    default:
        if (mIsRandom)
            setRandomStamp();
        else
            brushItem()->setTileLayer(mStamp);

        updatePosition();
    }
}

void StampBrush::languageChanged()
{
    setName(tr("Stamp Brush"));
    setShortcut(QKeySequence(tr("B")));
}

void StampBrush::mapDocumentChanged(MapDocument *oldDocument,
                                    MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    // Reset the brush, since it probably became invalid
    brushItem()->setTileRegion(QRegion());
    setStamp(0);
}

TileLayer *StampBrush::getRandomTileLayer() const
{
    if (mRandomList.empty())
        return 0;

    TileLayer *ret = new TileLayer(QString(), 0, 0, 1, 1, 1.0f);
    ret->setCell(0, 0, mRandomList.at(rand() % mRandomList.size()));
    return ret;
}

void StampBrush::updateRandomList()
{
    mRandomList.clear();

    if (!mStamp)
        return;

    for (int x = 0; x < mStamp->width(); x++)
        for (int y = 0; y < mStamp->height(); y++)
            if (!mStamp->cellAt(x, y).isEmpty())
                mRandomList.append(mStamp->cellAt(x, y));
}

void StampBrush::setStamp(TileLayer *stamp)
{
    if (mStamp == stamp)
        return;

    delete mStamp;
    mStamp = stamp;

    if (mIsRandom) {
        updateRandomList();
        setRandomStamp();
    } else {
        brushItem()->setTileLayer(mStamp);
    }

    updatePosition();
}

void StampBrush::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint(false, mStampX, mStampY);
}

void StampBrush::beginCapture()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Capture;

    mCaptureStart = tilePosition();

    setStamp(0);
}

void StampBrush::endCapture()
{
    if (mBrushBehavior != Capture)
        return;

    mBrushBehavior = Free;

    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    // Intersect with the layer and translate to layer coordinates
    QRect captured = capturedArea();
    captured &= QRect(tileLayer->x(), tileLayer->y(),
                      tileLayer->width(), tileLayer->height());

    if (captured.isValid()) {
        captured.translate(-tileLayer->x(), -tileLayer->y());
        TileLayer *capture = tileLayer->copy(captured);
        emit currentTilesChanged(capture);
        // A copy will have been created, so delete this version
        delete capture;
    } else {
        updatePosition();
    }
}

QRect StampBrush::capturedArea() const
{
    QRect captured = QRect(mCaptureStart, tilePosition()).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}

void StampBrush::doPaint(bool mergeable, int whereX, int whereY)
{
    TileLayer *stamp = brushItem()->tileLayer();

    if (!stamp)
        return;

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (!tileLayer->bounds().intersects(QRect(whereX, whereY,
                                              stamp->width(),
                                              stamp->height())))
        return;

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(), tileLayer,
                            whereX, whereY, stamp);
    paint->setMergeable(mergeable);
    mapDocument()->undoStack()->push(paint);
    mapDocument()->emitRegionEdited(brushItem()->tileRegion(), tileLayer);
}

/**
 * Updates the position of the brush item.
 */
void StampBrush::updatePosition()
{
    if (mIsRandom)
        setRandomStamp();

    const QPoint tilePos = tilePosition();

    if (!brushItem()->tileLayer()) {
        brushItem()->setTileRegion(QRect(tilePos, QSize(1, 1)));
        mStampX = tilePos.x();
        mStampY = tilePos.y();
    }

    if (mIsRandom || !mStamp) {
        mStampX = tilePos.x();
        mStampY = tilePos.y();
    } else {
        mStampX = tilePos.x() - mStamp->width() / 2;
        mStampY = tilePos.y() - mStamp->height() / 2;
    }
    brushItem()->setTileLayerPosition(QPoint(mStampX, mStampY));
}

void StampBrush::setRandom(bool value)
{
    mIsRandom = value;

    if (mIsRandom) {
        updateRandomList();
        setRandomStamp();
    } else {
        brushItem()->setTileLayer(mStamp);
    }
}

void StampBrush::setRandomStamp()
{
    TileLayer *t = getRandomTileLayer();
    brushItem()->setTileLayer(t);
    delete t;
}
