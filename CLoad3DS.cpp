#include "CLoad3DS.h"

#pragma warning (disable: 4996) 


// 下面的函数求两点决定的矢量
NBVector3 Vector(NBVector3 vPoint1, NBVector3 vPoint2)
{
	NBVector3 vVector;

	vVector.x = vPoint1.x - vPoint2.x;
	vVector.y = vPoint1.y - vPoint2.y;
	vVector.z = vPoint1.z - vPoint2.z;

	return vVector;
}

// 下面的函数两个矢量相加
NBVector3 AddVector(NBVector3 vVector1, NBVector3 vVector2)
{
	NBVector3 vResult;

	vResult.x = vVector2.x + vVector1.x;
	vResult.y = vVector2.y + vVector1.y;
	vResult.z = vVector2.z + vVector1.z;

	return vResult;
}

// 下面的函数处理矢量的缩放
NBVector3 DivideVectorByScaler(NBVector3 vVector1, float Scaler)
{
	NBVector3 vResult;

	vResult.x = vVector1.x / Scaler;
	vResult.y = vVector1.y / Scaler;
	vResult.z = vVector1.z / Scaler;

	return vResult;
}

// 下面的函数返回两个矢量的叉积
NBVector3 Cross(NBVector3 vVector1, NBVector3 vVector2)
{
	NBVector3 vCross;

	vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));

	vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));

	vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

	return vCross;
}

// 下面的函数规范化矢量
NBVector3 Normalize(NBVector3 vNormal)
{
	double Magnitude;

	Magnitude = Mag(vNormal);          // 获得矢量的长度

	vNormal.x /= (float)Magnitude;
	vNormal.y /= (float)Magnitude;
	vNormal.z /= (float)Magnitude;

	return vNormal;
}

