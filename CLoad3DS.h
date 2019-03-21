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


//初始化OpenGL环境

#include "freeglut.h"
//#pragma   comment(lib,"glaux.lib")


#define PICPATH "\\Data\\pic\\"     //纹理资源的地址



// 基本块(Primary Chunk)，位于文件的开始
#define PRIMARY 0x4D4D

// 主块(Main Chunks)
#define OBJECTINFO 0x3D3D        // 网格对象的版本号
#define VERSION 0x0002        // .3ds文件的版本
#define EDITKEYFRAME 0xB000        // 所有关键帧信息的头部

// 对象的次级定义(包括对象的材质和对象）
#define MATERIAL   0xAFFF        // 保存纹理信息
#define OBJECT     0x4000        // 保存对象的面、顶点等信息

// 材质的次级定义
#define MATNAME 0xA000        // 保存材质名称
#define MATDIFFUSE 0xA020        // 对象/材质的颜色
#define MATMAP 0xA200        // 新材质的头部
#define MATMAPFILE 0xA300        // 保存纹理的文件名

#define OBJECT_MESH 0x4100        // 新的网格对象

// OBJECT_MESH的次级定义
#define OBJECT_VERTICES 0x4110      // 对象顶点
#define OBJECT_FACES    0x4120      // 对象的面
#define OBJECT_MATERIAL    0x4130      // 对象的材质
#define OBJECT_UV      0x4140      // 对象的UV纹理坐标


// 下面的宏定义计算一个矢量的长度
#define Mag(Normal) (sqrt(Normal.x*Normal.x + Normal.y*Normal.y + Normal.z*Normal.z))


#define MAX_TEXTURES 100                // 最大的纹理数目



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

// 定义2D点类，用于保存模型的UV纹理坐标
class CVector2
{
public:
	float x, y;
};

// 面的结构定义
struct tFace
{
	int vertIndex[3];      // 顶点索引
	int coordIndex[3];      // 纹理坐标索引
};

// 材质信息结构体
struct tMaterialInfo
{
	char strName[255];      // 纹理名称
	char strFile[255];      // 如果存在纹理映射，则表示纹理文件名称
	BYTE color[3];        // 对象的RGB颜色
	int texureId;        // 纹理ID
	float uTile;        // u 重复
	float vTile;        // v 重复
	float uOffset;       // u 纹理偏移
	float vOffset;        // v 纹理偏移
};

// 对象信息结构体
struct t3DObject
{
	int numOfVerts;      // 模型中顶点的数目
	int numOfFaces;      // 模型中面的数目
	int numTexVertex;      // 模型中纹理坐标的数目
	int materialID;      // 纹理ID
	bool bHasTexture;      // 是否具有纹理映射
	char strName[255];      // 对象的名称
	NBVector3 *pVerts;      // 对象的顶点
	NBVector3 *pNormals;    // 对象的法向量
	CVector2 *pTexVerts;    // 纹理UV坐标
	tFace *pFaces;        // 对象的面信息
};

// 模型信息结构体
struct t3DModel
{
	UINT texture[MAX_TEXTURES];
	int numOfObjects;          // 模型中对象的数目
	int numOfMaterials;          // 模型中材质的数目
	vector<tMaterialInfo> pMaterials;  // 材质链表信息
	vector<t3DObject> pObject;      // 模型中对象链表信息
};



struct tIndices
{
	unsigned short a, b, c, bVisible;
};

// 保存块信息的结构
struct tChunk
{
	unsigned short int ID;          // 块的ID    
	unsigned int length;          // 块的长度
	unsigned int bytesRead;          // 需要读的块数据的字节数
};




typedef struct tagBoundingBoxStruct
{
	NBVector3  BoxPosMaxVertex;
	NBVector3  BoxNegMaxVertex;
} BoundingBoxVertex2;


// 下面的函数求两点决定的矢量
NBVector3 Vector(NBVector3 vPoint1, NBVector3 vPoint2);
// 下面的函数两个矢量相加
NBVector3 AddVector(NBVector3 vVector1, NBVector3 vVector2);

// 下面的函数处理矢量的缩放
NBVector3 DivideVectorByScaler(NBVector3 vVector1, float Scaler);
// 下面的函数返回两个矢量的叉积
NBVector3 Cross(NBVector3 vVector1, NBVector3 vVector2);

// 下面的函数规范化矢量
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

// CLoad3DS类处理所有的装入代码
class CLoad3DS
{
public:
	CLoad3DS();                // 初始化数据成员
	// 装入3ds文件到模型结构中
	bool Import3DS(t3DModel *pModel, char *strFileName);

private:
	// 读入一个纹理
	int BuildTexture(char *szPathName, GLuint &texid);
	// 读一个字符串
	int GetString(char *);
	// 读下一个块
	void ReadChunk(tChunk *);
	// 读下一个块
	void ProcessNextChunk(t3DModel *pModel, tChunk *);
	// 读下一个对象块
	void ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *);
	// 读下一个材质块
	void ProcessNextMaterialChunk(t3DModel *pModel, tChunk *);
	// 读对象颜色的RGB值
	void ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk);
	// 读对象的顶点
	void ReadVertices(t3DObject *pObject, tChunk *);
	// 读对象的面信息
	void ReadVertexIndices(t3DObject *pObject, tChunk *);
	// 读对象的纹理坐标
	void ReadUVCoordinates(t3DObject *pObject, tChunk *);
	// 读赋予对象的材质名称
	void ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk);
	// 计算对象顶点的法向量
	void ComputeNormals(t3DModel *pModel);
	// 关闭文件，释放内存空间
	void CleanUp();
	// 文件指针
	FILE *m_FilePointer;

	tChunk *m_CurrentChunk;
	tChunk *m_TempChunk;
};
void changeObject(float trans[10]);
void drawModel(t3DModel* Model, bool touming, bool outTex);
#endif
