//
// Created by xlj on 2022/2/23.
// 各个功能模块的输入数据格式
//

#ifndef CABINBEHAVIORPERCEPTION_FUNCTIONPARAMS_H
#define CABINBEHAVIORPERCEPTION_FUNCTIONPARAMS_H
#include "opencv2/opencv.hpp"
#include "ALGVariables.h"
#include "PublicVariables.h"
#include <vector>
#include "map"
using namespace std;

// todo: 各个功能检测频率记录
typedef pair<bool, int64_t> FunTime;

typedef struct tagShipAreaInfo
{
    int nId;           //区域ID
    char strName[128]; //区域名字，如tzq1,tzq2
    //vector<string> vecAreaThres; //本区域内的报警规则阈值

    int64_t StartTimeStamp; //进区域时间
    int64_t EndTimeStamp;   //出区域时间
    bool bIsInArea; //是否在本区域内
} ShipAreaInfo;

typedef struct tagCallbackInit{
    LPAlgAlarmResCallBack alarmcallback;
    LPAlgInfoResCallBack infocallback;
    LPAlgReDetCallBack algdetcallback;
    void *userdata;
}CallbackInit;

// 感知任务编号：0-人数不足 1-瞭望 2-船长缺岗 3-巡逻 4-安全帽 5-工作服 6-打电话 7-抽烟 8-火灾 9-区域入侵 10-人数统计 11-人脸识别
//主检测器类型编号：0-瞭望 1-望远镜 2-人 3-人头 4-举手 5-摸脸 6-抱怀 7-手交叉抱怀 8-喝水 9-摸脖子 10-摸头 11-叉腰 12-打电话
//typedef struct tagDetRes{
//    float fConf;
//    float fTrackConf;
//    cv::Rect rctTgt;
//    int64_t iObjectId;
//    int iType;
//}DetRes;
typedef struct tagAlgInput{
    int funid = -1;
    vector<DpResult> detres;
    cv::Mat image; // 原图
    int64_t iTimeStamp;
    char *camid;
    char szTime[32];
    int iRegionType; // 船只区域
}AlgInput;


// 各功能模块初始化参数
typedef struct tagFunIni{
    char *camid;
    char szDevId[32];
    int funid; // 功能id
    int iCamType = 0; // 相机区域 1-驾驶室 2-集控室
    int iAeraType = 0; // ROI区域 1-驾驶台 2-巡逻区域
    vector<cv::Point_<int>> areaRois;
    int threTimes; // m分钟
    int threCounts; // n人数
}FunIni;

typedef struct tagInfoStatus{
    int funid{}; // 功能id
    char *camid{};
    int reportStatus{}; // 0-没有实时消息上报 1-有消息等待上传 2-待验证
    int statusTime; // 2状态保持的时间
    Info info{};
    cv::Mat oriMat;
}InfoStatus;

typedef struct tagAlarmStatus{
    int funid{}; // 功能id
    char *camid{};
    int reportStatus{}; // 0-没有报警 1-有报警消息等待上传 2-待验证
    int statusTime; // 2状态保持的时间
    Alarm alarm;
    vector<cv::Mat> oriMat;
}AlarmStatus;
#endif //CABINBEHAVIORPERCEPTION_FUNCTIONPARAMS_H
