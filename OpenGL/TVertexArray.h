#pragma once
#include <initializer_list>

class TVertexArray
{
protected:
	unsigned int VAO;
	unsigned int VBO;

	//protected���캯��������������Ĺ��캯���У�
	//�ȶ������ݣ��ٵ���this->TVertexArray::TVertexArray(...)����
	TVertexArray() {}
private:
	unsigned int bytes;
	int group_count;

	//�������Ϊx,y,z,r,g,b,s,t
	//����{3,3,2}��Ӧlocation={0,1,2}
	void SetAttribPointer(std::initializer_list<int> init_list);
	virtual void Draw(unsigned int mode);
public:
	TVertexArray(unsigned int bytes, const float* data, std::initializer_list<int> init_list);
	virtual ~TVertexArray();

	virtual void Bind();

	//����ǰ��Bind�����򱨴�
	void DrawTriangles();
	void DrawLines();
	void DrawQuads();
};

