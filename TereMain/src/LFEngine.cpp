#include "LFEngine.h"
#include "LFEngineImpl.h"


LFEngine::LFEngine(const size_t nCams, const RENDER_MODE mode)
	: _pImpl(new LFEngineImpl(nCams, mode))
{
}

LFEngine::~LFEngine(void)
{
}

bool LFEngine::SetGeometry(const float *v, const size_t szV, bool GPU)
{
	return _pImpl->SetGeometry(v, szV, GPU);
}

bool LFEngine::SetGeometry(const float *v, const size_t szV, const int *f,
	const size_t szF, bool GPU)
{
	return _pImpl->SetGeometry(v, szV, f, szF, GPU);
}

bool LFEngine::SetRefImage(const size_t id, const uint8_t *rgb, const size_t w,
	const size_t h)
{
	return _pImpl->SetRefImage(id, rgb, w, h);
}

bool LFEngine::SetRefImage(const size_t id, const string &filename, 
	const float zoom)
{
	return _pImpl->SetRefImage(id, filename, zoom);
}

void LFEngine::RegisterDecFunc(const DecHeaderFunc hf, const DecImageFunc f)
{
	return _pImpl->RegisterDecFunc(hf, f);
}

bool LFEngine::SetCamera(const size_t id, const array<float, 9> &K, 
	const array<float, 16> &M, bool w2c, bool yIsUp)
{
	return _pImpl->SetCamera(id, K, M, w2c, yIsUp);
}

void LFEngine::SetRows(const size_t rows)
{
	return _pImpl->SetRows(rows);
}

bool LFEngine::HaveSetScene()
{
	return _pImpl->HaveSetScene();
}

bool LFEngine::HaveUpdatedScene()
{
	return _pImpl->HaveUpdatedScene();
}

void LFEngine::Draw()
{
	_pImpl->Draw();
}

void LFEngine::StartFPSThread(void)
{
	_pImpl->StartFPSThread();
}

void LFEngine::Resize(uint32_t width, uint32_t height)
{
	_pImpl->Resize(width, height);
}

void LFEngine::SetUI(UIType type, float sx, float sy)
{
	_pImpl->SetUI(type, sx, sy);
}

void LFEngine::SetLocationOfReferenceCamera(int id)
{
	_pImpl->SetLocationOfReferenceCamera(id);
}

float LFEngine::SetZoomScale(float zoom_scale)
{
	return _pImpl->SetZoomScale(zoom_scale);
}

bool LFEngine::GetScreenShot(unsigned char *buffer, int x, int y, int width, int height) const
{
	return _pImpl->GetScreenShot(buffer, x, y, width, height);
}

float LFEngine::GetFPS() const
{
	return _pImpl->GetFPS();
}

bool LFEngine::SetBackground(float r, float g, float b)
{
	return _pImpl->SetBackground(r, g, b);
}

bool LFEngine::SetBackground(const string &imagePath)
{
	return _pImpl->SetBackground(imagePath);
}

bool LFEngine::SetBackground(const uint8_t *bg, const int width, const int height)
{
	return _pImpl->SetBackground(bg, width, height);
}

void LFEngine::SetScreenFBO(unsigned int fbo)
{
    _pImpl->SetScreenFBO(fbo);
}
