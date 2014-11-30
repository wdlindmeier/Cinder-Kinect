#include "cinder/app/AppBasic.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "CinderFreenect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class _TBOX_PREFIX_App : public AppBasic {
  public:
	void prepareSettings( Settings* settings );
	void setup();
	void update();
	void draw();
	
	KinectRef		mKinect;
	gl::TextureRef	mColorTexture, mDepthTexture;
};

void _TBOX_PREFIX_App::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 480 );
}

void _TBOX_PREFIX_App::setup()
{
	mKinect = Kinect::create();
}

void _TBOX_PREFIX_App::update()
{	
	if( mKinect->checkNewDepthFrame() )
		mDepthTexture = gl::Texture::create(mKinect->getDepthImage());
	
	if( mKinect->checkNewVideoFrame() )
		mColorTexture = gl::Texture::create(mKinect->getVideoImage());
}

void _TBOX_PREFIX_App::draw()
{
	gl::clear(); 
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	if( mDepthTexture )
		gl::draw( mDepthTexture );
	if( mColorTexture )
		gl::draw( mColorTexture, ivec2( 640, 0 ) );
}

CINDER_APP_BASIC( _TBOX_PREFIX_App, RendererGl )
