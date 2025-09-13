#include "ParticlesDemo.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <Config.h>
#include <SpatialTransform.h>
#include <Vector3D.h>
#include <World.h>

using Arche::Core::WorldConfig;
using Arche::Math::SpatialTransform;
using Arche::Math::Vector3D;
using Arche::Scene::World;

void RunParticlesDemoWithGravity(const std::string &label,
                                 const Vector3D &gravity,
                                 int steps = 30) {
    std::cout << "\n=== " << label << " ===" << std::endl;
    WorldConfig config;
    config.stepDuration = 0.1f;
    auto world = World::Create(config);

    world->setGravity(gravity);

    std::vector<uint32_t> particleIds;
    for (int i = 0; i < 3; ++i) {
        SpatialTransform t;
        t.setPosition(Vector3D(0.0, 5.0 + i * 2.0, 0.0));
        particleIds.push_back(world->createParticle(t, 1.0f));
    }

    for (int step = 0; step < steps; ++step) {
        world->step();
        auto view = world->view();

        std::cout << "Step " << std::setw(2) << step << ": ";
        for (size_t i = 0; i < view.bodies.size(); ++i) {
            const auto &body = view.bodies[i];
            double speed = body.velocity.length();
            std::cout << "P" << i << " " << std::fixed << std::setprecision(2)
                      << speed << " m/s  ";
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
}

void RunParticlesOnEarth() {
    RunParticlesDemoWithGravity("Particles on Earth", Vector3D(0.0, -9.81, 0.0));
}

void RunParticlesOnMars() {
    RunParticlesDemoWithGravity("Particles on Mars", Vector3D(0.0, -3.71, 0.0));
}

void RunParticlesOnMoon() {
    RunParticlesDemoWithGravity("Particles on Moon", Vector3D(0.0, -1.62, 0.0));
}

void RunParticlesOnJupiter() {
    RunParticlesDemoWithGravity("Particles on Jupiter", Vector3D(0.0, -24.79, 0.0));
}

void RunParticlesOnSaturn() {
    RunParticlesDemoWithGravity("Particles on Saturn", Vector3D(0.0, -10.44, 0.0));
}

void RunParticlesOnSun() {
    RunParticlesDemoWithGravity("Particles on Sun", Vector3D(0.0, -274.0, 0.0));
}

int main() {
    RunParticlesOnEarth();
    RunParticlesOnMars();
    RunParticlesOnMoon();
    RunParticlesOnJupiter();
    RunParticlesOnSaturn();
    RunParticlesOnSun();
    return 0;
}