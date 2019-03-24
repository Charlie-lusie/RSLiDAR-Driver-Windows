#include "rs16.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <string>
#include <glut.h>
#include <pthread.h>
#include "unistd.h"
#include "freeglut.h"
#include "CLoad3DS.h"


#pragma pack(1)

// Mode 0: Sensor Steam  1: pcap file
#define DebugMode 0
#define SaveMode 0

#define PCAP_FILE_NAME "2018-02-04.pcap"

////////////////////////////////////////////


#define MODEL_FILE_NAME "Data/3ds/AUDIAVUS.3DS"
CLoad3DS *gothicLoader = new(CLoad3DS);
t3DModel gothicModel;
float gothicTrans[14] = {
	0, 0, -1.4,
	0.0006, 0.0006, 0.0006,
	-90, 0, 0, 1,
	90, 1, 0, 0
};

#if DebugMode
#define HAVE_REMOTE
#include "pcap.h"
#include "remote-ext.h"
#else
#include <winsock.h>
#endif

#define WindowWidth 960
#define WindowHeight 640

#define NumOfPackage 84//586


#if DebugMode

pcap_t *indesc;
char errbuf[PCAP_ERRBUF_SIZE];
char source[PCAP_BUF_SIZE];
#endif



int gRSPort = 6699;
char gRSIP[50] = "192.168.1.200";//the ip of rs lidar

int gPointBias = NumOfPackage / 2;
int gShiftCountRight = 0;
int gShiftCountLeft = 0;

int gSocketRS;
int gSocketRecv;
int gSocketSend;

struct sockaddr_in gRSAddress;


struct UDP_Package
{
	char Head[42];
	char FiringData[1200];
	char Tail[6];
};

union UDP_Data
{
	char Recvchars[1248];
	UDP_Package package;
};

UDP_Data udpdata;

union TwoCharsInt
{
	char datain[2];
	unsigned short dataout;
};


char gOriginalData[1200 * NumOfPackage] = { 0 };
char gOriginalHead[42 * NumOfPackage] = { 0 };
char gProcessedData[1200 * NumOfPackage] = { 0 };
char gProcessedHead[42 * NumOfPackage] = { 0 };
float gPointX[12 * 32 * NumOfPackage] = { 0 };
float gPointY[12 * 32 * NumOfPackage] = { 0 };
float gPointZ[12 * 32 * NumOfPackage] = { 0 };
float gPointI[12 * 32 * NumOfPackage] = { 0 };
char gGround[12 * 32 * NumOfPackage] = { 0 };
float gSinVertAngle[64] = { 0 };
float gCosVertAngle[64] = { 0 };
float gSinRotCorre[64] = { 0 };
float gCosRotCorre[64] = { 0 };

// draw variables
float sPointX[12 * 32 * NumOfPackage] = { 0 };
float sPointY[12 * 32 * NumOfPackage] = { 0 };
float sPointZ[12 * 32 * NumOfPackage] = { 0 };
float sPointI[12 * 32 * NumOfPackage] = { 0 };
char sGround[12 * 32 * NumOfPackage] = { 0 };

pthread_mutex_t gRecvAndProcMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gProcAndShowMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gStop = PTHREAD_MUTEX_INITIALIZER;

bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float cameraDistance;
float cameraAngleX;
float cameraAngleY;
float mousescale = 1.0;

string getCurrentTime()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	WORD wYear = time.wYear;
	WORD wMonth = time.wMonth;
	WORD wDay = time.wDay;
	WORD wHour = time.wHour;
	WORD wMinute = time.wMinute;
	WORD wSecond = time.wSecond;
	WORD wmSecond = time.wMilliseconds;
	char str[200];
	sprintf(&str[0], "%d %02d %02d  %02d %02d %02d %03d", wYear, wMonth, wDay, wHour, wMinute, wSecond, wmSecond);
	return str;
}

