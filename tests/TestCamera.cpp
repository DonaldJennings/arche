#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Camera.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

using namespace Arche::Scene;
using namespace Catch;

SCENARIO("Camera setters/getters and matrix generation", "[camera]") {
    Camera cam;

    GIVEN("a default camera") {
        THEN("position, pitch and yaw are zero-ish and matrices are valid") {
            CHECK(cam.GetPosition() == glm::dvec3(0));
            CHECK(cam.GetPitch() == Approx(0.0));
            CHECK(cam.GetYaw() == Approx(0.0));
            auto V = cam.GetViewMatrix();
            auto P = cam.GetProjectionMatrix();
            // matrices should be finite
            bool finite = true;
            for (int r=0;r<4;++r) for(int c=0;c<4;++c) finite = finite && std::isfinite(V[r][c]) && std::isfinite(P[r][c]);
            CHECK(finite);
        }

        WHEN("position and orientation are changed") {
            cam.SetPosition(glm::dvec3(1.0, 2.0, 3.0));
            cam.SetPitch(10.0);
            cam.SetYaw(20.0);

            THEN("getters reflect changes") {
                CHECK(cam.GetPosition() == glm::dvec3(1.0, 2.0, 3.0));
                CHECK(cam.GetPitch() == Approx(10.0));
                CHECK(cam.GetYaw() == Approx(20.0));
            }
        }

        WHEN("perspective parameters are set") {
            cam.setPerspective(60.0, 16.0/9.0, 0.1, 1000.0);
            auto P = cam.GetProjectionMatrix();
            THEN("matrix is finite") {
                bool finite = true;
                for (int r=0;r<4;++r) for(int c=0;c<4;++c) finite = finite && std::isfinite(P[r][c]);
                CHECK(finite);
            }
        }

        WHEN("screenToWorldRay is called with center of viewport") {
            cam.setPerspective(60.0, 1.0, 0.1, 1000.0);
            cam.SetPosition(glm::dvec3(0.0,0.0,0.0));
            cam.setPitchYaw(0.0, 0.0);
            auto dir = cam.screenToWorldRay(0.5f, 0.5f, 1.0f, 1.0f);
            THEN("direction is normalized") {
                CHECK(Approx(glm::length(dir)).margin(1e-10) == 1.0);
            }
        }
    }
}
