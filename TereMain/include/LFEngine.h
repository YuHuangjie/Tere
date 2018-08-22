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
	void Resize(uint32_t width, uint32_t height);

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
	int GetFPS(void) const;
    
	// Extract a subregion of current frame buffer. 
	// x and y specifies the lower left corner of this region, 
	// width and height specifies the dimension of this region
	bool GetScreenShot(unsigned char *buffer, int x, int y, int width, int height);

private:
	unique_ptr<LFEngineImpl> _pImpl;
};


#endif // LFENGINE_H
