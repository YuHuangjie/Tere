#ifndef LFENGINE_H
#define LFENGINE_H

#include <string>
#include <memory>
#include <cstdint>
#include <array>

#include "Type.h"

#define EXPORT __declspec(dllexport)

using std::string;
using std::unique_ptr;
using std::array;

class LFEngineImpl;

class LFEngine
{
public:
	EXPORT explicit LFEngine(const size_t nCams, const RENDER_MODE mode);
	EXPORT ~LFEngine(void);

	/*****************************************************************************
	 *			Scene Data
	 ****************************************************************************/
	// Set unindexed geometry (9 floats form a face)
	EXPORT bool SetGeometry(const float *v, const size_t szV, bool GPU = false);

	// Set indexed geometry
	EXPORT bool SetGeometry(const float *v, const size_t szV, const int *f,
		const size_t szF, bool GPU = false);

	// Set image raw data directly
	EXPORT bool SetRefImage(const size_t id, const uint8_t *rgb, const size_t w,
		const size_t h);

	// Set image file name (in this case, image decoding function must be 
	// registered)
	EXPORT bool SetRefImage(const size_t id, const string &filename,
		const float zoom = 1.f);

	// Set camera parameters
	EXPORT bool SetCamera(const size_t id, const array<float, 9> &K,
		const array<float, 16> &M, bool w2c, bool yIsUp);

	// Register image decoding function
	EXPORT void RegisterDecFunc(const DecHeaderFunc, const DecImageFunc);

	/*****************************************************************************
	 *			Scene Setting
	 ****************************************************************************/
	// ONLY in 'linear' mode. Set how many rows of cameras are there in the scene.
	EXPORT void SetRows(const size_t rows);

	// Inform Tere that data is initialized
	EXPORT bool HaveSetScene();

	// Inform Tere that data is updated
	EXPORT bool HaveUpdatedScene();

	/*****************************************************************************
	 *			Others
	 ****************************************************************************/
	// TODO: Other setting funcs in the case of LINEAR and SPHERE mode.

	// Draw one frame
	EXPORT void Draw(void);

	// Set up an background thread for counting fps
	EXPORT void StartFPSThread(void);

	// Set view port size
	EXPORT void Resize(uint32_t width, uint32_t height);

	// Allow client to interact with the scene
	EXPORT void SetUI(UIType type, float sx, float sy);

	// Set rendering camera to one of the reference cameras 
	// specified by 'id'
	EXPORT void SetLocationOfReferenceCamera(int id);

	// Zoom in(out). This function changes the FoV of rendering camera.
	// zoom_scale is restricted to be between [0.1, 10]. 
    // Return the actual zooming scale.
	EXPORT float SetZoomScale(float zoom_scale);

	// Get FPS (frames drawn in previous second)
	EXPORT float GetFPS(void) const;
    
	// Extract a subregion of current frame buffer. 
	// x and y specifies the lower left corner of this region, 
	// width and height specifies the dimension of this region
	EXPORT bool GetScreenShot(unsigned char *buffer, int x, int y, int width, int height) const;

	// set background color
	EXPORT bool SetBackground(float r, float g, float b);
	// set background image
	EXPORT bool SetBackground(const string &imagePath);
	EXPORT bool SetBackground(const uint8_t *bg, const int width, const int height);
    
    // set screen framebuffer (default is 0)
	EXPORT void SetScreenFBO(unsigned int fbo);

private:
	unique_ptr<LFEngineImpl> _pImpl;
};


#endif // LFENGINE_H
