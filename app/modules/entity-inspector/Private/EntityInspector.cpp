#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "EntityInspector.h"

#include <Material.h>
#include <RenderingSystem.h>
#include <ResourceRegistry.h>
#include <RigidBody.h>
#include <WorldSystem.h>
#include <imgui.h>

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <string>
#include <utility>

namespace Arche {
    namespace GUI {

        void EntityInspector::Draw() {
            if (!ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None)) {
                ImGui::End();
                return;
            }

            auto entity = resolveSelectedEntity();
            if (!entity) {
                ImGui::TextUnformatted("No entity selected.");
                ImGui::End();
                return;
            }

            ImGui::Text("Entity: %s", std::string(entity->getName()).c_str());
            ImGui::Text("ID: %llu", static_cast<unsigned long long>(entity->getID()));
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                glm::vec3 position = entity->getPosition();
                if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.05f)) {
                    entity->setPosition(position);
                }

                glm::vec3 rotation = entity->getRotation();
                if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.5f)) {
                    entity->setRotation(rotation);
                }

                glm::vec3 scale = entity->getScale();
                if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.05f, 0.001f)) {
                    scale.x = std::max(scale.x, 0.001f);
                    scale.y = std::max(scale.y, 0.001f);
                    scale.z = std::max(scale.z, 0.001f);
                    entity->setScale(scale);
                }
            }

            if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto rigidBody = entity->getRigidBody();
                if (rigidBody) {
                    bool isStatic = rigidBody->isStatic();
                    if (ImGui::Checkbox("Static", &isStatic)) {
                        rigidBody->setStatic(isStatic);
                    }

                    bool usesGravity = rigidBody->isUsingGravity();
                    if (ImGui::Checkbox("Use Gravity", &usesGravity)) {
                        rigidBody->setUseGravity(usesGravity);
                    }

                    float mass = rigidBody->getMass();
                    if (!isStatic && ImGui::DragFloat("Mass", &mass, 0.05f, 0.01f, 1000.0f)) {
                        rigidBody->setMass(std::max(mass, 0.01f));
                    }

                    glm::vec3 velocity = rigidBody->getVelocity();
                    if (ImGui::DragFloat3("Velocity", glm::value_ptr(velocity), 0.05f)) {
                        rigidBody->setVelocity(velocity);
                    }

                    glm::vec3 acceleration = rigidBody->getAcceleration();
                    if (ImGui::DragFloat3("Acceleration", glm::value_ptr(acceleration), 0.05f)) {
                        rigidBody->setAcceleration(acceleration);
                    }
                } else {
                    ImGui::TextUnformatted("No rigid body attached.");
                }
            }

            if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto renderer = context ? context->renderer() : nullptr;
                if (!renderer) {
                    ImGui::TextUnformatted("Renderer unavailable.");
                } else {
                    auto &registry = renderer->resources();
                    std::string currentMaterialName{entity->getMaterialId()};
                    ImGui::Text("Material ID: %s", currentMaterialName.c_str());

                    auto material = registry.getMaterial(currentMaterialName);
                    if (!material) {
                        ImGui::TextUnformatted("Material not found in registry.");
                    } else {
                        if (!material->getShaderName().empty()) {
                            ImGui::Text("Shader: %s", material->getShaderName().c_str());
                        }

                        auto editableMaterial = material;
                        auto ensureOverrideAndUpdate = [&](auto updater) {
                            editableMaterial = ensureEntityMaterialOverride(entity, registry, editableMaterial);
                            if (editableMaterial) {
                                updater(*editableMaterial);
                                currentMaterialName = std::string(entity->getMaterialId());
                            }
                        };

                        std::optional<glm::vec3> baseColor = editableMaterial->getVec3("uAlbedo");
                        if (!baseColor.has_value()) {
                            ImGui::TextUnformatted("Material missing 'uAlbedo' parameter.");
                        } else {
                            if (ImGui::ColorEdit3("Base Color", glm::value_ptr(baseColor.value()),
                                                  ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Float)) {
                                ensureOverrideAndUpdate(
                                    [&](Render::Material &mat) { mat.setVec3("uAlbedo", baseColor.value()); });
                            }
                        }

                        float pointSize = editableMaterial->getPointSize();
                        if (ImGui::DragFloat("Point Size", &pointSize, 0.1f, 0.0f, 2048.0f)) {
                            ensureOverrideAndUpdate([&](Render::Material &mat) { mat.setPointSize(pointSize); });
                        }

                        const auto &floatParams = editableMaterial->getFloats();
                        if (!floatParams.empty() && ImGui::TreeNode("Float Properties")) {
                            for (const auto &entry : floatParams) {
                                float value = entry.second;
                                ImGui::PushID(entry.first.c_str());
                                if (ImGui::DragFloat("##value", &value, 0.01f)) {
                                    ensureOverrideAndUpdate(
                                        [&](Render::Material &mat) { mat.setFloat(entry.first, value); });
                                }
                                ImGui::SameLine();
                                ImGui::TextUnformatted(entry.first.c_str());
                                ImGui::PopID();
                            }
                            ImGui::TreePop();
                        }

                        const auto &vec3Params = editableMaterial->getVec3s();
                        if (!vec3Params.empty() && ImGui::TreeNode("Vec3 Properties")) {
                            for (const auto &entry : vec3Params) {
                                glm::vec3 value = entry.second;
                                ImGui::PushID(entry.first.c_str());
                                if (ImGui::DragFloat3("##value", glm::value_ptr(value), 0.01f)) {
                                    ensureOverrideAndUpdate(
                                        [&](Render::Material &mat) { mat.setVec3(entry.first, value); });
                                }
                                ImGui::SameLine();
                                ImGui::TextUnformatted(entry.first.c_str());
                                ImGui::PopID();
                            }
                            ImGui::TreePop();
                        }

                        const auto &vec4Params = editableMaterial->getVec4s();
                        if (!vec4Params.empty() && ImGui::TreeNode("Vec4 Properties")) {
                            for (const auto &entry : vec4Params) {
                                glm::vec4 value = entry.second;
                                ImGui::PushID(entry.first.c_str());
                                if (ImGui::DragFloat4("##value", glm::value_ptr(value), 0.01f)) {
                                    ensureOverrideAndUpdate(
                                        [&](Render::Material &mat) { mat.setVec4(entry.first, value); });
                                }
                                ImGui::SameLine();
                                ImGui::TextUnformatted(entry.first.c_str());
                                ImGui::PopID();
                            }
                            ImGui::TreePop();
                        }
                    }
                }
            }

            ImGui::End();
        }

        std::shared_ptr<Scene::IEntity> EntityInspector::resolveSelectedEntity() const {
            if (!context) {
                return nullptr;
            }

            auto selected = context->getSelectedEntity();
            if (!selected.has_value()) {
                return nullptr;
            }

            auto world = context->worldSystem();
            if (!world) {
                return nullptr;
            }

            auto view = world->view();
            auto it = std::find_if(view.bodies.begin(), view.bodies.end(),
                                   [id = selected.value()](const std::shared_ptr<Scene::IEntity> &candidate) {
                                       return candidate && candidate->getID() == id;
                                   });
            return it != view.bodies.end() ? *it : nullptr;
        }

        std::string EntityInspector::makeEntityMaterialPrefix(std::uint64_t entityId) {
            std::ostringstream prefix;
            prefix << "entity_" << entityId << "_";
            return prefix.str();
        }

        std::string EntityInspector::makeEntityMaterialName(std::uint64_t entityId, std::string_view baseName) {
            auto prefix = makeEntityMaterialPrefix(entityId);
            std::string result = prefix;
            result.append(baseName.data(), baseName.size());
            return result;
        }

        bool EntityInspector::isEntityMaterialOverride(std::uint64_t entityId, std::string_view materialName) {
            auto prefix = makeEntityMaterialPrefix(entityId);
            return materialName.size() > prefix.size() && materialName.find(prefix) == 0;
        }

        std::shared_ptr<Render::Material>
        EntityInspector::ensureEntityMaterialOverride(const std::shared_ptr<Scene::IEntity> &entity,
                                                      Render::ResourceRegistry &registry,
                                                      std::shared_ptr<Render::Material> currentMaterial) const {
            if (!entity) {
                return nullptr;
            }

            std::string currentMaterialName{entity->getMaterialId()};
            if (isEntityMaterialOverride(entity->getID(), currentMaterialName)) {
                if (currentMaterial) {
                    return currentMaterial;
                }
                return registry.getMaterial(currentMaterialName);
            }

            if (!currentMaterial) {
                currentMaterial = registry.getMaterial(currentMaterialName);
            }

            if (!currentMaterial) {
                return nullptr;
            }

            auto overrideName = makeEntityMaterialName(entity->getID(), currentMaterialName);
            auto overrideMaterial = registry.getMaterial(overrideName);
            if (!overrideMaterial) {
                overrideMaterial = currentMaterial->cloneWithName(overrideName);
                registry.registerMaterial(overrideMaterial);
            }

            entity->setMaterialId(overrideName);
            return overrideMaterial;
        }

        std::string_view EntityInspector::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
