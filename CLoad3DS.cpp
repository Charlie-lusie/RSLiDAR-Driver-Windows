#include "CLoad3DS.h"

#pragma warning (disable: 4996) 


// ����ĺ��������������ʸ��
NBVector3 Vector(NBVector3 vPoint1, NBVector3 vPoint2)
{
	NBVector3 vVector;

	vVector.x = vPoint1.x - vPoint2.x;
	vVector.y = vPoint1.y - vPoint2.y;
	vVector.z = vPoint1.z - vPoint2.z;

	return vVector;
}

// ����ĺ�������ʸ�����
NBVector3 AddVector(NBVector3 vVector1, NBVector3 vVector2)
{
	NBVector3 vResult;

	vResult.x = vVector2.x + vVector1.x;
	vResult.y = vVector2.y + vVector1.y;
	vResult.z = vVector2.z + vVector1.z;

	return vResult;
}

// ����ĺ�������ʸ��������
NBVector3 DivideVectorByScaler(NBVector3 vVector1, float Scaler)
{
	NBVector3 vResult;

	vResult.x = vVector1.x / Scaler;
	vResult.y = vVector1.y / Scaler;
	vResult.z = vVector1.z / Scaler;

	return vResult;
}

// ����ĺ�����������ʸ���Ĳ��
NBVector3 Cross(NBVector3 vVector1, NBVector3 vVector2)
{
	NBVector3 vCross;

	vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));

	vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));

	vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

	return vCross;
}

// ����ĺ����淶��ʸ��
NBVector3 Normalize(NBVector3 vNormal)
{
	double Magnitude;

	Magnitude = Mag(vNormal);          // ���ʸ���ĳ���

	vNormal.x /= (float)Magnitude;
	vNormal.y /= (float)Magnitude;
	vNormal.z /= (float)Magnitude;

	return vNormal;
}

// ����һ������
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


// ���캯���Ĺ����ǳ�ʼ��tChunk����
CLoad3DS::CLoad3DS()
{
	m_CurrentChunk = new tChunk;        // ��ʼ����Ϊ��ǰ�Ŀ����ռ�
	m_TempChunk = new tChunk;          // ��ʼ��һ����ʱ�鲢����ռ�
}

// ��һ��3ds�ļ����������е����ݣ����ͷ��ڴ�
bool CLoad3DS::Import3DS(t3DModel *pModel, char *strFileName)
{
	char strMessage[255] = { 0 };

	// ��һ��3ds�ļ�
	m_FilePointer = fopen(strFileName, "rb");

	// ȷ������õ��ļ�ָ��Ϸ�
	if (!m_FilePointer)
	{
		sprintf(strMessage, "Unable to find the file: %s!", strFileName);
		//MessageBox(NULL, strMessage, "Error", MB_OK);
		return false;
	}

	// ���ļ���֮������Ӧ�ý��ļ��ʼ�����ݿ�������ж��Ƿ���һ��3ds�ļ�
	// �����3ds�ļ��Ļ�����һ����IDӦ����PRIMARY

	// ���ļ��ĵ�һ��������ж��Ƿ���3ds�ļ�
	ReadChunk(m_CurrentChunk);

	// ȷ����3ds�ļ�
	if (m_CurrentChunk->ID != PRIMARY)
	{
		sprintf(strMessage, "Unable to load PRIMARY chuck from file: %s!", strFileName);
		//MessageBox(NULL, strMessage, "Error", MB_OK);
		return false;
	}

	// ���ڿ�ʼ�������ݣ�ProcessNextChunk()��һ���ݹ麯��

	// ͨ����������ĵݹ麯�������������
	ProcessNextChunk(pModel, m_CurrentChunk);

	// �ڶ�������3ds�ļ�֮�󣬼��㶥��ķ���
	ComputeNormals(pModel);

	// �ͷ��ڴ�ռ�
	CleanUp();

	return true;
}

// ����ĺ����ͷ����е��ڴ�ռ䣬���ر��ļ�
void CLoad3DS::CleanUp()
{

	fclose(m_FilePointer);            // �رյ�ǰ���ļ�ָ��
	delete m_CurrentChunk;            // �ͷŵ�ǰ��
	delete m_TempChunk;              // �ͷ���ʱ��
}