// 读入一个纹理
int CLoad3DS::BuildTexture(char *szPathName, GLuint &texid)
{
	HDC      hdcTemp;                        // The DC To Hold Our Bitmap
	HBITMAP    hbmpTemp;                        // Holds The Bitmap Temporarily
	IPicture  *pPicture;                        // IPicture Interface
	OLECHAR    wszPath[MAX_PATH + 1];                  // Full Path To Picture (WCHAR)
	char    szPath[MAX_PATH + 1];                    // Full Path To Picture
	long    lWidth;                          // Width In Logical Units
	long    lHeight;                        // Height In Logical Units
	long    lWidthPixels;                      // Width In Pixels
	long    lHeightPixels;                      // Height In Pixels
	GLint    glMaxTexDim;                      // Holds Maximum Texture Size

	if (strstr(szPathName, "http://"))                  // If PathName Contains http:// Then...
	{
		strcpy(szPath, szPathName);                    // Append The PathName To szPath
	}
	else                                // Otherwise... We Are Loading From A File
	{
		//GetCurrentDirectory(MAX_PATH, szPath);              // Get Our Working Directory
		strcat(szPath, PICPATH);                      // Append "\" After The Working Directory
		strcat(szPath, szPathName);                    // Append The PathName
	}


	MultiByteToWideChar(CP_ACP, 0, szPath, -1, wszPath, MAX_PATH);    // Convert From ASCII To Unicode
	HRESULT hr = OleLoadPicturePath(wszPath, 0, 0, 0, IID_IPicture, (void**)&pPicture);

	if (FAILED(hr))                            // If Loading Failed
		return FALSE;                          // Return False

	hdcTemp = CreateCompatibleDC(GetDC(0));                // Create The Windows Compatible Device Context
	if (!hdcTemp)                            // Did Creation Fail?
	{
		pPicture->Release();                      // Decrements IPicture Reference Count
		return FALSE;                          // Return False (Failure)
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);          // Get Maximum Texture Size Supported

	pPicture->get_Width(&lWidth);                    // Get IPicture Width (Convert To Pixels)
	lWidthPixels = MulDiv(lWidth, GetDeviceCaps(hdcTemp, LOGPIXELSX), 2540);
	pPicture->get_Height(&lHeight);                    // Get IPicture Height (Convert To Pixels)
	lHeightPixels = MulDiv(lHeight, GetDeviceCaps(hdcTemp, LOGPIXELSY), 2540);

	// Resize Image To Closest Power Of Two
	if (lWidthPixels <= glMaxTexDim) // Is Image Width Less Than Or Equal To Cards Limit
		lWidthPixels = 1 << (int)floor((log((double)lWidthPixels) / log(2.0f)) + 0.5f);
	else // Otherwise Set Width To "Max Power Of Two" That The Card Can Handle
		lWidthPixels = glMaxTexDim;

	if (lHeightPixels <= glMaxTexDim) // Is Image Height Greater Than Cards Limit
		lHeightPixels = 1 << (int)floor((log((double)lHeightPixels) / log(2.0f)) + 0.5f);
	else // Otherwise Set Height To "Max Power Of Two" That The Card Can Handle
		lHeightPixels = glMaxTexDim;

	//  Create A Temporary Bitmap
	BITMAPINFO  bi = { 0 };                        // The Type Of Bitmap We Request
	DWORD    *pBits = 0;                        // Pointer To The Bitmap Bits

	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);        // Set Structure Size
	bi.bmiHeader.biBitCount = 32;                  // 32 Bit
	bi.bmiHeader.biWidth = lWidthPixels;              // Power Of Two Width
	bi.bmiHeader.biHeight = lHeightPixels;            // Make Image Top Up (Positive Y-Axis)
	bi.bmiHeader.biCompression = BI_RGB;                // RGB Encoding
	bi.bmiHeader.biPlanes = 1;                  // 1 Bitplane

	//  Creating A Bitmap This Way Allows Us To Specify Color Depth And Gives Us Imediate Access To The Bits
	hbmpTemp = CreateDIBSection(hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits, 0, 0);

	if (!hbmpTemp)                            // Did Creation Fail?
	{
		DeleteDC(hdcTemp);                        // Delete The Device Context
		pPicture->Release();                      // Decrements IPicture Reference Count
		return FALSE;                          // Return False (Failure)
	}

	SelectObject(hdcTemp, hbmpTemp);                  // Select Handle To Our Temp DC And Our Temp Bitmap Object

	// Render The IPicture On To The Bitmap
	pPicture->Render(hdcTemp, 0, 0, lWidthPixels, lHeightPixels, 0, lHeight, lWidth, -lHeight, 0);

	// Convert From BGR To RGB Format And Add An Alpha Value Of 255
	for (long i = 0; i < lWidthPixels * lHeightPixels; i++)        // Loop Through All Of The Pixels
	{
		BYTE* pPixel = (BYTE*)(&pBits[i]);              // Grab The Current Pixel
		BYTE temp = pPixel[0];                  // Store 1st Color In Temp Variable (Blue)
		pPixel[0] = pPixel[2];                  // Move Red Value To Correct Position (1st)
		pPixel[2] = temp;                      // Move Temp Value To Correct Blue Position (3rd)

		// This Will Make Any Black Pixels, Completely Transparent    (You Can Hardcode The Value If You Wish)
		if ((pPixel[0] == 0) && (pPixel[1] == 0) && (pPixel[2] == 0))      // Is Pixel Completely Black
			pPixel[3] = 0;                      // Set The Alpha Value To 0
		else                              // Otherwise
			pPixel[3] = 255;                      // Set The Alpha Value To 255
	}

	glGenTextures(1, &texid);                      // Create The Texture

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texid);                // Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);    // (Modify This For The Type Of Filtering You Want)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // (Modify This For The Type Of Filtering You Want)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lWidthPixels, lHeightPixels, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBits);  // (Modify This If You Want Mipmaps)

	DeleteObject(hbmpTemp);                        // Delete The Object
	DeleteDC(hdcTemp);                          // Delete The Device Context

	pPicture->Release();                        // Decrements IPicture Reference Count

	printf("load %s!", szPath);
	return TRUE;                            // Return True (All Good)

}


