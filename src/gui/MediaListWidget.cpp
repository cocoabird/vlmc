/*****************************************************************************
 * MediaListWidget.cpp: Multimedia list widget
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
 *
 * Authors: Christophe Courtaut <christophe.courtaut@gmail.com>
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

#include <QSvgRenderer>
#include <QPainter>
#include "MediaListWidget.h"

MediaListWidget::MediaListWidget( QWidget* parent ) : QListWidget( parent )
{
    m_svgRenderer = new QSvgRenderer( this );
}

void    MediaListWidget::setType( ListViewMediaItem::fType fileType )
{
    this->m_Type = fileType;
    switch ( fileType )
    {
    case ListViewMediaItem::Audio:
        m_svgRenderer->load( QString( ":/images/scalable/audio-scalable" ) );
        break;
    case ListViewMediaItem::Image:
        m_svgRenderer->load( QString( ":/images/scalable/image-scalable" ) );
        break;
    case ListViewMediaItem::Video:
        m_svgRenderer->load( QString( ":/images/scalable/video-scalable" ) );
        break;
    }
}

void    MediaListWidget::mousePressEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton)
        this->m_dragStartPos = event->pos();
    QListWidget::mousePressEvent( event );
}

void    MediaListWidget::mouseMoveEvent( QMouseEvent* event )
{
    if ( this->currentItem() == NULL )
        return;
    if ( !( event->buttons() & Qt::LeftButton ) )
         return;
    if ( ( event->pos() - m_dragStartPos ).manhattanLength()
          < QApplication::startDragDistance() )
        return;

    QMimeData* mimeData = new QMimeData;
    mimeData->setText( ( ( ListViewMediaItem* )( currentItem() ) )->fileInfo()->absoluteFilePath() );
    QDrag* drag = new QDrag( this );
    drag->setMimeData( mimeData );
    drag->exec( Qt::CopyAction | Qt::MoveAction, Qt::CopyAction );
}

void MediaListWidget::paintEvent( QPaintEvent* event )
{
    QPainter painter( viewport() );

    painter.setOpacity( 0.2 );
    if ( m_svgRenderer->isValid() )
    {
        QRect destRect = viewport()->rect();
        QRect viewportRect = viewport()->rect();
        QSize svgRect = m_svgRenderer->defaultSize();
        qreal wRatio = (qreal)viewportRect.width() / (qreal)svgRect.width();
        qreal hRatio = (qreal)viewportRect.height() / (qreal)svgRect.height();
        if ( wRatio < hRatio )
        {
            destRect.setWidth( viewportRect.width() );
            destRect.setHeight( svgRect.height() * wRatio );
        }
        else
        {
            destRect.setWidth( svgRect.width() * hRatio );
            destRect.setHeight( viewportRect.height() );
        }

        int x = ( viewportRect.width() / 2 ) - ( destRect.width() / 2 );
        int y = ( viewportRect.height() / 2 ) - ( destRect.height() / 2 );
        destRect.setCoords( x, y, x + destRect.width(), y + destRect.height() );
        m_svgRenderer->render( &painter, destRect );
    }
    QListWidget::paintEvent( event );
}