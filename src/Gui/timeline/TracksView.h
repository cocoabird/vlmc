/*****************************************************************************
 * TracksView.h: QGraphicsView that contains the TracksScene
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef TRACKSVIEW_H
#define TRACKSVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include "vlmc.h"
#include "MainWorkflow.h"
#include "AbstractGraphicsMediaItem.h"
#include "GraphicsCursorItem.h"

class QWheelEvent;
class QGraphicsWidget;
class QGraphicsLinearLayout;

class TracksScene;
class GraphicsMovieItem;
class GraphicsAudioItem;
class WorkflowRenderer;

class TracksView : public QGraphicsView
{
    Q_OBJECT

public:
    TracksView( QGraphicsScene* scene, MainWorkflow* mainWorkflow, WorkflowRenderer* renderer, QWidget* parent = 0 );
    void setDuration( int duration );
    int duration() const { return m_projectDuration; }
    int tracksHeight() const { return m_tracksHeight; }
    unsigned int tracksCount() const { return m_tracksCount; }
    void setCursorPos( qint64 pos );
    qint64 cursorPos();
    GraphicsCursorItem* tracksCursor() const { return m_cursorLine; }
    void setScale( double scaleFactor );
    QList<AbstractGraphicsMediaItem*> mediaItems( const QPoint& pos );
    void                    removeMediaItem( AbstractGraphicsMediaItem* item );
    void                    removeMediaItem( const QList<AbstractGraphicsMediaItem*>& items );
    void                    setTool( ToolButtons button );
    ToolButtons             tool() { return m_tool; }
    WorkflowRenderer*       getRenderer() { return m_renderer; }
    //MEGAFIXME
    //FIXME
    //REMOVEME
    //TODO
    //YOU'RE HIDING A BUG
    //WRONG
    //Ugly method provided by etix :)
    bool                    setItemOldTrack( const QUuid& uuid, uint32_t oldTrackNumber );

public slots:
    void                    clear();
    void                    addMediaItem( Clip* clip, unsigned int track, qint64 start );
    void                    moveMediaItem( const QUuid& uuid, unsigned int track, qint64 time );
    void                    removeMediaItem( const QUuid& uuid, unsigned int track );

protected:
    virtual void            resizeEvent( QResizeEvent* event );
    virtual void            drawBackground( QPainter* painter, const QRectF& rect );
    virtual void            mouseMoveEvent( QMouseEvent* event );
    virtual void            mousePressEvent( QMouseEvent* event );
    virtual void            mouseReleaseEvent( QMouseEvent* event );
    virtual void            wheelEvent( QWheelEvent* event );
    virtual void            dragEnterEvent( QDragEnterEvent* event );
    virtual void            dragMoveEvent( QDragMoveEvent* event );
    virtual void            dragLeaveEvent( QDragLeaveEvent* event );
    virtual void            dropEvent( QDropEvent* event );

private slots:
    void                    ensureCursorVisible();
    void                    updateDuration();
    void                    split( GraphicsMovieItem* item, qint64 frame );

private:
    void                    createLayout();
    void                    addVideoTrack();
    void                    addAudioTrack();
    void                    moveMediaItem( AbstractGraphicsMediaItem* item, QPoint position );
    void                    moveMediaItem( AbstractGraphicsMediaItem* item, quint32 track, qint64 time );
    GraphicsTrack*          getTrack( MainWorkflow::TrackType type, unsigned int number );
    QGraphicsScene*         m_scene;
    int                     m_tracksHeight;
    unsigned int            m_tracksCount;
    int                     m_projectDuration;
    GraphicsCursorItem*     m_cursorLine;
    QGraphicsLinearLayout*  m_layout;
    quint32                 m_numVideoTrack;
    quint32                 m_numAudioTrack;
    MainWorkflow*           m_mainWorkflow;
    GraphicsMovieItem*      m_dragVideoItem;
    GraphicsAudioItem*      m_dragAudioItem;
    QGraphicsWidget*        m_separator;
    ToolButtons             m_tool;
    WorkflowRenderer*       m_renderer;

    // Mouse actions on Medias
    bool                    m_actionMove;
    bool                    m_actionMoveExecuted;
    bool                    m_actionResize;
    qint64                  m_actionResizeStart;
    qint64                  m_actionResizeBase;
    qint64                  m_actionResizeOldBegin;
    AbstractGraphicsMediaItem::From m_actionResizeType;
    int                     m_actionRelativeX;
    AbstractGraphicsMediaItem* m_actionItem;

signals:
    void                    zoomIn();
    void                    zoomOut();
    void                    durationChanged( int duration );
    void                    videoTrackAdded( GraphicsTrack* );
    void                    audioTrackAdded( GraphicsTrack* );

friend class Timeline;
friend class TracksScene;
};

#endif // TRACKSVIEW_H