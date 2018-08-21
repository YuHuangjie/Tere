#ifndef LFENGINE_H
#define LFENGINE_H

#include <string>
#include <memory>
#include "LFLoader.h"
#include "camera/Camera.hpp"
#include "OBJRender.h"
#include "UserInterface.h"

enum UIType
{
	MOVE = 0,
	TOUCH = 1,
	LEAVE = 2,
};

class LFEngine
{
public:
	typedef vector<WeightedCamera>(*InterpStrgFunc)(const vector<Camera>&ref,
		const Camera &vir, const glm::vec3 &p, const size_t maxn);

	explicit LFEngine(const string &profile);
	LFEngine(const LFEngine &) = delete;
	LFEngine& operator=(const LFEngine&) = delete;
	~LFEngine(void);

	// Draw one frame
	void Draw(void);

	// Set up an background thread for counting fps
	void StartFPSThread(void);

	// Change view port size
	void Resize(GLuint width, GLuint height);

	// 
	void SetUI(UIType type, double sx, double sy);

	// Set rendering camera to one of the reference cameras 
	// specified by 'id'
	void SetLocationOfReferenceCamera(int id);

	// Zoom in(out). This function simply move rendering camera 
	// along lookat direction. If zoom_scale<1, move along negative
	// lookat direction, otherwise, positive 
	void SetZoomScale(float zoom_scale);

	// Get FPS (frames drawn in previous second)
	inline int GetFPS(void) { return fps; }
    
	// Extract a subregion of current frame buffer. 
	// x and y specifies the lower left corner of this region, 
	// width and height specifies the dimension of this region
	bool GetScreenShot(unsigned char *buffer, int x, int y, int width, int height);

private:
	enum Mode
	{
		FIX,		// Fix virtual camera as ref camera and disable interpolation
		INTERP		// Interpolation strategy
	};
	Mode _mode;			// select rendering mode
	size_t _fixRef;	// in FIX mode, select which ref camera to imitate

	// Initialization rendering engine given profile
	void InitEngine(const string &profile);

	// Background thread for FPS counting
	void VRFPS(void);
    
    vector<int> viewport;   // viewport size
		
	// Used for loading light field data(images, parameters...)
	unique_ptr<LFLoader> gLFLoader;	

	// Used for rendering
	unique_ptr<OBJRender> gOBJRender;	

	// Rendering camera object
	Camera gRenderCamera;

	int fps;    // frames per second
	int frames; // frames drawn

	UserInterface *ui;   // Used for user interaction

	InterpStrgFunc _interpStrgFunc;
};


#endif // LFENGINE_H
