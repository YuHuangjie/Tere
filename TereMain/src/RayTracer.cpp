#include "RayTracer.h"

int RayQuadIntersect(const vector<vec3> &nodes,
	const vector<ivec4> &quads, const vec3 &origin, const vec3 &dir)
{
	bool intersected = false;
	int intersected_quad_id = -1;
	const float EPS = 1e-5f;

	for (vector<glm::ivec4>::const_iterator it = quads.begin();
		it != quads.end() && !intersected; ++it) {
		glm::ivec4 quad_indices = *it;

		for (int i = 0; i != 2; ++i) {
			glm::vec3 v1, v2, v3;
			if (i == 0) {
				v1 = nodes[quad_indices.x];
				v2 = nodes[quad_indices.y];
				v3 = nodes[quad_indices.z];
			}
			else if (quad_indices.w != -1) {
				v1 = nodes[quad_indices.x];
				v2 = nodes[quad_indices.z];
				v3 = nodes[quad_indices.w];
			}
			else {
				break;
			}

			glm::vec3 u = v2 - v1;
			glm::vec3 v = v3 - v1;
			glm::vec3 normal = glm::normalize(glm::cross(u, v));

			// ray-plane parallel test
			glm::vec3 w0 = origin - v1;
			float a = -glm::dot(normal, w0);
			float b = glm::dot(normal, dir);

			if (abs(b) < EPS) {
				continue;
			}

			// get intersection point of ray with triangle plane
			float r = a / b;
			if (r < 0.0f) {
				continue;
			}		// ray goes away from triangle plane

					// point inside triangle test
			glm::vec3 p = origin + dir * r;
			float uu, uv, vv, wu, wv, D;
			glm::vec3 w;
			uu = glm::dot(u, u);
			uv = glm::dot(u, v);
			vv = glm::dot(v, v);
			w = p - v1;
			wu = glm::dot(w, u);
			wv = glm::dot(w, v);
			D = uv * uv - uu * vv;

			// get and test parametric coords
			float s, t;
			s = (uv * wv - vv * wu) / D;
			if (s < 0.0f || s > 1.0f) {
				continue;
			}
			t = (uv * wu - uu * wv) / D;
			if (t < 0.0f || (s + t) > 1.0f) {
				continue;
			}


			intersected = true;
			intersected_quad_id = (int)(it - quads.cbegin());
			break;
		}
	}

	return intersected_quad_id;
}
