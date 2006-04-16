/***************************************************************************
                          deepskyobject.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Feb 11 2001
    copyright            : (C) 2001 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kglobal.h>

#include <qfile.h>
#include <qregexp.h>
#include <qpainter.h>
#include <qimage.h>
#include <qstring.h>

#include "deepskyobject.h"
#include "kstarsdata.h"
#include "ksutils.h"
#include "dms.h"
#include "kspopupmenu.h"
#include "Options.h"

DeepSkyObject::DeepSkyObject( DeepSkyObject &o )
	: SkyObject( o ) {
	MajorAxis = o.a();
	MinorAxis = o.b();
	PositionAngle = o.pa();
	UGC = o.ugc();
	PGC = o.pgc();
	setCatalog( o.catalog() );
	Image = o.image();
}

DeepSkyObject::DeepSkyObject( int t, dms r, dms d, float m,
			QString n, QString n2, QString lname, QString cat,
			float a, float b, double pa, int pgc, int ugc )
	: SkyObject( t, r, d, m, n, n2, lname ) {
	MajorAxis = a;
	MinorAxis = b;
	PositionAngle = pa;
	PGC = pgc;
	UGC = ugc;
	setCatalog( cat );
	Image = 0;
}

DeepSkyObject::DeepSkyObject( int t, double r, double d, float m,
			QString n, QString n2, QString lname, QString cat,
			float a, float b, double pa, int pgc, int ugc )
	: SkyObject( t, r, d, m, n, n2, lname ) {
	MajorAxis = a;
	MinorAxis = b;
	PositionAngle = pa;
	PGC = pgc;
	UGC = ugc;
	setCatalog( cat );
	Image = 0;
}

float DeepSkyObject::e( void ) const {
	if ( MajorAxis==0.0 || MinorAxis==0.0 ) return 1.0; //assume circular
	return MinorAxis / MajorAxis;
}

QString DeepSkyObject::catalog() const {
	if ( isCatalogM() ) return QString("M");
	if ( isCatalogNGC() ) return QString("NGC");
	if ( isCatalogIC() ) return QString("IC");
	return QString();
}

void DeepSkyObject::setCatalog( const QString &cat ) {
	if ( cat.toUpper() == "M" ) Catalog = (unsigned char)CAT_MESSIER;
	else if ( cat.toUpper() == "NGC" ) Catalog = (unsigned char)CAT_NGC;
	else if ( cat.toUpper() == "IC"  ) Catalog = (unsigned char)CAT_IC;
	else Catalog = (unsigned char)CAT_UNKNOWN;
}

QImage* DeepSkyObject::readImage( void ) {
	QFile file;
	if ( Image==0 ) { //Image not currently set; try to load it from disk.
		QString fname = name().toLower().replace( QRegExp(" "), QString() ) + ".png";

		if ( KSUtils::openDataFile( file, fname ) ) {
			file.close();
			Image = new QImage( file.fileName(), "PNG" );
		}
	}

	return Image;
}

void DeepSkyObject::drawSymbol( QPainter &psky, float x, float y, double PositionAngle, double zoom, double scale ) {
	// if size is 0.0 set it to 1.0, this are normally stars (type 0 and 1)
	// if we use size 0.0 the star wouldn't be drawn
	float majorAxis = a();
	if ( majorAxis == 0.0 ) {	majorAxis = 1.0; }

	float size = scale * majorAxis * dms::PI * zoom / 10800.0;
	float dx1 = -0.5*size;
	float dx2 =  0.5*size;
	float dy1 = -1.0*e()*size/2.;
	float dy2 = e()*size/2.;
	float x1 = x + dx1;
	float x2 = x + dx2;
	float y1 = y + dy1;
	float y2 = y + dy2;

	float dxa = -size/4.;
	float dxb =  size/4.;
	float dya = -1.0*e()*size/4.;
	float dyb = e()*size/4.;
	float xa = x + dxa;
	float xb = x + dxb;
	float ya = y + dya;
	float yb = y + dyb;

	float psize;

	QBrush tempBrush;
	
	switch ( type() ) {
		case 0:
		case 1: //catalog star
			//Some NGC/IC objects are stars...changed their type to 1 (was double star)
			if (size<2.) size = 2.;
			psky.drawEllipse( QRectF(x1, y1, size/2., size/2.) );
			break;
		case 2: //Planet
			break;
		case 3: //Open cluster
			tempBrush = psky.brush();
			psky.setBrush( psky.pen().color() );
			psize = 2.;
			if ( size > 50. )  psize *= 2.;
			if ( size > 100. ) psize *= 2.;
			psky.drawEllipse( QRectF(xa, y1, psize, psize) ); // draw circle of points
			psky.drawEllipse( QRectF(xb, y1, psize, psize) );
			psky.drawEllipse( QRectF(xa, y2, psize, psize) );
			psky.drawEllipse( QRectF(xb, y2, psize, psize) );
			psky.drawEllipse( QRectF(x1, ya, psize, psize) );
			psky.drawEllipse( QRectF(x1, yb, psize, psize) );
			psky.drawEllipse( QRectF(x2, ya, psize, psize) );
			psky.drawEllipse( QRectF(x2, yb, psize, psize) );
			psky.setBrush( tempBrush );
			break;
		case 4: //Globular Cluster
			if (size<2.) size = 2.;
			psky.save();
			psky.translate( x, y );
			psky.rotate( PositionAngle );  //rotate the coordinate system
			psky.drawEllipse( QRectF(dx1, dy1, size, e()*size) );
			psky.drawLine( QPointF(0., dy1), QPointF(0., dy2) );
			psky.drawLine( QPointF(dx1, 0.), QPointF(dx2, 0.) );
			psky.restore(); //reset coordinate system
			break;
		case 5: //Gaseous Nebula
			if (size <2.) size = 2.;
			psky.save();
			psky.translate( x, y );
			psky.rotate( PositionAngle );  //rotate the coordinate system
			psky.drawLine( QPointF(dx1, dy1), QPointF(dx2, dy1) );
			psky.drawLine( QPointF(dx2, dy1), QPointF(dx2, dy2) );
			psky.drawLine( QPointF(dx2, dy2), QPointF(dx1, dy2) );
			psky.drawLine( QPointF(dx1, dy2), QPointF(dx1, dy1) );
			psky.restore(); //reset coordinate system
			break;
		case 6: //Planetary Nebula
			if (size<2.) size = 2.;
			psky.save();
			psky.translate( x, y );
			psky.rotate( PositionAngle );  //rotate the coordinate system
			psky.drawEllipse( QRectF(dx1, dy1, size, e()*size) );
			psky.drawLine( QPointF(0., dy1), QPointF(0., dy1 - e()*size/2. ) );
			psky.drawLine( QPointF(0., dy2), QPointF(0., dy2 + e()*size/2. ) );
			psky.drawLine( QPointF(dx1, 0.), QPointF(dx1 - size/2., 0.) );
			psky.drawLine( QPointF(dx2, 0.), QPointF(dx2 + size/2., 0.) );
			psky.restore(); //reset coordinate system
			break;
		case 7: //Supernova remnant
			if (size<2) size = 2;
			psky.save();
			psky.translate( x, y );
			psky.rotate( PositionAngle );  //rotate the coordinate system
			psky.drawLine( QPointF(0., dy1), QPointF(dx2, 0.) );
			psky.drawLine( QPointF(dx2, 0.), QPointF(0., dy2) );
			psky.drawLine( QPointF(0., dy2), QPointF(dx1, 0.) );
			psky.drawLine( QPointF(dx1, 0.), QPointF(0., dy1) );
			psky.restore(); //reset coordinate system
			break;
		case 8: //Galaxy
			if ( size <1. && zoom > 20*MINZOOM ) size = 3.; //force ellipse above zoomFactor 20
			if ( size <1. && zoom > 5*MINZOOM ) size = 1.; //force points above zoomFactor 5
			if ( size>2. ) {
				psky.save();
				psky.translate( x, y );
				psky.rotate( PositionAngle );  //rotate the coordinate system
				psky.drawEllipse( QRectF(dx1, dy1, size, e()*size) );
				psky.restore(); //reset coordinate system

			} else if ( size>0. ) {
				psky.drawPoint( QPointF(x, y) );
			}
			break;
	}
}

void DeepSkyObject::drawImage( QPainter &psky, float x, float y, double PositionAngle, double zoom, double scale ) {
	QImage *image = readImage();
	QImage ScaledImage;
	
	if ( image ) {
		float w = a() * scale * dms::PI * zoom/10800.0;

		if ( w < 0.75*psky.window().width() ) {
			float h = w*image->height()/image->width(); //preserve image's aspect ratio
			float dx = 0.5*w;
			float dy = 0.5*h;
			ScaledImage = image->scaled( int(w), int(h) );
			psky.save();
			psky.translate( x, y );
			psky.rotate( PositionAngle );  //rotate the coordinate system
			psky.drawImage( QPointF( -dx, -dy ), ScaledImage );
			psky.restore();
		}
	}
}

double DeepSkyObject::labelOffset( double scale ) {
	//Calculate object size in pixels
	double majorAxis = a();
	if ( majorAxis == 0.0 && type() == 1 ) majorAxis = 1.0; //catalog stars
	double size = majorAxis * scale * dms::PI * Options::zoomFactor()/10800.0;
	return 0.5*size + 4.;
}
