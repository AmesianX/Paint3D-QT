#pragma once
#define UPDATE_REGION_INVALID_COORD 65535		// ���һ�����Ϸ��ľ��θ�������Ҳ���ǲ��ø��£�
#define TEX_DIM 1024
#define CANVAS_MAX_SIZE	4096					// һ��ͼ������ߴ�Ϊ CANVAS_MAX_SIZE * CANVAS_MAX_SIZE
// һ����ɫ�������شӸ�λ����λ�� ARGB ����
#define R_I2F(x)   ((x >>16) & 0xff) / 255.0f	
#define G_I2F(x)   ((x >> 8) & 0xff) / 255.0f
#define B_I2F(x)        ((x) & 0xff) / 255.0f
#define A_I2F(x)   ((x >>24) & 0xff) / 255.0f
// һ��surface�������شӸ�λ����λ�� IOR ���� �߹� ����  ����
#define RL_I2F(x)        ((x) & 0xff) / 255.0f
#define GL_I2F(x)   ((x >> 8) & 0xff) / 255.0f
#define RR_I2F(x)   ((x >>16) & 0xff) / 255.0f
#define ID_I2F(x)   ((x >>24) & 0xff) / 255.0f

#define INIT_COLOR_BITS 0x00ffffff
#define INIT_BACKBROUND_COLOR_BITS 0xffffffff
#define INIT_SURF_BITS  0x00ff0000
#define INIT_THICKNESS  0.0f


struct CanvasLayer
{
public:
	enum LayerChannel
	{
		CHNL_REFLECTION = 0,
		CHNL_GLOSSNESS  = 1,
		CHNL_REFRACTION = 2,
		CHNL_IOR        = 3,
		CHNL_COLOR      = 4,
		CHNL_THICKNESS  = 5
	};
	CanvasLayer(int width, int height);
	CanvasLayer(const CanvasLayer& layer);
	CanvasLayer();
	~CanvasLayer();

	inline float& getThickness(int x, int y)
	{return thickness[x + y * width];}

	inline unsigned& getColorPixel(int x, int y)
	{return ((unsigned*)colorImage.scanLine(y))[x];}
	
	inline unsigned& getSurfPixel(int x, int y)
	{return ((unsigned*)surfaceImage.scanLine(y))[x];}

	inline unsigned* getColorPointer()
	{return (unsigned*)colorImage.scanLine(0);}

	inline unsigned* getSurfPointer()
	{return (unsigned*)surfaceImage.scanLine(0);}

	inline float* getThicknessPointer()
	{ return thickness;	}


	void setLayerFromImage(const QImage& image, const LayerChannel channel);
	void saveLayer(const QString& fileName, const LayerChannel channel);
	// ͼ����,ָ��һ����������������������д�ŵ�����ֵ�����,���д����������
	// isContinuous = 1ʱ��������ֻ����Ҫ���µ����أ�Ҳ����˵�����µ��ڴ�������������
	// isContinuous = 0ʱ�������а�������ͼ�����أ�Ҳ����˵�����µ��ڴ治������������
	void blendRegion(int xmin, int ymin, 
		int regionWidth, int regionHeight, 
		unsigned* bufColor, unsigned* bufSurf, float* bufThick, 
		int isContinuous = 1);

	void blendRegion(int xmin, int ymin, 
		int regionWidth, int regionHeight, float* bufThick, 
		int isContinuous = 1);
	void blendRegion(int xmin, int ymin, int regionWidth, int regionHeight, PixelPos* updatePixels, unsigned updateCount, unsigned* bufColor, unsigned* bufSurf, float* bufThick, int isContinuous);
	void blendRegion(int xmin, int ymin, int regionWidth, int regionHeight, PixelPos*updatePixels, unsigned updateCount, float* bufThick, int isContinuous);
	void setRegion(int xmin, int ymin, 
		int regionWidth, int regionHeight, 
		unsigned* bufColor, unsigned* bufSurf, float* bufThick, 
		int isContinuous = 1);

