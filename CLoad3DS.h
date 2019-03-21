#ifndef _CLoad3DS_h_
#define _CLoad3DS_h_



#include <windows.h>
#include <cassert>
#include <cmath>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>                  

#include <olectl.h>              
#include <cmath>  
#include <ctime>
#include <algorithm>


//��ʼ��OpenGL����

#include "freeglut.h"
//#pragma   comment(lib,"glaux.lib")


#define PICPATH "\\Data\\pic\\"     //������Դ�ĵ�ַ



// ������(Primary Chunk)��λ���ļ��Ŀ�ʼ
#define PRIMARY 0x4D4D

// ����(Main Chunks)
#define OBJECTINFO 0x3D3D        // �������İ汾��
#define VERSION 0x0002        // .3ds�ļ��İ汾
#define EDITKEYFRAME 0xB000        // ���йؼ�֡��Ϣ��ͷ��

// ����Ĵμ�����(��������Ĳ��ʺͶ���
#define MATERIAL   0xAFFF        // ����������Ϣ
#define OBJECT     0x4000        // ���������桢�������Ϣ

// ���ʵĴμ�����
#define MATNAME 0xA000        // �����������
#define MATDIFFUSE 0xA020        // ����/���ʵ���ɫ
#define MATMAP 0xA200        // �²��ʵ�ͷ��
#define MATMAPFILE 0xA300        // ����������ļ���

#define OBJECT_MESH 0x4100        // �µ��������

// OBJECT_MESH�Ĵμ�����
#define OBJECT_VERTICES 0x4110      // ���󶥵�
#define OBJECT_FACES    0x4120      // �������
#define OBJECT_MATERIAL    0x4130      // ����Ĳ���
#define OBJECT_UV      0x4140      // �����UV��������


// ����ĺ궨�����һ��ʸ���ĳ���
#define Mag(Normal) (sqrt(Normal.x*Normal.x + Normal.y*Normal.y + Normal.z*Normal.z))


#define MAX_TEXTURES 100                // ����������Ŀ



using namespace std;
class NBVector3
{
public:
	NBVector3() {}
	NBVector3(float X, float Y, float Z)
	{
		x = X; y = Y; z = Z;
	}
	inline NBVector3 operator+(NBVector3 vVector)
	{
		return NBVector3(vVector.x + x, vVector.y + y, vVector.z + z);
	}
	inline NBVector3 operator-(NBVector3 vVector)
	{
		return NBVector3(x - vVector.x, y - vVector.y, z - vVector.z);
	}
	inline NBVector3 operator-()
	{
		return NBVector3(-x, -y, -z);
	}
	inline NBVector3 operator*(float num)
	{
		return NBVector3(x * num, y * num, z * num);
	}
	inline NBVector3 operator/(float num)
	{
		return NBVector3(x / num, y / num, z / num);
	}

	inline NBVector3 operator^(const NBVector3 &rhs) const
	{
		return NBVector3(y * rhs.z - rhs.y * z, rhs.x * z - x * rhs.z, x * rhs.y - rhs.x * y);
	}

	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		float v[3];
	};
};

// ����2D���࣬���ڱ���ģ�͵�UV��������
class CVector2
{
public:
	float x, y;
};

// ��Ľṹ����
struct tFace
{
	int vertIndex[3];      // ��������
	int coordIndex[3];      // ������������
};

// ������Ϣ�ṹ��
struct tMaterialInfo
{
	char strName[255];      // ��������
	char strFile[255];      // �����������ӳ�䣬���ʾ�����ļ�����
	BYTE color[3];        // �����RGB��ɫ
	int texureId;        // ����ID
	float uTile;        // u �ظ�
	float vTile;        // v �ظ�
	float uOffset;       // u ����ƫ��
	float vOffset;        // v ����ƫ��
};

// ������Ϣ�ṹ��
struct t3DObject
{
	int numOfVerts;      // ģ���ж������Ŀ
	int numOfFaces;      // ģ���������Ŀ
	int numTexVertex;      // ģ���������������Ŀ
	int materialID;      // ����ID
	bool bHasTexture;      // �Ƿ��������ӳ��
	char strName[255];      // ���������
	NBVector3 *pVerts;      // ����Ķ���
	NBVector3 *pNormals;    // ����ķ�����
	CVector2 *pTexVerts;    // ����UV����
	tFace *pFaces;        // ���������Ϣ
};

// ģ����Ϣ�ṹ��
struct t3DModel
{
	UINT texture[MAX_TEXTURES];
	int numOfObjects;          // ģ���ж������Ŀ
	int numOfMaterials;          // ģ���в��ʵ���Ŀ
	vector<tMaterialInfo> pMaterials;  // ����������Ϣ
	vector<t3DObject> pObject;      // ģ���ж���������Ϣ
};



