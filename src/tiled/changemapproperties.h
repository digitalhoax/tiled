/*
 * changemapproperties.h
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
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

#ifndef CHANGEMAPPROPERTIES_H
#define CHANGEMAPPROPERTIES_H

#include "map.h"

#include <QColor>
#include <QUndoCommand>

namespace Tiled {

namespace Internal {

class MapDocument;

class ChangeMapProperties : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Change Map Properties' command.
     *
     * @param mapDocument       the map document of the map
     * @param backgroundColor   the new color to apply for the background
     * @param layerDataFormat   the new layer data format
     * @param mapRenderOrder   	the new map render order
     */
    ChangeMapProperties(MapDocument *mapDocument,
                        const QColor &backgroundColor,
                        Map::LayerDataFormat layerDataFormat,
                        Map::RenderOrder mapRenderOrder);

    void undo();
    void redo();

private:
    void swap();

    MapDocument *mMapDocument;
    QColor mBackgroundColor;
    Map::LayerDataFormat mLayerDataFormat;
    Map::RenderOrder mMapRenderOrder;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGEMAPPROPERTIES_H
