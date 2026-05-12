#pragma once
#define _CRT_SECURE_NO_WARNINGS


// #define COPY_RAYTRACING_FRAMEBUFFER		// 有効でレイトレの結果をフレームバッファに書き出す。

#include "k2EngineLowPreCompile.h"
using namespace nsK2EngineLow;
//#include "collision/CollisionObject.h"
#include "Graphics/IRenderer.h"
#include "Graphics/RenderingEngine.h"
#include "Graphics/GraphicsEnums.h"
//#include "geometry/AABB.h"
#include "Geometry/Frustum.h"

#include "Resource/ResourceManager.h"
#include "Resource/ModelResource.h"
#include "Resource/AnimationClipResource.h"
#include "Resource/SkeletonResource.h"

#include "Graphics/ModelRender.h"
#include "Graphics/SpriteRender.h"
#include "Graphics/FontRender.h"

//#include "Physics/ICollider.h"
#include "Physics/Physics.h"
#include "Physics/BoxCollider.h"
#include "Physics/SphereCollider.h"
#include "Physics/CapsuleCollider.h"
#include "Physics/MeshCollider.h"
#include "Physics/RigidBody.h"
#include "Physics/CharacterController.h"

#include "Graphics/light/SceneLight.h"
#include "Graphics/light/HemisphereLight.h"
#include "Graphics/light/SpotLight.h"
#include "Graphics/light/PointLight.h"
#include "Graphics/OcclusionDitherManager.h"

#include "graphics/effect/EffectEmitter.h"

#include "BeastEngine.h"
#include "Nature/SkyCube.h"