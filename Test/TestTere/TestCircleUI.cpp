#include "CircleUI.h"
#include <gtest/gtest.h>


namespace {
	TEST(CircleUI, SemiCircle) {
		glm::vec3 pos(0.f, 0.f, 1.f);
		glm::vec3 target(0.f);
		glm::vec3 up(0.f, 1.f, 0.f);
		double cx = 1.0, cy = 1.0, fx = 1.0, fy = 1.0; // inrelevent
		Camera cam(Extrinsic(pos, target, up), Intrinsic(cx, cy, fx, fy));

		CircleUI ui(1024, up, target);
		
		ui.Touch(0, 0);
		cam = ui.Move(1024.0, 400.0, cam);
		cam = ui.Leave(1024.0, 400.0, cam);

		// intrinsic mustn't be changed
		ASSERT_EQ(cam.GetCx(), cx);
		ASSERT_EQ(cam.GetCy(), cy);
		ASSERT_EQ(cam.GetFx(), fx);
		ASSERT_EQ(cam.GetFy(), fy);

		// new position should be (0.f, 0.f, -1.f)
		ASSERT_NEAR(cam.GetPosition().x, 0.f, 1e-7);
		ASSERT_NEAR(cam.GetPosition().y, 0.f, 1e-7);
		ASSERT_NEAR(cam.GetPosition().z, -1.f, 1e-7);
		// up shouldn't be changed
		ASSERT_NEAR(cam.GetUp().x, up.x, 1e-7);
		ASSERT_NEAR(cam.GetUp().y, up.y, 1e-7);
		ASSERT_NEAR(cam.GetUp().z, up.z, 1e-7);
		// new dir should be (0.f, 0.f, -1.f)
		ASSERT_NEAR(cam.GetDir().x, 0.f, 1e-7);
		ASSERT_NEAR(cam.GetDir().y, 0.f, 1e-7);
		ASSERT_NEAR(cam.GetDir().z, -1.f, 1e-7);
	}

	TEST(CircleUI, QuadCircle) {
		glm::vec3 pos(0.f, 0.f, 1.f);
		glm::vec3 target(0.f);
		glm::vec3 up(0.f, 1.f, 0.f);
		double cx = 1.0, cy = 1.0, fx = 1.0, fy = 1.0; // inrelevent
		Camera _cam(Extrinsic(pos, target, up), Intrinsic(cx, cy, fx, fy));

		CircleUI ui(1024, up, target);

		bool dragToRight = true;

		for (int i = 0; i != 2; ++i) {
			Camera cam(_cam);
			ui.Touch(512, 0);
			if (dragToRight) {
				cam = ui.Move(1024, 400.0, cam);
				cam = ui.Leave(1024, 400.0, cam);
			}
			else {
				cam = ui.Move(0, 400.0, cam);
				cam = ui.Leave(0, 400.0, cam);
			}

			// intrinsic mustn't be changed
			ASSERT_EQ(_cam.GetCx(), cam.GetCx());
			ASSERT_EQ(_cam.GetCy(), cam.GetCy());
			ASSERT_EQ(_cam.GetFx(), cam.GetFx());
			ASSERT_EQ(_cam.GetFy(), cam.GetFy());

			// new position should be (0.f, 0.f, -1.f)
			ASSERT_NEAR(cam.GetPosition().x, 1.f * (dragToRight ? -1 : 1), 1e-7);
			ASSERT_NEAR(cam.GetPosition().y, 0.f, 1e-7);
			ASSERT_NEAR(cam.GetPosition().z, 0.f, 1e-7);
			// up shouldn't be changed
			ASSERT_EQ(cam.GetUp(), _cam.GetUp());
			// new dir should be (0.f, 0.f, -1.f)
			ASSERT_NEAR(cam.GetDir().x, 1.f * (dragToRight ? -1 : 1), 1e-7);
			ASSERT_NEAR(cam.GetDir().y, 0.f, 1e-7);
			ASSERT_NEAR(cam.GetDir().z, 0.f, 1e-7);
		}
	}
}