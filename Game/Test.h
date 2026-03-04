#pragma once


class Player;

class Test : public IGameObject
{
public:
	Test() {}
	~Test() {}
	bool Start();
	void Update();
	void Render(RenderContext& rc);

private:
	nsBeastEngine::ModelRender m_modelRender;
	nsBeastEngine::SpriteRender m_spriteRender;
	nsBeastEngine::FontRender m_fontRender;
	Vector3 m_pos;

public:
	/// <summary>
	/// 空を初期化。
	/// </summary>
	void InitSky();

	nsBeastEngine::SkyCube* m_skyCube = nullptr;		//スカイキューブ。
	int m_skyCubeType = nsBeastEngine::enSkyCubeType_Day;
};

