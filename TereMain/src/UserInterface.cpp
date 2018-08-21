#include "UserInterface.h"

UserInterface::UserInterface()
	: screen_width(0),
	screen_height(0)
{}

void UserInterface::SetResolution(const int width, const int height)
{
	screen_width = width;
	screen_height = height;
}
