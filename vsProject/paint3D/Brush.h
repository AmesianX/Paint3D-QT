#pragma once
#define BRUSH_CAM_NEAR_PLANE -1.0f
#define BRUSH_CAM_FAR_PLANE  10.0f
#define BRUSH_DRAW_SEGMENTS  16
#define BRUSH_UPDATE_INIT_BUFSIZE	128 * 128	// һ�������µ����أ�����Ϊ��������С��1/16��
#define BRUSH_PIXEL_BLEND_THRESHOLD	0.0001	// Ȩ�ش������ֵ�����زſ�ʼ���
#define BRUSH_BLEND_CURVE_SEGMENTS  50
#define BRUSH_MAX_GE_BUFFERS	5		// �洢��geometry exposer���������

#define TRIPROJ_PIXEL_RATIO		2.0f	// Ӱ�췶Χռ������ˢ����
#define TRIPROJ_WEIGHT_RATIO	0.1f	// Ȩ��ϵ��
class Scene;
// ������¼�����α����һЩ���ݣ����������������
class TriangleProjData
{
public:
	// ����һЩ�ڲ��������������귶Χ���Լ�һ���ݴ����
	inline void computeParam()		
	{
		// ������������UV�ռ�İ�Χ��,���귶Χ��Ӧ�ó��������Χ��
		QVector2D* t = triData.texCoord;
		float triUVBBox[2][2] = {{t[0].x(),t[0].y()},{t[0].x(),t[0].y()}};
		triUVBBox[0][0] = min(min(triUVBBox[0][0], t[1].x()), t[2].x());
		triUVBBox[0][1] = min(min(triUVBBox[0][1], t[1].y()), t[2].y());
		triUVBBox[1][0] = max(max(triUVBBox[1][0], t[1].x()), t[2].x());
		triUVBBox[1][1] = max(max(triUVBBox[1][1], t[1].y()), t[2].y());

		float pixelCRx = sqrt(pixelCoordInUVSpace[0][0] * pixelCoordInUVSpace[0][0] + 
								   pixelCoordInUVSpace[1][0] * pixelCoordInUVSpace[1][0]) * TRIPROJ_PIXEL_RATIO / 2;
		float pixelCRy = sqrt(pixelCoordInUVSpace[0][1] * pixelCoordInUVSpace[0][1] + 
								   pixelCoordInUVSpace[1][1] * pixelCoordInUVSpace[1][1]) * TRIPROJ_PIXEL_RATIO / 2;

		uvCoordRange[0] = min(pixelCRx, triUVBBox[1][0] - triUVBBox[0][0]);
		uvCoordRange[1] = min(pixelCRx, triUVBBox[1][1] - triUVBBox[0][1]);

		float deno = pixelCoordInUVSpace[0][0] * pixelCoordInUVSpace[1][1] - 
					 pixelCoordInUVSpace[0][1] * pixelCoordInUVSpace[1][0];
		invPixelCoordDet = 1.0f / (deno * deno * TRIPROJ_WEIGHT_RATIO * TRIPROJ_WEIGHT_RATIO);
	}

	// ������귶Χ
	QVector2D getCoordRange(){return QVector2D(uvCoordRange[0], uvCoordRange[1]);}

	// ��������ռ�һ���������������ϵ����������ռ��ԭ������꣬����������������ϵ�е�Ȩ��
	inline float getWeightInEWA(float deltaCoordX, float deltaCoordY)
	{
		float px = deltaCoordX * pixelCoordInUVSpace[1][1] - deltaCoordY * pixelCoordInUVSpace[1][0];
		float py = deltaCoordX * pixelCoordInUVSpace[0][1] - deltaCoordY * pixelCoordInUVSpace[0][0];
		float exponent = (px * px + py * py) * invPixelCoordDet + 1.0f;
// 		int isBigger = exponent > 29.0;
// 		return expTable[int(exponent) * (1 - isBigger) + 29 * isBigger]; 
		return exponent / (exponent * exponent + 1);
	}

	int objID, faceID;
	TriangleData triData;					// �����εļ�����Ϣ
	float pixelCoordInUVSpace[2][2];		// ӳ�䵽��������ռ����������ϵ
	QMatrix4x4 modelMatrix;					// ����ı任����

	float uvCoordRange[2];					// ���귶ΧuvC(oord)R(ange),����������ռ�Ӱ���uv��Χ
											// ����Ӱ��ķ�ΧΪ[-uvCR.x, uvCR.x],[-uvCR.y,uvCR.y]
	float invPixelCoordDet;					// ����ʱ�ķ�ĸ�� ���� 1 / (det ^2 * TRIPROJ_WEIGHT_RATIO)