// 构造函数的功能是初始化tChunk数据
CLoad3DS::CLoad3DS()
{
	m_CurrentChunk = new tChunk;        // 初始化并为当前的块分配空间
	m_TempChunk = new tChunk;          // 初始化一个临时块并分配空间
}

// 打开一个3ds文件，读出其中的内容，并释放内存
bool CLoad3DS::Import3DS(t3DModel *pModel, char *strFileName)
{
	char strMessage[255] = { 0 };

	// 打开一个3ds文件
	m_FilePointer = fopen(strFileName, "rb");

	// 确保所获得的文件指针合法
	if (!m_FilePointer)
	{
		sprintf(strMessage, "Unable to find the file: %s!", strFileName);
		//MessageBox(NULL, strMessage, "Error", MB_OK);
		return false;
	}

	// 当文件打开之后，首先应该将文件最开始的数据块读出以判断是否是一个3ds文件
	// 如果是3ds文件的话，第一个块ID应该是PRIMARY

	// 将文件的第一块读出并判断是否是3ds文件
	ReadChunk(m_CurrentChunk);

	// 确保是3ds文件
	if (m_CurrentChunk->ID != PRIMARY)
	{
		sprintf(strMessage, "Unable to load PRIMARY chuck from file: %s!", strFileName);
		//MessageBox(NULL, strMessage, "Error", MB_OK);
		return false;
	}

	// 现在开始读入数据，ProcessNextChunk()是一个递归函数

	// 通过调用下面的递归函数，将对象读出
	ProcessNextChunk(pModel, m_CurrentChunk);

	// 在读完整个3ds文件之后，计算顶点的法线
	ComputeNormals(pModel);

	// 释放内存空间
	CleanUp();

	return true;
}

// 下面的函数释放所有的内存空间，并关闭文件
void CLoad3DS::CleanUp()
{

	fclose(m_FilePointer);            // 关闭当前的文件指针
	delete m_CurrentChunk;            // 释放当前块
	delete m_TempChunk;              // 释放临时块
}

// 下面的函数读出3ds文件的主要部分
void CLoad3DS::ProcessNextChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
	t3DObject newObject = { 0 };          // 用来添加到对象链表
	tMaterialInfo newTexture = { 0 };        // 用来添加到材质链表
	unsigned int version = 0;          // 保存文件版本
	int buffer[50000] = { 0 };          // 用来跳过不需要的数据

	m_CurrentChunk = new tChunk;        // 为新的块分配空间    

	// 下面每读一个新块，都要判断一下块的ID，如果该块是需要的读入的，则继续进行
	// 如果是不需要读入的块，则略过

	// 继续读入子块，直到达到预定的长度
	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
		// 读入下一个块
		ReadChunk(m_CurrentChunk);

		// 判断块的ID号
		switch (m_CurrentChunk->ID)
		{
		case VERSION:              // 文件版本号

			// 在该块中有一个无符号短整型数保存了文件的版本

			// 读入文件的版本号，并将字节数添加到bytesRead变量中
			m_CurrentChunk->bytesRead += fread(&version, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);

			// 如果文件版本号大于3，给出一个警告信息
			if (version > 0x03)
				//MessageBox(NULL, "This 3DS file is over version 3 so it may load incorrectly", "Warning", MB_OK);
			break;

		case OBJECTINFO:            // 网格版本信息

			// 读入下一个块
			ReadChunk(m_TempChunk);

			// 获得网格的版本号
			m_TempChunk->bytesRead += fread(&version, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

			// 增加读入的字节数
			m_CurrentChunk->bytesRead += m_TempChunk->bytesRead;

			// 进入下一个块
			ProcessNextChunk(pModel, m_CurrentChunk);
			break;

		case MATERIAL:              // 材质信息

			// 材质的数目递增
			pModel->numOfMaterials++;

			// 在纹理链表中添加一个空白纹理结构
			pModel->pMaterials.push_back(newTexture);

			// 进入材质装入函数
			ProcessNextMaterialChunk(pModel, m_CurrentChunk);
			break;

		case OBJECT:              // 对象的名称

			// 该块是对象信息块的头部，保存了对象了名称

			// 对象数递增
			pModel->numOfObjects++;

			// 添加一个新的tObject节点到对象链表中
			pModel->pObject.push_back(newObject);

			// 初始化对象和它的所有数据成员
			memset(&(pModel->pObject[pModel->numOfObjects - 1]), 0, sizeof(t3DObject));

			// 获得并保存对象的名称，然后增加读入的字节数
			m_CurrentChunk->bytesRead += GetString(pModel->pObject[pModel->numOfObjects - 1].strName);

			// 进入其余的对象信息的读入
			ProcessNextObjectChunk(pModel, &(pModel->pObject[pModel->numOfObjects - 1]), m_CurrentChunk);
			break;

		case EDITKEYFRAME:

			// 跳过关键帧块的读入，增加需要读入的字节数
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;

		default:

			// 跳过所有忽略的块的内容的读入，增加需要读入的字节数
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;
		}

		// 增加从最后块读入的字节数
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// 释放当前块的内存空间
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}