// ����ĺ�������3ds�ļ�����Ҫ����
void CLoad3DS::ProcessNextChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
	t3DObject newObject = { 0 };          // ������ӵ���������
	tMaterialInfo newTexture = { 0 };        // ������ӵ���������
	unsigned int version = 0;          // �����ļ��汾
	int buffer[50000] = { 0 };          // ������������Ҫ������

	m_CurrentChunk = new tChunk;        // Ϊ�µĿ����ռ�    

	// ����ÿ��һ���¿飬��Ҫ�ж�һ�¿��ID������ÿ�����Ҫ�Ķ���ģ����������
	// ����ǲ���Ҫ����Ŀ飬���Թ�

	// ���������ӿ飬ֱ���ﵽԤ���ĳ���
	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
		// ������һ����
		ReadChunk(m_CurrentChunk);

		// �жϿ��ID��
		switch (m_CurrentChunk->ID)
		{
		case VERSION:              // �ļ��汾��

			// �ڸÿ�����һ���޷��Ŷ��������������ļ��İ汾

			// �����ļ��İ汾�ţ������ֽ�����ӵ�bytesRead������
			m_CurrentChunk->bytesRead += fread(&version, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);

			// ����ļ��汾�Ŵ���3������һ��������Ϣ
			if (version > 0x03)
				//MessageBox(NULL, "This 3DS file is over version 3 so it may load incorrectly", "Warning", MB_OK);
			break;

		case OBJECTINFO:            // ����汾��Ϣ

			// ������һ����
			ReadChunk(m_TempChunk);

			// �������İ汾��
			m_TempChunk->bytesRead += fread(&version, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

			// ���Ӷ�����ֽ���
			m_CurrentChunk->bytesRead += m_TempChunk->bytesRead;

			// ������һ����
			ProcessNextChunk(pModel, m_CurrentChunk);
			break;

		case MATERIAL:              // ������Ϣ

			// ���ʵ���Ŀ����
			pModel->numOfMaterials++;

			// ���������������һ���հ�����ṹ
			pModel->pMaterials.push_back(newTexture);

			// �������װ�뺯��
			ProcessNextMaterialChunk(pModel, m_CurrentChunk);
			break;

		case OBJECT:              // ���������

			// �ÿ��Ƕ�����Ϣ���ͷ���������˶���������

			// ����������
			pModel->numOfObjects++;

			// ���һ���µ�tObject�ڵ㵽����������
			pModel->pObject.push_back(newObject);

			// ��ʼ������������������ݳ�Ա
			memset(&(pModel->pObject[pModel->numOfObjects - 1]), 0, sizeof(t3DObject));

			// ��ò������������ƣ�Ȼ�����Ӷ�����ֽ���
			m_CurrentChunk->bytesRead += GetString(pModel->pObject[pModel->numOfObjects - 1].strName);

			// ��������Ķ�����Ϣ�Ķ���
			ProcessNextObjectChunk(pModel, &(pModel->pObject[pModel->numOfObjects - 1]), m_CurrentChunk);
			break;

		case EDITKEYFRAME:

			// �����ؼ�֡��Ķ��룬������Ҫ������ֽ���
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;

		default:

			// �������к��ԵĿ�����ݵĶ��룬������Ҫ������ֽ���
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;
		}

		// ���Ӵ����������ֽ���
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// �ͷŵ�ǰ����ڴ�ռ�
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}