void display(void)
{
	int lCounti, lCountj;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 100, 100, 0, 0, 0, 0, 0, 1);

	glTranslatef(0, 0, cameraDistance);
	glRotatef(cameraAngleX, 1, 0, 0);
	glRotatef(cameraAngleY, 0, 0, 1);
	glScalef(mousescale, mousescale, mousescale);

	//-------------------------------vehicle
	glPushMatrix();
	changeObject(gothicTrans);
	drawModel(&gothicModel, true, false);
	glPopMatrix();

	pthread_mutex_lock(&gProcAndShowMutex);
	// copy data
	for (lCounti = 0; lCounti < NumOfPackage * 12 * 32; lCounti++)
	{
		sPointX[lCounti] = gPointX[lCounti];
		sPointY[lCounti] = gPointY[lCounti];
		sPointZ[lCounti] = gPointZ[lCounti];
		sPointI[lCounti] = gPointI[lCounti];
		//sGround[lCounti] = gGround[lCounti];
	}
	pthread_mutex_unlock(&gProcAndShowMutex);

	glBegin(GL_POINTS);

	for (int i = 0; i < 12*32*NumOfPackage; i++)
	{
		//long dec = (sPointZ[i] + 2) > 0 ? sPointZ[i] + 2 : 0;
		//dec = dec * 2785237 + 255;
		//char hex[6] = { 0 };
		//for (int j = 6 - 1; j >= 0; j--)//将高度z 从[-2m, 4m]放缩到 #0000FF-#FF0000(255~16711680)
		//{
		//	hex[i] = (dec % 256) & 0xFF;
		//	dec /= 256;
		//}
		//int r = 0, g=  0, b = 0;
		//for (int j = 0; j < 2; j++)//#0000FF 每两位转为一个十进制数字（0-255）,对应rgb
		//{
		//	r += (int)(hex[j]) << (8 * (2 - 1 - j));
		//	g += (int)(hex[j + 2]) << (8 * (2 - 1 - j));
		//	b += (int)(hex[j + 4]) << (8 * (2 - 1 - j));
		//}
		//glColor3f(r/255, g/255, b/255);
		glColor3f(0, 1, 0);
		glVertex3f(sPointX[i], sPointY[i], sPointZ[i]);
	}
	glEnd();
	glutSwapBuffers();
}

void mouseCB(int button, int state, int x, int y)
{
	mouseX = x;
	mouseY = y;

	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			mouseLeftDown = true;
		}
		else if (state == GLUT_UP)
			mouseLeftDown = false;
	}

	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			pthread_mutex_lock(&gStop);
			mouseRightDown = true;
		}
		else if (state == GLUT_UP)
		{
			pthread_mutex_unlock(&gStop);
			mouseRightDown = false;
		}
	}

	else if (button == 3)
	{
		mousescale += 0.1;
		glutPostRedisplay();
	}

	else if (button == 4)
	{
		mousescale *= 0.9;
		glutPostRedisplay();
	}
}

void mouseMotionCB(int x, int y)
{
	if (mouseLeftDown)
	{
		cameraAngleY += (x - mouseX);
		cameraAngleX += (y - mouseY);
		mouseX = x;
		mouseY = y;
	}
	if (mouseRightDown)
	{
		cameraDistance += (y - mouseY) * 0.2f;
		mouseY = y;
	}

	glutPostRedisplay();
}

