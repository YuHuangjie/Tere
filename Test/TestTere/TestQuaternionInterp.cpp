#include "Interpolation.h"
#include <gtest/gtest.h>

namespace {
	TEST(TestQuaternion, TestBothEnd) {
		Extrinsic left(glm::vec3(0, 0, 0), 
			glm::vec3(-0.009816, 0.031380, 0.999459),
			glm::vec3(-0.003281, -0.999503, 0.031349));
		Extrinsic right(glm::vec3(-0.058647, 0.006684, 0.005672),
			glm::vec3(0.105584, -0.015854,   0.991836),
			glm::vec3(-0.029634, -0.999400, -0.017905));

		for (int i = 0; i != 2; ++i) {
			Extrinsic result;
			if (i == 0) {
				result = Interp(left, right, 0.f);
			}
			else {
				result = Interp(left, right, 1.f);
			}

			Extrinsic toComp = (i == 0 ? left : right);

			ASSERT_NEAR(toComp.GetPos().x, result.GetPos().x, 1e-6);
			ASSERT_NEAR(toComp.GetPos().y, result.GetPos().y, 1e-6);
			ASSERT_NEAR(toComp.GetPos().z, result.GetPos().z, 1e-6);

			ASSERT_NEAR(toComp.GetUp().x, result.GetUp().x, 1e-6);
			ASSERT_NEAR(toComp.GetUp().y, result.GetUp().y, 1e-6);
			ASSERT_NEAR(toComp.GetUp().z, result.GetUp().z, 1e-6);

			ASSERT_NEAR(toComp.GetDir().x, result.GetDir().x, 1e-6);
			ASSERT_NEAR(toComp.GetDir().y, result.GetDir().y, 1e-6);
			ASSERT_NEAR(toComp.GetDir().z, result.GetDir().z, 1e-6);

			ASSERT_NEAR(toComp.GetRight().x, result.GetRight().x, 1e-6);
			ASSERT_NEAR(toComp.GetRight().y, result.GetRight().y, 1e-6);
			ASSERT_NEAR(toComp.GetRight().z, result.GetRight().z, 1e-6);
		}
	}

	TEST(TestQuaternion, TestMiddle) {
		Extrinsic left(glm::vec3(0, 0, 0),
			glm::vec3(-0.009816, 0.031380, 0.999459),
			glm::vec3(-0.003281, -0.999503, 0.031349));
		Extrinsic right(glm::vec3(-0.058647, 0.006684, 0.005672),
			glm::vec3(0.1056, -0.0159, 0.9918),
			glm::vec3(-0.029634, -0.999400, -0.017905));

		Extrinsic result = Interp(left, right, 0.5f);

		ASSERT_NEAR(-0.0293235, result.GetPos().x, 1e-6);
		ASSERT_NEAR(0.003342, result.GetPos().y, 1e-6);
		ASSERT_NEAR(0.002836, result.GetPos().z, 1e-6);

		ASSERT_NEAR(-0.015386, result.GetUp().x, 1e-4);
		ASSERT_NEAR(-0.999862, result.GetUp().y, 1e-4);
		ASSERT_NEAR(0.006207, result.GetUp().z, 1e-4);

		ASSERT_NEAR(-0.077767, result.GetDir().x, 1e-4);
		ASSERT_NEAR(-0.004993, result.GetDir().y, 1e-4);
		ASSERT_NEAR(-0.996959, result.GetDir().z, 1e-4);

		ASSERT_NEAR(0.996852, result.GetRight().x, 1e-4);
		ASSERT_NEAR(-0.015822, result.GetRight().y, 1e-4);
		ASSERT_NEAR(-0.077680, result.GetRight().z, 1e-4);
	}
}