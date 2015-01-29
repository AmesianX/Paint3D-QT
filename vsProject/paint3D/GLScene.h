#pragma once
#include "globject.h"
#include "GLTranslateManipulator.h"
#include "GLRotateManipulator.h"
#include "GLScaleManipulator.h"
class GLScene
{
public:
    enum ToolType{TOOL_SELECT = 0, TOOL_TRANSLATE, TOOL_ROTATE, TOOL_SCALE, TOOL_FACE_SELECT};

    GLScene(void);
    ~GLScene(void);

    // �������
    void loadMesh(const char* path);
    //GLMesh* getMesh(const CString name);
    //bool removeMesh(const CString name);

    // ����դ�����
    void addGrid();
	void addMatlabGrid();

    // һ�����
    RenderableObject* getCurSelectedObject(int filter = RenderableObject::GL_GRID | RenderableObject::GL_MESH);
    void getObjectNames(std::vector<QString>& names, int filter = RenderableObject::GL_GRID | RenderableObject::GL_MESH);
    RenderableObject* selectObject(const QString name, int filter = RenderableObject::GL_GRID | RenderableObject::GL_MESH);
    RenderableObject* selectObject(int x, int y);
    RenderableObject* getObject(QString& name, int filter = RenderableObject::GL_GRID | RenderableObject::GL_MESH);
    bool removeObject(const QString name, int filter = RenderableObject::GL_GRID | RenderableObject::GL_MESH);
    bool removeCurSelected(int filter = RenderableObject::GL_GRID | RenderableObject::GL_MESH);
    void draw();
    bool rename(const QString& oldName, const QString& newName);
    
    // ���������
    void centerCamera();

    // ѡ����ݹ���
    void setTool(GLScene::ToolType type);
    // ��갴��ʱ������ captureObject����ʾ���Բ�׽����
    // ����϶�ʱ������ manipulateObject�� ��ʾ��������в���
    // ����ɿ�ʱ������ releaseObject����ʾ�ͷŶ�����Ĳ���
    bool captureObject   (int x, int y);
    bool manipulateObject(int x, int y);
    bool releaseObject   (int x, int y);
    bool isManipulating();

    // ����ƶ�ʱ������һЩ��Ӧ
    bool onMouseHover(int x, int y);

    GLCamera& getCamera(){return m_camera;}

    bool m_showBackgroundGrid;
private:
    std::vector<RenderableObject*> m_pObjects;

    Camera  m_camera;
    RenderableObject* m_curSelectObj;

    TranslateManipulator m_translateTool;
    RotateManipulator    m_rotateTool;
    ScaleManipulator     m_scaleTool;
    Manipulator*         m_curTool;
    ToolType               m_curToolType;

    int m_mouseBeginPos[2];    // ��ʱ��������¼��갴��ʱ��Ļ����
    int m_mouseCurrPos[2];
    bool m_isRectSelecting;

    RenderableObject* selectObject(const CVector3d& ori, const CVector3d& dir);
    GLMesh* searchMesh(const QString name);
    void    removeAllMesh();
    void    makeNameUnique(RenderableObject* newObj);
    // ���������ཻ����������壬����ѡ������
    RenderableObject* intersectObject( const CVector3d& ori, const CVector3d& dir );
};

