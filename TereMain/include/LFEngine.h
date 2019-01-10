#ifndef LFENGINE_H
#define LFENGINE_H

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include "glm/fwd.hpp"

class LFEngineImpl;
class WeightedCamera;
class Camera;
enum UIType : unsigned int;

using std::vector;
using std::string;
using std::unique_ptr;


class LFEngine
{
public:
	explicit LFEngine(const string &profile);
	LFEngine(const LFEngine &) = delete;
	LFEngine& operator=(const LFEngine&) = delete;
	~LFEngine(void);

	// Draw one frame
	void Draw(void);

	// Set up an background thread for counting fps
	void StartFPSThread(void);

	// Change view port size
	void Resize(uint32_t width, uint32_t height);

	// 
	void SetUI(UIType type, double sx, double sy);

	// Set rendering camera to one of the reference cameras 
	// specified by 'id'
	void SetLocationOfReferenceCamera(int id);

	// Zoom in(out). This function changes the FoV of rendering camera.
	// zoom_scale is restricted to be between [0.1, 10]. 
    // Return the actual zooming scale.
	float SetZoomScale(float zoom_scale);

	// Get FPS (frames drawn in previous second)
	int GetFPS(void) const;
    
	// Extract a subregion of current frame buffer. 
	// x and y specifies the lower left corner of this region, 
	// width and height specifies the dimension of this region
	bool GetScreenShot(unsigned char *buffer, int x, int y, int width, int height);

	// set background color
	bool SetBackground(float r, float g, float b);
	// set background image
	bool SetBackground(const string &imagePath);
    
    // set screen framebuffer
    void SetScreenFBO(unsigned int fbo);

private:
	unique_ptr<LFEngineImpl> _pImpl;
};


#endif // LFENGINE_H
