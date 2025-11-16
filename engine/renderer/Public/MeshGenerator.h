#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Mesh.h"
#include "ResourceRegistry.h"

namespace Arche::Render {

    /**
     * @brief Utility class for procedurally generating common 3D meshes.
     * 
     * MeshGenerator provides static methods for creating standard geometric
     * shapes like spheres, cubes, and planes. These meshes are used for
     * prototyping, debug visualization, and as default shapes for entities.
     * 
     * The class can register all built-in meshes with a ResourceRegistry
     * for easy access throughout the engine.
     */
    class MeshGenerator {
      public:
        /**
         * @brief Register all built-in procedural meshes.
         * 
         * Creates and registers standard meshes (sphere, cube, plane, etc.)
         * with the resource registry. Typically called during engine initialization.
         * 
         * @param registry Resource registry to populate with meshes
         */
        static void registerBuiltinMeshes(ResourceRegistry &registry);

        /**
         * @brief Generate a point-based sphere mesh.
         * 
         * Creates a sphere made of individual points rather than triangles.
         * Useful for particle effects or special visualization.
         * 
         * @param name Mesh identifier
         * @return Shared pointer to the generated mesh
         */
        static std::shared_ptr<Mesh> makePointSphere(const std::string &name);
        
        /**
         * @brief Generate a UV-mapped sphere mesh.
         * 
         * Creates a triangulated sphere with proper normals and texture coordinates.
         * Quality is controlled by the number of segments in U and V directions.
         * 
         * @param name Mesh identifier
         * @param radius Sphere radius
         * @param segmentsU Horizontal resolution (longitude)
         * @param segmentsV Vertical resolution (latitude)
         * @return Shared pointer to the generated mesh
         */
        static std::shared_ptr<Mesh> makeUvSphere(const std::string &name, float radius, uint32_t segmentsU,
                                                  uint32_t segmentsV);
        
        /**
         * @brief Generate a cube mesh.
         * 
         * Creates a box with 6 faces, proper normals, and texture coordinates.
         * 
         * @param name Mesh identifier
         * @param size Side length of the cube
         * @return Shared pointer to the generated mesh
         */
        static std::shared_ptr<Mesh> makeCube(const std::string &name, float size);
        
        /**
         * @brief Generate a plane mesh.
         * 
         * Creates a flat rectangular surface subdivided into a grid. Useful
         * for floors, terrain, and other flat surfaces.
         * 
         * @param name Mesh identifier
         * @param width Plane width along X axis
         * @param height Plane depth along Z axis
         * @param segU Number of subdivisions along width
         * @param segV Number of subdivisions along height
         * @return Shared pointer to the generated mesh
         */
        static std::shared_ptr<Mesh> makePlane(const std::string &name, float width, float height, uint32_t segU,
                                               uint32_t segV);
    };

} // namespace Arche::Render
