#include "StdAfx.h"
#include "CanvasSeamFiller.h"

CanvasSeamFiller::CanvasSeamFiller(void)
{
	fillWidth = 0.01;
}

CanvasSeamFiller::~CanvasSeamFiller(void)
{
}

void CanvasSeamFiller::init( int width, int height, QVector<QVector3D>& vertices, QVector<QVector2D>& texcoords, QVector<ObjTriangle>& faces )
{
	this->width = width;
	this->height = height;

	QMultiHash<EdgePointID, EdgeTriID> borderTexcoordEdge;
	QHash<EdgePointID, EdgePointID> commonPosEdgeMap;

	extractCommonPositionEdge(faces, texcoords, commonPosEdgeMap);
	extractBorderTexcoordEdge(faces, texcoords, borderTexcoordEdge);
	
	QVector<TexcoordEdge> texEdgeArray;
	QVector<TexcoordVertex> texVtxArray;
	QVector<int> texEdgeSetArray;
	// �����߼���
	buildTexcoordEdge(
		texcoords, faces, borderTexcoordEdge, commonPosEdgeMap,
		texEdgeArray, texVtxArray, texEdgeSetArray );

	// ��䵽һ�ŷֱ��ʵ�����ͼ��С��λͼ��
	QImage triangleMaskImage = QImage(width, height,QImage::Format_ARGB32);
	triangleMaskImage.fill(0x0);
	QPainter painter(&triangleMaskImage);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	for (unsigned ithEdge = 0; ithEdge < texEdgeArray.size(); ++ithEdge)
	{
		QRgb edgeColor = ithEdge;
		painter.setPen(QPen(QColor(edgeColor)));
		painter.setBrush(QBrush(QColor(edgeColor)));
		TexcoordEdge& edge = texEdgeArray[ithEdge];
		TexcoordVertex& v1 = texVtxArray[edge.startVtxID];
		TexcoordVertex& v2 = texVtxArray[edge.endVtxID];
		QVector2D v1n = v1.vertex + v1.normal;
		QVector2D v2n = v2.vertex + v2.normal;
		QPointF points[4] = 
		{
			QPointF(v1.vertex.x() * width, (1-v1.vertex.y()) * height),
			QPointF(v1n.x() * width, (1-v1n.y()) * height),
			QPointF(v2n.x() * width, (1-v2n.y()) * height),
			QPointF(v2.vertex.x() * width, (1-v2.vertex.y()) * height)
		};
		painter.drawPolygon(points, 4);
	}
	painter.setPen(QPen(QColor(0,0,0,0)));
	painter.setBrush(QBrush(QColor(0,0,0,0)));
	for (int ithTri = 0; ithTri < faces.size(); ++ithTri)
	{
		QVector2D& t1 = texcoords[faces[ithTri].texcoordIndex[0]];
		QVector2D& t2 = texcoords[faces[ithTri].texcoordIndex[1]];
		QVector2D& t3 = texcoords[faces[ithTri].texcoordIndex[2]];

		QPointF points[3] = 
		{
			QPointF(t1.x() * width, (1-t1.y()) * height),
			QPointF(t2.x() * width, (1-t2.y()) * height),
			QPointF(t3.x() * width, (1-t3.y()) * height)
		};
		painter.drawPolygon(points, 3);
	}
	triangleMaskImage.save("triangleMask.png");
/*
	// �������ز�������
	fillIDMap.push_back(QVector<PixelPos>());				// ����һ��������
	pixelMap.insert(0,width * height, 0);	// Ĭ���������ض�ָ�����������
	const unsigned* pPixel = (unsigned*)triangleMaskImage.constBits();
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x,++pPixel)
		{
			// �����ڱ߽總�����ұ����
			if (*pPixel != 0)
			{
				QVector2D uvSrcCoord(float(x) / (width  - 1),
					                 float(y) / (height - 1)), uvDstCoord;
				TexcoordEdge& edge = texEdgeArray[*pPixel];
				
				// �ҳ���ɫ��䵽������ص�ԭ����λ��
				if (edge.mapper.convert(uvSrcCoord, uvDstCoord))
				{
					int dstX = uvDstCoord.x() * (width - 1);
					int dstY = uvDstCoord.y() * (height - 1);
					if (dstX >= 0 && dstX < width &&
						dstY >= 0 && dstY < height)
					{
						int& pixelID = pixelMap[dstY * width + dstX];
						if (pixelID == 0)
						{	// ����һ���µ�
							pixelID = fillIDMap.size();
							fillIDMap.push_back(QVector<PixelPos>(1,PixelPos(x, y)));
						}
						else
						{
							fillIDMap[pixelID].push_back(PixelPos(x, y));
						}
					}
				}
				
			}
		}
	}*/
}

