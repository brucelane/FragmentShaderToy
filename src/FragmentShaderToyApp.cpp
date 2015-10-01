#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/gl.h"

//#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

/**
FragmentShaderToyApp:

Demonstrates shader loading and uniform passing.
Modeled after shadertoy.com, which is a more fully-fledged
solution to experimenting with fragment shaders.
Shaders that don't use channels should work unmodified, except for
changing gl_FragColor to oColor and adding the uniform declaration.

Loads default.fs by default.
Try dragging ripple.fs or your own fragment shader onto the app window.
Press any key to reload the last shader from disk.
*/

class FragmentShaderToyApp : public App {
public:

	void setup();
	void update();
	void draw();
	void loadShader(const fs::path &fragment_path);
private:
	gl::GlslProgRef	mProg;
	gl::VboMeshRef	mMesh;
	ci::vec4		mMouseCoord;
	fs::path		mCurrentShaderPath;
	ci::gl::TextureRef			texture;
	bool			mDoSave = false;
};


void FragmentShaderToyApp::setup()
{
	// load fragment shader from the provided path
	// we always use the same vertex shader, so it isn't specified
	loadShader(getAssetPath("default.fs"));
	string pathToStartupFile = (getAssetPath("") / "startup.jpg").string();
	if (fs::exists(pathToStartupFile))
	{
		texture = gl::Texture::create(loadImage(loadAsset("startup.jpg")));
	}
	// create a rectangle to be drawn with our shader program
	// default is from -0.5 to 0.5, so we scale by 2 to get -1.0 to 1.0
	mMesh = gl::VboMesh::create(geom::Rect());// .scale(vec2(2.0f, 2.0f)));

	// load a new shader on file drop
	getWindow()->getSignalFileDrop().connect([this](FileDropEvent &event)
	{
		auto file = event.getFile(0);
		if (fs::is_regular_file(file));
		{ loadShader(file); }
	});
	// set iMouse XY on mouse move
	getWindow()->getSignalMouseMove().connect([this](MouseEvent &event)
	{
		mMouseCoord.x = event.getX();
		mMouseCoord.y = getWindowHeight() - event.getY();
	});
	// set iMouse ZW on mouse down
	getWindow()->getSignalMouseDown().connect([this](MouseEvent &event)
	{
		mMouseCoord.z = event.getX();
		mMouseCoord.w = getWindowHeight() - event.getY();
	});
	// reload last fragment shader on keypress, or save frame if key == 's'
	getWindow()->getSignalKeyDown().connect([this](KeyEvent &event)
	{
		if (event.getCode() == KeyEvent::KEY_s)
		{
			mDoSave = true;
		}
		else
		{
			loadShader(mCurrentShaderPath);
		}
	});
	// set shader resolution uniform on resize (if shader has that uniform)
	getWindow()->getSignalResize().connect([this]()
	{
			mProg->uniform("iResolution", vec3(getWindowWidth(), getWindowHeight(), 0.0f));
	});
}

void FragmentShaderToyApp::loadShader(const fs::path &fragment_path)
{
	try
	{	// load and compile our shader
		mProg = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("default.vs"))
			.fragment(loadFile(fragment_path)));
		// no exceptions occurred, so store the shader's path for reloading on keypress
		mCurrentShaderPath = fragment_path;
		mProg->uniform("iResolution", vec3(getWindowWidth(), getWindowHeight(), 0.0f));

	}
	catch (ci::gl::GlslProgCompileExc &exc)
	{
		console() << "Error compiling shader: " << exc.what() << endl;
	}
	catch (ci::Exception &exc)
	{
		console() << "Error loading shader: " << exc.what() << endl;
	}
}

void FragmentShaderToyApp::update()
{
		mProg->uniform("iGlobalTime", static_cast<float>(getElapsedSeconds()));
		mProg->uniform("iMouse", mMouseCoord);
		mProg->uniform("iChannel0", 0);
}

void FragmentShaderToyApp::draw()
{
	// clear out the window with black
	gl::clear(Color(0.2, 0, 0));
	// use our shader for this draw loop
	//gl::GlslProgScope prog(mProg);
	gl::ScopedGlslProg shader(mProg);
	texture->bind(0);
	// draw our screen rectangle
	gl::draw(mMesh);

	// if you want to save a screenshot
	if (mDoSave)
	{
		mDoSave = false;
		try
		{
			writeImage(getSaveFilePath(), copyWindowSurface());
		}
		catch (ci::Exception &exc)
		{
			console() << "Failed to save image: " << exc.what() << endl;
		}
	}
}

CINDER_APP(FragmentShaderToyApp, RendererGl)