void* RSRecvThread(void * arg)
{
	int lLength, lCount, lCounti, lCountj;
	int SockSize = sizeof(SOCKADDR_IN);
	//int ANGLE_HEAD = -360001, last_azimuth = -36001, now_azimuth;
	char lOriginalData[1200 * NumOfPackage];
	char lOriginalHead[42 * NumOfPackage];
	int lPointBias;
	//lCount = NumOfPackage+1 ;
	lCount = 0;
	lPointBias = 0;

	while (1)
	{
		// changed 07.09
		lLength = recvfrom(gSocketRS, udpdata.Recvchars, 1248, 0, (struct sockaddr *)&gRSAddress, &SockSize);
		////////////////
		if (lLength == 1248)
		{
			//now_azimuth = 256 * udpdata.package.FiringData[2] + udpdata.package.FiringData[3];//Azimuth
			if (lCount == NumOfPackage)
			{
				pthread_mutex_lock(&gRecvAndProcMutex);
				for (lCountj = 0; lCountj < 1200 * NumOfPackage; lCountj++)
					gOriginalData[lCountj] = lOriginalData[lCountj];
				for (lCountj = 0; lCountj < 42 * NumOfPackage; lCountj++)
					gOriginalHead[lCountj] = lOriginalHead[lCountj];
				gPointBias = lPointBias;
				pthread_mutex_unlock(&gRecvAndProcMutex);
			}

			if (lCount < NumOfPackage)
			{
				for (lCountj = 0; lCountj < 1200; lCountj++)
				{
					lOriginalData[lCount * 1200 + lCountj] = udpdata.package.FiringData[lCountj];
				}
				for (lCountj = 0; lCountj < 42; lCountj++)
				{
					lOriginalData[lCount * 42 + lCountj] = udpdata.package.Head[lCountj];
				}
				lCount++;
			}
			else
				lCount = 0;
			/////////
		}
		else
		{
			printf("Length: %d is not 1248!\n", lLength);
		}
	}

	return nullptr;
}

//void loadConfigFile()
//{
//	int numOfLasers = 16;
//	int intensityFactor = 51;
//	int intensity_mode_ = 1;
//	//bool Curvesis_new;
//
//	/// 读参数文件 2017-02-27
//	FILE* f_inten = fopen("D:\\Projects\\ros_rslidar\\configuration_data\\curves.csv", "r");
//	int loopi = 0;
//	int loopj = 0;
//	int loop_num;
//	if (!f_inten)
//	{
//		std::cout << "curves path does not exist" << std::endl;
//	}
//	else
//	{
//		fseek(f_inten, 0, SEEK_END);  //定位到文件末
//		int size = ftell(f_inten);    //文件长度
//
//		if (size > 10000)  //老版的curve
//		{
//			Curvesis_new = false;
//			loop_num = 1600;
//		}
//		else
//		{
//			Curvesis_new = true;
//			loop_num = 7;
//		}
//		fseek(f_inten, 0, SEEK_SET);
//		while (!feof(f_inten))
//		{
//			float a[32];
//			loopi++;
//
//			if (loopi > loop_num)
//				break;
//			if (numOfLasers == 16)
//			{
//				int tmp = fscanf(f_inten, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", &a[0], &a[1], &a[2], &a[3],
//					&a[4], &a[5], &a[6], &a[7], &a[8], &a[9], &a[10], &a[11], &a[12], &a[13], &a[14], &a[15]);
//			}
//			if (Curvesis_new)
//			{
//				for (loopj = 0; loopj < numOfLasers; loopj++)
//				{
//					aIntensityCal[loopi - 1][loopj] = a[loopj];
//				}
//			}
//			else
//			{
//				for (loopj = 0; loopj < numOfLasers; loopj++)
//				{
//					aIntensityCal_old[loopi - 1][loopj] = a[loopj];
//				}
//			}
//			// ROS_INFO_STREAM("new is " << a[0]);
//		}
//		fclose(f_inten);
//	}
//
//
//	//
//	//=============================================================
//	FILE* f_angle = fopen("D:\\Projects\\ros_rslidar\\configuration_data\\angle.csv", "r");
//	if (!f_angle)
//	{
//		std::cout << "Angle path does not exist"<<std::endl;
//	}
//	else
//	{
//		float b[32], d[32];
//		int loopk = 0;
//		int loopn = 0;
//		while (!feof(f_angle))
//		{
//			int tmp = fscanf(f_angle, "%f,%f\n", &b[loopk], &d[loopk]);
//			loopk++;
//			if (loopk > (numOfLasers - 1))
//				break;
//		}
//		for (loopn = 0; loopn < numOfLasers; loopn++)
//		{
//			VERT_ANGLE[loopn] = b[loopn] / 180 * 3.141926;
//			HORI_ANGLE[loopn] = d[loopn] * 100;
//		}
//		fclose(f_angle);
//	}
//
//	//=============================================================
//	FILE* f_channel = fopen("D:\\Projects\\ros_rslidar\\configuration_data\\ChannelNum.csv", "r");
//	if (!f_channel)
//	{
//		std::cout<< "Channel path does not exist"<<endl;
//	}
//	else
//	{
//		int loopl = 0;
//		int loopm = 0;
//		int c[51];
//		int tempMode = 1;
//		while (!feof(f_channel))
//		{
//			if (numOfLasers == 16)
//			{
//				int tmp = fscanf(f_channel,
//					"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%"
//					"d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
//					&c[0], &c[1], &c[2], &c[3], &c[4], &c[5], &c[6], &c[7], &c[8], &c[9], &c[10], &c[11], &c[12],
//					&c[13], &c[14], &c[15], &c[16], &c[17], &c[18], &c[19], &c[20], &c[21], &c[22], &c[23], &c[24],
//					&c[25], &c[26], &c[27], &c[28], &c[29], &c[30], &c[31], &c[32], &c[33], &c[34], &c[35], &c[36],
//					&c[37], &c[38], &c[39], &c[40]);
//			}
//
//			for (loopl = 0; loopl < TEMPERATURE_RANGE + 1; loopl++)
//			{
//				g_ChannelNum[loopm][loopl] = c[tempMode * loopl];
//			}
//			loopm++;
//			if (loopm >(numOfLasers - 1))
//			{
//				break;
//			}
//		}
//		fclose(f_channel);
//	}
//
//}

