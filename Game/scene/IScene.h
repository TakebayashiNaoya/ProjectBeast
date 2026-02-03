/**
 * IScene.h
 * 全てのシーンの基底クラス
 */
#pragma once


#define appScene(name)\
public:\
static constexpr uint32_t ID(){return Hash32(#name);}

static constexpr uint32_t INVALID_SCENE_ID = 0xffffffff;


/**
 * シーンの基底クラス
 */
class IScene
{
public:
	IScene() {}
	virtual ~IScene() {}

	virtual bool Start() = 0;
	virtual void Update() = 0;
	virtual void Render(RenderContext& rc) = 0;

	/** 次のシーンを要求する。idは次のシーンのIDを返す。*/
	virtual bool RequesutScene(uint32_t& id) = 0;
};