void CanvasSeamFiller::extractCommonPositionEdge( const QVector<ObjTriangle>& faces, const QVector<QVector2D>texcoords, QHash<EdgePointID, EdgePointID>& edgePointMap )
{
	QMultiHash<EdgePointID, EdgeTriID> edgeMap;
	// �����б��Զ���IDΪkey�ӵ�HashMap
	for (int ithTri = 0; ithTri < faces.size(); ++ithTri)
	{
		const ObjTriangle& tri = faces[ithTri];
		for (int ithEdge = 0; ithEdge < 3; ++ithEdge)
		{
			EdgePointID triEdge;
			triEdge.v1ID = tri.vertIndex[ithEdge];
			triEdge.v2ID = tri.vertIndex[(ithEdge + 1) % 3];

			EdgeTriID edgePos;
			edgePos.edgeID = ithEdge;
			edgePos.faceID = ithTri;
			edgeMap.insertMulti(triEdge, edgePos);
		}
	}

	// �ҳ�������ռ��б����������ι���
	// ��������ռ������ڱ߽�ı�
	// ����������ռ��е�һ�鶥�������ӵ�һ��edgeMap����
	for (QMultiHash<EdgePointID, EdgeTriID>::iterator pCurEdge = edgeMap.begin(),pNextEdge = pCurEdge + 1;
		 pNextEdge != edgeMap.end();++pCurEdge, ++pNextEdge)
	{
		// ������ռ����ǹ�����
		if(pNextEdge.key() == pCurEdge.key())
		{
			const EdgeTriID& curEdgeTriID = pCurEdge.value();
			const EdgeTriID& nextEdgeTriID = pNextEdge.value();

			// �ҳ������������ߵ���ͼ���궥�����
			EdgePointID curPointID, nextPointID;
			curPointID.v1ID = faces[curEdgeTriID.faceID].texcoordIndex[curEdgeTriID.edgeID];
			curPointID.v2ID = faces[curEdgeTriID.faceID].texcoordIndex[(curEdgeTriID.edgeID+1)%3];
			nextPointID.v1ID = faces[nextEdgeTriID.faceID].texcoordIndex[nextEdgeTriID.edgeID];
			nextPointID.v2ID = faces[nextEdgeTriID.faceID].texcoordIndex[(nextEdgeTriID.edgeID+1)%3];

			// ������㲻һ�£�˵����������������ռ䲻����
			if (!(curPointID == nextPointID))
			{

				// �����ߵ�˳��ʹ��������������ʱ�뷽��
				QVector2D v1  = texcoords[faces[curEdgeTriID.faceID].texcoordIndex[0]];
				QVector2D v21 = texcoords[faces[curEdgeTriID.faceID].texcoordIndex[1]] - v1;
				QVector2D v31 = texcoords[faces[curEdgeTriID.faceID].texcoordIndex[2]] - v1;
				if (v21.x()*v31.y() - v21.y()*v31.x() < 0)
				{
					// v21 x v31 < 0 ˵������˳ʱ�뷽��
					int tmp = curPointID.v1ID;
					curPointID.v1ID = curPointID.v2ID;
					curPointID.v2ID = tmp;
				}

				v1  = texcoords[faces[nextEdgeTriID.faceID].texcoordIndex[0]];
				v21 = texcoords[faces[nextEdgeTriID.faceID].texcoordIndex[1]] - v1;
				v31 = texcoords[faces[nextEdgeTriID.faceID].texcoordIndex[2]] - v1;
				if (v21.x()*v31.y() - v21.y()*v31.x() < 0)
				{
					// v21 x v31 < 0 ˵������˳ʱ�뷽��
					int tmp = nextPointID.v1ID;
					nextPointID.v1ID = nextPointID.v2ID;
					nextPointID.v2ID = tmp;
				}

				edgePointMap.insert(curPointID, nextPointID);
				edgePointMap.insert(nextPointID, curPointID);
			}
		}
	}
}

