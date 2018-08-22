//
//  UserInterface.hpp
//  GLEssentials
//
//  Created by apchen on 2017/6/11.
//  Copyright © 2017年 Dan Omachi. All rights reserved.
//

#ifndef UserInterface_h
#define UserInterface_h

#include "camera/Camera.hpp"
#include <string>

class UserInterface
{
public:
	UserInterface();
	virtual ~UserInterface() {}

	virtual std::string Name() const = 0;
	virtual void SetResolution(const int width, const int height);
    virtual void Touch(const double x, const double y) = 0;
	virtual Camera Leave(const double x, const double y, const Camera &view) = 0;
	virtual Camera Move(const double x, const double y, const Camera &view) = 0;

protected:
	// screen size
	int screen_width, screen_height;
};

#endif /* UserInterface_hpp */