// ����ĺ����������е��ļ��ж������Ϣ
void CLoad3DS::ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
	int buffer[50000] = { 0 };          // ���ڶ��벻��Ҫ������

	// ���µĿ����洢�ռ�
	m_CurrentChunk = new tChunk;

	// ��������������ֱ�����ӿ����
	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
		// ������һ����
		ReadChunk(m_CurrentChunk);

		// ������������ֿ�
		switch (m_CurrentChunk->ID)
		{
		case OBJECT_MESH:          // ���������һ���¿�

			// ʹ�õݹ麯�����ã�������¿�
			ProcessNextObjectChunk(pModel, pObject, m_CurrentChunk);
			break;

		case OBJECT_VERTICES:        // �����Ƕ��󶥵�
			ReadVertices(pObject, m_CurrentChunk);
			break;

		case OBJECT_FACES:          // ������Ƕ������
			ReadVertexIndices(pObject, m_CurrentChunk);
			break;

		case OBJECT_MATERIAL:        // ������Ƕ���Ĳ�������

			// �ÿ鱣���˶�����ʵ����ƣ�������һ����ɫ��Ҳ������һ������ӳ�䡣ͬʱ�ڸÿ���Ҳ������
			// ����������������

			// ����������Ĳ�������
			ReadObjectMaterial(pModel, pObject, m_CurrentChunk);
			break;

		case OBJECT_UV:            // ��������UV��������

			// ��������UV��������
			ReadUVCoordinates(pObject, m_CurrentChunk);
			break;

		default:

			// �Թ�����Ҫ����Ŀ�
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;
		}

		// ��Ӵ������ж�����ֽ�����ǰ��Ķ�����ֽ���
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// �ͷŵ�ǰ����ڴ�ռ䣬���ѵ�ǰ������Ϊǰ���
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}

// ����ĺ����������еĲ�����Ϣ
void CLoad3DS::ProcessNextMaterialChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
	int buffer[50000] = { 0 };          // ���ڶ��벻��Ҫ������

	// ����ǰ�����洢�ռ�
	m_CurrentChunk = new tChunk;

	// ����������Щ�飬֪�����ӿ����
	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
		// ������һ��
		ReadChunk(m_CurrentChunk);

		// �ж϶������ʲô��
		switch (m_CurrentChunk->ID)
		{
		case MATNAME:              // ���ʵ�����

			// ������ʵ�����
			m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;

		case MATDIFFUSE:            // �����R G B��ɫ
			ReadColorChunk(&(pModel->pMaterials[pModel->numOfMaterials - 1]), m_CurrentChunk);
			break;

		case MATMAP:              // ������Ϣ��ͷ��

			// ������һ�����ʿ���Ϣ
			ProcessNextMaterialChunk(pModel, m_CurrentChunk);
			break;

		case MATMAPFILE:            // �����ļ�������

			// ������ʵ��ļ�����
			m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strFile, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;

		default:

			// �ӹ�����Ҫ����Ŀ�
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;
		}

		// ��Ӵ������ж�����ֽ���
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// ɾ����ǰ�飬������ǰ������Ϊǰ��Ŀ�
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}

// ���溯��������ID�ź������ֽڳ���
void CLoad3DS::ReadChunk(tChunk *pChunk)
{
	// ������ID�ţ�ռ����2���ֽڡ����ID����OBJECT��MATERIALһ����˵�����ڿ���������������
	pChunk->bytesRead = fread(&pChunk->ID, 1, 2, m_FilePointer);

	// Ȼ������ռ�õĳ��ȣ��������ĸ��ֽ�
	pChunk->bytesRead += fread(&pChunk->length, 1, 4, m_FilePointer);
}

// ����ĺ�������һ���ַ���
int CLoad3DS::GetString(char *pBuffer)
{
	int index = 0;

	// ����һ���ֽڵ�����
	fread(pBuffer, 1, 1, m_FilePointer);

	// ֱ������
	while (*(pBuffer + index++) != 0) {

		// ����һ���ַ�ֱ��NULL
		fread(pBuffer + index, 1, 1, m_FilePointer);
	}

	// �����ַ����ĳ���
	return strlen(pBuffer) + 1;
}

// ����ĺ�������RGB��ɫ
void CLoad3DS::ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk)
{
	// ������ɫ����Ϣ
	ReadChunk(m_TempChunk);

	// ����RGB��ɫ
	m_TempChunk->bytesRead += fread(pMaterial->color, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

	// ���Ӷ�����ֽ���
	pChunk->bytesRead += m_TempChunk->bytesRead;
}

// ����ĺ������붥������
void CLoad3DS::ReadVertexIndices(t3DObject *pObject, tChunk *pPreviousChunk)
{
	unsigned short index = 0;          // ���ڶ��뵱ǰ�������

	// ����ö����������Ŀ
	pPreviousChunk->bytesRead += fread(&pObject->numOfFaces, 1, 2, m_FilePointer);

	// ����������Ĵ洢�ռ䣬����ʼ���ṹ
	pObject->pFaces = new tFace[pObject->numOfFaces];
	memset(pObject->pFaces, 0, sizeof(tFace)* pObject->numOfFaces);

	// �������������е���
	for (int i = 0; i < pObject->numOfFaces; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			// ���뵱ǰ��ĵ�һ���� 
			pPreviousChunk->bytesRead += fread(&index, 1, sizeof(index), m_FilePointer);

			if (j < 3)
			{
				// ��������������Ľṹ��
				pObject->pFaces[i].vertIndex[j] = index;
			}
		}
	}
}