void CanvasSeamFiller::extractBorderTexcoordEdge( const QVector<ObjTriangle>& faces, const QVector<QVector2D>texcoords, QMultiHash<EdgePointID, EdgeTriID>& edgeMap )
{
	// �����б��Զ���IDΪkey�ӵ�HashMap
	for (int ithTri = 0; ithTri < faces.size(); ++ithTri)
	{
		const ObjTriangle& tri = faces[ithTri];
		for (int ithEdge = 0; ithEdge < 3; ++ithEdge)
		{
			EdgePointID triEdge;

			// �����ߵ�˳��ʹ��������������ʱ�뷽��
			QVector2D v1 = texcoords[tri.texcoordIndex[0]];
			QVector2D v2 = texcoords[tri.texcoordIndex[1]];
			QVector2D v3 = texcoords[tri.texcoordIndex[2]];
			QVector2D v21 = v2 - v1;
			QVector2D v31 = v3 - v1;
			if (v21.x()*v31.y() - v21.y()*v31.x() < 0)
			{
				// v21 x v31 < 0 ˵������˳ʱ�뷽��
				triEdge.v2ID = tri.texcoordIndex[ithEdge];
				triEdge.v1ID = tri.texcoordIndex[(ithEdge + 1) % 3];
			}
			else
			{	// ������ʱ�뷽��
				triEdge.v1ID = tri.texcoordIndex[ithEdge];
				triEdge.v2ID = tri.texcoordIndex[(ithEdge + 1) % 3];
			}

			EdgeTriID edgePos;
			edgePos.edgeID = ithEdge;
			edgePos.faceID = ithTri;
			edgeMap.insertMulti(triEdge, edgePos);
		}
	}

	// ȥ������������ι���ı�
	QList<EdgePointID> keys = edgeMap.uniqueKeys();
	for (QList<EdgePointID>::iterator pKey = keys.begin();
		pKey != keys.end(); ++pKey)
	{
		if (edgeMap.count(*pKey) != 1)
		{
			edgeMap.remove(*pKey);
		}
	}
}

