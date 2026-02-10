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
	Vector3 m_pos;
};