// ����ĺ�����������UV����
void CLoad3DS::ReadUVCoordinates(t3DObject *pObject, tChunk *pPreviousChunk)
{
	// Ϊ�˶�������UV���꣬������Ҫ����UV�����������Ȼ��Ŷ�����������

	// ����UV���������
	pPreviousChunk->bytesRead += fread(&pObject->numTexVertex, 1, 2, m_FilePointer);

	// ���䱣��UV������ڴ�ռ�
	pObject->pTexVerts = new CVector2[pObject->numTexVertex];

	// ������������
	pPreviousChunk->bytesRead += fread(pObject->pTexVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

// �������Ķ���
void CLoad3DS::ReadVertices(t3DObject *pObject, tChunk *pPreviousChunk)
{
	// �ڶ���ʵ�ʵĶ���֮ǰ�����ȱ���ȷ����Ҫ������ٸ����㡣

	// ���붥�����Ŀ
	pPreviousChunk->bytesRead += fread(&(pObject->numOfVerts), 1, 2, m_FilePointer);

	// ���䶥��Ĵ洢�ռ䣬Ȼ���ʼ���ṹ��
	pObject->pVerts = new NBVector3[pObject->numOfVerts];
	memset(pObject->pVerts, 0, sizeof(NBVector3)* pObject->numOfVerts);

	// ���붥������
	pPreviousChunk->bytesRead += fread(pObject->pVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);

	// �����Ѿ����������еĶ��㡣
	// ��Ϊ3D Studio Max��ģ�͵�Z����ָ���ϵģ������Ҫ��y���z�ᷭת������
	// ����������ǽ�Y���Z�ύ����Ȼ��Z�ᷴ��

	// �������еĶ���
	for (int i = 0; i < pObject->numOfVerts; i++)
	{
		// ����Y���ֵ
		float fTempY = pObject->pVerts[i].y;

		// ����Y���ֵ����Z���ֵ
		pObject->pVerts[i].y = pObject->pVerts[i].z;

		// ����Z���ֵ����-Y���ֵ 
		pObject->pVerts[i].z = -fTempY;
	}
}

// ����ĺ����������Ĳ�������
void CLoad3DS::ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
	char strMaterial[255] = { 0 };      // �����������Ĳ�������
	int buffer[50000] = { 0 };        // �������벻��Ҫ������

	// ���ʻ�������ɫ�������Ƕ��������Ҳ���ܱ������������ȡ�����ȵ���Ϣ��

	// ������븳�赱ǰ����Ĳ�������
	pPreviousChunk->bytesRead += GetString(strMaterial);

	// �������е�����
	for (int i = 0; i < pModel->numOfMaterials; i++)
	{
		//�������������뵱ǰ����������ƥ��
		if (strcmp(strMaterial, pModel->pMaterials[i].strName) == 0)
		{
			// ���ò���ID
			pObject->materialID = i;

			// �ж��Ƿ�������ӳ�䣬���strFile��һ�����ȴ���1���ַ�������������
			if (strlen(pModel->pMaterials[i].strFile) > 0) {

				//��������
				BuildTexture(pModel->pMaterials[i].strFile, pModel->texture[pObject->materialID]);
				// ���ö��������ӳ���־
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
			// ����ö���û�в��ʣ�������IDΪ-1
			pObject->materialID = -1;
		}
	}

	pPreviousChunk->bytesRead += fread(buffer, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

// �������Щ������Ҫ�������㶥��ķ�����������ķ�������Ҫ�����������



// ����ĺ������ڼ������ķ�����
void CLoad3DS::ComputeNormals(t3DModel *pModel)
{
	NBVector3 vVector1, vVector2, vNormal, vPoly[3];

	// ���ģ����û�ж����򷵻�
	if (pModel->numOfObjects <= 0)
		return;

	// ����ģ�������еĶ���
	for (int index = 0; index < pModel->numOfObjects; index++)
	{
		// ��õ�ǰ�Ķ���
		t3DObject *pObject = &(pModel->pObject[index]);

		// ������Ҫ�Ĵ洢�ռ�
		NBVector3 *pNormals = new NBVector3[pObject->numOfFaces];
		NBVector3 *pTempNormals = new NBVector3[pObject->numOfFaces];
		pObject->pNormals = new NBVector3[pObject->numOfVerts];
		int i = 0;
		// ���������������
		for (i = 0; i < pObject->numOfFaces; i++)
		{
			vPoly[0] = pObject->pVerts[pObject->pFaces[i].vertIndex[0]];
			vPoly[1] = pObject->pVerts[pObject->pFaces[i].vertIndex[1]];
			vPoly[2] = pObject->pVerts[pObject->pFaces[i].vertIndex[2]];

			// ������ķ�����

			vVector1 = Vector(vPoly[0], vPoly[2]);    // ��ö���ε�ʸ��
			vVector2 = Vector(vPoly[2], vPoly[1]);    // ��ö���εĵڶ���ʸ��

			vNormal = Cross(vVector1, vVector2);    // �������ʸ���Ĳ��
			pTempNormals[i] = vNormal;          // ����ǹ淶��������
			vNormal = Normalize(vNormal);        // �淶����õĲ��

			pNormals[i] = vNormal;            // ����������ӵ��������б���
		}

		// �����󶥵㷨����
		NBVector3 vSum(0.0, 0.0, 0.0);
		NBVector3 vZero = vSum;
		int shared = 0;
		// �������еĶ���
		for (i = 0; i < pObject->numOfVerts; i++)
		{
			for (int j = 0; j < pObject->numOfFaces; j++)  // �������е���������
			{                        // �жϸõ��Ƿ����������湲��
				if (pObject->pFaces[j].vertIndex[0] == i ||
					pObject->pFaces[j].vertIndex[1] == i ||
					pObject->pFaces[j].vertIndex[2] == i)
				{
					vSum = AddVector(vSum, pTempNormals[j]);
					shared++;
				}
			}

			pObject->pNormals[i] = DivideVectorByScaler(vSum, float(-shared));

			// �淶�����Ķ��㷨��
			pObject->pNormals[i] = Normalize(pObject->pNormals[i]);

			vSum = vZero;
			shared = 0;
		}

		// �ͷŴ洢�ռ䣬��ʼ��һ������
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

	// ��������?��D��?D?����D��????��
	for (int i = 0; i < Model->numOfObjects; i++)
	{
		// ??��?�̡�?��??��?��????��
		t3DObject *pObject = &Model->pObject[i];
		// ?D???????����?��?��D??������3��?
		if (!outTex) {
			if (pObject->bHasTexture) {

				// �䨰?a??������3��?
				glEnable(GL_TEXTURE_2D);

				glBindTexture(GL_TEXTURE_2D, Model->texture[pObject->materialID]);
			}
			else {

				// 1?��???������3��?
				glDisable(GL_TEXTURE_2D);
				glColor3ub(255, 255, 255);
			}
		}
		// ?a��?��?g_ViewMode?�꨺?????
		glBegin(GL_TRIANGLES);
		// ��������?����D��???
		for (int j = 0; j < pObject->numOfFaces; j++)
		{
			// ����������y??D?��??����D��?
			for (int whichVertex = 0; whichVertex < 3; whichVertex++)
			{
				// ??��?????????��?��??�¨�y
				int index = pObject->pFaces[j].vertIndex[whichVertex];
				// ??3?����?����?
				glNormal3f(pObject->pNormals[index].x, pObject->pNormals[index].y, pObject->pNormals[index].z);
				//��?1????��??��D??����
				if (pObject->bHasTexture) {

					// ����?����?��?��DUVW??������?����
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

		glEnd();                // ?????����?
	}
	if (touming)
		glDisable(GL_BLEND);

}