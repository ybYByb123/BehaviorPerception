//
// Created by xlj on 2022/2/28.
//

#ifndef CABINBEHAVIORPERCEPTION_FUNCTIONPROCESS_H
#define CABINBEHAVIORPERCEPTION_FUNCTIONPROCESS_H

#include <utility>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "FunctionParams.h"
#include "ALGVariables.h"
#include "utils.h"

using namespace std;
// 全局互斥锁，控制功能类回调自己的消息，仅用于当前各功能类
static unique_lock<std::mutex> MUTEX;
static condition_variable CONVAR;

// 全局变量
// 控制各个功能类是否可以调用回调传消息
extern vector<bool> CALLBACKABLE;
// 相机所在区域和camera_id的对应关系{1: {"1001", "1002"}, 2:{"1003", "1004"}}
extern map<int, vector<char *>> MAPTYPECAMID;
// 结果图片保存路径
extern char EVENTIMAGEPATH[256]; // 事件图片和视频保存路径
// 帧率
extern int FPS;
// 相机所在区域与各功能模块的对应关系
extern map<int, vector<int>> MAPTYPEFUN;

# define FUN_NUM 12 //感知功能个数
// 报警等级
extern int LEVEL[FUN_NUM];

//各个功能获取输入的频率
// 0-人数不足， 1-瞭望， 2-船长缺席，3-区域巡逻，4-安全帽,5-工作服，6-打电话，7-抽烟，8-火灾，9-入侵，10-人数统计，11-身份识别
extern int FRENQUENCY[12];


// 功能基类
class BaseFunProcess {
public:
    BaseFunProcess() = default;

    virtual bool Initial(vector<FunIni> &funInis, CallbackInit callbackInit);

    virtual void SendAlgInput(AlgInput algInput) {};

    virtual ~BaseFunProcess() = default;

    //回调函数
    static LPAlgAlarmResCallBack m_AlarmRes;
    static LPAlgInfoResCallBack m_InfoRes;
    static LPAlgReDetCallBack m_AlgDet;
    static void *m_userData;

    void SetCallBack(LPAlgAlarmResCallBack lpAlarmRes, LPAlgInfoResCallBack lpInfoRes,
                                     LPAlgReDetCallBack lpReDetRes,void *userdata);

protected:
    // cameraid的各个roi区域 {"10001": {1, points}}
    map<pair<char *, int>, vector<cv::Point_<int>>> mapCamRoi;

    //各个cameraid里面的追踪目标
    map<char *, vector<int64_t>> mapCamTrackid;
    // 功能与各个cameraid
    map<int, vector<char *>> mapFunCam;

    // 相机区域与各个cameraid的对应关系
    map<int, vector<char *>> mapTypeCam;

    // 每个相机id的每个功能等待时长
    map<pair<char *, int>, int> mapCamFunTime;

    // 临时存放输入数据
    AlgInput m_algInput;
};

class FunGroup1 : public BaseFunProcess {
    // 单个摄像头就可以确认的简单功能
    // 3-区域未巡逻 巡逻区域
    // 4-安全帽
    // 5-工作服
    // 6-打电话
    // 7-抽烟
    // 9-区域入侵
    // ?-海盗刺
public:
    FunGroup1() = default;
    bool Initial(vector<FunIni> &funInis, CallbackInit callbackInit) override;
    void SendAlgInput(AlgInput algInput) override;
    void SendClassifierResult(ClassResult& classRes);
    void SendYoloV5Result(YoloV5Result yolov5Res, int iDpType);
    bool anti_pirate_thorn = false;
    bool helmatcloth_on = true;
    bool smokephone_on = true;
    bool roiround_on = true;
    int roiround_thresh = 20; //20分钟内区域没有人巡逻则报警


private:
    // 功能实现函数
    void helmat(const AlgInput& algInput);

    void workcloth(const AlgInput& algInput);

    void smoke_phone(const AlgInput& algInput);

    void roiround(AlgInput algInput);

    void roiInvade(AlgInput algInput);

    // 等待二次验证的计时器
    //void timeCount();

    int thisfuns[6] = {3, 4, 5, 6, 7, 9};

    // 各个功能上报初始化,巡逻、安全帽、工作服、打电话、抽烟、区域入侵
    vector<map<char*, AlarmStatus>> mapFunAlarms;