float computeTemperature(unsigned char bit1, unsigned char bit2)
{
	float Temp;
	float bitneg = bit2 & 128;   // 10000000
	float highbit = bit2 & 127;  // 01111111
	float lowbit = bit1 >> 3;
	if (bitneg == 128)
	{
		Temp = -1 * (highbit * 32 + lowbit) * 0.0625f;
	}
	else
	{
		Temp = (highbit * 32 + lowbit) * 0.0625f;
	}

	return Temp;
}

int estimateTemperature(float Temper)
{
	int temp = (int)floor(Temper + 0.5);
	if (temp < TEMPERATURE_MIN)
	{
		temp = TEMPERATURE_MIN;
	}
	else if (temp > TEMPERATURE_MIN + TEMPERATURE_RANGE)
	{
		temp = TEMPERATURE_MIN + TEMPERATURE_RANGE;
	}

	return temp;
}

float pixelToDistance(int pixelValue, int passageway)
{
	float DistanceValue;
	int indexTemper = estimateTemperature(temper) - TEMPERATURE_MIN;
	if (pixelValue <= 0)//if (pixelValue <= g_ChannelNum[passageway][indexTemper])
	{
		DistanceValue = 0.0;
	}
	else
	{
		DistanceValue = (float)(pixelValue - 0);//(float)(pixelValue - g_ChannelNum[passageway][indexTemper])
	}
	return DistanceValue;
}

