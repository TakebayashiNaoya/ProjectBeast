#pragma once

#define appScene(name)\
public:\
static constexpr uint32_t ID(){return Hash32(#name);}

static constexpr uint32_t INVALID_SCENE_ID = 0xffffffff;

//基底クラス
class IScene
{
public:
	IScene() {}
	virtual ~IScene() {}

	virtual bool Start() = 0;
	virtual void Update() = 0;
	virtual void Render(RenderContext& rc) = 0;

	virtual bool RequesutScene(uint32_t& id, float& waitTime) = 0;
};