	void setRegion(int xmin, int ymin, 
		int regionWidth, int regionHeight, 
		float* bufThick, 
		int isContinuous = 1);
	void setRegion(int xmin, int ymin, int regionWidth, int regionHeight, PixelPos*updatePixels, unsigned updateCount, unsigned* bufColor, unsigned* bufSurf, float * bufThick, int isContinuous);
	void setRegion(int xmin, int ymin, int regionWidth, int regionHeight, PixelPos*updatePixels, unsigned updateCount, float* bufThick, int isContinuous);
	void setVisible(bool isVisible){this->isVisible = isVisible;}
	void setName(const QString& name)
	{
		this->name = name;
	}
	const QString& getName(){return name;}
	CanvasLayer& operator=(const CanvasLayer& layer);

	friend QDataStream& operator<<(QDataStream& out, const QSharedPointer<CanvasLayer>& layer);
	friend QDataStream& operator>>(QDataStream& in, QSharedPointer<CanvasLayer>& layer);
private:
	void resetPixels();


	QString name;					// ͼ������
	bool isVisible;

	int width, height;				// �ֱ��ʳߴ�
	QImage colorImage, surfaceImage;// �洢���ظ�ʽ
	float* thickness;				// ������
};

class Canvas
{
public:
	Canvas();
	~Canvas(void);


	void init( 
		QVector<QVector3D>& vertices, 
		QVector<QVector3D>& normals, 
		QVector<QVector2D>& texcoords, 
		QVector<ObjTriangle>& faces);

	void release();

	void getResolution(int& width, int& height)
	{	width = this->width; height = this->height;}

	// ���ز���
	// ��д��ǰѡ�еĲ������ֵ,����ǰ��ȷ����ǰѡ���˲�
	inline QVector4D getCurLayerColorPixel(int x, int y)
	{	unsigned &c = curLayerColorPointer[y * width + x];	
	return QVector4D(R_I2F(c), G_I2F(c), B_I2F(c), A_I2F(c));}

	inline QVector4D getCurLayerSurfacePixel(int x, int y)
	{	unsigned &c = curLayerSurfacePointer[y * width + x];	
	return QVector4D(RL_I2F(c), GL_I2F(c), RR_I2F(c), ID_I2F(c));}

	inline float getCurLayerThicknessPixel(int x, int y)
	{return curLayerThicknessPointer[y * width + x];}

	inline void setCurLayerColorPixel(int x, int y, const QVector4D& pixel)
	{
		unsigned char c[4] = {pixel.z()*255, pixel.y()*255, pixel.x()*255, pixel.w()*255};
		curLayerColorPointer[y * width + x] = *((unsigned int*)c);
	    mergeUpdateRegion(x,y); isDirty = true;
	}
	inline void setCurLayerSurfacePixel(int x, int y, const QVector4D& pixel)
	{
		unsigned char c[4] = {pixel.x()*255, pixel.y()*255, pixel.z()*255, pixel.w()*255};
		curLayerSurfacePointer[y * width + x] = *((unsigned int*)c);
		mergeUpdateRegion(x,y);	isDirty = true;
	}

	inline void setCurLayerColorPixel(int x, int y, unsigned rgba)
	{
		curLayerColorPointer[y * width + x] = rgba;
		mergeUpdateRegion(x,y); isDirty = true;
	}
	inline void setCurLayerSurfacePixel(int x, int y, unsigned surf)
	{
		curLayerSurfacePointer[y * width + x] = surf;
		mergeUpdateRegion(x,y);	isDirty = true;
	}

	inline void setCurLayerThicknessPixel(int x, int y, float thickness)
	{
		curLayerThicknessPointer[y * width + x] = thickness;
		mergeUpdateRegion(x,y);	isDirty = true;
	}

	inline float getTotalThicknessPixel(int x, int y)
	{return totalThick[y * width + x];}
	inline QVector4D getTotalColorPixel(int x, int y)
	{	unsigned &c = totalColor[y * width + x];	
	return QVector4D(R_I2F(c), G_I2F(c), B_I2F(c), A_I2F(c));}

	inline QVector4D getTotalSurfacePixel(int x, int y)
	{	unsigned &c = totalSurf[y * width + x];	
	return QVector4D(RL_I2F(c), GL_I2F(c), RR_I2F(c), ID_I2F(c));}

