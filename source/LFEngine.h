#ifndef LFENGINE_H
#define LFENGINE_H

#include <string>
#include <memory>
#include "LFLoader.h"
#include "camera/Camera.h"
#include "OBJRender.h"
#include "UserInterface.h"

class LFEngine
{
public:
	explicit LFEngine(const string &profile);
	LFEngine(const LFEngine &) = delete;
	LFEngine& operator=(const LFEngine&) = delete;
	~LFEngine(void);

	// Given current status, draw one frame
	void Draw(void);
	// Set up an background thread for FPS counting
	void StartFPSThread(void);
	// Change view port size
	void Resize(GLuint width, GLuint height);
	// 
	void SetUI(int type, double sx, double sy);
	// Set rendering camera pose to one of the reference cameras 
	// specified by 'id'
	void SetLocationOfReferenceCamera(int id);
	// Zoom in(out). This function simply move rendering camera 
	// along lookat direction. If zoom_scale<1, move along negative
	// lookat direction, otherwise, positive 
	void SetZoomScale(float zoom_scale);
	// Get FPS (frames drawn in previous second)
	inline int GetFPS(void) { return fps; }
	// Set framebuffer to draw to. On Windows/Android, 
	// the default FBO is 0. On iOS, the default FBO is undefined.
	void SetDefaultFBO(GLuint fbo = 0);
	// Get framebuffer name
	inline GLuint GetDefaultFBO(void) const { return default_fbo; }
	// Extract a subregion of current frame buffer. 
	// x and y specifies the lower left corner of this region, 
	// width and height specifies the dimension of this region
	bool GetScreenShot(unsigned char *buffer, int x, int y, int width, int height);

private:
	// Initialization rendering engine given profile
	void InitEngine(const string &profile);
	// Background thread for FPS counting
	void VRFPS(void);
		
	// Used for loading light field data(images, parameters...)
	unique_ptr<LFLoader> gLFLoader;	
	// Used for rendering
	unique_ptr<OBJRender> gOBJRender;	
	// Rendering camera object
	Camera gRenderCamera;

	int fps;    // frames per second
	int frames; // frames drawn

	UserInterface *ui;   // Used for user interaction
	GLuint default_fbo;  // default frame buffer
};


#endif // LFENGINE_H
