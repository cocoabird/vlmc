/*****************************************************************************
 * TrackWorkflow.cpp : Will query the Clip workflow for each successive clip in the track
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
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

#include <QtDebug>

#include "TrackWorkflow.h"

TrackWorkflow::TrackWorkflow( unsigned int trackId ) :
        m_trackId( trackId ),
        m_length( 0 )
{
    m_mediaPlayer = new LibVLCpp::MediaPlayer();
}

TrackWorkflow::~TrackWorkflow()
{
    delete m_mediaPlayer;
}

void    TrackWorkflow::addClip( Clip* clip, qint64 start )
{
    ClipWorkflow* cw = new ClipWorkflow( clip );
    m_clips.insert( start, cw );
    computeLength();
}

void                TrackWorkflow::computeLength()
{
    if ( m_clips.count() == 0 )
        m_length = 0;
    QMap<qint64, ClipWorkflow*>::const_iterator it = m_clips.end() - 1;
    m_length = (it.key() + it.value()->getClip()->getLength() );
}

qint64              TrackWorkflow::getLength() const
{
    return m_length;
}

unsigned char*      TrackWorkflow::renderClip( ClipWorkflow* cw, qint64 currentFrame,
                                        qint64 start , bool needRepositioning )
{
    unsigned char*      ret = NULL;

    cw->getStateLock()->lockForRead();

    if ( cw->getState() == ClipWorkflow::Rendering )
    {
        //The rendering state meens... whell it means that the frame is
        //beeing rendered, so we wait.
        cw->getStateLock()->unlock();
        while ( cw->isRendering() == true )
        {
            usleep( 100 );
        }
        //This way we can trigger the appropriate if just below.
        //by restoring the initial state of the function, and just pretend that
        //nothing happened.
        cw->getStateLock()->lockForRead();
    }

    //If frame has been rendered :
    if ( cw->getState() == ClipWorkflow::Sleeping )
    {
        cw->getStateLock()->unlock();
        if ( needRepositioning == true )
        {
            float   pos = ( (float)( currentFrame - start ) / (float)(cw->getClip()->getLength()) );
            cw->setPosition( pos );
        }
        ret = cw->getOutput();
        cw->wake();
        //FIXME: sometimes, the renderer isn't awake soon enough, and we can
        //pass though this function many times before the frame is actually rendered.
    }
    else if ( cw->getState() == ClipWorkflow::Stopped )
    {
        cw->getStateLock()->unlock();
        cw->initialize( m_mediaPlayer );
        cw->startRender();
        if ( start != currentFrame ) //Clip was not started as its real begining
        {
            float   pos = ( (float)( currentFrame - start ) / (float)(cw->getClip()->getLength()) );
            cw->setPosition( pos );
        }
    }
    else if ( cw->getState() == ClipWorkflow::Ready ||
              cw->getState() == ClipWorkflow::Initializing )
    {
        //If the state is Initializing, then the workflow will wait.
        //Otherwise, it will start directly.
        cw->getStateLock()->unlock();
        cw->startRender();
    }
    else if ( cw->getState() == ClipWorkflow::EndReached )
    {
        cw->getStateLock()->unlock();
        //The stopClipWorkflow() method will take care of that.
    }
    else
    {
//        qDebug() << "Unexpected ClipWorkflow::State when rendering:" << cw->getState();
        cw->getStateLock()->unlock();
    }
    return ret;
}

void                TrackWorkflow::preloadClip( ClipWorkflow* cw )
{
    cw->getStateLock()->lockForRead();

    if ( cw->getState() == ClipWorkflow::Stopped )
    {
        cw->getStateLock()->unlock();
        cw->initialize( m_mediaPlayer );
        return ;
    }
    cw->getStateLock()->unlock();
}

void                TrackWorkflow::stopClipWorkflow( ClipWorkflow* cw )
{
    cw->getStateLock()->lockForRead();

    if ( cw->getState() == ClipWorkflow::Stopped )
    {
        cw->getStateLock()->unlock();
        return ;
    }
    if ( cw->getState() == ClipWorkflow::Sleeping ||
         cw->getState() == ClipWorkflow::Ready ||
         cw->getState() == ClipWorkflow::EndReached )
    {
        cw->getStateLock()->unlock();
        cw->queryStateChange( ClipWorkflow::Stopping );
        cw->wake();
        cw->stop();
    }
    else if ( cw->getState() == ClipWorkflow::Rendering )
    {
        cw->getStateLock()->unlock();
        while ( cw->isRendering() == true )
            usleep( 100 );
        cw->queryStateChange( ClipWorkflow::Stopping );
        cw->wake();
        cw->stop();
    }
    else if ( cw->getState() == ClipWorkflow::Initializing )
    {
        cw->getStateLock()->unlock();
        while ( cw->isReady() == false )
            usleep( 20 );
        cw->stop();
    }
    else
    {
//        qDebug() << "Unexpected ClipWorkflow::State when stopping :" << cw->getState();
        cw->getStateLock()->unlock();
    }
}

bool                TrackWorkflow::checkEnd( qint64 currentFrame ) const
{
    if ( m_clips.size() == 0 )
        return true;
    //This is the last video by chronological order :
    QMap<qint64, ClipWorkflow*>::const_iterator   it = m_clips.end() - 1;
    //If it ends before the current frame, we reached end.
    return ( it.value()->getClip()->getLength() + it.key() < currentFrame );
}

void                    TrackWorkflow::stop()
{
    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();

    while ( it != end )
    {
        stopClipWorkflow( it.value() );
        ++it;
    }
}

unsigned char*      TrackWorkflow::getOutput( qint64 currentFrame )
{
    unsigned char*  ret = NULL;
    QMap<qint64, ClipWorkflow*>::iterator       it = m_clips.begin();
    QMap<qint64, ClipWorkflow*>::iterator       end = m_clips.end();
    static  qint64                              lastFrame = 0;
    bool                                        needRepositioning;

    if ( checkEnd( currentFrame ) == true )
    {
        emit trackEndReached( m_trackId );
        //We continue, as there can be ClipWorkflow that required to be stopped.
    }
    needRepositioning = ( abs( currentFrame - lastFrame ) > 1 ) ? true : false;
    while ( it != end )
    {
        qint64          start = it.key();
        ClipWorkflow*   cw = it.value();

        //Is the clip supposed to render now ?
        if ( start <= currentFrame && currentFrame <= start + cw->getClip()->getLength() )
        {
            ret = renderClip( cw, currentFrame, start, needRepositioning );
            lastFrame = currentFrame;
        }
        //Is it about to be rendered ?
        else if ( start > currentFrame &&
                start - currentFrame < TrackWorkflow::nbFrameBeforePreload )
        {
            preloadClip( cw );
        }
        //Is it supposed to be stopped ?
        else
            stopClipWorkflow( cw );

        ++it;
    }
    return ret;
}