struct tIndices
{
	unsigned short a, b, c, bVisible;
};

// �������Ϣ�Ľṹ
struct tChunk
{
	unsigned short int ID;          // ���ID    
	unsigned int length;          // ��ĳ���
	unsigned int bytesRead;          // ��Ҫ���Ŀ����ݵ��ֽ���
};




typedef struct tagBoundingBoxStruct
{
	NBVector3  BoxPosMaxVertex;
	NBVector3  BoxNegMaxVertex;
} BoundingBoxVertex2;


// ����ĺ��������������ʸ��
NBVector3 Vector(NBVector3 vPoint1, NBVector3 vPoint2);
// ����ĺ�������ʸ�����
NBVector3 AddVector(NBVector3 vVector1, NBVector3 vVector2);

// ����ĺ�������ʸ��������
NBVector3 DivideVectorByScaler(NBVector3 vVector1, float Scaler);
// ����ĺ�����������ʸ���Ĳ��
NBVector3 Cross(NBVector3 vVector1, NBVector3 vVector2);

// ����ĺ����淶��ʸ��
NBVector3 Normalize(NBVector3 vNormal);

void DrawModel(t3DModel& Model, bool touming = false);


//////////////////////////////////////////////////////////////////////////
#define FRAND   (((float)rand()-(float)rand())/RAND_MAX)
#define Clamp(x, min, max)  x = (x<min  ? min : x<max ? x : max);

#define SQUARE(x)  (x)*(x)
struct vector3_t
{
	vector3_t(float x, float y, float z) : x(x), y(y), z(z) {}
	vector3_t(const vector3_t &v) : x(v.x), y(v.y), z(v.z) {}
	vector3_t() : x(0.0f), y(0.0f), z(0.0f) {}

	vector3_t& operator=(const vector3_t &rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}

	// vector add
	vector3_t operator+(const vector3_t &rhs) const
	{
		return vector3_t(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	// vector subtract
	vector3_t operator-(const vector3_t &rhs) const
	{
		return vector3_t(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	// scalar multiplication
	vector3_t operator*(const float scalar) const
	{
		return vector3_t(x * scalar, y * scalar, z * scalar);
	}

	// dot product
	float operator*(const vector3_t &rhs) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	// cross product
	vector3_t operator^(const vector3_t &rhs) const
	{
		return vector3_t(y * rhs.z - rhs.y * z, rhs.x * z - x * rhs.z, x * rhs.y - rhs.x * y);
	}

	float& operator[](int index)
	{
		return v[index];
	}

	float Length()
	{
		float length = (float)sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));
		return (length != 0.0f) ? length : 1.0f;
	}

	/*****************************************************************************
	Normalize()

	Helper function to normalize vectors
	*****************************************************************************/
	vector3_t Normalize()
	{
		*this = *this * (1.0f / Length());
		return *this;
	}

	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		float v[3];
	};
};

// CLoad3DS�ദ�����е�װ�����
class CLoad3DS
{
public:
	CLoad3DS();                // ��ʼ�����ݳ�Ա
	// װ��3ds�ļ���ģ�ͽṹ��
	bool Import3DS(t3DModel *pModel, char *strFileName);

private:
	// ����һ������
	int BuildTexture(char *szPathName, GLuint &texid);
	// ��һ���ַ���
	int GetString(char *);
	// ����һ����
	void ReadChunk(tChunk *);
	// ����һ����
	void ProcessNextChunk(t3DModel *pModel, tChunk *);
	// ����һ�������
	void ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *);
	// ����һ�����ʿ�
	void ProcessNextMaterialChunk(t3DModel *pModel, tChunk *);
	// ��������ɫ��RGBֵ
	void ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk);
	// ������Ķ���
	void ReadVertices(t3DObject *pObject, tChunk *);
	// �����������Ϣ
	void ReadVertexIndices(t3DObject *pObject, tChunk *);
	// ���������������
	void ReadUVCoordinates(t3DObject *pObject, tChunk *);
	// ���������Ĳ�������
	void ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk);
	// ������󶥵�ķ�����
	void ComputeNormals(t3DModel *pModel);
	// �ر��ļ����ͷ��ڴ�ռ�
	void CleanUp();
	// �ļ�ָ��
	FILE *m_FilePointer;

	tChunk *m_CurrentChunk;
	tChunk *m_TempChunk;
};
void changeObject(float trans[10]);
void drawModel(t3DModel* Model, bool touming, bool outTex);
#endif
