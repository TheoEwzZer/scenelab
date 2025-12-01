#include "GeometryManager.hpp"
#include "GeometryGenerator.hpp"
#include "OBJLoader.hpp"
#include "objects/DynamicLine.hpp"

#include <iostream>
#include <format>
#include <glm/glm.hpp>

#include "objects/Object2D.hpp"
#include "objects/Object3D.hpp"
#include "objects/AnalyticalSphere.hpp"
#include "objects/AnalyticalPlane.hpp"

GeometryManager::GeometryManager(SceneGraph &sceneGraph,
    std::unique_ptr<IRenderer> &renderer,
    DynamicGeometryManager &parametricCurveManager) :
    m_sceneGraph(sceneGraph),
    m_renderer(renderer), m_dynamicGeometryManager(parametricCurveManager)
{
}

void GeometryManager::initGeometryWindow(
    const std::function<void()> &onObjectCreated)
{
    m_geometryWindow.onSpawnCube = [this, onObjectCreated](float size) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateCube(size) };

        auto object = std::make_unique<Object3D>(
            data.vertices, std::vector<unsigned int> {}, glm::vec3(1.0f));

        // Default material properties
        object->setEmissive(glm::vec3(0.0f));
        object->setPercentSpecular(0.0f);
        object->setRoughness(0.5f);
        object->setSpecularColor(glm::vec3(1.0f));

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
                                         int sectors, int stacks) {
        GameObject new_obj;

        auto object = std::make_unique<AnalyticalSphere>(
            radius, sectors, stacks, glm::vec3(1.0f));

        // Default material properties
        object->setEmissive(glm::vec3(0.0f));
        object->setPercentSpecular(0.0f);
        object->setRoughness(0.5f);
        object->setSpecularColor(glm::vec3(1.0f));

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
                                           float height, int sectors) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateCylinder(
            radius, height, sectors) };

        auto object = std::make_unique<Object3D>(
            data.vertices, std::vector<unsigned int> {}, glm::vec3(1.0f));

        // Default material properties
        object->setEmissive(glm::vec3(0.0f));
        object->setPercentSpecular(0.0f);
        object->setRoughness(0.5f);
        object->setSpecularColor(glm::vec3(1.0f));

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
                                        float height,
                                        const glm::vec3 &normal) {
        GameObject new_obj;

        auto object = std::make_unique<AnalyticalPlane>(
            width, height, normal, glm::vec3(1.0f));

        // Default material properties
        object->setEmissive(glm::vec3(0.0f));
        object->setPercentSpecular(0.0f);
        object->setRoughness(0.5f);
        object->setSpecularColor(glm::vec3(1.0f));

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

    m_geometryWindow.onSpawnParametricCurve = [this, onObjectCreated](
                                                  int controlPoints) {
        auto curve = std::make_unique<ParametricCurve>();
        for (int i = 0; i < controlPoints; ++i) {
            GameObject point;
            auto [vertices, aabbCorner1, aabbCorner2] {
                GeometryGenerator::generateSphere(0.05, 36, 18)
            };
            glm::vec3 neutralGray { 0.75f, 0.75f, 0.75f };

            point.rendererId = m_renderer->registerObject(
                std::make_unique<Object3D>(
                    vertices, std::vector<unsigned int> {}),
                neutralGray);
            // Spread control points along X axis to avoid degenerate
            // Catmull-Rom
            float spacing = 0.5f;
            point.setPosition({ i * spacing, 0.0f, 0.0f });
            point.setAABB(aabbCorner1, aabbCorner2);
            point.setName(
                std::format("Sphere {}", m_geometryWindow.m_sphereCount));
            point.setHelper(true);
            m_renderer->updateTransform(
                point.rendererId, point.getModelMatrix());

            auto childNode = std::make_unique<SceneGraph::Node>();
            childNode->setData(point);
            SceneGraph::Node *rawPtr = childNode.get();
            m_sceneGraph.getRoot()->addChild(std::move(childNode));
            curve->addControlPoint(rawPtr);
        }
        glm::vec3 randomColor { rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };
        int id = m_renderer->registerObject(
            std::make_unique<DynamicLine>(randomColor));
        curve->registerCurve(id);
        curve->updateGeometry(*m_renderer);
        m_dynamicGeometryManager.addCurve(std::move(curve));

        if (onObjectCreated) {
            onObjectCreated();
        }
    };

    m_geometryWindow.onAddPoint = [this, onObjectCreated]() {
        Triangulation *mesh = m_dynamicGeometryManager.getLastEmpty();
        GameObject point;
        auto [vertices, aabbCorner1,
            aabbCorner2] { GeometryGenerator::generateSphere(0.05, 36, 18) };
        glm::vec3 neutralGray { 0.75f, 0.75f, 0.75f };

        point.rendererId = m_renderer->registerObject(
            std::make_unique<Object3D>(vertices, std::vector<unsigned int> {}),
            neutralGray);

        // Spread points in a non-colinear pattern for Delaunay triangulation
        int pointIndex = static_cast<int>(mesh->getPointCount());
        float spacing = 0.5f;
        float x, y;
        // First 3 points form a triangle, then continue in grid
        if (pointIndex == 0) {
            x = 0.0f;
            y = 0.0f;
        } else if (pointIndex == 1) {
            x = spacing;
            y = 0.0f;
        } else if (pointIndex == 2) {
            x = spacing * 0.5f;
            y = spacing * 0.866f;
        } else {
            int idx = pointIndex - 3;
            int col = idx % 3;
            int row = idx / 3 + 1;
            x = col * spacing + (row % 2) * (spacing * 0.5f);
            y = row * spacing * 0.866f + spacing;
        }
        point.setPosition({ x, y, 0.0f });
        point.setAABB(aabbCorner1, aabbCorner2);
        point.setName(
            std::format("Sphere {}", m_geometryWindow.m_sphereCount));
        point.setHelper(true); // Control point - not rendered in path tracing
        m_renderer->updateTransform(point.rendererId, point.getModelMatrix());

        auto childNode = std::make_unique<SceneGraph::Node>();
        childNode->setData(point);
        SceneGraph::Node *rawPtr = childNode.get();
        m_sceneGraph.getRoot()->addChild(std::move(childNode));
        mesh->addPoint(rawPtr);

        if (onObjectCreated) {
            onObjectCreated();
        }
    };

    m_geometryWindow.onGenerateMesh = [this, onObjectCreated]() {
        if (Triangulation *mesh = m_dynamicGeometryManager.getLastEmpty();
            mesh->canTriangulate()) {
            glm::vec3 randomColor { rand() / (float)RAND_MAX,
                rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };
            const int id = m_renderer->registerObject(
                std::make_unique<DynamicLine>(randomColor, GL_LINES));
            mesh->registerRenderable(id);
            mesh->updateGeometry(*m_renderer);

            if (onObjectCreated) {
                onObjectCreated();
            }
        }
    };
}

void GeometryManager::renderUI(bool *p_open)
{
    m_geometryWindow.render(p_open);
}
