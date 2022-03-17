#ifndef _AFX_PUBLIC_VARIABLES_H_
#define _AFX_PUBLIC_VARIABLES_H_

#include <cstdlib>
#include <unistd.h>
#include "opencv2/opencv.hpp"

#define DelayContinue(x)  \
	{                     \
		usleep(x * 1000); \
		continue;         \
	}

#define DelayTime(x) usleep(x * 1000)

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) \
	if (NULL != p)           \
	{                        \
		delete[] p;          \
		p = NULL;            \
	}
#endif

typedef struct tagCPoint
{
    int x;
    int y;
} CPoint;

#define IMAGE_WIDTH 4096
#define IMAGE_HEIGHT 2160
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT)
#define IMAGE_SIZE_RGB (IMAGE_WIDTH * IMAGE_HEIGHT * 3)
#define IMAGE_SIZE_YUV (IMAGE_SIZE * 3 / 2)

#define MAX_IMAGE_QUE_SIZE 500

// 阈值
#define AID_MAX_OBJECT_NUM 128 //一幅图像中目标个数最多为128个,同时最大跟踪轨迹数也是1024
#define MAX_DET_CHN_NUM 32     // 检测路数最大值
#define MAX_PARA_POINT_NUM 20    // 最多参数点个数
#define MAX_AREA_TYPE_NUM 5    //区域类型的最大个数

// 公共参数结构体
struct PubSysStruct
{
    int iGetVidType;  // 获取视频的方式0-软件下发 1-本地视频
    char LocalFilePath[256];// 本地视频路径
    int iDetFrmInter; // 算法处理帧间隔
    int iShowVidWin;  // 是否显示实时视频窗口

    int iSaveDpRes;		 // 是否保存带框结果
    char DpResPath[256]; // 深度学习带框结果图像保存路径

    char AlgIniPath[256];	// 算法所需ini所在路径
    char EventResPath[256]; // 事件图片和视频保存路径

    int iDiskSpace;	  // 硬盘所需最小空余空间GB
    int iDelInterval; // 清理数据库及硬盘的时间间隔

    char MySqlIP[32];       // MySQL数据库IP
    int iMySqlPort;          // MySQL数据库端口
    char MySqlUsr[32];      // MySQL数据库用户名
    char MySqlPass[32];     // MYSQL数据库密码
    char MySqlDbName[32];   // MYSQL数据库名字

    int iTrtNetType;	  // Tensorrt网络类型0-YOLOV3 1-YOLOV3_TINY 2-YOLOV4 3-YOLOV4_TINY 4-YOLOV5
    char TrtCfg[256];	  // Tensorrt Cfg路径
    char TrtWeights[256]; // Tensorrt Weights路径
    char TrtImgList[256]; // Tensorrt ImageList路径
    int iTrtPrecison;	  // 0-INT8 1-FP16 2-FP32
    float fTrtThresh;	  // 检测阈值

    char EnginePath[256]; // yolov5 enigine路径
    float fConfThresh;
    float fNmsThresh;

    char DevName[64];          // 设备名称
    int WDServerPort;          // 看门狗服务器端口
    int WDHttpPort;            // http通信端口
    char WDVersion[32];        // 软件版本
    char OutName[32];          // 可执行文件名字

    char HttpAlert[256];    // 报警消息消息地址
    char HttpArea[256];     // 实时报警消息地址
    char HttpFace[256];     // 人脸识别消息地址
    char HttpLookOut[256];  // 瞭望消息地址
    char HttpCross[256];    // 区域穿越地址
    char HttpPic[256];      // 图片传输地址

    char HttpDevStatus[256];  // 设备状态消息地址
    char HttpAreaReq[256];    // 区域请求消息地址
    char HttpAlarmRules[256]; // 报警条件请求消息地址
    char HttpSoftUpdate[256]; // 软件更新请求消息地址
    char HttpFinishUpdate[256];// 软件更新结束消息地址

    char DevId[128];   // 设备编号
    int iRequestType;  // 区域和报警规则请求方式0-信息化部 1-智能系统部

    float fFaceDetThresh;
    float fFaceRegThresh;
    char FaceDetWts[256];   //ydy人脸检测wts路径
    char FaceDetEngine[256]; //ydy人脸检测engine路径
    char FaceRegWts[256];   //ydy人脸识别wts路径
    char FaceRegEngine[256]; //ydy人脸识别engine路径
    char FaceRegSample[256]; //ydy人脸识别样本路径

    char InferConfPath[256]; //deepstream中推理配置文件所在路径
    char TrackConfPath[256]; //deepstream中跟踪配置文件所在路径
};