float CalibrateIntensity(float intensity, int calIdx, int distance)
{
	int algDist;
	int sDist;
	int uplimitDist;
	float realPwr;
	float refPwr;
	float tempInten;
	float distance_f;
	float endOfSection1, endOfSection2;
	int intensity_mode_ = 1, intensityFactor = 51;

	int temp = estimateTemperature(temper);

	realPwr = std::max((float)(intensity / (1 + (temp - TEMPERATURE_MIN) / 24.0f)), 1.0f);
	// realPwr = intensity;

	if (intensity_mode_ == 1)
	{
		// transform the one byte intensity value to two byte
		if ((int)realPwr < 126)
			realPwr = realPwr * 4.0f;
		else if ((int)realPwr >= 126 && (int)realPwr < 226)
			realPwr = (realPwr - 125.0f) * 16.0f + 500.0f;
		else
			realPwr = (realPwr - 225.0f) * 256.0f + 2100.0f;
	}
	else if (intensity_mode_ == 2)
	{
		// the caculation for the firmware after T6R23V8(16) and T9R23V6(32)
		if ((int)realPwr < 64)
			realPwr = realPwr;
		else if ((int)realPwr >= 64 && (int)realPwr < 176)
			realPwr = (realPwr - 64.0f) * 4.0f + 64.0f;
		else
			realPwr = (realPwr - 176.0f) * 16.0f + 512.0f;
	}
	else
	{
		std::cout << "The intensity mode is not right" << std::endl;
	}

	int indexTemper = estimateTemperature(temper) - TEMPERATURE_MIN;
	uplimitDist = g_ChannelNum[calIdx][indexTemper] + DISTANCE_MAX_UNITS;
	// limit sDist
	sDist = (distance > g_ChannelNum[calIdx][indexTemper]) ? distance : g_ChannelNum[calIdx][indexTemper];
	sDist = (sDist < uplimitDist) ? sDist : uplimitDist;
	// minus the static offset (this data is For the intensity cal useage only)
	algDist = sDist - g_ChannelNum[calIdx][indexTemper];

	// calculate intensity ref curves
	float refPwr_temp = 0.0f;
	int order = 3;
	endOfSection1 = 5.0f;
	endOfSection2 = 40.0;

	if (dis_resolution_mode == 0)
	{
		distance_f = (float)algDist * DISTANCE_RESOLUTION_NEW;
	}
	else
	{
		distance_f = (float)algDist * DISTANCE_RESOLUTION;
	}

	if (intensity_mode_ == 1)
	{
		if (distance_f <= endOfSection1)
		{
			refPwr_temp = aIntensityCal[0][calIdx] * exp(aIntensityCal[1][calIdx] - aIntensityCal[2][calIdx] * distance_f) +
				aIntensityCal[3][calIdx];
			//   printf("a-calIdx=%d,distance_f=%f,refPwr=%f\n",calIdx,distance_f,refPwr_temp);
		}
		else
		{
			for (int i = 0; i < order; i++)
			{
				refPwr_temp += aIntensityCal[i + 4][calIdx] * (pow(distance_f, order - 1 - i));
			}
			// printf("b-calIdx=%d,distance_f=%f,refPwr=%f\n",calIdx,distance_f,refPwr_temp);
		}
	}
	else if (intensity_mode_ == 2)
	{
		if (distance_f <= endOfSection1)
		{
			refPwr_temp = aIntensityCal[0][calIdx] * exp(aIntensityCal[1][calIdx] - aIntensityCal[2][calIdx] * distance_f) +
				aIntensityCal[3][calIdx];
			//   printf("a-calIdx=%d,distance_f=%f,refPwr=%f\n",calIdx,distance_f,refPwr_temp);
		}
		else if (distance_f > endOfSection1 && distance_f <= endOfSection2)
		{
			for (int i = 0; i < order; i++)
			{
				refPwr_temp += aIntensityCal[i + 4][calIdx] * (pow(distance_f, order - 1 - i));
			}
			// printf("b-calIdx=%d,distance_f=%f,refPwr=%f\n",calIdx,distance_f,refPwr_temp);
		}
		else
		{
			float refPwr_temp0 = 0.0f;
			float refPwr_temp1 = 0.0f;
			for (int i = 0; i < order; i++)
			{
				refPwr_temp0 += aIntensityCal[i + 4][calIdx] * (pow(40.0f, order - 1 - i));
				refPwr_temp1 += aIntensityCal[i + 4][calIdx] * (pow(39.0f, order - 1 - i));
			}
			refPwr_temp = 0.3f * (refPwr_temp0 - refPwr_temp1) * distance_f + refPwr_temp0;
		}
	}
	else
	{
		std::cout << "The intensity mode is not right" << std::endl;
	}

	refPwr = std::max(std::min(refPwr_temp, 500.0f), 4.0f);

	tempInten = (intensityFactor * refPwr) / realPwr;
	if (numOfLasers == 32)
	{
		tempInten = tempInten * CurvesRate[calIdx];
	}
	tempInten = (int)tempInten*255 > 255 ? 255.0f : tempInten;
	//std::cout << tempInten << endl;
	return tempInten; // 0~1
}

