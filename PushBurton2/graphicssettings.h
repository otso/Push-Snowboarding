/*
 * Copyright (c) 2011 Nokia Corporation
 *
 * This file is part of the Push Snowboarding Project, More info at:
 * www.pushsnowboading.com
 *
 * Author: Clovis Scotti <scotti@ieee.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef GRAPHICSSETTINGS_H
#define GRAPHICSSETTINGS_H

#include <QObject>
#include <QGraphicsObject>
#include "graphictextbt.h"
#include "graphicpixmapbt.h"

#include <QDebug>

#include "pushdevicesholder.h"

//Gambiarra, xunxo, workaround... (LiveView)
#include "liveview.h"

class GraphicsSettings : public QGraphicsObject
{
    Q_OBJECT
public:
    GraphicsSettings(PushDevicesHolder*, QObject*);
    ~GraphicsSettings();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *,QWidget *);

private:
    GraphicTextBt * startLiveViewBt;
    GraphicPixmapBt * homeBt;
    LiveView * liveView;
    PushDevicesHolder * configuredDevices;

signals:
    void home_bt_clicked();

private slots:
    void start_live_view();
    void end_live_view();
};

#endif // GRAPHICSSETTINGS_H