    // 周期性功能时间，开始、中间、结束三个时间点
    int RoiWaitTime=0;
    vector<char *> szTimeRoiRound; // 记录周期性的三个时间点

    // 瞬时功能重复确认三次结果
    vector<bool> ishelmat;
    vector<bool> isworkcloth;
    vector<bool> issmoke;
    vector<bool> isphone;

};

class FunGroup2 : public BaseFunProcess {
    //与人数统计相关的功能，多个摄像头联动
    // 0-人数不足， m分钟小于n人 驾驶台区域
    // 10-人数统计，定时上报人数
public:
    FunGroup2() = default;
    bool Initial(vector<FunIni> &funInis, CallbackInit callbackInit) override;

    void SendAlgInput(AlgInput algInput) override;

    void SendMrcnnResult(DeepLearnResult mrcnnRes);

    int num_thresh = 2;  // 人数不足报警阈值,外传参数
    float alarm_time = 0.5; // todo:人数不足持续多长时间产生报警？(单位分)
    int info_time = 10; // todo: 实时人数上报周期(单位分)

private:
    int time_thresh = 60; //检测时长1分钟
    char *mainCam{}; // 驾驶室用于检测人数的主相机，根据驾驶台区域大小决定
    int maxArea = 0; // 驾驶台最大的面积
    //功能中需要联合测试的相机组合{1: {'10001', '10002'}} 驾驶室，相机1和相机2
    map<int, vector<char *>> mapCamRel; //

    // 上报初始化
    map<int, AlarmStatus> mapPersonAlarm; // 驾驶室驾驶台有报警
    map<char*, InfoStatus> mapCountInfo;

    // 区域roi的最小外接矩形
    map<pair<char*, int>, cv::Rect_<int>> mapCamRoiMinRect;

    // todo: 最近一次上报时间,上报实时消息之前判断时是否满足上报间隔要求
    map<char*, int64_t> mapLastInfoTime;

    // 周期性功能时间，开始、中间、结束三个时间点
    int PersonLackTime=0;
    vector<char *> szTimePersonLack; // 记录周期性的三个时间点
};


class FunGroup3 : public BaseFunProcess {
    // 与人脸识别相关的功能
    // 11-实时身份识别 单个摄像头
    // 2-船长缺席 多摄像头联动 驾驶台区域
public:
    FunGroup3() = default;
    bool Initial(vector<FunIni> &funInis, CallbackInit callbackInit) override;
    void SendAlgInput(AlgInput algInput) override;
    void SendFaceRecoRes(FaceRegResult faceRegResult);

    bool captain_in_position = true;
    int captian_alarm_time = 30; // 要求连续在岗时长，单位分

private:
    // 多摄像头联动
    map<int, vector<char *>> mapCaptain; // 2-船长缺席
    map<int, vector<char *>> mapRoiInvade; // 9-区域入侵

    // 上报初始化
    map<char*, AlarmStatus> mapCaptainAlarm;
    map<char*, InfoStatus> mapIdInfo; // 身份识别消息上报

    // 周期性功能时间，开始、中间、结束三个时间点
    // 多个相机下船长在岗情况
    map<char*, vector<char*>> mapCamTimeCaptainLack;// 记录每个相机周期性的三个时间点
    map<char*, int> mapCamCaptainLackTime; // 记录每个相机周期记录时间
};


class FunGroup4 : public BaseFunProcess {
    // 多摄像头联动，一级报警
    // 1-瞭望
    // 8-火灾

public:
    FunGroup4() = default;
    bool Initial(vector<FunIni> &funInis, CallbackInit callbackInit) override;
    void SendAlgInput(AlgInput algInput) override;

    void SendMrcnnResult(DeepLearnResult mrcnnRes);

    int look_out_interval = 60; //瞭望产生报警时间要求（分钟）
    // 是否需要检测火灾
    bool fire_on = true;

private:
    // 需要联合测试的相机
    map<int, vector<char *>> mapLookout; // 1-瞭望
    map<char*, AlarmStatus> mapFireAlarm;
    map<char*, AlarmStatus> mapLookoutAlarm;

};

#endif //CABINBEHAVIORPERCEPTION_FUNCTIONPROCESS_H
