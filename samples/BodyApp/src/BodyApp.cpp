/*
* 
* Copyright (c) 2013, Wieden+Kennedy
* Stephen Schieberl, Michael Latzoni
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"

#include "Kinect2.h"

class BodyApp : public ci::app::AppBasic
{
public:
	void						draw();
	void						prepareSettings( ci::app::AppBasic::Settings* settings );
	void						setup();
	void						update();
private:
	Kinect2::DeviceRef			mDevice;
	Kinect2::Frame				mFrame;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void BodyApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::black() );
	gl::enableAlphaBlending();
	gl::color( ColorAf::white() );

	if ( mFrame.getDepth() ) {
		gl::TextureRef tex = gl::Texture::create( Kinect2::channel16To8( mFrame.getDepth() ) );
		gl::draw( tex, tex->getBounds(), Rectf( getWindowBounds() ) );
	}

	if ( mFrame.getBodyIndex() ) {
		gl::TextureRef tex = gl::Texture::create( Kinect2::colorizeBodyIndex( mFrame.getBodyIndex() ) );
		gl::draw( tex, tex->getBounds(), Rectf( getWindowBounds() ) );
	}

	if ( mFrame.getDepth() && mDevice ) {
		gl::pushMatrices();
		gl::scale( Vec2f( getWindowSize() ) / Vec2f( mFrame.getDepth().getSize() ) );
		for ( const Kinect2::Body& body : mDevice->getFrame().getBodies() ) {
			for ( const auto& joint : body.getJointMap() ) {
				Vec2f pos = Kinect2::mapBodyCoordToDepth( joint.second.getPosition( ), mDevice->getCoordinateMapper() );
				gl::color( ColorAf::white() );
				gl::drawSolidCircle( pos, 7.0f, 32 );
				gl::color( Kinect2::getBodyColor( body.getIndex() ) );
				gl::drawSolidCircle( pos, 5.0f, 32 );
			}
		}
		gl::popMatrices();
	}
	mParams->draw();
}

void BodyApp::prepareSettings( Settings* settings )
{
	settings->prepareWindow( Window::Format().size( 1280, 720 ).title( "Body App" ) );
	settings->setFrameRate( 60.0f );
}

void BodyApp::setup()
{
	gl::enable( GL_TEXTURE_2D );
	
	mFrameRate	= 0.0f;
	mFullScreen	= false;

	mDevice = Kinect2::Device::create();
	mDevice->start( Kinect2::DeviceOptions().enableColor( false ).enableBody().enableBodyIndex() );
	
	console( ) << Kinect2::getDeviceCount() << " device(s) connected." << endl;
	map<size_t, string> deviceMap = Kinect2::getDeviceMap();
	for ( const auto& device : deviceMap ) {
		console( ) << "Index: " << device.first << ", ID: " << device.second << endl;
	}

	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 100 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,			"", true );
	mParams->addParam( "Full screen",	&mFullScreen,			"key=f" );
	mParams->addButton( "Quit", bind(	&BodyApp::quit, this ),	"key=q" );

}

void BodyApp::update()
{
	mFrameRate = getAverageFps();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}

	if ( mDevice && mDevice->getFrame().getTimeStamp() > mFrame.getTimeStamp() ) {
		mFrame = mDevice->getFrame();
	}
}

CINDER_APP_BASIC( BodyApp, RendererGl )
