# Devlog - SceneLab

## Semaine 1 (15-21 septembre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | Initialisation du projet, configuration CMake (C++20, OpenGL 3.3, glad, imgui), implementation du drag-and-drop d'images et de la gestion de palette de couleurs. |
| **Yohan** | Configuration des submodules (glad, glm), mise en place du rendu de base OpenGL. |
| **Lucas** | - |
| **Virgile** | - |
| **Clement** | - |

---

## Semaine 3 (29 septembre - 5 octobre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | - |
| **Yohan** | - |
| **Lucas** | Implementation des primitives vectorielles de base (polygone) et du remplissage de polygones. |
| **Virgile** | - |
| **Clement** | - |

---

## Semaine 4 (6-12 octobre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | - |
| **Yohan** | - |
| **Lucas** | Ajout des primitives vectorielles (carre, rectangle, cercle, ellipse), creation des shapes composites et debut du travail sur l'UI vectoriel. |
| **Virgile** | - |
| **Clement** | - |

---

## Semaine 5 (13-19 octobre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | Implementation du calcul d'histogramme et de l'echantillonnage d'images, connexion de la palette de couleurs avec le module vectoriel, ajout des curseurs dynamiques. |
| **Yohan** | Implementation du scene graph (sans transformations hierarchiques). |
| **Lucas** | Ajout des couleurs pour les primitives et shapes, documentation. |
| **Virgile** | Configuration CMake avancee (glm, mold linker, ccache, DPI scaling), correction compilation Linux, implementation de la scene geometrique avec primitives 3D, chargement de fichiers .obj et bounding boxes AABB. |
| **Clement** | Implementation du multi-view dans les fenetres ImGui. |

---

## Semaine 6 (20-26 octobre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | Auto-focus sur clic d'image, correction du rendu des bounding boxes, gestion des projections ortho/perspective par camera, suppression d'objets, support AABB, initialisation du scene graph avec lumiere ponctuelle. |
| **Yohan** | Implementation d'ImGui docking, selection multiple avec verification parent/enfant, transformations locales/monde dans le scene graph, implementation des gizmos, preparation des documents livrables. |
| **Lucas** | Nommage valide pour l'UI de scene. |
| **Virgile** | Ajout des fichiers .obj d'exemple, renommage de la fenetre en "SceneLab". |
| **Clement** | - |

---

## Semaine 8 (3-9 novembre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | Implementation du systeme de textures, refactoring majeur (introduction de GeometryManager, TransformManager, CameraController), correction du skybox en mode orthographique. |
| **Yohan** | - |
| **Lucas** | - |
| **Virgile** | - |
| **Clement** | - |

---

## Semaine 9 (10-16 novembre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | Gestion du mode de projection dans le renderer, correction du mode orthographique pour ImGuizmo, refactoring du code. |
| **Yohan** | - |
| **Lucas** | - |
| **Virgile** | - |
| **Clement** | - |

---

## Semaine 10 (17-23 novembre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | Refactoring du code, formatage et nettoyage. |
| **Yohan** | - |
| **Lucas** | Implementation du support multi-lumieres et des materiaux, preparation pour differents types de lumieres (directionnelles, ponctuelles, spots). |
| **Virgile** | - |
| **Clement** | Creation de la classe RenderableObject derivable et adaptation du Renderer pour le nouveau systeme d'objets. |

---

## Semaine 11 (24-30 novembre 2025)

| Membre | Tâches effectuees |
| :--- | :--- |
| **Theo** | Refactoring pour typage securise, implementation de spheres/plans analytiques pour le path tracing, ajout du jitter sub-pixel, implementation de la refraction, amelioration de l'UI avec menu et switching de renderer, illumination moderne, implementation de la structure d'acceleration BVH. |
| **Yohan** | Implementation du path tracing de base avec accumulation, refactoring de l'architecture renderer (ARenderer vers IRenderer), abstraction Window pour la gestion GLFW/ImGui, passage des attributs materiaux au shader avec reflexion speculaire. |
| **Lucas** | Implementation des shaders d'illumination (Phong, Blinn-Phong, Gouraud), UI complete pour l'editeur de materiaux et l'illumination, ajout des presets de materiaux. |
| **Virgile** | Implementation de PBR, Métallicité, Microfacettes, Éclairage environnemental et Rendu en différé |
| **Clement** | Implementation de la courbe parametrique avec algorithme Catmull-Rom, implementation de la triangulation de Delaunay. |
