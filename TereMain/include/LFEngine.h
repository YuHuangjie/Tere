#ifndef LFENGINE_H
#define LFENGINE_H

#include <string>
#include <memory>
#include <cstdint>
#include <array>

#include "Type.h"

using std::string;
using std::unique_ptr;
using std::array;

class LFEngineImpl;

class __declspec(dllexport) LFEngine
{
public:
	explicit LFEngine(const size_t nCams, const RENDER_MODE mode);
	~LFEngine(void);

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
	float GetFPS(void) const;
    
	// Extract a subregion of current frame buffer. 
	// x and y specifies the lower left corner of this region, 
	// width and height specifies the dimension of this region
	bool GetScreenShot(unsigned char *buffer, int x, int y, int width, int height) const;

	// set background color
	bool SetBackground(float r, float g, float b);
	// set background image
	bool SetBackground(const string &imagePath);
    
    // set screen framebuffer (default is 0)
    void SetScreenFBO(unsigned int fbo);

private:
	unique_ptr<LFEngineImpl> _pImpl;
};


#endif // LFENGINE_H
