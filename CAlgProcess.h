#ifndef _AFX_ALG_PROCESS_H_
#define _AFX_ALG_PROCESS_H_

#include <vector>
#include <string>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <vector>
#include "map"
#include <sys/timeb.h>
#include "utils.h"
#include "ALGVariables.h"
#include "PublicVariables.h"
#include "FunctionParams.h"
#include "FunctionProcess.h"

using namespace std;

class CALGProcess {
public:
    explicit CALGProcess();

    ~CALGProcess() = default;

public:
    void SetAlgPara(ConfigParaStruct cps, char *szEventPath);
    void SetRegionInfo(vector<vector<string>> vecRegInfo); // 定时更新
    void SetAlarmRules(vector<vector<string>> vecRules);   // 定时更新
    // iDpType 0-deepstream yolov5 1-maskrcnn 瞭望 2-maskrcnn人 3-yolov5打电话识别  4-yolov5抽烟识别
    void SendDeepLearnResult(DeepLearnResult dlRes, int iDpType); // 发送深度学习检测结果到算法模块
    // void SendFaceDetResult(FaceDetResult fdResResult);
    void SendFaceRegResult(FaceRegResult faceRegResult);
    void SendClassifierResult(ClassResult classRes);
    void SendYoloV5Result(YoloV5Result yolov5Res, int iDpType);

    // 回调设置
    LPAlgAlarmResCallBack m_AlarmRes{};
    LPAlgInfoResCallBack m_InfoRes{};
    LPAlgReDetCallBack m_AlgDet{};
    void *m_userData{};

    void SetCallBack(LPAlgAlarmResCallBack lpAlarmRes, LPAlgInfoResCallBack lpInfoRes, LPAlgReDetCallBack lpReDetRes,void *userdata);


private:
    // 船只区域
    string ShipAreaString[8] = {
            "NRZ", //普通区域NORMAL
            "BWZ", //恶劣天气BAD_WEATHER
            "SWZ", //特战区SPECIAL_WAR
            "SAZ", //六区一线
            "PRZ", //防海盗区PIRATE
            "SPZ", //特殊港口SPECAIL_PORT
            "NWZ", //狭水道NARROW_WATER
            "ANZ"  //靠离泊ANCHIR
    };
    vector<ShipAreaInfo> m_vecShipAreaInfo;
    vector<vector<string>> m_vecAlarmRules;
    bool m_bInNormalArea{}; //是否在普通区域
    int m_nShipAreaId=0; //船舶所在区域，随机选其一，用于传给服务软件
    int m_nBridgeOutNumberTimeLast{};

    // deepstream结果
    AlgInput algInput;

    // camera_id与功能，功能输入提取时间的对应关系 {"10001":[{false, 12345679}, {false, 12345679}, {true，456789}，...]}
    map<char *, vector<FunTime>> mapCamFunTime;

    // 相机区域与各个cameraid的对应关系
    map<char*, int> mapCamType;

    //线程管理
    bool m_getinput{};
    pthread_t thread1{};
    std::condition_variable mCondVar1; // 唤醒infer线程的条件变量
    mutex mtx1; // 定义一个互斥锁

    // 事件检测线程
    static void *InferProcess(void *arg);

    // 由功能类中产生消息回调处理函数
    static void GetAlarmCallBack(const Alarm& alarmRes, void *userData);
    static void GetInfoCallBack(const Info& infoRes, void *userData);
    static void GetAlgReDetCallBack(const ImgUnit& imgUnit, void *userData);
    //功能类初始化
    FunGroup1 *funGroup1{};// = new FunGroup1;    // 3-区域未巡逻 4-安全帽 5-工作服 6-打电话 7-抽烟
    FunGroup2 *funGroup2{};// = new FunGroup2;    // 0-人数不足 10-人数统计
    FunGroup3 *funGroup3{}; // = new FunGroup3;    // 11-实时身份识别 12-船长缺席 13-区域入侵 多摄像头
    FunGroup4 *funGroup4{}; // = new FunGroup4;    // 1-瞭望 8-火灾
};
#endif