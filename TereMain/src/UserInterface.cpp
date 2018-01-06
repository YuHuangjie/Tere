#include "UserInterface.h"
#include "glm/gtx/transform.hpp"

UserInterface::UserInterface(int sw, int sh, glm::vec3 last_up, glm::vec3 look_center)
{
    screen_width = sw;
    screen_height = sh;
    this->last_up = last_up;
    this->look_center = look_center;
}

void UserInterface::FingerDown(bool finger_down, double x, double y)
{
    arcball_on = finger_down;
    last_mx = x;
    last_my = y;
}

void UserInterface::FingerMove(double x, double y, Camera &view)
{
    if (!arcball_on) { return; }
    
    cur_mx = x;
    cur_my = y;
    
    if (cur_mx == last_mx && cur_my == last_my) { return; }
    
    glm::vec3 va = GetArcballVector(last_mx, last_my, view);
    glm::vec3 vb = GetArcballVector(cur_mx, cur_my, view);
    float angle = glm::acos(glm::min(1.0f, glm::dot(va, vb))) * 1.5;
    glm::vec3 axis_in_camera = glm::cross(va, vb);
    glm::mat4 trans(1.0f);
    trans = glm::rotate(trans, angle, axis_in_camera);
    
    glm::vec4 view_dir = glm::vec4(view.GetPosition(), 1) - glm::vec4(look_center, 1);
    glm::vec4 _location = glm::transpose(trans) * view_dir + glm::vec4(look_center, 1);
    
    glm::vec3 location(_location);
    glm::vec3 lookat = glm::normalize(look_center - location);
    glm::vec3 right = glm::cross(lookat, last_up);
    glm::vec3 up = glm::cross(right, lookat);
    lookat = glm::normalize(lookat);
    up = glm::normalize(up);
    last_up = up;
	view.SetExtrinsic(Extrinsic(location, location + lookat, up));
    
    last_mx = cur_mx;
    last_my = cur_my;
}

glm::vec3 UserInterface::GetArcballVector(double x, double y, Camera view)
{
    glm::vec3 P(1.0 * x / screen_width * 2 - 1.0,
                1.0 * y / screen_height * 2 - 1.0,
                0);
    
    P.y = -P.y;
    float op_squared = P.x * P.x + P.y * P.y;
    if (op_squared < 1.0) {
        P.z = sqrt(1.0 - op_squared);
    }
    else {
        P = glm::normalize(P);
    }
    
	P = view.GetRight() * P.x + view.GetUp() * P.y + view.GetDir() * P.z;
    
    return P;
}

void UserInterface::SetResolution(int width, int height)
{
    this->screen_width = width;
    this->screen_height = height;
}
