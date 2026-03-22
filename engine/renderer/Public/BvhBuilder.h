#pragma once
#include <algorithm>
#include <limits>
#include <numeric>
#include <vector>
#include <glm/glm.hpp>

namespace Arche {
    namespace Render {

        // ── GPU-side structs (std430 compatible) ──────────────────────────────

        /**
         * @brief A sphere primitive for the path-tracing acceleration structure.
         *
         * std430 layout: 32 bytes
         *   vec3  center  (12 B)
         *   float radius  ( 4 B)
         *   int   matIdx  ( 4 B)
         *   float _pad[3] (12 B)
         */
        struct GpuSphere {
            glm::vec3 center{0.0f};
            float     radius{1.0f};
            int       matIdx{0};
            float     _pad0{0}, _pad1{0}, _pad2{0};
        };
        static_assert(sizeof(GpuSphere) == 32);

        /**
         * @brief A material for path tracing.
         *
         * std430 layout: 32 bytes
         *   vec3  albedo     (12 B)
         *   float fuzzOrIOR  ( 4 B)  — Metal: fuzz (0-1); Dielectric: IOR (e.g. 1.5)
         *   int   type       ( 4 B)  — 0=Lambertian, 1=Metal, 2=Dielectric
         *   float _pad[3]    (12 B)
         */
        struct GpuMaterial {
            glm::vec3 albedo{0.5f, 0.5f, 0.5f};
            float     fuzzOrIOR{0.0f};
            int       type{0};
            float     _pad0{0}, _pad1{0}, _pad2{0};
        };
        static_assert(sizeof(GpuMaterial) == 32);

        /**
         * @brief Flat BVH node for GPU traversal.
         *
         * std430 layout: 32 bytes
         *   vec3 aabbMin         (12 B)
         *   int  leftOrSphereIdx ( 4 B) — internal: left child idx; leaf: sphere idx
         *   vec3 aabbMax         (12 B)
         *   int  rightIsLeaf     ( 4 B) — internal: right child idx; leaf: -1
         */
        struct GpuBvhNode {
            glm::vec3 aabbMin{0.0f};
            int       leftOrSphereIdx{0};
            glm::vec3 aabbMax{0.0f};
            int       rightIsLeaf{-1};  ///< -1 = leaf node
        };
        static_assert(sizeof(GpuBvhNode) == 32);

        // ── CPU-side BVH construction ─────────────────────────────────────────

        /**
         * @brief Builds a flat BVH from a sphere list for GPU consumption.
         *
         * Uses midpoint splitting on the longest centroid-AABB axis.  The
         * resulting nodes are stored in depth-first pre-order; node 0 is always
         * the root.  Leaf nodes have rightIsLeaf == -1 and leftOrSphereIdx is
         * the index into the sphere array.  Internal nodes store child indices.
         */
        class BvhBuilder {
          public:
            /**
             * @brief Build a flat BVH from the given sphere list.
             *
             * The sphere list ordering is preserved; leaf indices reference
             * positions in the original array.
             *
             * @param spheres  Sphere list to accelerate.
             * @return         Flat GpuBvhNode array (node 0 = root).
             */
            static std::vector<GpuBvhNode> build(const std::vector<GpuSphere> &spheres) {
                if (spheres.empty()) return {};
                std::vector<GpuBvhNode> nodes;
                // 2N-1 nodes for N spheres
                nodes.reserve(2 * spheres.size());
                // Work on a mutable index list to avoid reordering the sphere array
                std::vector<int> indices(spheres.size());
                std::iota(indices.begin(), indices.end(), 0);
                buildNode(spheres, indices, 0, (int)indices.size(), nodes);
                return nodes;
            }

          private:
            struct Aabb {
                glm::vec3 lo{ std::numeric_limits<float>::max()};
                glm::vec3 hi{-std::numeric_limits<float>::max()};
                void extend(glm::vec3 p) { lo = glm::min(lo, p); hi = glm::max(hi, p); }
            };

            static Aabb sphereAabb(const GpuSphere &s) {
                glm::vec3 r(s.radius);
                Aabb b;
                b.lo = s.center - r;
                b.hi = s.center + r;
                return b;
            }

            // Returns the index of the newly created node.
            static int buildNode(const std::vector<GpuSphere>  &spheres,
                                 std::vector<int>               &indices,
                                 int start, int end,
                                 std::vector<GpuBvhNode>        &nodes)
            {
                int idx = (int)nodes.size();
                nodes.emplace_back();

                // Enclosing AABB of all spheres in [start, end)
                Aabb box;
                for (int i = start; i < end; i++) {
                    const GpuSphere &s = spheres[indices[i]];
                    box.extend(s.center - glm::vec3(s.radius));
                    box.extend(s.center + glm::vec3(s.radius));
                }

                int count = end - start;

                if (count == 1) {
                    // Leaf
                    nodes[idx].aabbMin          = box.lo;
                    nodes[idx].aabbMax          = box.hi;
                    nodes[idx].leftOrSphereIdx  = indices[start];
                    nodes[idx].rightIsLeaf      = -1;
                    return idx;
                }

                // Split on longest centroid-AABB dimension
                Aabb centBox;
                for (int i = start; i < end; i++)
                    centBox.extend(spheres[indices[i]].center);
                glm::vec3 ext = centBox.hi - centBox.lo;
                int axis = 0;
                if (ext.y > ext.x) axis = 1;
                if (ext.z > ext[axis]) axis = 2;

                int mid = start + count / 2;
                std::nth_element(indices.begin() + start,
                                 indices.begin() + mid,
                                 indices.begin() + end,
                                 [&](int a, int b) {
                                     return spheres[a].center[axis] < spheres[b].center[axis];
                                 });

                int left  = buildNode(spheres, indices, start, mid, nodes);
                int right = buildNode(spheres, indices, mid,   end, nodes);

                // Fill this node after recursion (vector may have grown, but idx is stable)
                nodes[idx].aabbMin          = box.lo;
                nodes[idx].aabbMax          = box.hi;
                nodes[idx].leftOrSphereIdx  = left;
                nodes[idx].rightIsLeaf      = right;
                return idx;
            }
        };

    } // namespace Render
} // namespace Arche
