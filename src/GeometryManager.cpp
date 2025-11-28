#include "GeometryManager.hpp"
#include "GeometryGenerator.hpp"
#include "OBJLoader.hpp"
#include <iostream>
#include <format>
#include <glm/glm.hpp>

#include "objects/Object2D.hpp"
#include "objects/Object3D.hpp"
#include "objects/AnalyticalSphere.hpp"
#include "objects/AnalyticalPlane.hpp"

GeometryManager::GeometryManager(
    SceneGraph &sceneGraph, std::unique_ptr<IRenderer> &renderer) :
    m_sceneGraph(sceneGraph),
    m_renderer(renderer)
{
}

void GeometryManager::initGeometryWindow(std::function<void()> onObjectCreated)
{
    m_geometryWindow.onSpawnCube = [this, onObjectCreated](float size,
                                       const MaterialProperties &mat) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateCube(size) };

        auto object = std::make_unique<Object3D>(
            data.vertices, std::vector<unsigned int> {}, mat.color);

        // Set material properties
        object->setEmissive(mat.emissive);
        object->setPercentSpecular(mat.percentSpecular);
        object->setRoughness(mat.roughness);
        object->setSpecularColor(mat.specularColor);

        new_obj.rendererId = m_renderer->registerObject(std::move(object));
        new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
        new_obj.setAABB(data.aabbCorner1, data.aabbCorner2);
        new_obj.setName(std::format("Cube {}", m_geometryWindow.m_cubeCount));
        m_renderer->updateTransform(
            new_obj.rendererId, new_obj.getModelMatrix());

        std::unique_ptr<SceneGraph::Node> childNode
            = std::make_unique<SceneGraph::Node>();
        childNode->setData(new_obj);
        m_sceneGraph.getRoot()->addChild(std::move(childNode));

        if (onObjectCreated) {
            onObjectCreated();
        }
    };

    m_geometryWindow.onSpawnSphere = [this, onObjectCreated](float radius,
                                         int sectors, int stacks,
                                         const MaterialProperties &mat) {
        GameObject new_obj;

        auto object = std::make_unique<AnalyticalSphere>(
            radius, sectors, stacks, mat.color);

        // Set material properties
        object->setEmissive(mat.emissive);
        object->setPercentSpecular(mat.percentSpecular);
        object->setRoughness(mat.roughness);
        object->setSpecularColor(mat.specularColor);

        new_obj.rendererId = m_renderer->registerObject(std::move(object));
        new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
        new_obj.setAABB(glm::vec3(-radius, -radius, -radius),
            glm::vec3(radius, radius, radius));
        new_obj.setName(
            std::format("Sphere {}", m_geometryWindow.m_sphereCount));
        m_renderer->updateTransform(
            new_obj.rendererId, new_obj.getModelMatrix());

        std::unique_ptr<SceneGraph::Node> childNode
            = std::make_unique<SceneGraph::Node>();
        childNode->setData(new_obj);
        m_sceneGraph.getRoot()->addChild(std::move(childNode));

        if (onObjectCreated) {
            onObjectCreated();
        }
    };

    m_geometryWindow.onSpawnCylinder = [this, onObjectCreated](float radius,
                                           float height, int sectors,
                                           const MaterialProperties &mat) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateCylinder(
            radius, height, sectors) };

        auto object = std::make_unique<Object3D>(
            data.vertices, std::vector<unsigned int> {}, mat.color);

        object->setEmissive(mat.emissive);
        object->setPercentSpecular(mat.percentSpecular);
        object->setRoughness(mat.roughness);
        object->setSpecularColor(mat.specularColor);

        new_obj.rendererId = m_renderer->registerObject(std::move(object));
        new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
        new_obj.setAABB(data.aabbCorner1, data.aabbCorner2);
        new_obj.setName(
            std::format("Cylinder {}", m_geometryWindow.m_cylinderCount));
        m_renderer->updateTransform(
            new_obj.rendererId, new_obj.getModelMatrix());

        std::unique_ptr<SceneGraph::Node> childNode
            = std::make_unique<SceneGraph::Node>();
        childNode->setData(new_obj);
        m_sceneGraph.getRoot()->addChild(std::move(childNode));

        if (onObjectCreated) {
            onObjectCreated();
        }
    };

    m_geometryWindow.onSpawnPlane = [this, onObjectCreated](float width,
                                        float height, const glm::vec3 &normal,
                                        const MaterialProperties &mat) {
        GameObject new_obj;

        auto object = std::make_unique<AnalyticalPlane>(
            width, height, normal, mat.color);

        // Set material properties
        object->setEmissive(mat.emissive);
        object->setPercentSpecular(mat.percentSpecular);
        object->setRoughness(mat.roughness);
        object->setSpecularColor(mat.specularColor);

        new_obj.rendererId = m_renderer->registerObject(std::move(object));
        new_obj.setPosition({ 0.0f, 0.0f, 0.0f });

        // Calculate AABB for plane (approximate based on width/height)
        float halfW = width * 0.5f;
        float halfH = height * 0.5f;
        new_obj.setAABB(
            glm::vec3(-halfW, -0.01f, -halfH), glm::vec3(halfW, 0.01f, halfH));
        new_obj.setName(
            std::format("Plane {}", m_geometryWindow.m_planeCount));
        m_renderer->updateTransform(
            new_obj.rendererId, new_obj.getModelMatrix());

        std::unique_ptr<SceneGraph::Node> childNode
            = std::make_unique<SceneGraph::Node>();
        childNode->setData(new_obj);
        m_sceneGraph.getRoot()->addChild(std::move(childNode));

        if (onObjectCreated) {
            onObjectCreated();
        }
    };

    m_geometryWindow.onLoadModel
        = [this](const std::string &objName, const std::string &objPath) {
              auto data { OBJLoader::loadOBJ(objName, objPath) };

              m_geometryWindow.m_modelLibrary.addModel(objName, objPath, data);
          };

    m_geometryWindow.onSpawnModelInstance = [this, onObjectCreated](
                                                const std::string &name,
                                                const std::string &filepath) {
        auto &modelLib = m_geometryWindow.m_modelLibrary;

        auto maybeGData = modelLib.getModelData(filepath);

        if (!maybeGData.has_value()) {
            std::cerr << std::format(
                "[ERROR] Model not found in library: {}\n", name);
            return;
        }

        const GData &data = maybeGData.value();
        GameObject new_obj;
        glm::vec3 randomColor { rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };

        new_obj.rendererId = m_renderer->registerObject(
            std::make_unique<Object3D>(
                data.vertices, std::vector<unsigned int> {}),
            randomColor);
        new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
        new_obj.setAABB(data.aabbCorner1, data.aabbCorner2);

        const auto &models = modelLib.getModels();
        auto it = models.find(filepath);
        if (it != models.end()) {
            modelLib.incrementInstanceCount(filepath);
            std::size_t instanceNum = modelLib.getInstanceCount(filepath);
            new_obj.setName(
                std::format("{} {}", it->second.name, instanceNum));
        }

        m_renderer->updateTransform(
            new_obj.rendererId, new_obj.getModelMatrix());

        std::unique_ptr<SceneGraph::Node> childNode
            = std::make_unique<SceneGraph::Node>();
        childNode->setData(new_obj);
        m_sceneGraph.getRoot()->addChild(std::move(childNode));

        if (onObjectCreated) {
            onObjectCreated();
        }
    };
}

void GeometryManager::renderUI() { m_geometryWindow.render(); }
