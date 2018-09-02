#include "LFEngine.h"
#include "LFEngineImpl.h"
#include "UIType.h"


LFEngine::LFEngine(const string &profile)
	: _pImpl(new LFEngineImpl(profile))
{
}

LFEngine::~LFEngine(void)
{
}

void LFEngine::Draw(void)
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

void LFEngine::SetUI(UIType type, double sx, double sy)
{
	_pImpl->SetUI(type, sx, sy);
}

void LFEngine::SetLocationOfReferenceCamera(int id)
{
	_pImpl->SetLocationOfReferenceCamera(id);
}

void LFEngine::SetZoomScale(float zoom_scale)
{
	_pImpl->SetZoomScale(zoom_scale);
}

bool LFEngine::GetScreenShot(unsigned char *buffer, int x, int y, int width, int height)
{
	return _pImpl->GetScreenShot(buffer, x, y, width, height);
}

int LFEngine::GetFPS() const
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