	static float expTable[30];				// ���� exp(-x) �Ĳ��ұ�
};



struct PixelUpdateItem 
{
	QVector4D rgba;						// ��ɫ����
	QVector4D surf;						// ��������
	float thickness;					// ���
	float weight;						// Ȩ��
	unsigned x,y;						// ����λ��
	float norProj, depth;				// ����ͶӰ�����
};

struct PixelUpdateArray
{
public:
	PixelUpdateArray()
	{
		curUpdateCount = 0;
		curArraySize = BRUSH_UPDATE_INIT_BUFSIZE;
		int bitmapSize = Canvas::maxCanvasSize * Canvas::maxCanvasSize;
		updateBitmap = new int[bitmapSize];
		memset(updateBitmap, 0xff, bitmapSize * sizeof(int));
		updateArray = new PixelUpdateItem[curArraySize];
	}
	~PixelUpdateArray()
	{
		delete[] updateBitmap;
		delete[] updateArray;
	}
	inline void clearBitmapItem(int ithItem)
	{	updateBitmap[ithItem] = 0xffffffff;	}

	void reset(){curUpdateCount=0;}

	inline int  getBitmapItem(int ithItem)
	{return updateBitmap[ithItem];}

	inline PixelUpdateItem& getUpdateItem(int ithItem)
	{
		return updateArray[ithItem];
	}

	inline int getUpdatePixelCount(){return curUpdateCount;}

	void insertItem(int ithItem, const PixelUpdateItem& item)
	{
		if (curUpdateCount >= curArraySize)
		{
			// ���·���ռ�
			curArraySize *= 2;
			PixelUpdateItem* newArray = new PixelUpdateItem[curArraySize];
			memcpy(newArray,  updateArray,  curUpdateCount * sizeof(PixelUpdateItem));
			delete[] updateArray;
			updateArray  = newArray;
			qDebug() << "cur array size: " 
				<< curArraySize * sizeof(PixelUpdateItem) / 1024.0f / 1024.0f  << "MB"<< endl;
		}
		if (updateBitmap[ithItem] == -1)
		{
			// ��������µ����أ������ظ��±��в���һ��
			updateBitmap[ithItem] = curUpdateCount;
			updateArray[curUpdateCount++] = item;
		}
		else
		{
			// ���򣬼��뵽ԭ����Ӧ��������
			PixelUpdateItem& curItem = updateArray[updateBitmap[ithItem]];
			curItem.rgba += item.rgba;
			curItem.surf += item.surf;
			curItem.thickness += item.thickness;
			curItem.weight += item.weight;
			curItem.norProj += item.norProj;
			curItem.depth += item.depth;
		}
	}
private:
	int* updateBitmap;				// ����������ݴ����λ�õ�λͼ
	PixelUpdateItem* updateArray;	// ��¼���ظ���λ�õ�λͼ
	int curArraySize;					// ��ǰ���仺������С
	int curUpdateCount;				// ��ǰҪ���µ����ظ���
};



class Brush
{
	friend class PaintThread;
public:
	enum BrushMode{MODE_BRUSH, MODE_ERASER};
	Brush(Scene* scene);
	~Brush();

	void setMode(BrushMode mode);
	void setSize(float size);
	void setResolutionRatio(float ratio);
	void setAngle(float angle){this->angle = angle;}
	void setRatio(float ratio){this->ratio = ratio;}
	void setSizeJitter(float sizeJitter){this->sizeJitter = sizeJitter;}
	void setAngleJitter(float angleJitter){this->angleJitter = angleJitter;}
	void setRatioJitter(float ratioJitter){this->ratioJitter = ratioJitter;}
	void setOffsetJitter(float offsetJitter){this->offsetJitter = offsetJitter;}
	void setDepthRange(float nearPlane, float farPlane)
	{ this->nearPlane = nearPlane; this->farPlane = farPlane;	}

	QSharedPointer<Stroke> getStroke(){return stroke;}
	void setStroke(QSharedPointer<Stroke> stroke){this->stroke = stroke;}


	void activate(bool isActive){this->isActive = isActive;}

