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
};

