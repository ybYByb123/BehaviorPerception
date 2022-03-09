#ifndef _AFX_ALG_PROCESS_H_
#define _AFX_ALG_PROCESS_H_

#include <vector>
#include <string>
#include <pthread.h>
#include "ALGVariables.h"
#include "PublicVariables.h"

using namespace std;

class CALGProcess
{
public:
    CALGProcess(/* args */);
    ~CALGProcess();

public:
    void SetAlgPara(ConfigParaStruct cps, char *szEventPath);
    void SetRegionInfo(vector<vector<string>> vecRegInfo); // 定时更新
    void SetAlarmRules(vector<vector<string>> vecRules);   // 定时更新
    // iDpType 0-deepstream yolov5 1-maskrcnn 瞭望 2-maskrcnn人 3-yolov5打电话识别  4-yolov5抽烟识别
    void SendDeepLearnResult(DeepLearnResult dlRes, int iDpType); // 发送深度学习检测结果到算法模块
                                                                  // void SendFaceDetResult(FaceDetResult fdResResult);
    void SendFaceRegResult(FaceRegResult faceRegResult);
    void SendClassifierResult(ClassResult classRes); 
    void SendYoloV5Result(YoloV5Result yolov5Res, iDpType);
    void SetCallBack(LPAlgAlarmResCallBack lpAlarmRes, LPAlgInfoResCallBack lpInfoRes, LPAlgReDetCallBack lpReDetRes, void *userdata);
};

#endif