//此结构体必须和深度学习模块保持一致
typedef struct DP_OBJECTS_PIC //一副图片的所有坐标及类型信息,用于深度学习检测结果给算法模块传参
{
    int iDetID;
    int objects_num;
    int64_t iTimeStamp;						 // 图像时间戳
    int object_type[AID_MAX_OBJECT_NUM];	 //目标类型:1为行人，2自行车，3摩托车，4car，5bus，6truck。。。。，最多20个种类
    cv::Rect object_pos[AID_MAX_OBJECT_NUM]; //目标坐标,每帧最多128个
    float object_prob[AID_MAX_OBJECT_NUM];	 //目标置信度
} DP_OBJECTS_PIC;

// 事件录像结构体
typedef struct tagSaveVideoStruct
{
    int64_t iTimeStamp;	   // 时间戳 当同时发生多事件时，存储历史事件发生的时间戳
    char szVideoPath[256]; // 录像文件完整路径
    int64_t iEndTime;
    int iShortVideoEndTime;
    bool bSaveShortVideo;
} SaveVideoStruct;

//抓拍规则结构参数
struct RuleParameter
{
    int wfType;					//违法类型
    int nLineNum;				//直线数量
    CPoint szLinePoints[12];	//直线点数量
    int nAreaNum;				//区域数量
    int szPtNum[4];				//每个区域点数量
    CPoint szAreaPoints[4][24]; //组成区域的点
    int nDriveDirID;			//道路方向
    char szAddr[64];			//路名信息
    int iFactor1;
    int iFactor2;
    int id; //规则编号
};

// 参数结构体
struct AreaDataStruct
{
    int iType;	   // 区域类型 0-多边形 1-直线
    int iAreaType; // 1驾驶台，2巡逻区 3-电子围栏区（禁止入侵区域）
    int iPosNum;
    CPoint area[MAX_PARA_POINT_NUM]; //
};
struct CameraDataStruct
{
    char devid[64];						  // 设备id
    int iDetIDLen;						  // 设备id长度
    char cIpaddr[32];					  // 相机Ip
    int icport;							  // 相机port
    char cUser[32];						  // 相机用户名
    char cPass[32];						  // 相机密码
    char camid[64];						  // 相机id
    int iBrand;							  // 相机品牌 1-HK 2-DH 3-MindVision 4-other
    int iStreamType;					  // 解码方式 1-SDK 2-RTSP/FFMPEG
    char rtspAddr[128];					  // rtsp地址
    int iCamType;       // 1-驾驶室 2-集控室
};

struct DevDataStruct
{
    CameraDataStruct cds;  // 各相机信息
    double fwidth;          // 实际图像尺寸宽
    double fheight;         // 实际图像尺寸高
    double fAW;             // 图像尺寸宽
    double fAH;             // 图像尺寸高
    int iAreaNum;       // 区域个数
    AreaDataStruct ad[MAX_AREA_TYPE_NUM]; //检测区域信息，总共MAX_AREA_TYPE_NUM个
};

struct ConfigParaStruct
{
    float dthresh;   // 目标识别检测阈值
    float fdthresh;  // 人脸检测阈值
    float frthresh;  // 人脸识别阈值
    int iCamNum;     // 相机数量
    DevDataStruct dds[MAX_DET_CHN_NUM];
};

typedef struct DpResultStruct
{
    float fConf;
    float fTrackConf;
    cv::Rect rctTgt;
    int64_t iObjectId;
    int iType;
}DpResult;

typedef struct DeepLearnStruct
{
    char szCamID[64];
    char szTime[32];
    int64_t iTimeStamp;
    cv::Mat imgMat;
    std::vector<DpResult> vDpResult;
}DeepLearnResult;

typedef struct
{
    int iIndex;
    char rtspPath[256];
    char CamID[32];
}CamConf;

//yolov5检测目标ID
enum YOLOV5_OBJECT_TYPE
{
    LOOKOUT,  //0-瞭望
    TELESCOPE, //1-望远镜
    PERSON,    //2-人
    HEAD,      //3-手
    HANDSUP,   //4-举手
    TOUCHFACE, //5-摸脸
    HOLDCHEST, //6-抱怀
    CROSSCHEST,//7-手交叉抱怀
    DRINKWATER,//8-喝水
    TOUCHNECK, //9-摸脖子
    TOUCHHEAD, //10-摸头
    HANDSONWAIST,//11-叉腰
    HANDTELEPHONE//12-打电话
};

#endif