static const int SIZE_BLOCK = 100;
static const int RAW_SCAN_SIZE = 3;
static const int SCANS_PER_BLOCK = 32;
static const int BLOCK_DATA_SIZE = (SCANS_PER_BLOCK * RAW_SCAN_SIZE);  // 96

static const float ROTATION_RESOLUTION = 0.01f;   /**< degrees 旋转角分辨率*/
static const int ROTATION_MAX_UNITS = 36000; /**< hundredths of degrees */

static const float DISTANCE_MAX = 200.0f;            /**< meters */
static const float DISTANCE_MIN = 0.2f;              /**< meters */
static const float DISTANCE_RESOLUTION = 0.01f;      /**< meters */
static const float DISTANCE_RESOLUTION_NEW = 0.005f; /**< meters */
static const float DISTANCE_MAX_UNITS = (DISTANCE_MAX / DISTANCE_RESOLUTION + 1.0f);
/** @todo make this work for both big and little-endian machines */
static const int UPPER_BANK = 0xeeff;  //
static const int LOWER_BANK = 0xddff;

/** Special Defines for RS16 support **/
static const int RS16_FIRINGS_PER_BLOCK = 2;
static const int RS16_SCANS_PER_FIRING = 16;
static const float RS16_BLOCK_TDURATION = 100.0f;  // [µs]
static const float RS16_DSR_TOFFSET = 3.0f;        // [µs]
static const float RS16_FIRING_TOFFSET = 50.0f;    // [µs]

/** Special Defines for RS32 support **/
static const int RS32_FIRINGS_PER_BLOCK = 1;
static const int RS32_SCANS_PER_FIRING = 32;
static const float RS32_BLOCK_TDURATION = 50.0f;  // [µs]
static const float RS32_DSR_TOFFSET = 3.0f;       // [µs]
static const float RL32_FIRING_TOFFSET = 50.0f;   // [µs]

static const int TEMPERATURE_MIN = 31;

static const int PACKET_SIZE = 1248;
static const int BLOCKS_PER_PACKET = 12;
static const int PACKET_STATUS_SIZE = 4;
static const int SCANS_PER_PACKET = (SCANS_PER_BLOCK * BLOCKS_PER_PACKET);

float VERT_ANGLE[32];
float HORI_ANGLE[32];
float aIntensityCal[7][32] =
{
	15.93, 14.87, 15.57, 15.2, 15.74, 15.41, 14.85, 14.34, 13.65, 14.13, 11.75, 14.07, 14.95, 12.78, 13.16, 14.71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2.326, 1.968, 2.24, 2.045, 2.274, 2.193, 1.961, 1.861, 1.83, 1.803, 1.64, 1.79, 2.057, 1.751, 1.794, 1.931, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1.251, 1, 1.089, 1, 1.113, 1.31, 1.159, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	17.21, 13.97, 18.22, 15.71, 15.71, 18.77, 12.68, 7.538, 6.427, 7.733, 6.566, 7.815, 6.33, 4.101, 6.153, 6.828, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0.07167, 0.05687, 0.06466, 0.04543, 0.05589, 0.04863, 0.04687, 0.04295, 0.06817, 0.05155, 0.05767, 0.05295, 0.05875, 0.05891, 0.04616, 0.04495, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	-1.04, -0.1706, -0.5561, -0.00134, -0.9172, -0.1956, -0.3575, -0.3689, -0.9922, -0.5034, -0.8223, -0.4162, -1.086, -1.022, -0.7208, -0.2351, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	15.79, 10.55, 15.7, 7.863, 13.21, 8.758, 11.74, 8.483, 11.23, 7.975, 10.7, 7.957, 12.22, 9.969, 10, 6.786, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float aIntensityCal_old[1600][32];
int g_ChannelNum[32][51];
float CurvesRate[32];
bool Curvesis_new = true;


float temper = 31.0;
int tempPacketNum = 0;
int numOfLasers = 16;
int TEMPERATURE_RANGE = 40;
int dis_resolution_mode = 1;// 0 -> 0.5cm resolution 1-> 1cm resolution

float Calibration_Angle[16] = { -15.0161, -13.0276, -11.0066, -9.0274, -7.0263, -5.0078, -3.0124, -1.0097, 14.9693, 12.98, 10.9928, 8.9785, 6.9769, 4.9935, 2.9981, 0.9954 };
float Calibration_Curves[7][16] = {
	15.93, 14.87, 15.57, 15.2, 15.74, 15.41, 14.85, 14.34, 13.65, 14.13, 11.75, 14.07, 14.95, 12.78, 13.16, 14.71,
	2.326, 1.968, 2.24, 2.045, 2.274, 2.193, 1.961, 1.861, 1.83, 1.803, 1.64, 1.79, 2.057, 1.751, 1.794, 1.931,
	1, 1, 1, 1, 1, 1, 1, 1, 1.251, 1, 1.089, 1, 1.113, 1.31, 1.159, 1,
	17.21, 13.97, 18.22, 15.71, 15.71, 18.77, 12.68, 7.538, 6.427, 7.733, 6.566, 7.815, 6.33, 4.101, 6.153, 6.828,
	0.07167, 0.05687, 0.06466, 0.04543, 0.05589, 0.04863, 0.04687, 0.04295, 0.06817, 0.05155, 0.05767, 0.05295, 0.05875, 0.05891, 0.04616, 0.04495,
	-1.04, -0.1706, -0.5561, -0.00134, -0.9172, -0.1956, -0.3575, -0.3689, -0.9922, -0.5034, -0.8223, -0.4162, -1.086, -1.022, -0.7208, -0.2351,
	15.79, 10.55, 15.7, 7.863, 13.21, 8.758, 11.74, 8.483, 11.23, 7.975, 10.7, 7.957, 12.22, 9.969, 10, 6.786
};