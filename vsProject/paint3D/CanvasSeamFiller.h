#pragma once

// ����ṹ��¼��obj�ļ���ȡ�õ�������
struct ObjTriangle
{
	unsigned int vertIndex[3];			//vertex index
	unsigned int texcoordIndex[3];		//texture coordinate index
	unsigned int norIndex[3];			//normal index
	unsigned int mtlIndex;				//material index
	friend QDataStream& operator<<(QDataStream& out, const ObjTriangle&tri);
	friend QDataStream& operator<<(QDataStream& in, ObjTriangle&tri);
};

// ����ṹ���������ε�һ����
struct EdgePointID
{
	EdgePointID(){v1ID = v2ID = -1;}
	int v1ID, v2ID;						// �������������
	bool operator==(const EdgePointID& e)const
	{
		return (v1ID == e.v1ID && v2ID == e.v2ID) ||
			   (v1ID == e.v2ID && v2ID == e.v1ID);
	}
};

inline uint qHash(const EdgePointID &key)
{
	return uint(key.v1ID ^ key.v2ID);
}

// ��¼һ��������λ�õĽṹ
struct EdgeTriID
{
	unsigned faceID, edgeID;
};
struct TexcoordVertex
{
	TexcoordVertex(){}
	TexcoordVertex(const QVector2D& texcoord)
	{
		this->vertex = texcoord;
	}
	QVector2D vertex;
	QVector2D normal;
	QVector<unsigned> enterEdgeID;			// ��ڱߵ�ID,һ����������������������
	QVector<unsigned> exitEdgeID;			// ���ڱߵ�ID
};
// ����λ��
struct PixelPos
{
	PixelPos(){}
	PixelPos(short x, short y)
	{
		this->x = x;
		this->y = y;
	}
	bool operator==(const PixelPos& pixel)const
	{return x == pixel.x && y == pixel.y;}
	short x, y;
};
inline uint qHash(const PixelPos& pixel)
{
	return (pixel.y << 16) | pixel.x; 
}
// ����ฺ���ı��β�ֵ
class CoordMapper
{
public:
	CoordMapper(){}
	void setSrcShape(
		const QVector2D& srcO1,
        const QVector2D& srcO2,
		const QVector2D& srcV1,
		const QVector2D& srcV2, 
		const QVector2D& edgeV);

	void setDstShape(
		const QVector2D& dstO1,
		const QVector2D& dstO2,
		const QVector2D& dstV1,
		const QVector2D& dstV2, 
		const QVector2D& edgeV);

	void getSrcQuad(QVector2D& p1, QVector2D& p2, 
		            QVector2D& p3, QVector2D& p4)const;
	void getDstQuad(QVector2D& p1, QVector2D& p2, 
					QVector2D& p3, QVector2D& p4)const;
	// �����ı��β�ֵ
	bool convert( const QVector2D& srcPoint, QVector2D& dstPoint )const;
private:
	// �����ı��ε���״
	QVector2D srcO, dstO,
		      srcU, dstU,
			  srcV1, dstV1,
			  srcV21, dstV21;
	float A,B,a,b1;			// Ԥ���������
};
struct TexcoordEdge
{
	TexcoordEdge(){commonEdgeID = -1;}
	TexcoordEdge(const unsigned startVtxID,
		         const unsigned endVtxID,
				 const unsigned faceID,
				 const unsigned edgeID,
				 const QVector2D& normal)
	{
		this->startVtxID = startVtxID;
		this->endVtxID = endVtxID;
		this->commonEdgeID = -1;
		this->normal = normal;
		this->faceID = faceID;
		this->edgeID = edgeID;
	}
	unsigned startVtxID, endVtxID;				// ��ʾ���������ӵĶ���ID������ߵķ���Ϊ��startVtx��endVtx
	unsigned commonEdgeID;						// ��ʾ������ռ��У��������߶�Ӧ�ıߵ�ID,û����Ϊ-1��
												// ����һ���ߵķ����෴��
												// ��this->startVtxID == commonEdge.endVtxID,
												// ������λ��Ϊ1��������Ϊ0
	unsigned faceID;							// ������ID
	unsigned edgeID;							// ���������ε�λ��
	QVector2D normal;							// �߷���
	CoordMapper mapper;							// ������ת������֮��Ӧ���ı�����
};
class CanvasSeamFiller
{
public:
	CanvasSeamFiller(void);
	~CanvasSeamFiller(void);
	void init(int width, int height,
		QVector<QVector3D>& vertices,
		QVector<QVector2D>& texcoords,
		QVector<ObjTriangle>& faces);

	QVector<PixelPos>& getExtraFillPixels(int x, int y);

private:
	// ������ռ�Ĺ�������ȡ����
	void extractCommonPositionEdge( 
		const QVector<ObjTriangle>& faces, 
		const QVector<QVector2D>texcoords, 
		QHash<EdgePointID, EdgePointID>& edgePointMap );
	// ������ռ�ı߽����ȡ����
	void extractBorderTexcoordEdge( 
		const QVector<ObjTriangle>& faces, 
		const QVector<QVector2D>texcoords, 
		QMultiHash<EdgePointID, EdgeTriID>& edgeMap );
	// ��������ռ��еı߽�ߺͶ�������
	void buildTexcoordEdge( 
		const QVector<QVector2D>& texcoords, 
		const QVector<ObjTriangle>& faces, 
		const QMultiHash<EdgePointID, EdgeTriID>& edgeMap, 
		const QHash<EdgePointID, EdgePointID>& edgeCommonPointMap, 
		QVector<TexcoordEdge>& edgeArray, 
		QVector<TexcoordVertex>& vertexArray, 
		QVector<int>& edgeSetArray );
	
	// �ֿ����������ϱ߹���Ķ���
	void splitCommonVertex( QVector<TexcoordEdge>& edgeArray, QVector<TexcoordVertex>& vertexArray);

	// �ѱ���������ͬʱ��˳startVtx ͬ endVtx,�����㶥�㷨��
	void linkTexcoordEdge( QVector<TexcoordEdge>& edgeArray, QVector<TexcoordVertex>& vertexArray, QVector<int>& edgeSetArray);
	// �����ı��β�ֵ��
	void buildCoordMapper( QVector<TexcoordEdge>& edgeArray, QVector<TexcoordVertex>& vertexArray);
	// ��������ͼ��
	void drawOutlineImage(const QVector<TexcoordEdge>& texEdgeArray, const QVector<TexcoordVertex>& texVtxArray, const QVector<QVector2D>& texcoords, const QVector<ObjTriangle>& faces, QImage& triangleMaskImage);
	// ���������������
	void buildPixelLookupTable(const QImage& triangleMaskImage, const QVector<TexcoordEdge>& texEdgeArray);
	float fillWidth;
	int width, height;
	QVector<int> pixelMap;
	QVector<QVector<PixelPos>> fillIDMap;

};