// 下面的函数处理所有的文件中对象的信息
void CLoad3DS::ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
	int buffer[50000] = { 0 };          // 用于读入不需要的数据

	// 对新的块分配存储空间
	m_CurrentChunk = new tChunk;

	// 继续读入块的内容直至本子块结束
	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
		// 读入下一个块
		ReadChunk(m_CurrentChunk);

		// 区别读入是哪种块
		switch (m_CurrentChunk->ID)
		{
		case OBJECT_MESH:          // 正读入的是一个新块

			// 使用递归函数调用，处理该新块
			ProcessNextObjectChunk(pModel, pObject, m_CurrentChunk);
			break;

		case OBJECT_VERTICES:        // 读入是对象顶点
			ReadVertices(pObject, m_CurrentChunk);
			break;

		case OBJECT_FACES:          // 读入的是对象的面
			ReadVertexIndices(pObject, m_CurrentChunk);
			break;

		case OBJECT_MATERIAL:        // 读入的是对象的材质名称

			// 该块保存了对象材质的名称，可能是一个颜色，也可能是一个纹理映射。同时在该块中也保存了
			// 纹理对象所赋予的面

			// 下面读入对象的材质名称
			ReadObjectMaterial(pModel, pObject, m_CurrentChunk);
			break;

		case OBJECT_UV:            // 读入对象的UV纹理坐标

			// 读入对象的UV纹理坐标
			ReadUVCoordinates(pObject, m_CurrentChunk);
			break;

		default:

			// 略过不需要读入的块
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;
		}

		// 添加从最后块中读入的字节数到前面的读入的字节中
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// 释放当前块的内存空间，并把当前块设置为前面块
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}

// 下面的函数处理所有的材质信息
void CLoad3DS::ProcessNextMaterialChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
	int buffer[50000] = { 0 };          // 用于读入不需要的数据

	// 给当前块分配存储空间
	m_CurrentChunk = new tChunk;

	// 继续读入这些块，知道该子块结束
	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
		// 读入下一块
		ReadChunk(m_CurrentChunk);

		// 判断读入的是什么块
		switch (m_CurrentChunk->ID)
		{
		case MATNAME:              // 材质的名称

			// 读入材质的名称
			m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;

		case MATDIFFUSE:            // 对象的R G B颜色
			ReadColorChunk(&(pModel->pMaterials[pModel->numOfMaterials - 1]), m_CurrentChunk);
			break;

		case MATMAP:              // 纹理信息的头部

			// 进入下一个材质块信息
			ProcessNextMaterialChunk(pModel, m_CurrentChunk);
			break;

		case MATMAPFILE:            // 材质文件的名称

			// 读入材质的文件名称
			m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strFile, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;

		default:

			// 掠过不需要读入的块
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;
		}

		// 添加从最后块中读入的字节数
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// 删除当前块，并将当前块设置为前面的块
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}

