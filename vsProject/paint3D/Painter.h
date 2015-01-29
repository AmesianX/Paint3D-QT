#pragma once
#define SAMPLE_PIXEL_INTERVAL 1.f		// ÿ�����ٸ����ز���һ�� 
#define BRUSH_CAMERA_DISTANCE 0.05f		// ��������������ľ���
// ��ˢ�����������������ת���ɱ�ˢ��ʵ��λ��
class Painter
{
public:
	Painter(Scene* s);;
	~Painter(){}

	// ���û����Ķ���
	void setObject(QWeakPointer<Mesh> obj){curObj = obj;}
	// ���ñ�ˢ
	void setBrush(QWeakPointer<Brush> b){curBrush = b;}

	// ������������������ʱ������ beginPaint
	// ����϶�ʱ������ goOnPaint
	// �������ɿ�ʱ������ endPaint
	// Ϊ��ʹ��ˢ���������λ�ý��в�ֵ
	virtual void beginPaint(const QVector2D& beginPos) = 0;
	virtual void goOnPaint(const QVector2D& newPos) = 0;
	virtual void endPaint(const QVector2D& endPos) = 0;

	virtual void onMouseHover( const QVector2D& newPos ) = 0;

protected:
	bool getWorldPos(const QVector2D& ratio, QVector3D& pos);

	bool isPainting;
	QVector2D lastScreenPos;	// ��һ���ƶ���ˢ����Ļλ��
	float paintLength;			// �������ߵ��ܳ�
	float deltaLength;			// ��һ�λ��º󻭱��־����Ļ���

	QWeakPointer<Mesh> curObj;
	QWeakPointer<Brush> curBrush;
	Scene* scene;
};

// ��ѻ��ʷ������������
class SurfacePainter:public Painter
{
public:
	SurfacePainter(Scene* s);
	~SurfacePainter(){}

	void beginPaint( const QVector2D& beginPos );
	void goOnPaint(const QVector2D& newPos);
	void endPaint(const QVector2D& endPos);

	void onMouseHover( const QVector2D& newPos );
protected:
	//QVector3D lastWorldPos;
};