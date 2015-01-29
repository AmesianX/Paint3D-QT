#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "DifferentialGeometry.h"

//ѡ�����ְ취�������εĽ���
//�������������A�ȽϿ�
#define ISECTMETHOD_A 1
#define ISECTMETHOD_B 2
#define INTERSECTION_METHOD ISECTMETHOD_A

class Triangle
{
public:
	Vector3 point1,point2,point3;
	Vector2 texCoord1,texCoord2,texCoord3;
	Vector3 normal1,normal2,normal3;
	unsigned int mtlId;

	Triangle(
		const Vector3&ipoint1,
		const Vector3&ipoint2,
		const Vector3&ipoint3,
		const Vector2&itexCoord1,
		const Vector2&itexCoord2,
		const Vector2&itexCoord3,
		const Vector3&inormal1,
		const Vector3&inormal2,
		const Vector3&inormal3,
		unsigned int mtlId
		);

	Triangle(void);
	virtual ~Triangle(void);

	float area();

	//����������������󽻵ĺ���
	//-----------------------
	//intersectTest�����ཻ��ʱ��Ĺ���tֵ,������ཻ��
	//���ع��ߵ�M_INF_BIG����ʾ������Զ���ཻ
	//���Ե�ʱ��ֻ���ⷵ��ֵ�Ƿ�С��tMax����
	//-----------------------
	//intersect�ٶ��Ѿ�����������߱��н��㣬
	//��������DifferentialGeometry
	//-----------------------
	//����������󽻵�ʱ����ѭ������intersectTest����
	//�õ��������������ཻ�Ķ���Σ��ٵ���intersect����
	//��������DifferentialGeometry,�����¹��ߵ�tMaxֵ
	//DifferentialGeometry�ڴ�����ʱ������ʼ��
	//Ȼ������intersect������ʼ����������
	float intersectTest(const Ray&r);
	void intersect(Ray&r,DifferentialGeometry&DG);
	//��uv������ñ���һ�㼰�䷨����
	//���ڵƹ�Ĳ���
	void getPoint(float u,float v,
		Vector3&position,Vector3&normal,Vector2&texCoord);

	//��openGL�л��������Σ�����ʱ��
	void draw(bool showNormal=false,bool fillMode=false);
	//������������ݣ�����ʱ��
	void showCoords();
	//��������ζ������ݣ�����ʱ��
	void showVertexCoords();
	//��������
	Vector3 getCentroid();
};


