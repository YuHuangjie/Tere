#ifndef LFENGINEIMPL_H
#define LFENGINEIMPL_H

#include <string>
#include <memory>
#include <queue>
#include <array>

#include "camera/Camera.h"
#include "Type.h"
#include "Strategy.h"

class Renderer;
class TextureFuser;
class Poster;
class UserInterface;
struct TereScene;

using std::vector;
using std::shared_ptr;
using std::unique_ptr;
using std::array;
using std::string;

class LFEngineImpl
{
public:
	explicit LFEngineImpl(const size_t nCams, const RENDER_MODE mode);
	~LFEngineImpl(void);

	/*****************************************************************************
	 *			Scene Data
	 ****************************************************************************/
	// Set unindexed geometry (9 floats form a face)
	bool SetGeometry(const float *v, const size_t szV, bool GPU = false);

	// Set indexed geometry
	bool SetGeometry(const float *v, const size_t szV, const int *f,
		const size_t szF, bool GPU = false);

	// Set image raw data directly
	bool SetRefImage(const size_t id, const uint8_t *rgb, const size_t w,
		const size_t h);

	// Set image file name (in this case, image decoding function must be 
	// registered)
	bool SetRefImage(const size_t id, const string &filename,
		const float zoom = 1.f);

	// Set camera parameters
	bool SetCamera(const size_t id, const array<float, 9> &K, 
		const array<float, 16> &M, bool w2c, bool yIsUp);

	// Register image decoding function
	void RegisterDecFunc(const DecHeaderFunc, const DecImageFunc);

	/*****************************************************************************
	 *			Scene Setting
	 ****************************************************************************/
	// ONLY in 'linear' mode. Set how many rows of cameras are there in the scene.
	void SetRows(const size_t rows);

	// Inform Tere that data is initialized
	bool HaveSetScene();

	// Inform Tere that data is updated
	bool HaveUpdatedScene();

	/*****************************************************************************
	 *			Others
	 ****************************************************************************/
	// TODO: Other setting funcs in the case of LINEAR and SPHERE mode.

	// Draw one frame
	void Draw(void);

	// Set up an background thread for counting fps
	void StartFPSThread(void);

	// Set view port size
	void Resize(uint32_t width, uint32_t height);

	// Allow client to interact with the scene
	void SetUI(UIType type, float sx, float sy);

	// Set rendering camera to one of the reference cameras 
	// specified by 'id'
	void SetLocationOfReferenceCamera(int id);

	// Zoom in(out). This function changes the FoV of rendering camera.
	// zoom_scale is restricted to be between [0.1, 10]. 
	// Return the actual zooming scale.
	float SetZoomScale(float zoom_scale);

	// Get FPS (frames drawn in previous second)
	float GetFPS(void) const { return _fps; }

	// Extract a subregion of current frame buffer. 
	// x and y specifies the lower left corner of this region, 
	// width and height specifies the dimension of this region
	bool GetScreenShot(unsigned char *buffer, int x, int y, int width, int height) const;

	// set background color
	bool SetBackground(float r, float g, float b);
	// set background image
	bool SetBackground(const string &imagePath);
	bool SetBackground(const uint8_t *bg, const int width, const int height);

	// set screen framebuffer (default is 0)
	void SetScreenFBO(unsigned int fbo);

private:
	void _Draw(void);

	// Background thread for FPS counting
	void VRFPS(void);
	
	// A slot is an internal rendering camera which the client cannot control.
	// The purpose of slots is to acheive smooth rendering effects during the 
	// perioud when rendering camera is moving to the closest reference camera.
	void EnqueueSlots(const Extrinsic &start, const Extrinsic &end);

private:
	enum InterpMode
	{
		// Set rendering camera to any ref cameras and disable interpolation
		FIX,
		// Use interpolation
		INTERP
	} _mode;

	shared_ptr<TereScene> _scene;		// describe the scene setting and data
	
	DecHeaderFunc fdh;						// image header decoding function
	DecImageFunc fdi;						// image decoding function

	unique_ptr<Renderer> _renderer;			// TERE scene renderer
	unique_ptr<TextureFuser> _textureFuser;	// background fuser
	unique_ptr<Poster> _poster;				// render to screen

	Camera _renderCam;						// rendering camera

	float _stdFx, _stdFy;					// focal length on initialization

	unique_ptr<UserInterface> _UI;			// user interaction

	size_t _fixRef;			// in FIX mode, select which ref camera to render

	vector<int> _screenViewport;			// poster's viewport
	vector<int> _offlineViewport;			// scene and fuser' viewport

	float _fps;								// rendering fps
	int _frames;							// frames drawn

	// strategy for searching interpolation cameras
	shared_ptr<SearchStrategy> _schStrg;
	// strategy for weighing interpolation cameras
	shared_ptr<WeighStrategy> _wghStrg;

	bool _locked;							// Tere is rendering internal slots
	float _SLOT_MULTIPLIER;					// slot multiplier
	std::deque<Extrinsic> _slotQueue;		// slots queue
};

#endif /* LFENGINEIMPL_H */
