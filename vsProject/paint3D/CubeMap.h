#pragma once

class CubeMap
{
public:
	CubeMap(void);
	~CubeMap(void);
	void load(const QString& fileName,
		int cubeMapSize);
	void initGLBuffer();
	GLuint getGLTexObj(){return glCubeMapID;}
	void releaseGLBuffer();
	void saveCubeMap(const QString& fileName);
private:
	// ֱ������ת������
	inline void cartesian2Spherical(const QVector3D& car, float& alpha, float& beta);

	int cubeMapSize;
	// ���ļ������ͼƬ
	QImage srcImg;
	// ��ͼƬת������������ͼ��6����
	QImage cubeImgs[6];

	static const GLuint glCubeMapTable[];
	GLuint glCubeMapID;

};
