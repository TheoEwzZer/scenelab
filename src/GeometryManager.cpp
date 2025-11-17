#include "GeometryManager.hpp"
#include "GeometryGenerator.hpp"
#include "OBJLoader.hpp"
#include <iostream>
#include <format>
#include <glm/glm.hpp>

#include "objects/Object2D.hpp"
#include "objects/Object3D.hpp"

GeometryManager::GeometryManager(
    SceneGraph &sceneGraph, std::unique_ptr<ARenderer> &renderer) :
    m_sceneGraph(sceneGraph),
    m_renderer(renderer)
{
}

void GeometryManager::initGeometryWindow(std::function<void()> onObjectCreated)
{
    m_geometryWindow.onSpawnCube = [this, onObjectCreated](float size) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateCube(size) };
        glm::vec3 randomColor { rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };

        new_obj.rendererId = m_renderer->registerObject(
            std::make_unique<Object3D>(
                data.vertices, std::vector<unsigned int> {}),
            randomColor);
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

    m_geometryWindow.onSpawnSphere
        = [this, onObjectCreated](float radius, int sectors, int stacks) {
              GameObject new_obj;
              auto data { GeometryGenerator::generateSphere(
                  radius, sectors, stacks) };
              glm::vec3 randomColor { rand() / (float)RAND_MAX,
                  rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };

              new_obj.rendererId = m_renderer->registerObject(
                  std::make_unique<Object3D>(
                      data.vertices, std::vector<unsigned int> {}),
                  randomColor);
              new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
              new_obj.setAABB(data.aabbCorner1, data.aabbCorner2);
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
                                           float height, int sectors) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateCylinder(
            radius, height, sectors) };
        glm::vec3 randomColor { rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };

        new_obj.rendererId = m_renderer->registerObject(
            std::make_unique<Object3D>(
                data.vertices, std::vector<unsigned int> {}),
            randomColor);
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
