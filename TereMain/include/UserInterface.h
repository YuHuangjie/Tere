//
//  UserInterface.hpp
//  GLEssentials
//
//  Created by apchen on 2017/6/11.
//  Copyright © 2017年 Dan Omachi. All rights reserved.
//

#ifndef UserInterface_h
#define UserInterface_h

#include "glm/glm.hpp"
//#include "RenderCamView.h"
#include "camera/Camera.hpp"


class UserInterface
{
public:
    UserInterface(int sw, int sh, glm::vec3 last_up, glm::vec3 look_center);
    void SetResolution(int width, int height);
    void FingerDown(bool finger_down, double x, double y);
	//void FingerMove(double x, double y, RenderCamView &view);
	void FingerMove(double x, double y, Camera &view);
    
private:
	//glm::vec3 GetArcballVector(double x, double y, RenderCamView view);
	glm::vec3 GetArcballVector(double x, double y, Camera view);
    
    glm::vec3 look_center;
    glm::vec3 last_up;
    
    // screen size
    int screen_width, screen_height;
    
    // UI
    bool arcball_on = false;
    double last_mx, last_my;
    double cur_mx, cur_my;
};

#endif /* UserInterface_hpp */
