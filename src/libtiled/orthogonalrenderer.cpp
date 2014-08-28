/*
 * orthogonalrenderer.cpp
 * Copyright 2009-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "orthogonalrenderer.h"

#include "map.h"
#include "mapobject.h"
//#include "mapview.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <cmath>
#include <iostream>

using namespace Tiled;

QSize OrthogonalRenderer::mapSize() const
{
    return QSize(map()->width() * map()->tileWidth(),
                 map()->height() * map()->tileHeight());
}

QRect OrthogonalRenderer::boundingRect(const QRect &rect) const
{
    float parallax = getParallaxFromCurrentLayer();
//    std::cout << parallax << std::endl;

    const int tileWidth = map()->tileWidth();//*parallax;
    const int tileHeight = map()->tileHeight();//*parallax;

    return QRect(rect.x() * tileWidth,
                 rect.y() * tileHeight,
                 rect.width() * tileWidth,
                 rect.height() * tileHeight);
}

QRectF OrthogonalRenderer::boundingRect(const MapObject *object) const
{
    const QRectF bounds = object->bounds();

    QRectF boundingRect;

    if (!object->cell().isEmpty()) {
        const QPointF bottomLeft = bounds.topLeft();
        const Tile *tile = object->cell().tile;
        const QSize imgSize = tile->image().size();
        const QPoint tileOffset = tile->tileset()->tileOffset();
        boundingRect = QRectF(bottomLeft.x() + tileOffset.x(),
                              bottomLeft.y() + tileOffset.y() - imgSize.height(),
                              imgSize.width(),
                              imgSize.height()).adjusted(-1, -1, 1, 1);
    } else {
        const qreal extraSpace = qMax(objectLineWidth() / 2, qreal(1));

        switch (object->shape()) {
        case MapObject::Ellipse:
        case MapObject::Rectangle:
            if (bounds.isNull()) {
                boundingRect = bounds.adjusted(-10 - extraSpace,
                                               -10 - extraSpace,
                                               10 + extraSpace + 1,
                                               10 + extraSpace + 1);
            } else {
                const int nameHeight = object->name().isEmpty() ? 0 : 15;
                boundingRect = bounds.adjusted(-extraSpace,
                                               -nameHeight - extraSpace,
                                               extraSpace + 1,
                                               extraSpace + 1);
            }
            break;

        case MapObject::Polygon:
        case MapObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = pixelToScreenCoords(polygon);
            boundingRect = screenPolygon.boundingRect().adjusted(-extraSpace,
                                                                 -extraSpace,
                                                                 extraSpace + 1,
                                                                 extraSpace + 1);
            break;
        }
        }
    }

    return boundingRect;
}

QPainterPath OrthogonalRenderer::shape(const MapObject *object) const
{
    QPainterPath path;

    if (!object->cell().isEmpty()) {
        path.addRect(boundingRect(object));
    } else {
        switch (object->shape()) {
        case MapObject::Rectangle: {
            const QRectF bounds = object->bounds();

            if (bounds.isNull()) {
                path.addEllipse(bounds.topLeft(), 20, 20);
            } else {
                path.addRoundedRect(bounds, 10, 10);
            }
            break;
        }
        case MapObject::Polygon:
        case MapObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            const QPolygonF screenPolygon = pixelToScreenCoords(polygon);
            if (object->shape() == MapObject::Polygon) {
                path.addPolygon(screenPolygon);
            } else {
                for (int i = 1; i < screenPolygon.size(); ++i) {
                    path.addPolygon(lineToPolygon(screenPolygon[i - 1],
                                    screenPolygon[i]));
                }
                path.setFillRule(Qt::WindingFill);
            }
            break;
        }
        case MapObject::Ellipse: {
            const QRectF bounds = object->bounds();

            if (bounds.isNull()) {
                path.addEllipse(bounds.topLeft(), 20, 20);
            } else {
                path.addEllipse(bounds);
            }
            break;
        }
        }
    }

    return path;
}

void OrthogonalRenderer::drawGrid(QPainter *painter, const QRectF &rect,
                                  QColor gridColor) const
{
    //TODO (ALEX)
//    double parallax = layer->parallax();
    float parallax = getParallaxFromCurrentLayer();

    const int tileWidth = map()->tileWidth() * parallax;
    const int tileHeight = map()->tileHeight() * parallax;

//    const int tileWidth = map()->tileWidth();
//    const int tileHeight = map()->tileHeight();

    if (tileWidth <= 0 || tileHeight <= 0)
        return;

    //TODO (ALEX) draw grid complete
    int startX = 0;
    int startY = 0;
    int endX = map()->width() * tileWidth + 1;
    int endY = map()->height() * tileHeight + 1;

//    const int startX = qMax(0, (int) (rect.x() / tileWidth) * tileWidth);
//    const int startY = qMax(0, (int) (rect.y() / tileHeight) * tileHeight);
//    const int endX = qMin((int) std::ceil(rect.right()),
//                          map()->width() * tileWidth + 1);
//    const int endY = qMin((int) std::ceil(rect.bottom()),
//                          map()->height() * tileHeight + 1);

    gridColor.setAlpha(128);

    QPen gridPen(gridColor);
    gridPen.setCosmetic(true);
    gridPen.setDashPattern(QVector<qreal>() << 2 << 2);

    if (startY < endY) {
        gridPen.setDashOffset(startY);
        painter->setPen(gridPen);
        for (int x = startX; x < endX; x += tileWidth)
            painter->drawLine(x, startY, x, endY - 1);
    }

    if (startX < endX) {
        gridPen.setDashOffset(startX);
        painter->setPen(gridPen);
        for (int y = startY; y < endY; y += tileHeight)
            painter->drawLine(startX, y, endX - 1, y);
    }
}

int i = 0;

void OrthogonalRenderer::drawTileLayer(QPainter *painter,
                                       const TileLayer *layer,
                                       const QRectF &exposed) const
{
    const QTransform savedTransform = painter->transform();
    double parallax = layer->parallax();
    const int tileWidth = map()->tileWidth() * parallax;
    const int tileHeight = map()->tileHeight() * parallax;
    const QPointF layerPos(layer->x() * tileWidth,
                           layer->y() * tileHeight );

    painter->translate(layerPos);

    int startX = 0;
    int startY = 0;
    int endX = layer->width() -1;
    int endY = layer->height() -1;

//    if (!exposed.isNull()) {
        QMargins drawMargins = layer->drawMargins();

        drawMargins.setTop(drawMargins.top() - tileHeight);
        drawMargins.setRight(drawMargins.right() - tileWidth);

//        QRectF rect = exposed.adjusted(-drawMargins.right(),
//                                       -drawMargins.bottom(),
//                                       drawMargins.left(),
//                                       drawMargins.top());

//        std::cout << "ex rect    : " << exposed.left() << ":" << exposed.right() << "  view: " << exposed.right()-exposed.left() << std::endl;
//        std::cout << "ex div     : " << exposed.left() << ":" << map()->width()*map()->tileWidth()  << std::endl;
//        std::cout << "ex distance: " << (exposed.left()) / (map()->width()*map()->tileWidth())  << "%" << std::endl;

//        std::cout << "rect    : " << rect.left()+ 194 << ":" << rect.right() << "  view: " << rect.right()-rect.left() << std::endl;
//        std::cout << "div     : " << rect.left()+ 194 << ":" << map()->width()*map()->tileWidth()+ 194  << std::endl;
//        std::cout << "distance: " << (rect.left()+ 194) / (map()->width()*map()->tileWidth()+ 194)  << "%" << std::endl;

//        rect.translate(-layerPos);

//        startX = qMax((int) rect.x() / tileWidth, 0);
//        startY = qMax((int) rect.y() / tileHeight, 0);
//        endX = qMin((int) std::ceil(rect.right()) / tileWidth, endX);
//        endY = qMin((int) std::ceil(rect.bottom()) / tileHeight, endY);
//    }

    CellRenderer renderer(painter);

    Map::RenderOrder renderOrder = map()->renderOrder();

    int incX = 1, incY = 1;
    switch (renderOrder) {
    case Map::RightUp:
        std::swap(startY, endY);
        incY = -1;
        break;
    case Map::LeftDown:
        std::swap(startX, endX);
        incX = -1;
        break;
    case Map::LeftUp:
        std::swap(startX, endX);
        std::swap(startY, endY);
        incX = -1;
        incY = -1;
        break;
    case Map::RightDown:
    default:
        break;
    }

    endX += incX;
    endY += incY;

    float zoom = m_zoom;
    float p = parallax -1;
    float hb = ( static_cast<float>(m_scrollBarX) / static_cast<float>(m_maxX) ) * 100.0f;

    // TODO (ALEX) : find values
    float offset = tileWidth * hb * p * 4 * zoom;
//    std::cout << hb << "   "<< p << std::endl;

    for (int y = startY; y != endY; y += incY) {
        for (int x = startX; x != endX; x += incX) {
            const Cell &cell = layer->cellAt(x, y);
            if (cell.isEmpty())
                continue;
//            std::cout << i << ": " << x << ",  "<< y << std::endl;
            renderer.render(cell,
                            QPointF(x * tileWidth  - offset, (y + 1) * tileHeight),
                            CellRenderer::BottomLeft, parallax);
        }
    }

    renderer.flush();

    painter->setTransform(savedTransform);
    i++;
}

void OrthogonalRenderer::drawTileSelection(QPainter *painter,
                                           const QRegion &region,
                                           const QColor &color,
                                           const QRectF &exposed) const
{
    foreach (const QRect &r, region.rects()) {
    	// TODO (ALEX)
        const QRectF toFill = QRectF(boundingRect(r)).intersected(exposed);
        if (!toFill.isEmpty())
            painter->fillRect(toFill, color);
    }
}

void OrthogonalRenderer::drawMapObject(QPainter *painter,
                                       const MapObject *object,
                                       const QColor &color) const
{
    painter->save();

    const QRectF bounds = object->bounds();
    QRectF rect(bounds);

    painter->translate(rect.topLeft());
    rect.moveTopLeft(QPointF(0, 0));

    if (!object->cell().isEmpty()) {
        const Cell &cell = object->cell();

        CellRenderer(painter).render(cell, QPointF(),
                                     CellRenderer::BottomLeft);

        if (testFlag(ShowTileObjectOutlines)) {
            const QRect rect = cell.tile->image().rect();
            QPen pen(Qt::SolidLine);
            pen.setCosmetic(true);
            painter->setPen(pen);
            painter->drawRect(rect);
            pen.setStyle(Qt::DotLine);
            pen.setColor(color);
            painter->setPen(pen);
            painter->drawRect(rect);
        }
    } else {
        const qreal lineWidth = objectLineWidth();
        const qreal scale = painterScale();
        const qreal shadowDist = (lineWidth == 0 ? 1 : lineWidth) / scale;
        const QPointF shadowOffset = QPointF(shadowDist * 0.5,
                                             shadowDist * 0.5);

        QPen linePen(color, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        linePen.setCosmetic(true);
        QPen shadowPen(linePen);
        shadowPen.setColor(Qt::black);

        QColor brushColor = color;
        brushColor.setAlpha(50);
        const QBrush fillBrush(brushColor);

        painter->setRenderHint(QPainter::Antialiasing);

        // Trying to draw an ellipse with 0-width is causing a hang in
        // CoreGraphics when drawing the path requested by the
        // QCoreGraphicsPaintEngine. Draw them as rectangle instead.
        MapObject::Shape shape = object->shape();
        if (shape == MapObject::Ellipse &&
                ((rect.width() == qreal(0)) ^ (rect.height() == qreal(0)))) {
            shape = MapObject::Rectangle;
        }

        switch (shape) {
        case MapObject::Rectangle: {
            if (rect.isNull())
                rect = QRectF(QPointF(-10, -10), QSizeF(20, 20));

            const QFontMetrics fm = painter->fontMetrics();
            QString name = fm.elidedText(object->name(), Qt::ElideRight,
                                         rect.width() + 2);

            // Draw the shadow
            painter->setPen(shadowPen);
            painter->drawRect(rect.translated(shadowOffset));
            if (!name.isEmpty())
                painter->drawText(QPointF(0, -4 - lineWidth / 2) + shadowOffset, name);

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawRect(rect);
            if (!name.isEmpty())
                painter->drawText(QPointF(0, -4 - lineWidth / 2), name);

            break;
        }

        case MapObject::Polyline: {
            QPolygonF screenPolygon = pixelToScreenCoords(object->polygon());

            painter->setPen(shadowPen);
            painter->drawPolyline(screenPolygon.translated(shadowOffset));

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawPolyline(screenPolygon);
            break;
        }

        case MapObject::Polygon: {
            QPolygonF screenPolygon = pixelToScreenCoords(object->polygon());

            painter->setPen(shadowPen);
            painter->drawPolygon(screenPolygon.translated(shadowOffset));

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawPolygon(screenPolygon);
            break;
        }

        case MapObject::Ellipse: {
            if (rect.isNull())
                rect = QRectF(QPointF(-10, -10), QSizeF(20, 20));

            const QFontMetrics fm = painter->fontMetrics();
            QString name = fm.elidedText(object->name(), Qt::ElideRight,
                                         rect.width() + 2);

            // Draw the shadow
            painter->setPen(shadowPen);
            painter->drawEllipse(rect.translated(shadowOffset));
            if (!name.isEmpty())
                painter->drawText(QPoint(1, -5 + 1), name);

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawEllipse(rect);
            if (!name.isEmpty())
                painter->drawText(QPoint(0, -5), name);

            break;
        }
        }
    }

    painter->restore();
}

QPointF OrthogonalRenderer::pixelToTileCoords(qreal x, qreal y) const
{
    float parallax = getParallaxFromCurrentLayer();
    return QPointF(x / map()->tileWidth() / parallax,
                   y / map()->tileHeight() / parallax);
}

QPointF OrthogonalRenderer::tileToPixelCoords(qreal x, qreal y) const
{
    float parallax = getParallaxFromCurrentLayer();
    return QPointF(x * map()->tileWidth()* parallax,
                   y * map()->tileHeight()* parallax);
}

QPointF OrthogonalRenderer::screenToTileCoords(qreal x, qreal y) const
{
    // TODO (ALEX) : add parallax size
     float parallax = getParallaxFromCurrentLayer();
     return QPointF( x / map()->tileWidth() / parallax,
                     y / map()->tileHeight() / parallax );
}

//QPointF OrthogonalRenderer::screenToTileCoordsToCreate(qreal x, qreal y) const
//{
//    // TODO (ALEX) : add parallax size
//     float parallax = getParallaxFromCurrentLayer();
//     return QPointF( (x + getParallaxZoomOffset()) / map()->tileWidth() / parallax,
//                     y / map()->tileHeight() / parallax );
//}

QPointF OrthogonalRenderer::tileToScreenCoords(qreal x, qreal y) const
{
    float parallax = getParallaxFromCurrentLayer();
    return QPointF(x * map()->tileWidth()* parallax,
                   y * map()->tileHeight()* parallax);
}

QPointF OrthogonalRenderer::screenToPixelCoords(qreal x, qreal y) const
{
    return QPointF(x, y);
}

QPointF OrthogonalRenderer::pixelToScreenCoords(qreal x, qreal y) const
{
    return QPointF(x, y);
}

float OrthogonalRenderer::getParallaxFromCurrentLayer() const {
    TileLayer * layer = static_cast<TileLayer *>(m_currentLayer);
     float parallax = 1.0f;

     if(layer) {
          parallax = layer->parallax();
     }
     return parallax;
}

float OrthogonalRenderer::getParallaxZoomOffset() const {
    float parallax = getParallaxFromCurrentLayer();
    int tileWidth = map()->tileWidth() * parallax;
    float zoom = m_zoom;
    float p = parallax -1;
    float hb = ( static_cast<float>(m_scrollBarX) / static_cast<float>(m_maxX) ) * 100.0f;

    // TODO (ALEX) : find values
    float offset = tileWidth * hb * p * 4 * zoom;
    return offset;
}