void* RSProcess(void *arg)
{
	float lPointX[12 * 32 * NumOfPackage] = { 0 };
	float lPointY[12 * 32 * NumOfPackage] = { 0 };
	float lPointZ[12 * 32 * NumOfPackage] = { 0 };
	float lPointI[12 * 32 * NumOfPackage] = { 0 };
	float azimuth, azimuth_diff, azimuth_corrected_f, intensity;
	int lDistance, azimuth_corrected;
	float temper = 31.0;

	while (1)
	{
		pthread_mutex_lock(&gStop);
		pthread_mutex_lock(&gRecvAndProcMutex);
		for (int i = 0; i < 1200 * NumOfPackage; i++)
		{
			gProcessedData[i] = gOriginalData[i];
		}
		for (int i = 0; i < 42 * NumOfPackage; i++)
		{
			gProcessedHead[i] = gOriginalHead[i];
		}
		//lPointBias = gPointBias;
		pthread_mutex_unlock(&gRecvAndProcMutex);
		pthread_mutex_unlock(&gStop);
		for (int lCounti = 0; lCounti < NumOfPackage; lCounti++)
		{
			for (int lCountj = 0; lCountj < 12; lCountj++)
			{

				if (tempPacketNum < 20000 && tempPacketNum > 0)  // update temperature information per 20000 packets
				{
					tempPacketNum++;
				}
				else
				{
					temper = computeTemperature(gProcessedHead[lCounti * 42 + 38], gProcessedHead[lCounti * 42 + 39]);//pkt.data[38], pkt.data[39]);
					tempPacketNum = 1;
				}

				azimuth = (float)(256 * gProcessedData[lCounti * 1200 + lCountj * 100 + 2] + gProcessedData[lCounti * 1200 + lCountj * 100 + 3]);
			/*	TwoCharsInt A;
				A.datain[1] = gProcessedData[lCounti * 1200 + lCountj * 100 + 2];
				A.datain[0] = gProcessedData[lCounti * 1200 + lCountj * 100 + 3];
				azimuth = A.dataout;*/

				if (lCountj < 11)
				{
					int azi1, azi2;
					azi1 = 256 * gProcessedData[lCounti * 1200 + (lCountj + 1) * 100 + 2] + gProcessedData[lCounti * 1200 + (lCountj + 1) * 100 + 3];
					azi2 = 256 * gProcessedData[lCounti * 1200 + lCountj * 100 + 2] + gProcessedData[lCounti * 1200 + lCountj * 100 + 3];
					azimuth_diff = (float)((36000 + azi1 - azi2) % 36000);

					// Ingnore the block if the azimuth change abnormal
					if (azimuth_diff <= 0.0 || azimuth_diff > 75.0)
					{
						continue;
					}
				}
				else
				{
					int azi1, azi2;
					azi1 = 256 * gProcessedData[lCounti * 1200 + lCountj * 100 + 2] + gProcessedData[lCounti * 1200 + lCountj * 100 + 3];
					azi2 = 256 * gProcessedData[lCounti * 1200 + (lCountj - 1) * 100 + 2] + gProcessedData[lCounti * 1200 + (lCountj - 1) * 100 + 3];
					azimuth_diff = (float)((36000 + azi1 - azi2) % 36000);

					// Ingnore the block if the azimuth change abnormal
					if (azimuth_diff <= 0.0 || azimuth_diff > 75.0)
					{
						continue;
					}
				}

				for (int lCountk = 0; lCountk < 32; lCountk++)
				{
					int dsr = lCountk % 16;
					int firing = (int)lCountk/ 16;
					azimuth_corrected_f = azimuth + (azimuth_diff * ((dsr * RS16_DSR_TOFFSET) + (firing * RS16_FIRING_TOFFSET)) /
						RS16_BLOCK_TDURATION);

					azimuth_corrected = ((int)round(azimuth_corrected_f)) % 36000;  // convert to integral value...
					TwoCharsInt tmp;
					tmp.datain[1] = gProcessedData[lCounti * 1200 + lCountj * 100 + lCountk * 3 + 4];//raw->blocks[block].data[k];
					tmp.datain[0] = gProcessedData[lCounti * 1200 + lCountj * 100 + lCountk * 3 + 5];// raw->blocks[block].data[k + 1];
					lDistance = tmp.dataout;
					tmp.datain[1] = 0x00;
					tmp.datain[0] = gProcessedData[lCounti * 1200 + lCountj * 100 + lCountk * 3 + 6];
					intensity = tmp.dataout;//gProcessedData[lCounti * 1200 + lCountj * 100 + lCountk * 3 + 6];

					//Intensity Calibration Code Here
					bool Curvesis_new = true;
					intensity = CalibrateIntensity(intensity, dsr, lDistance);


					float distance2 = lDistance;
					//float distance2 = pixelToDistance(lDistance, dsr);// still dont understand this function
					if (dis_resolution_mode == 0)  // distance resolution is 0.5cm
					{
						distance2 = distance2 * DISTANCE_RESOLUTION_NEW;
						//printf("distance : %.2f\n", distance2);
					}
					else
					{
						distance2 = distance2 * DISTANCE_RESOLUTION;
					}

					float arg_horiz = (float)azimuth_corrected / 18000.0f * 3.1415926;
					float arg_vert = Calibration_Angle[dsr] / 180 * 3.1415926;
					float max_distance = 20.0, min_distance = 0.2;

					if (distance2 > max_distance || distance2 < min_distance)  // invalid distance
					{
						//point.x = NAN;
						//point.y = NAN;
						//point.z = NAN;
						//point.intensity = 0;
						//pointcloud->at(2 * this->block_num + firing, dsr) = point;
						int pos = lCounti * 12 * 32 + lCountj * 32 + lCountk;
						lPointX[pos] = 0;
						lPointY[pos] = 0;
						lPointZ[pos] = 0;
						lPointI[pos] = 0;
					}
					else
					{
						// If you want to fix the rslidar Y aixs to the front side of the cable, please use the two line below
						// point.x = dis * cos(arg_vert) * sin(arg_horiz);
						// point.y = dis * cos(arg_vert) * cos(arg_horiz);

						// If you want to fix the rslidar X aixs to the front side of the cable, please use the two line below
						int pos = lCounti * 12 * 32 + lCountj * 32 + lCountk;
						float x = distance2 * cos(arg_vert) * cos(arg_horiz);
						float y = -distance2 * cos(arg_vert) * sin(arg_horiz);
						lPointX[pos] = x;
						lPointY[pos] = y;
						lPointZ[pos] = distance2 * sin(arg_vert);
						lPointI[pos] = intensity;
					}

				}
			}
		}


		pthread_mutex_lock(&gProcAndShowMutex);
		//gBoundaryPoints.clear();
		//gBoundaryPoints.assign(lBpoints.begin(), lBpoints.end());
		for (int lCounti = 0; lCounti < NumOfPackage * 12 * 32; lCounti++)
		{
			gPointX[lCounti] = lPointX[lCounti];
			gPointY[lCounti] = lPointY[lCounti];
			gPointZ[lCounti] = lPointZ[lCounti];
			gPointI[lCounti] = lPointI[lCounti];

		}
		pthread_mutex_unlock(&gProcAndShowMutex);

		glutPostRedisplay();

		//Sleep(100);
	}
	return nullptr;
}

