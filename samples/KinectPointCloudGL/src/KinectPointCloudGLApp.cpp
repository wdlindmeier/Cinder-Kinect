#include "cinder/app/AppBasic.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "cinder/gl/Batch.h"
#include "cinder/ImageIo.h"
#include "CinderFreenect.h"
#include "Resources.h"

static const int VBO_X_RES  = 640;
static const int VBO_Y_RES  = 480;

using namespace ci;
using namespace ci::app;
using namespace std;

struct Particle
{
    vec3		pos;
    vec2		texCoord;
};

class PointCloudGl : public AppBasic {
  public:
	void prepareSettings( Settings* settings );
	void setup();
	void createBatch();
	void update();
	void draw();
	
	// PARAMS
	params::InterfaceGlRef	mParams;
    float           mPointSize;
	
	// CAMERA
	CameraPersp		mCam;
	quat			mSceneRotation;
	vec3			mEye, mCenter, mUp;
	float			mCameraDistance;
	float			mKinectTilt;
	
	// KINECT AND TEXTURES
	KinectRef		mKinect;
	gl::TextureRef	mDepthTexture;
	float			mScale;
	float			mXOff, mYOff;
	
	// BATCH AND SHADER
	gl::GlslProgRef	mShader;
    gl::BatchRef	mParticleBatch;

};

void PointCloudGl::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 720 );
}

void PointCloudGl::setup()
{	
	// SETUP PARAMS
	mParams = params::InterfaceGl::create( "KinectPointCloud", ivec2( 200, 180 ) );
	mParams->addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams->addParam( "Cam Distance", &mCameraDistance, "min=100.0 max=5000.0 step=100.0 keyIncr=s keyDecr=w" );
	mParams->addParam( "Kinect Tilt", &mKinectTilt, "min=-31 max=31 keyIncr=T keyDecr=t" );
    mParams->addParam( "Point Size", &mPointSize, "min=1.0 max=20.0 keyIncr=P keyDecr=p step=0.5" );
	
    mPointSize = 1.f;
    
	// SETUP CAMERA
	mCameraDistance = 1000.0f;
	mEye			= vec3( 0.0f, 0.0f, mCameraDistance );
	mCenter			= vec3(0);
	mUp				= vec3(0,1,0);
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 1.0f, 8000.0f );
	
	// SETUP KINECT AND TEXTURES
	mKinect			= Kinect::create(); // use the default Kinect
    mDepthTexture	= gl::Texture::create( 640, 480 );
	
	// SETUP BATCH AND SHADER
    mShader	= gl::GlslProg::create( loadAsset( "mainVert.glsl" ), loadAsset( "mainFrag.glsl" ) );
    createBatch();

	// SETUP GL
	gl::enableDepthWrite();
	gl::enableDepthRead();
    gl::enable( GL_VERTEX_PROGRAM_POINT_SIZE );
}

void PointCloudGl::createBatch()
{
    // Create initial particle layout.
    vector<Particle> particles;
    for( int x = 0; x < VBO_X_RES; ++x )
    {
        for( int y = 0; y < VBO_Y_RES; ++y )
        {
			float xPer	= x / (float)(VBO_X_RES-1);
			float yPer	= y / (float)(VBO_Y_RES-1);
            Particle p;
			p.pos = vec3( ( xPer * 2.0f - 1.0f ) * VBO_X_RES, ( yPer * 2.0f - 1.0f ) * VBO_Y_RES, 0.0f );
			p.texCoord = vec2( xPer, 1.0-yPer );
            particles.push_back(p);
        }
    }
    
    // Create particle buffer on GPU and copy over data.
    // Mark as streaming, since we will copy new data every frame.
    gl::VboRef particleVbo = gl::Vbo::create( GL_ARRAY_BUFFER, particles, GL_STREAM_DRAW );
    
    // Describe particle semantics for GPU.
    geom::BufferLayout particleLayout;
    particleLayout.append( geom::Attrib::POSITION, 3, sizeof( Particle ), offsetof( Particle, pos ) );
    particleLayout.append( geom::Attrib::TEX_COORD_0, 2, sizeof( Particle ), offsetof( Particle, texCoord ) );
    
    assert( mShader );
    
    // Create mesh by pairing our particle layout with our particle Vbo.
    // A VboMesh is an array of layout + vbo pairs
    auto mesh = gl::VboMesh::create( particles.size(), GL_POINTS, { { particleLayout, particleVbo } } );
    mParticleBatch = gl::Batch::create( mesh, mShader );
}

void PointCloudGl::update()
{
	if( mKinect->checkNewDepthFrame() )
        mDepthTexture = gl::Texture::create( mKinect->getDepthImage() );
	
	if( mKinectTilt != mKinect->getTilt() )
		mKinect->setTilt( mKinectTilt );
		
	mEye = vec3( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
}

void PointCloudGl::draw()
{
	gl::clear( Color( 0.0f, 0.0f, 0.0f ), true );
	
    {
        gl::ScopedMatrices matRotation;
		gl::scale( vec3( -1.0f, -1.0f, 1.0f ) );
		gl::rotate( mSceneRotation );
		mDepthTexture->bind( 0 );
        mShader->uniform( "uDepthTex", 0 );
        mShader->uniform( "uPointSize", mPointSize );
        mParticleBatch->draw();
    }

    mParams->draw();
}


CINDER_APP_BASIC( PointCloudGl, RendererGl )
