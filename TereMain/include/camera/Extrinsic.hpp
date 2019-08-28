#ifndef EXTRINSIC_HPP
#define EXTRINSIC_HPP

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "Type.h"

struct Extrinsic
{
	glm::mat4 viewMat;

	Extrinsic()
	{}

	Extrinsic(const float pos[3], const float target[3], const float up[3])
	{
		glm::vec3 _pos(pos[0], pos[1], pos[2]);
		glm::vec3 _target(target[0], target[1], target[2]);
		glm::vec3 _up = glm::normalize(glm::vec3(up[0], up[1], up[2]));

		glm::vec3 _dir = glm::normalize(_pos - _target);
		glm::vec3 _right = glm::cross(_up, _dir);
		_up = glm::cross(_dir, _right);

		viewMat = glm::lookAt(_pos, _target, _up);
	}

	Extrinsic(const float M[16], bool w2c, bool yIsUp)
	{
		glm::mat4 _M(glm::vec4(M[0], M[4], M[8], M[12]),
			glm::vec4(M[1], M[5], M[9], M[13]),
			glm::vec4(M[2], M[6], M[10], M[14]),
			glm::vec4(M[3], M[7], M[11], M[15]));

		if (!w2c) {
			_M = glm::inverse(_M);
		}

		if (!yIsUp) {
			_M[0].y = -_M[0].y; _M[1].y = -_M[1].y; _M[2].y = -_M[2].y; _M[3].y = -_M[3].y;
			_M[0].z = -_M[0].z; _M[1].z = -_M[1].z; _M[2].z = -_M[2].z; _M[3].z = -_M[3].z;
		}

		viewMat = _M;
	}

	glm::vec3 Right() const { return glm::row(viewMat, 0); }
	glm::vec3 Up()    const { return glm::row(viewMat, 1); }
	glm::vec3 Dir()   const { return glm::row(viewMat, 2); }
	glm::vec3 Pos()   const { return glm::inverse(viewMat)[3]; }
};

#endif