// 下面函数读入块的ID号和它的字节长度
void CLoad3DS::ReadChunk(tChunk *pChunk)
{
	// 读入块的ID号，占用了2个字节。块的ID号象OBJECT或MATERIAL一样，说明了在块中所包含的内容
	pChunk->bytesRead = fread(&pChunk->ID, 1, 2, m_FilePointer);

	// 然后读入块占用的长度，包含了四个字节
	pChunk->bytesRead += fread(&pChunk->length, 1, 4, m_FilePointer);
}

// 下面的函数读入一个字符串
int CLoad3DS::GetString(char *pBuffer)
{
	int index = 0;

	// 读入一个字节的数据
	fread(pBuffer, 1, 1, m_FilePointer);

	// 直到结束
	while (*(pBuffer + index++) != 0) {

		// 读入一个字符直到NULL
		fread(pBuffer + index, 1, 1, m_FilePointer);
	}

	// 返回字符串的长度
	return strlen(pBuffer) + 1;
}

// 下面的函数读入RGB颜色
void CLoad3DS::ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk)
{
	// 读入颜色块信息
	ReadChunk(m_TempChunk);

	// 读入RGB颜色
	m_TempChunk->bytesRead += fread(pMaterial->color, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

	// 增加读入的字节数
	pChunk->bytesRead += m_TempChunk->bytesRead;
}

// 下面的函数读入顶点索引
void CLoad3DS::ReadVertexIndices(t3DObject *pObject, tChunk *pPreviousChunk)
{
	unsigned short index = 0;          // 用于读入当前面的索引

	// 读入该对象中面的数目
	pPreviousChunk->bytesRead += fread(&pObject->numOfFaces, 1, 2, m_FilePointer);

	// 分配所有面的存储空间，并初始化结构
	pObject->pFaces = new tFace[pObject->numOfFaces];
	memset(pObject->pFaces, 0, sizeof(tFace)* pObject->numOfFaces);

	// 遍历对象中所有的面
	for (int i = 0; i < pObject->numOfFaces; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			// 读入当前面的第一个点 
			pPreviousChunk->bytesRead += fread(&index, 1, sizeof(index), m_FilePointer);

			if (j < 3)
			{
				// 将索引保存在面的结构中
				pObject->pFaces[i].vertIndex[j] = index;
			}
		}
	}
}