void CanvasSeamFiller::buildTexcoordEdge(
	const QVector<QVector2D>& texcoords,
	const QVector<ObjTriangle>& faces,
	const QMultiHash<EdgePointID, EdgeTriID>& edgeMap,
	const QHash<EdgePointID, EdgePointID>& edgeCommonPointMap,
	QVector<TexcoordEdge>& edgeArray, 
	QVector<TexcoordVertex>& vertexArray, 
	QVector<int>& edgeSetArray )
{
	QVector<int> vertexMap(texcoords.size(), -1);// ��¼��texcoords�����±굽vertexArray�±��ӳ��

	QHash<EdgePointID, int> edgeOffsetMap;		 // ��¼��EdgePointID���ñ���edgeArray�е��±��ӳ��
	// ���ȹ����߱�Ͷ����
	for (QMultiHash<EdgePointID, EdgeTriID>::const_iterator pEdge = edgeMap.begin();
		 pEdge != edgeMap.end(); ++pEdge)
	{
		const EdgePointID& edgeKey = pEdge.key();

		// ����ߵ�����һ�˶���δ���������Ķ������飬
		// �ȰѶ�����ӽ�ȥ
		if (vertexMap[edgeKey.v1ID] == -1)
		{
			vertexMap[edgeKey.v1ID] = vertexArray.size();
			vertexArray.push_back(TexcoordVertex(texcoords[edgeKey.v1ID]));
		}
		if (vertexMap[edgeKey.v2ID] == -1)
		{
			vertexMap[edgeKey.v2ID] = vertexArray.size();
			vertexArray.push_back(TexcoordVertex(texcoords[edgeKey.v2ID]));
		}

		// ���ö���ı����
		TexcoordVertex& v1 = vertexArray[vertexMap[edgeKey.v1ID]];
		TexcoordVertex& v2 = vertexArray[vertexMap[edgeKey.v2ID]];

		// �ѱ߼�����ֹ��
		int edgeID = edgeArray.size();
		v1.exitEdgeID.push_back(edgeID);
		v2.enterEdgeID.push_back(edgeID);

		// ����±�,ͬʱ�����±ߵĶ������
		const EdgeTriID& edgeTri = pEdge.value();
		QVector2D start = texcoords[edgeKey.v1ID];
		QVector2D end   = texcoords[edgeKey.v2ID];
		QVector3D edgeDir = (end - start).toVector3D();
		QVector2D normal  = QVector2D(edgeDir.y(), -edgeDir.x());
		normal.normalize();
		edgeOffsetMap.insert(edgeKey, edgeArray.size());						// ��¼��ǰ���ڱ������λ��
		edgeArray.push_back(TexcoordEdge(
			vertexMap[edgeKey.v1ID],
			vertexMap[edgeKey.v2ID], 
			edgeTri.faceID,
			edgeTri.edgeID,
			normal));
	}
	// �ҳ��ߵĹ�����
	int ithEdge = 0;
	for (QVector<TexcoordEdge>::iterator pEdge = edgeArray.begin();
		 pEdge != edgeArray.end(); ++pEdge, ++ithEdge)
	{
		EdgePointID edgeID;
		edgeID.v1ID = pEdge->startVtxID;
		edgeID.v2ID = pEdge->endVtxID;
		// ��鵱ǰ���Ƿ��й�����
		if (edgeCommonPointMap.contains(edgeID))
		{
			const EdgePointID& commonEdgeID = edgeCommonPointMap[edgeID];
			int commonEdgeOffset = edgeOffsetMap[commonEdgeID];
			edgeArray[ithEdge].commonEdgeID = commonEdgeOffset;
			edgeArray[commonEdgeOffset].commonEdgeID = ithEdge;
		}
	}
	splitCommonVertex(edgeArray, vertexArray);
	// �ѱ���������ͬʱ��˳startVtx ͬ endVtx,�����㶥�㷨��
	linkTexcoordEdge(edgeArray, vertexArray, edgeSetArray);
	buildCoordMapper(edgeArray, vertexArray);
}
void CanvasSeamFiller::splitCommonVertex(
						QVector<TexcoordEdge>& edgeArray,
						QVector<TexcoordVertex>& vertexArray)
{
	struct CommonEdge
	{
		int edgeID;			// �ߵ����
		float angle;		// �ߵĽǶ�
		int type;			// �ߵ����ͣ����Ϊ0������Ϊ1
	};
	int vertexArrayLength = vertexArray.size();
	for (int ithVert = 0; ithVert < vertexArrayLength; ++ithVert)
	{
		TexcoordVertex& vtx = vertexArray[ithVert];
		// ���ֱ��������ϱ߹���Ķ���
		if (vtx.enterEdgeID.size() > 1 && vtx.exitEdgeID.size() > 1 &&
			vtx.enterEdgeID.size() == vtx.exitEdgeID.size())
		{
			// �ѱ߶���ӵ�������
			QVector<CommonEdge> comEdges;
			for (int ithEnter = 0; ithEnter < vtx.enterEdgeID.size();++ithEnter)
			{
				int enterEdgeID = vtx.enterEdgeID[ithEnter];
				TexcoordEdge& enterEdge = edgeArray[enterEdgeID];
				QVector2D dir = vertexArray[enterEdge.startVtxID].vertex - vtx.vertex;
				CommonEdge comEdge;
				comEdge.edgeID = enterEdgeID;
				comEdge.angle = atan2(dir.y(), dir.x());
				comEdge.type = 0;
				comEdges.push_back(comEdge);
			}
			for (int ithExit = 0; ithExit < vtx.exitEdgeID.size();++ithExit)
			{
				int exitEdgeID = vtx.exitEdgeID[ithExit];
				TexcoordEdge& exitEdge = edgeArray[exitEdgeID];
				QVector2D dir = vertexArray[exitEdge.endVtxID].vertex - vtx.vertex;
				CommonEdge comEdge;
				comEdge.edgeID = exitEdgeID;
				comEdge.angle = atan2(dir.y(), dir.x());
				comEdge.type = 1;
				comEdges.push_back(comEdge);
			}
			// ���ݽǶȶԱ�����
			for (int i = 0; i < comEdges.size() - 1; ++i)
			{
				for (int j = i+1; j < comEdges.size(); ++j)
				{
					if (comEdges[i].angle > comEdges[j].angle)
					{
						const CommonEdge tmp = comEdges[i];
						comEdges[i] = comEdges[j];
						comEdges[j] = tmp;
					}
				}
			}
			
			for (int edge0ID = 0, count = 0; 
				 edge0ID < comEdges.size(); ++edge0ID)
			{
				int edge1ID = (edge0ID + 1) % comEdges.size();
				CommonEdge& edge0 = comEdges[edge0ID];
				CommonEdge& edge1 = comEdges[edge1ID];
				// �������ڵıߣ���0Ϊ��ߣ� 1Ϊ����
				if (edge0.type == 0 && edge1.type == 1)
				{
					int curVtxID;
					if (count == 0)
						curVtxID = ithVert;
					else
					{
						curVtxID = vertexArray.size();
						vertexArray.push_back(TexcoordVertex(vtx.vertex));
					}
					// ʹ�ö���ָ���µı�
					TexcoordVertex& curVertex = vertexArray[curVtxID];
					curVertex.enterEdgeID.clear();
					curVertex.exitEdgeID.clear();
					curVertex.enterEdgeID.push_back(edge0.edgeID);
					curVertex.exitEdgeID.push_back(edge1.edgeID);

					// ʹ�ñ�ָ���µĶ���
					edgeArray[edge0.edgeID].endVtxID = curVtxID;
					edgeArray[edge1.edgeID].startVtxID = curVtxID;
					count++;
				}
			}
		}
	}
}
void CanvasSeamFiller::linkTexcoordEdge(
					  QVector<TexcoordEdge>& edgeArray,
					  QVector<TexcoordVertex>& vertexArray,
					  QVector<int>& edgeSetArray)
{
	QSet<unsigned> edgeIDSet;								// �洢δ�����edgeArray���±�ļ���
	for (int i = 0; i < edgeArray.size(); ++i)
		edgeIDSet.insert(i);

	while (!edgeIDSet.isEmpty())
	{
		// �����µı߼�
		int startEdgeID = *edgeIDSet.begin();
		edgeSetArray.push_back(startEdgeID);
		for (int edgeID = startEdgeID;;)
		{
			TexcoordEdge& edge = edgeArray[edgeID];
			TexcoordVertex& endVtx = vertexArray[edge.endVtxID];

			// ������ǰ����ָ��Ķ���
			if (edgeID != endVtx.enterEdgeID[0])
			{
				endVtx.exitEdgeID[0] = endVtx.enterEdgeID[0];
				endVtx.enterEdgeID[0] = edgeID;
			}

			// ���㶥�㷨��
			TexcoordEdge& nextEdge = edgeArray[endVtx.exitEdgeID[0]];
			QVector2D& normal1 = edge.normal;
			QVector2D& normal2 = nextEdge.normal;
			QVector2D  vtxNormal = normal1 + normal2;
			float length = vtxNormal.length();
			float factor = fillWidth * min(sqrt(2.0f / (QVector2D::dotProduct(normal1, normal2) + 1)), 3.0f);
			endVtx.normal = factor / length * vtxNormal;

			edgeIDSet.remove(edgeID);
			// ������ʼ��
			if (endVtx.exitEdgeID[0] == startEdgeID)
				break;

			// ������һ����
			if (nextEdge.startVtxID != edge.endVtxID)
			{
				nextEdge.endVtxID = nextEdge.startVtxID;
				nextEdge.startVtxID = edge.endVtxID;
			}

			edgeID = endVtx.exitEdgeID[0];
		}
	}
}
void CanvasSeamFiller::buildCoordMapper(
					QVector<TexcoordEdge>& edgeArray,
					QVector<TexcoordVertex>& vertexArray)
{
	for (int ithEdge = 0; ithEdge < edgeArray.size(); ++ithEdge)
	{
		TexcoordEdge& edge = edgeArray[ithEdge];
		QVector2D& srcO1 = vertexArray[edge.startVtxID].vertex;
		QVector2D& srcO2 = vertexArray[edge.endVtxID].vertex;
		QVector2D& srcV1 = vertexArray[edge.startVtxID].normal;
		QVector2D& srcV2 = vertexArray[edge.endVtxID].normal;
		edge.mapper.setSrcShape(srcO1, srcO2, srcV1, srcV2);

		if (edge.commonEdgeID == -1)
		{
			edge.mapper.setDstShape(srcO1, srcO2, -srcV1, -srcV2);
		}
		else
		{
			TexcoordEdge& dstEdge = edgeArray[edge.commonEdgeID];
			QVector2D& dstO1 = vertexArray[dstEdge.startVtxID].vertex;
			QVector2D& dstO2 = vertexArray[dstEdge.endVtxID].vertex;
			QVector2D& dstV1 = vertexArray[dstEdge.startVtxID].normal;
			QVector2D& dstV2 = vertexArray[dstEdge.endVtxID].normal;
			edge.mapper.setDstShape(dstO1, dstO2, -dstV1, -dstV2);
		}
	}
}

