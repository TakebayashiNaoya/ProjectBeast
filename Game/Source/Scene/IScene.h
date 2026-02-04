/**
 * @file IScene.h
 * @brief シーンの基底クラス
 * @author 立山
 */
#pragma once

 /**
  * Hash32は文字列を数値に変換するもの。数値に変換するときに別の文字列でも同じ数値になるケースがあるが、被りずらいようなアルゴリズムを使っている。
  * constexprはコンパイル時計算。
  */
#define appScene(name)\
public:\
static constexpr uint32_t ID(){return Hash32(#name);}

static constexpr uint32_t INVALID_SCENE_ID = 0xffffffff;


/**
 * @brief シーンの基底クラス
 */
class IScene
{
public:
	IScene() {}
	virtual ~IScene() {}

	virtual bool Start() = 0;
	virtual void Update() = 0;
	virtual void Render(RenderContext& rc) = 0;

	/**
	 * @brief シーンを要求
	 * @param id シーンのID
	 */
	virtual bool RequesutScene(uint32_t& id) = 0;
};