int RS_Initial(int& sockfd, char address[50], int port, int bindflag)
{

	//int flag;
	struct sockaddr_in ca;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	ca.sin_family = AF_INET;
	ca.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ca.sin_port = htons(port);
	if (1 == bindflag)
	{
		// changed 07.09
		gRSAddress = ca;
		////////////////
		if (::bind(sockfd, (sockaddr *)&gRSAddress, sizeof(SOCKADDR)) == SOCKET_ERROR)
		{
			DWORD dwErr = GetLastError();																//bind 错误码查看 10049
			printf("bind failed!\n");
			return 0;
		}
	}
	return 1;

}

void* saveThread(void *arg)
{
	float lPointX[12 * 32 * NumOfPackage] = { 0 };
	float lPointY[12 * 32 * NumOfPackage] = { 0 };
	float lPointZ[12 * 32 * NumOfPackage] = { 0 };
	float lPointI[12 * 32 * NumOfPackage] = { 0 };
	char lGround[12 * 32 * NumOfPackage] = { 0 };
	bool label;
	string baseHDLPath = "D:\\Work\\RSLiDAR16\\";
	while (1)
	{
		string timeString = getCurrentTime();
		char filename[128];

		pthread_mutex_lock(&gProcAndShowMutex);
		for (int lCounti = 0; lCounti < NumOfPackage * 12 * 32; lCounti++)
		{
			lPointX[lCounti] = gPointX[lCounti];
			lPointY[lCounti] = gPointY[lCounti];
			lPointZ[lCounti] = gPointZ[lCounti];
			lPointI[lCounti] = gPointI[lCounti];
			//lGround[lCounti] = gGround[lCounti];
		}
		pthread_mutex_unlock(&gProcAndShowMutex);

		sprintf(filename, "D:\\Work\\RSLiDAR16\\%s.txt", &timeString[0]);
		FILE *fs = fopen(filename, "wt");
		for (int lCounti = 0; lCounti < NumOfPackage * 12 * 32; lCounti++)
		{
			if (lPointX[lCounti] == 0 && lPointY[lCounti] == 0) continue;
			fprintf(fs, "%.3f %.3f %.3f %.3f\n", lPointX[lCounti], lPointY[lCounti], lPointZ[lCounti], lPointI[lCounti]);
		}
		fclose(fs);
	}

}

