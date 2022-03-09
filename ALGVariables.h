#ifndef _AFX_ALG_VARIABLES_H_
#define _AFX_ALG_VARIABLES_H_

#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PublicVariables.h"

using namespace std;

#define PIC_NUM 5

/*typedef struct tagFaceDetResult{
    cv::Rect rctTgt;     // 人脸框坐标
    vector<cv::Point> landmarks; // 人脸五个关键点
    cv::Mat face_img; // 矫正后的人脸图片
    char szCamID[64];
	int64_t iTimeStamp;
    char szTime[32];
}FaceDetResult;*/

typedef struct tagRegInfo
{
    char szPID[64];
    char szPName[64];
    cv::Rect rctTgt;
} RegInfo;

typedef struct tagFaceRegResult
{
    vector<RegInfo> regInfo;
    char szCamID[64];
    int64_t iTimeStamp;
    char szTime[32];
} FaceRegResult;

typedef struct tagClassInfo
{
    bool bRet;
    cv::Rect vecTgt;
}ClassInfo;

typedef struct tagClassResult
{
    vector<ClassInfo> classInfo;
    char szCamID[64];
    int64_t iTimeStamp;
    char szTime[32];
    int iType;
} ClassResult;

typedef struct tagAlarm
{
    int iType; //报警类型 0-人数不足 2-瞭望缺失 3-缺岗 4-区域巡逻 5-安全帽 6-工作服 7-抽烟 8-火灾 9-区域入侵
    cv::Rect rctTgt;
    int64_t iTimeStamp;
    char szTime[32];
    char szImgPath[PIC_NUM][128];
    int iDetArea;
    char szDevId[32];
    int ilevel;
    int iPicNum;
    int iRegionType;
    char szCamID[64];
} Alarm; //1.2.3.4

typedef struct tagInfo
{
    int iType;     //实时消息类型 0-人数， 1-身份识别结果
    int personNum; // 人数
    int64_t iTimeStamp;
    char szTime[32];
    char szImgPath[128]; // 人数：当前场景图片  身份识别：带人脸框和姓名的图片
    char szPID[64];
    char szPName[64];
    int iDetArea;
    char szDevId[32];
    int iRegionType;
    char szCamID[64];
} Info; //12有身份

typedef struct tagImgUnit
{
    char szCamID[64];
    int64_t iTimeStamp;
    char szTime[32];
    cv::Mat imgMat;
    // iDetType 0-不校正 1-maskrcnn 瞭望校正 2-maskrcnn人校正 3-yolov5打电话识别  4-yolov5抽烟识别 5-安全帽分类器 6-工作服分类器 7-人脸检测 8-人脸识别
    int iDetType;
    vector<cv::Rect> rctTgt;
} ImgUnit;

// 报警1级>2级>3级
//0-人数不足 2级报警  maskrcnn人校正
//1-瞭望动作缺失 1级报警 maskrcnn瞭望校正
//2-船长长期缺失 1级报警
//3-区域未巡逻 2级报警
//4-未佩戴安全帽 2级报警  分类器安全帽
//5-未穿工作服 2级报警   分类器工作服
//6-打电话 3级报警  yolov5
//7-抽烟 3级报警    yolov5
//8-火灾烟雾 1级报警
//9-区域入侵 2级报警
//10-人数统计 maskrcnn人
//12-身份识别

typedef void (*LPAlgAlarmResCallBack)(Alarm alarmRes, void *userData);
typedef void (*LPAlgInfoResCallBack)(Info infoRes, void *userData);
typedef void (*LPAlgReDetCallBack)(ImgUnit imgUnit, void *userData);

#endif