	inline unsigned* getTriangleMaskImage(){return (unsigned*)triangleMaskImage.scanLine(0);}


	
	// ͼ�����
	QSharedPointer<CanvasLayer> generateLayer();	// ����һ����
	void insertLayer(QSharedPointer<CanvasLayer> pLayer, int ithLayer);
	bool removeLayer(QSharedPointer<CanvasLayer>& pLayer, int ithLayer);
	bool replaceLayer( QSharedPointer<CanvasLayer>& newLayer, QSharedPointer<CanvasLayer>& oldLayer,int ithLayer );
	bool setLayer( QSharedPointer<CanvasLayer>& newLayer,int ithLayer );
	bool moveUpLayer(int ithLayer);
	bool moveDownLayer(int ithLayer);
	void setCurLayerFromImage(const QImage& image, CanvasLayer::LayerChannel channel);
	void getLayerNames(QStringList& names);
	QSharedPointer<CanvasLayer> getCurLayer(){return layers[curSelectedLayer];}
	int  getNumLayers(){return layers.size();}
	int  getCurLayerID(){return curSelectedLayer;}
	void copyCurLayerColorArray(unsigned* colorArray);
	void copyCurLayerSurfaceArray(unsigned* surfArray);
	void copyCurLayerThicknessArray(float* thicknessArray);
	bool selectLayer(int ithLayer);
	void saveCanvas(const QString& fileName, const CanvasLayer::LayerChannel channel);

	// openGL����
	void updateGLTextures();		// ���޸ĺ��ͼ���͵�openGL
	GLint getGLColorTexObj(){return glColorTexObj;}
	GLint getGLSurfTexObj(){return glSurfTexObj;}
	GLint getGLThicknessTexObj(){return glThicknessTexObj;}
	GLint getGLBaseThicknessTexObj(){return glBaseThicknessTexObj;}

	// �ӷ������ز���
	CanvasSeamFiller& getSeamFiller(){return seamFiller;}


	friend QDataStream& operator<<(QDataStream& out, const Canvas& canvas);
	friend QDataStream& operator>>(QDataStream& in, Canvas& canvas);

	static const int maxCanvasSize;	// ����ͼ��ߴ�

private:
	void updateAll();
	void setUpdateRegionEmpty();
	void setUpdateRegionFull();
	void clearArray(unsigned* color, unsigned* surf, float* thick, int nPixels);
	void mergeUpdateRegion(int x, int y);;
	void clearRegion(int xmin, int ymin, int regionWidth, int regionHeight, PixelPos* updatePixels, unsigned updateCount, unsigned* bufColor, unsigned* bufSurf, float * bufThick, int isContinuous);

	int width, height;

	QVector<QSharedPointer<CanvasLayer>> layers;

	// ��¼��ǰѡ�еĲ��Լ���Ӧ��ָ��,ָ�벻��ҪԤ�ȷ���ռ�
	int curSelectedLayer;			
	unsigned int* curLayerColorPointer;
	unsigned int* curLayerSurfacePointer;
	float *       curLayerThicknessPointer;

	// ��¼ͼ�������Ч�������ݣ�ָ����ҪԤ�ȷ���ռ�
	float * curLayerBaseThickness;	// ��ǰͼ�����µ�ͼ����ܺ��

	unsigned int* totalColor;
	unsigned int* totalSurf;
	float*        totalThick;

	// ��¼Ҫ���µ�����λ��
	PixelPos* updatePixelSet;
	int* updatePixelMask;
	unsigned updatePixelCount;
	int updateRegion[2][2];			// �����Ҫ�������� {{xmin,ymin},{xmax,ymax}}
	bool isDirty;					// ��ʾ�Ƿ����û�и��µ�opengl���������

	// ���ٽӷ�Ӱ��Ķ���
	QImage triangleMaskImage;		// ��¼�����θ������ص�ͼƬ
	CanvasSeamFiller seamFiller;	// ��������ѷ�

	// opengl �������
	GLuint glColorTexObj;			
	GLuint glSurfTexObj;			
	GLuint glThicknessTexObj;
	GLuint glBaseThicknessTexObj;
};