// 下面的函数读入对象的UV坐标
void CLoad3DS::ReadUVCoordinates(t3DObject *pObject, tChunk *pPreviousChunk)
{
	// 为了读入对象的UV坐标，首先需要读入UV坐标的数量，然后才读入具体的数据

	// 读入UV坐标的数量
	pPreviousChunk->bytesRead += fread(&pObject->numTexVertex, 1, 2, m_FilePointer);

	// 分配保存UV坐标的内存空间
	pObject->pTexVerts = new CVector2[pObject->numTexVertex];

	// 读入纹理坐标
	pPreviousChunk->bytesRead += fread(pObject->pTexVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

// 读入对象的顶点
void CLoad3DS::ReadVertices(t3DObject *pObject, tChunk *pPreviousChunk)
{
	// 在读入实际的顶点之前，首先必须确定需要读入多少个顶点。

	// 读入顶点的数目
	pPreviousChunk->bytesRead += fread(&(pObject->numOfVerts), 1, 2, m_FilePointer);

	// 分配顶点的存储空间，然后初始化结构体
	pObject->pVerts = new NBVector3[pObject->numOfVerts];
	memset(pObject->pVerts, 0, sizeof(NBVector3)* pObject->numOfVerts);

	// 读入顶点序列
	pPreviousChunk->bytesRead += fread(pObject->pVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);

	// 现在已经读入了所有的顶点。
	// 因为3D Studio Max的模型的Z轴是指向上的，因此需要将y轴和z轴翻转过来。
	// 具体的做法是将Y轴和Z轴交换，然后将Z轴反向。

	// 遍历所有的顶点
	for (int i = 0; i < pObject->numOfVerts; i++)
	{
		// 保存Y轴的值
		float fTempY = pObject->pVerts[i].y;

		// 设置Y轴的值等于Z轴的值
		pObject->pVerts[i].y = pObject->pVerts[i].z;

		// 设置Z轴的值等于-Y轴的值 
		pObject->pVerts[i].z = -fTempY;
	}
}

// 下面的函数读入对象的材质名称
void CLoad3DS::ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
	char strMaterial[255] = { 0 };      // 用来保存对象的材质名称
	int buffer[50000] = { 0 };        // 用来读入不需要的数据

	// 材质或者是颜色，或者是对象的纹理，也可能保存了象明亮度、发光度等信息。

	// 下面读入赋予当前对象的材质名称
	pPreviousChunk->bytesRead += GetString(strMaterial);

	// 遍历所有的纹理
	for (int i = 0; i < pModel->numOfMaterials; i++)
	{
		//如果读入的纹理与当前的纹理名称匹配
		if (strcmp(strMaterial, pModel->pMaterials[i].strName) == 0)
		{
			// 设置材质ID
			pObject->materialID = i;

			// 判断是否是纹理映射，如果strFile是一个长度大于1的字符串，则是纹理
			if (strlen(pModel->pMaterials[i].strFile) > 0) {

				//载入纹理
				BuildTexture(pModel->pMaterials[i].strFile, pModel->texture[pObject->materialID]);
				// 设置对象的纹理映射标志
				pObject->bHasTexture = true;

				char strMessage[100];
				sprintf(strMessage, "file name : %s!", pModel->pMaterials[i].strFile);
				printf("%s\n", strMessage);
				//				MessageBox(NULL, strMessage, "Error", MB_OK);
			}
			break;
		}
		else
		{
			// 如果该对象没有材质，则设置ID为-1
			pObject->materialID = -1;
		}
	}

	pPreviousChunk->bytesRead += fread(buffer, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

// 下面的这些函数主要用来计算顶点的法向量，顶点的法向量主要用来计算光照



// 下面的函数用于计算对象的法向量
void CLoad3DS::ComputeNormals(t3DModel *pModel)
{
	NBVector3 vVector1, vVector2, vNormal, vPoly[3];

	// 如果模型中没有对象，则返回
	if (pModel->numOfObjects <= 0)
		return;

	// 遍历模型中所有的对象
	for (int index = 0; index < pModel->numOfObjects; index++)
	{
		// 获得当前的对象
		t3DObject *pObject = &(pModel->pObject[index]);

		// 分配需要的存储空间
		NBVector3 *pNormals = new NBVector3[pObject->numOfFaces];
		NBVector3 *pTempNormals = new NBVector3[pObject->numOfFaces];
		pObject->pNormals = new NBVector3[pObject->numOfVerts];
		int i = 0;
		// 遍历对象的所有面
		for (i = 0; i < pObject->numOfFaces; i++)
		{
			vPoly[0] = pObject->pVerts[pObject->pFaces[i].vertIndex[0]];
			vPoly[1] = pObject->pVerts[pObject->pFaces[i].vertIndex[1]];
			vPoly[2] = pObject->pVerts[pObject->pFaces[i].vertIndex[2]];

			// 计算面的法向量

			vVector1 = Vector(vPoly[0], vPoly[2]);    // 获得多边形的矢量
			vVector2 = Vector(vPoly[2], vPoly[1]);    // 获得多边形的第二个矢量

			vNormal = Cross(vVector1, vVector2);    // 获得两个矢量的叉积
			pTempNormals[i] = vNormal;          // 保存非规范化法向量
			vNormal = Normalize(vNormal);        // 规范化获得的叉积

			pNormals[i] = vNormal;            // 将法向量添加到法向量列表中
		}

		// 下面求顶点法向量
		NBVector3 vSum(0.0, 0.0, 0.0);
		NBVector3 vZero = vSum;
		int shared = 0;
		// 遍历所有的顶点
		for (i = 0; i < pObject->numOfVerts; i++)
		{
			for (int j = 0; j < pObject->numOfFaces; j++)  // 遍历所有的三角形面
			{                        // 判断该点是否与其它的面共享
				if (pObject->pFaces[j].vertIndex[0] == i ||
					pObject->pFaces[j].vertIndex[1] == i ||
					pObject->pFaces[j].vertIndex[2] == i)
				{
					vSum = AddVector(vSum, pTempNormals[j]);
					shared++;
				}
			}

			pObject->pNormals[i] = DivideVectorByScaler(vSum, float(-shared));

			// 规范化最后的顶点法向
			pObject->pNormals[i] = Normalize(pObject->pNormals[i]);

			vSum = vZero;
			shared = 0;
		}

		// 释放存储空间，开始下一个对象
		delete[] pTempNormals;
		delete[] pNormals;
	}
}
void changeObject(float trans[10])
{
	glTranslatef(trans[0], trans[1], trans[2]);
	glScalef(trans[3], trans[4], trans[5]);
	glRotatef(trans[6], trans[7], trans[8], trans[9]);
	glRotatef(trans[10], trans[11], trans[12], trans[13]);
}
void drawModel(t3DModel* Model, bool touming, bool outTex)
{

	if (touming){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1, 1, 1, 0.5);
	}

	// ±éàú?￡Dí?D?ùóDμ????ó
	for (int i = 0; i < Model->numOfObjects; i++)
	{
		// ??μ?μ±?°??ê?μ????ó
		t3DObject *pObject = &Model->pObject[i];
		// ?D???????óê?·?óD??àíó3é?
		if (!outTex) {
			if (pObject->bHasTexture) {

				// ′ò?a??àíó3é?
				glEnable(GL_TEXTURE_2D);

				glBindTexture(GL_TEXTURE_2D, Model->texture[pObject->materialID]);
			}
			else {

				// 1?±???àíó3é?
				glDisable(GL_TEXTURE_2D);
				glColor3ub(255, 255, 255);
			}
		}
		// ?aê?ò?g_ViewMode?￡ê?????
		glBegin(GL_TRIANGLES);
		// ±éàú?ùóDμ???
		for (int j = 0; j < pObject->numOfFaces; j++)
		{
			// ±éàúèy??D?μ??ùóDμ?
			for (int whichVertex = 0; whichVertex < 3; whichVertex++)
			{
				// ??μ?????????μ?μ??÷òy
				int index = pObject->pFaces[j].vertIndex[whichVertex];
				// ??3?·¨?òá?
				glNormal3f(pObject->pNormals[index].x, pObject->pNormals[index].y, pObject->pNormals[index].z);
				//è?1????ó??óD??àí
				if (pObject->bHasTexture) {

					// è·?¨ê?·?óDUVW??àí×?±ê
					if (pObject->pTexVerts) {
						glColor3f(1.0, 1.0, 1.0);
						glTexCoord2f(pObject->pTexVerts[index].x, pObject->pTexVerts[index].y);
					}
				}
				else{

					if (Model->pMaterials.size() && pObject->materialID >= 0)
					{
						BYTE *pColor = Model->pMaterials[pObject->materialID].color;
						glColor3ub(pColor[0], pColor[1], pColor[2]);
					}
				}
				glVertex3f(pObject->pVerts[index].x, pObject->pVerts[index].y, pObject->pVerts[index].z);
			}

		}

		glEnd();                // ?????áê?
	}
	if (touming)
		glDisable(GL_BLEND);

}