int main(int argc, char* argv[])
{
	pthread_t lRS, lRSProcess, lSave;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow("Test");
	glutDisplayFunc(&display);
	glutMouseFunc(mouseCB);
	glutMotionFunc(mouseMotionCB);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 16.0 / 9.0, 5, 1000);


	//gothicLoader->Import3DS(&gothicModel, MODEL_FILE_NAME);

	pthread_setconcurrency(2);

	WSADATA WSAData;																					//WSAStartup initialize，only once
																										//
	if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0)
	{																									//
		return 0;																						//
	}																									//

#if DebugMode

	if (pcap_createsrcstr(source,
		PCAP_SRC_FILE,
		NULL,
		NULL,
		PCAP_FILE_NAME,
		errbuf
	) != 0)
	{
		fprintf(stderr, "\nError creating a source string\n");
		return 0;
	}
	/* 打开捕获文件 */
	if ((indesc = pcap_open(source, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the file %s.\n", source);
		return 0;
	}

#else


	if (RS_Initial(gSocketRS, gRSIP, gRSPort, 1))
	{
		printf("UDP Socket For RS16 ... Ready!\n");
	}
	else
	{
		printf("UDP Socket For RS16 ... Initial Error!\n");
	}

#endif

	pthread_create(&lRS, NULL, RSRecvThread, NULL);
	pthread_create(&lRSProcess, NULL, RSProcess, NULL);
#if SaveMode == 1
	pthread_create(&lSave, NULL, saveThread, NULL);
#endif

	while (1)
	{
		glutMainLoopEvent();  //event
		Sleep(1);
	}
	return 1;
}