	// �滭ǰ���ô˺�����׼����ʷ��¼
	void beginPaint();
	// ���Ҫ���ı�ˢλ�ã������ɺ󣬵���worldPaintͳһ������
	void addPaintPos(const QVector3D& ori, const QVector3D& x, const QVector3D& y, const QVector3D& z);
	// worldPaint 
	// ������ռ��ϻ��������ʿ��ܸ�����������棬����ĳ��Լ������
	// ��������£���Ҫ���ڷ��õ�λ�ð�������Ⱦһ�Σ��õ����漸����Ϣ��
	// �ٰ�����ӳ�䵽����ռ䡣
	void worldPaint();
	// �滭����ô˺�����������ʷ��¼
	void endPaint();;
	// ���û���������ռ��λ��
	void setPosWorld(const QVector3D& ori, const QVector3D& x, const QVector3D& y, const QVector3D& z);

	// ����Ļ�ռ��ϻ��������ʱ�Լ������Ļ�ռ䣬���θ���ά��ͼ����
	// ��������£�����Ҫ����Ⱦһ�Σ�����ͨ����ȡ����������Ϣ��������ӳ
	// �䵽��������ռ䡣
	// �������Ϊ��λ������Ļ���꣨ע����Ļ����ϵԭ�������Ͻǣ�
	void setPosScreen(const QVector2D& screenRatio, const QVector2D &dir){}
	// ���û�������Ļ�ռ��λ��
	void screenPaint(){}

	// ��ʾ��ˢ
	void draw();

	PurePaintPicker& getPurePaintPicker(){return *(purePaintPicker.data());}
	PaintPicker*     getCurPaintPicker(){return curPaintPicker.data();}
	// ��õ�ǰ���ڷ��û��ʵĶ���ID
	// ����ǰ���ʲ��ô�ɫ�������IDΪҪ���������ID�������ʸ�����������滭
	// ���������ø��ּ��������PaintPicker������IDΪ��PaintPicker��Mesh�����ID
	int   getPlacerObjectID();		

	// ���Ʋ���
	float getInterval()const{return intervalRatio * size;}
	void  setIntervalRatio(float i){intervalRatio = i;}
	void  setObject(QWeakPointer<Mesh> obj);
	void  setColor(const QVector4D& colorParam, const QVector4D& surfParam, const float depth);
	void  setDefaultPaintPicker();
	void setCustomPaintPicker( QSharedPointer<PaintPicker> picker );
	// �趨�Ƿ�Ӱ��ĳ��ͨ��
	void  setAffectColor(int isAffect);
	void  setAffectReflLevel(int isAffect);
	void  setAffectRefrLevel(int isAffect);
	void  setAffectGlossness(int isAffect);
	void  setAffectIOR(int isAffect);
	void  setAffectThickness(int isAffect);

	// ����/��ȵ���
	void  setNormalBlendCurveCoef(float zeroPos, float onePos, float zeroWeight, float oneWeight);
	void  setDepthBlendCurveCoef(float zeroPos, float onePos, float zeroWeight, float oneWeight);

	static void computeBlendCurve(float* curve, int nSegs, float zeroPos, float onePos, float zeroWeight, float oneWeight);
	void setDepthAffectMode(GeometryExposer::DepthType mode);

public:
	bool  getAffectColor(){return isAffectColor == 1.0f;}
	bool  getAffectReflLevel(){return isAffectReflLevel == 1.0f;}
	bool  getAffectRefrLevel(){return isAffectRefrLevel == 1.0f;}
	bool  getAffectGlossness(){return isAffectGlossness == 1.0f;}
	bool  getAffectIOR(){return isAffectIOR == 1.0f;}
	bool  getAffectThickness(){return isAffectThickness == 1.0f;}

private:
	void setResolution(int x, int y);

	// ���������������ֵ�����µ�ǰ������
	void updateCurParams();
	void paintToObject(int ithBuffer);// �����ػ���ģ����
	TriangleProjData getTriangleData( int triID, int objID);
	// �����һ������ӳ�䵽����������ռ������
	bool computeTriangleProj(TriangleProjData& data);

	inline float getRandomFloat_01()
	{randInt = randInt * 32773 + 239664539; return float(randInt) / 4294967295.0f;}
	inline float getRandomFloat_m11()
	{ randInt = randInt * 32773 + 239664539; return float(randInt) / 2147483647.5 - 1.0;}

	// ---------------------------------------------------------
	// �������Կ����ڽ��������ã����ٴα�����֮ǰ��ֵ����仯
	// ---------------------------------------------------------
	QWeakPointer<Mesh> curObj;				// ��ǰ���ƵĶ���
	QSharedPointer<PurePaintPicker> purePaintPicker;		// �洢��ˢ��ɫ
	QWeakPointer<PaintPicker> curPaintPicker;			// ��ǰʹ�õ�ʰɫ��
	bool isActive;							// �Ƿ�ɼ�
	int  resolution[2];						// �ֱ���
	float resRatio;							// ��С��ֱ��ʵı���
	float intervalRatio;					// ���Ƽ��
	float nearPlane;						// ������ƽ��
	float farPlane;							// Զ����ƽ��

	float size;								// ��С
	float angle;							// �Ƕ�
	float ratio;							// �߿�ȣ���/��

	// һЩ����ֵ����ΧΪ0-1��
	// ����ʵ�ʲ���ʱ���� ��1 + jitter * rnd�� * param����
	float sizeJitter;						// ��С����
	float angleJitter;						// �Ƕȶ���
	float offsetJitter;						// ƫ�ƶ���
	float ratioJitter;						// �߿�ȶ���

	float isAffectColor;					// �Ƿ�Ӱ���Ӧ����
	float isAffectReflLevel;
	float isAffectRefrLevel;
	float isAffectGlossness;
	float isAffectIOR;
	float isAffectThickness;

	float isBrushMode;						// �Ƿ��ˢģʽ������Ϊ��Ƥ��ģʽ

	QSharedPointer<Stroke> stroke;			// �ʴ�

	// �滭�������õ��Ļ�Ϻ����Ĳ��ұ�
	float normalBlendCurve[BRUSH_BLEND_CURVE_SEGMENTS];
	float depthBlendCurve[BRUSH_BLEND_CURVE_SEGMENTS];

	// ---------------------------------------------------------
	// �������Կ��ܻ���ÿ�λ���ǰ���£���������ָ�����ֵ��ʱ��
	// ---------------------------------------------------------
	unsigned int randInt;					// �����
	float curAngle;							// ��ת�Ƕ�
	QVector2D curSize;						// ��С������size�͸߿�������
	float curOffset[2];						// ƫ����
	QVector3D paintLocalCoord[3];			// ���ڻ����ľֲ�����
	QVector3D paintOrigin;					// ���ڻ���������ԭ��
	QMatrix4x4 viewMatrix, projMatrix;		// �۲������paintLocalCoord��paintOrigin���ɣ�

	// ---------------------------------------------------------
	// �����������ڻ���ʱ���߳�ͬ��
	// ---------------------------------------------------------
	struct PaintPosition
	{
		QVector3D ori;
		QVector3D localCoord[3];
		QVector3D paintOri;
		QVector3D paintLocalCoord[3];
	};
	struct GeometryInfoBuffer
	{
		GeometryExposer::NormalTexPixelData* norArray;
		GeometryExposer::UVTexPixelData* uvArray;
	};
	// ��Ҫ���л��Ƶ�λ��
	QVector<PaintPosition> paintPos;	
	// ����Ϊ�����̼߳�ͨ�ŵı���
	QSemaphore* emptySlots, *fullSlots;
	QMutex paintMutex;
	// ���GeometryExposer���������
	GeometryInfoBuffer geoInfoBuf[BRUSH_MAX_GE_BUFFERS];


	// ---------------------------------------------------------
	// ��������������ʷ��¼
	// ---------------------------------------------------------
	// ����֮ǰ��ͼ������
	QVector<unsigned> oldColorBuf, oldSurfBuf;
	QVector<float> oldThicknessBuf;
	// ---------------------------------------------------------
	// �������������ÿ���ƶ�ʱ�����
	// ---------------------------------------------------------
	QVector3D dispLocalCoord[3];			// ������ʾ��ˢͼ��ľֲ�����ϵ
	QVector3D dispOrigin;					// ������ʾ��ˢͼ�������ԭ��

	// ---------------------------------------------------------
	// ��������ɽ�������
	// ---------------------------------------------------------
	GeometryExposer  brushExposer;			// ����ռ仭��ʱ����ó�����������Ϣ

	// ---------------------------------------------------------
	// ��������
	// ---------------------------------------------------------
	Scene* scene;							// ����
	GeometryExposer* sceneExposer;			// ��¼������ǰ״��
	Camera* sceneCamera;					// ���������

	// ---------------------------------------------------------
	// ����Ϊ�滭�������õ�����ʱ����������Ƶ�������ͷſռ�ʮ��ռʱ�䣬��������Ϊ��̬����
	// ---------------------------------------------------------
	static PixelUpdateArray updateArray;
};

class PaintThread : public QThread
{
	Q_OBJECT
public:
	PaintThread(Brush* brush, QObject *parent = 0);
	~PaintThread(){};

protected:
	void run();
private:
	Brush* brush;
};