QVector<PixelPos>& CanvasSeamFiller::getExtraFillPixels( int x, int y )
{
	return fillIDMap[pixelMap[y * width + x]];
}

void CoordMapper::setSrcShape( const QVector2D& srcO1, const QVector2D& srcO2, const QVector2D& srcV1, const QVector2D& srcV2 )
{
	this->srcO = srcO1;
	this->srcU = srcO2 - srcO1;
	this->srcV1 = srcV1;
	this->srcV21 = srcV2 - srcV1;
	A = srcU.x() * srcV21.y() - srcU.y() * srcV21.x();
	B = srcV1.x() * srcV21.y() - srcV1.y() * srcV21.x();
	a = srcU.y() * srcV21.x() - srcU.x() * srcV21.y();
	b1 = srcU.y() * srcV1.x() - srcU.x() * srcV1.y();
}

void CoordMapper::setDstShape( const QVector2D& dstO1, const QVector2D& dstO2, const QVector2D& dstV1, const QVector2D& dstV2 )
{
	this->dstO = dstO1;
	this->dstU = dstO2 - dstO1;
	this->dstV1 = dstV1;
	this->dstV21 = dstV2 - dstV1;
}

bool CoordMapper::convert( const QVector2D& srcPoint, QVector2D& dstPoint )
{
	QVector2D srcD = srcPoint - srcO;
	float C = srcD.x() * srcV21.y() - srcD.y() * srcV21.x();
	float b = b1 + C;
	float c = srcV1.y() * srcD.x() - srcV1.x() * srcD.y();
	float delta = b * b - 4 * a * c;
	if (delta < 0)
		return false;
	float det = sqrt(delta);
	float u1 = (-b + det) / (2 * a);
	float u2 = (-b - det) / (2 * a);
	float u  = (u1 >= 0 && u1 <= 1) ? u1 : u2;
	float v  = (C - A * u) / B;

	dstPoint = dstO + u * dstU + u * v * dstV21 + v * dstV1;
	return true;
}


