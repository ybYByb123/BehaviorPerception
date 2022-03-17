//
// Created by xlj on 2022/2/23.
//
#include "CAlgProcess.h"

#include <utility>

CALGProcess::CALGProcess() {
    // 创建事件处理线程
    pthread_attr_t attr1;
    pthread_attr_init(&attr1);
    pthread_create(&thread1, &attr1, InferProcess, (void *) this);//传递this 指针

    /*创建各个相机的功能处理线程
    std::queue<AlgInput> queue;
    for (int i = 0; i < MAX_DET_CHN_NUM; i++)
    {
        pthread_t hThread;
        m_ThreadPara.contentData = this;
        m_ThreadPara.iId = i;
        pthread_create(&hThread, nullptr, CamThread, &m_ThreadPara);
        // 为各个相机创建功能输入队列
        QAlgInputs.insert({i, queue});
    }*/

    /* 创建相机区域与功能时间戳之间的关系
    vector<InputFrequency> inputFrequency;
    for (auto &item:MAPTYPEFUN) {
        inputFrequency.clear();
        int icamtype = item.first;
        vector<int> funarray = item.second;
        for (int i = 0; i < FUN_NUM; ++i) {
            if (std::find(funarray.begin(), funarray.end(), i) != funarray.end())
                inputFrequency.emplace_back(true, timestamp);
            else
                inputFrequency.emplace_back(false, 0);
        }
        mapTypeFunTime.insert({icamtype, inputFrequency});
    }*/
}


void CALGProcess::SetAlgPara(ConfigParaStruct cps, char *szEventPath) {
    m_LogFile.TraceInfo("设置行为感知模块参数");
//    vector<int> fun1= {0, 1, 2, 3, 4, 5, 6, 8, 9, 11, 12};  //1-驾驶室
//    vector<int> fun2 = {0, 5, 7, 8, 9, 11, 12}; // 2-集控室
//    MAPTYPEFUN={make_pair(1, fun1), make_pair(2, fun2)};

    // 获取当前时间戳(毫秒)
    timeb t{};
    ftime(&t);
    int64_t timestamp = t.time * 1000 + t.millitm;

    // 记录这个相机id下所有的功能,初始化公共功能
    vector<int> thiscamfuns = {4, 5, 6, 7, 8, 11};
    // 图片保存路径
    memcpy(EVENTIMAGEPATH, szEventPath, 256 * sizeof(char));

    vector<FunIni> funinis_1; // Fungroup1的初始化参数
    vector<FunIni> funinis_2; // Fungroup2的初始化参数
    vector<FunIni> funinis_3; // Fungroup3的初始化参数
    vector<FunIni> funinis_4; // Fungroup4的初始化参数

    //记录驾驶室和集控室的功能
    for (int i = 0; i < cps.iCamNum; ++i) {
        FunIni funIni;
        char *camera_id = cps.dds[i].cds.camid;
        memcpy(funIni.camid, camera_id, sizeof(char) * 64);

        // 记录相机区域：驾驶室/集控室
        int m_CamType = cps.dds[i].cds.iCamType;
        funIni.iCamType = m_CamType;
        memcpy(funIni.szDevId, cps.dds[i].cds.devid, sizeof(char)*64);
        // 记录相机中划定的ROI
        map<int, vector<cv::Point_<int>>> areas;
        for (int j = 0; j < cps.dds[i].iAreaNum; ++j) {
            AreaDataStruct areaDataStruct = cps.dds[i].ad[j];
            vector<cv::Point_<int>> points;
            points.clear();
            for (int k = 0; k < areaDataStruct.iPosNum; ++k)
                points.emplace_back(areaDataStruct.area[k].x, areaDataStruct.area[k].y);
            if (areas.find(areaDataStruct.iAreaType) != areas.end())
                m_LogFile.TraceWarning("相机id: %d， 重复定义画面中相同的区域%d", camera_id, areaDataStruct.iAreaType);
            areas[areaDataStruct.iAreaType] = points;
        }

        // 4安全帽、5工作服、6电话、7抽烟、8火灾、11身份识别都是没有ROI的公共功能
        for (int j = 4; j < 8; ++j) {
            funIni.funid = j;
            funinis_1.push_back(funIni);
        }
        funIni.funid = 8;
        funinis_4.push_back(funIni);
        funIni.funid = 11;
        funinis_3.push_back(funIni);

        if (m_CamType == 1) {
            // 瞭望发生在驾驶室任意地方
            funIni.funid = 1;
            funinis_4.push_back(funIni);
            thiscamfuns.push_back(1);

            //区域中有驾驶台
            if (areas.find(1) != areas.end()) {
                funIni.areaRois = areas.find(1)->second;
                funIni.iAeraType = 1;
                // 人数不足与人数统计
                funIni.funid = 0;
//                thiscamfuns.push_back(0);
                funinis_2.push_back(funIni);
                funIni.funid = 10;
                thiscamfuns.push_back(10);
                funinis_2.push_back(funIni);

                // 船长缺席
                funIni.funid = 2;
                thiscamfuns.push_back(2);
                funinis_3.push_back(funIni);
            }
            //区域中有巡逻区
            if (areas.find(2) != areas.end()) {
                funIni.funid = 3;
                thiscamfuns.push_back(3);
                funIni.iAeraType = 2;
                funIni.areaRois = areas.find(2)->second;
                funinis_1.push_back(funIni);
            }
        } else if (m_CamType == 2) {
            // 集控室统计实时人数
            funIni.funid = 10;
            thiscamfuns.push_back(10);
            funinis_2.push_back(funIni);
        } //todo:以后可能还有其他地点加入


        /*记录每个区域与相机id的对应关系
        auto iter = MAPTYPECAMID.find(m_CamType);
        if (iter != MAPTYPECAMID.end()){
            iter->second.push_back(camera_id);
        }else{
            vector<char*> camera_ids;
            camera_ids.push_back(camera_id);
            MAPTYPECAMID.insert({m_CamType, camera_ids});
        }*/
        //相机id与功能模块的对应关系，检测时间，存储到mapCamfunTime字典中
        bool tag;
        auto iter_1 = mapCamFunTime.find(camera_id);
        if (iter_1 == mapCamFunTime.end()) {
            mapCamFunTime[camera_id] = {};
            for (int j = 0; j < FUN_NUM; ++j) {
                tag = false;
                if (std::find(thiscamfuns.begin(), thiscamfuns.end(), j) != thiscamfuns.end())
                    // 在相机功能队列中找到这个功能
                    mapCamFunTime[camera_id].push_back(make_pair(true, timestamp));
                else
                    mapCamFunTime[camera_id].push_back(make_pair(false, timestamp));
            }
        } else
            m_LogFile.TraceWarning("重复初始化相机%d, 请检查参数配置！", camera_id);
    }

    //对各功能类传入参数
    //参数下发到各个功能类
    CallbackInit callbackInit{
            reinterpret_cast<LPAlgAlarmResCallBack>(GetAlarmCallBack),
            reinterpret_cast<LPAlgInfoResCallBack>(GetInfoCallBack),
            reinterpret_cast<LPAlgReDetCallBack>(GetAlgReDetCallBack),
            nullptr
    };
    funGroup1->Initial(funinis_1, callbackInit);
    funGroup2->Initial(funinis_2, callbackInit);
    funGroup3->Initial(funinis_3, callbackInit);
    funGroup4->Initial(funinis_4, callbackInit);
}

// 传入主检测器结果\maskrcnn\yolov5抽烟打电话检测结果
void CALGProcess::SendDeepLearnResult(DeepLearnResult dlRes, int iDpType) {
    if (iDpType == 0) {
        if (!dlRes.vDpResult.empty()) {
            unique_lock<std::mutex> lock(mtx1);
            algInput.iRegionType = m_nShipAreaId;
            strncpy(algInput.camid, dlRes.szCamID, strlen(dlRes.szCamID));
            algInput.iTimeStamp = dlRes.iTimeStamp;
            algInput.image = dlRes.imgMat.clone();
            algInput.detres = dlRes.vDpResult;
            memcpy(algInput.szTime, dlRes.szTime, sizeof(char)*32);
            m_getinput = true;
            mCondVar1.notify_one();
        } else
            m_getinput = false;
    } else {
        if (iDpType == 1){
            // maskrcnn瞭望验证结果
            funGroup4->SendMrcnnResult(dlRes);
        }

        if (iDpType == 2){
            // maskrcnn人数验证
            funGroup2->SendMrcnnResult(dlRes);
        }
    }
}

void CALGProcess::SendFaceRegResult(FaceRegResult faceRegResult) {
    funGroup3->SendFaceRecoRes(faceRegResult);
}

void CALGProcess::SendClassifierResult(ClassResult classRes) {
    //安全帽、工作服穿戴分类结果
    funGroup1->SendClassifierResult(classRes);
}

void CALGProcess::SendYoloV5Result(YoloV5Result yolov5Res, int iDpType) {
    // 抽烟打电话yolov5结果传入
    funGroup1->SendYoloV5Result(yolov5Res, iDpType);
}

void *CALGProcess::InferProcess(void *arg) {
    auto *p = (CALGProcess *) arg;
    AlgInput algInput;
    int64_t thistime;
    while (true) {
        std::unique_lock<std::mutex> lock(p->mtx1); // std::unique_lock管理的锁对象会一直保持上锁状态
        p->mCondVar1.wait(lock, [&p]() { return !p->m_getinput; });
        // 唤醒线程
        algInput = p->algInput;
        thistime = algInput.iTimeStamp;
        auto iter = p->mapCamFunTime.find(p->algInput.camid);
        auto fun_time = iter->second;  //这个相机下的功能和上次取到结果的时间戳

        for (int i = 0; i < fun_time.size(); ++i) {
            if (fun_time[i].first and (fun_time[i].second - algInput.iTimeStamp) / 1000 >= FRENQUENCY[i]){
                algInput.funid = i;
                fun_time[i].second = thistime;
                if (i==3 or i==4 or i==5 or i==6 or i==7)
                    p->funGroup1->SendAlgInput(algInput);
                if (i==0 or i==10)
                    p->funGroup2->SendAlgInput(algInput);
                if (i==11 or i==2 or i==9)
                    p->funGroup3->SendAlgInput(algInput);
                if (i==1 or i==8)
                    p->funGroup4->SendAlgInput(algInput);
            }
        }
        p->m_getinput = false;
    }
    return nullptr;
}

// 消息回调函数,接收到功能类的消息，即可回调给主进程处理
void CALGProcess::SetCallBack(LPAlgAlarmResCallBack lpAlarmRes, LPAlgInfoResCallBack lpInfoRes,
                              LPAlgReDetCallBack lpReDetRes,void *userdata){
    m_AlarmRes = lpAlarmRes;
    m_InfoRes = lpInfoRes;
    m_AlgDet = lpReDetRes;
    m_userData = userdata;
}

void CALGProcess::GetAlarmCallBack(const Alarm& alarmRes, void *userData) {
    m_AlarmRes(alarmRes, userData);
    CALLBACKABLE[0] = true;
}

void CALGProcess::GetInfoCallBack(const Info& infoRes, void *userData) {
    m_InfoRes(infoRes, userData);
    CALLBACKABLE[1] = true;
}

void CALGProcess::GetAlgReDetCallBack(const ImgUnit& imgUnit, void *userData) {
    m_AlgDet(imgUnit, userData);
    CALLBACKABLE[2] = true;
}


void CALGProcess::SetAlarmRules(vector<vector<string>> vecRules)
{
    //m_vecAlarmRules.assign(vecRules.begin(), vecRules.end());

    m_vecAlarmRules.clear();
    for (int i = 1; i < vecRules.size(); i++) //从1开始，0为字段名称
    {
        vector<string> AreaTmp;
        AreaTmp.assign(vecRules[i].begin(), vecRules[i].end());

        m_vecAlarmRules.push_back(AreaTmp);
    }
}

//0-mmsi 1-regiontype 2-crosstype
void CALGProcess::SetRegionInfo(vector<vector<string>> vecRegInfo) //0-regiontype 1-crosstype(进/出) 2-time 3-name
{
    //更新区域进出时间,不包含普通区域
    for (int i = vecRegInfo.size() - 1; i >= 0; i--) //8个区域，每个区域的最新2条数据。vector倒序，即按时间顺序处理
    {
        //找到区域编号
        int nAreaIdx;
        for (int j = 0; j < 8; j++) //8个区域,通过字符串找到区域编号？？？
        {
            if (!strcmp(ShipAreaString[j].c_str(), vecRegInfo[i][0].c_str()))
            {
                nAreaIdx = j;
                break;
            }
        }

        //找到同一个区域id同一个name的历史记录
        bool bFindFlag = false;
        int nFindIdx = -1;
        for (int j = 0; j < m_vecShipAreaInfo.size(); j++)
        {
            if (m_vecShipAreaInfo[j].nId == nAreaIdx && !strcmp(m_vecShipAreaInfo[j].strName, vecRegInfo[i][3].c_str())) //找到已有的区域记录
            {
                bFindFlag = true;
                nFindIdx = j;
                break;
            }
        }

        int64_t nTimeStamp = Time2StampTime(vecRegInfo[i][2], 3);

        if (!strcmp(vecRegInfo[i][1].c_str(), "IN")) //进入某个区域
        {
            if (bFindFlag) //找到则更新，找不到则push
            {
                m_vecShipAreaInfo[nFindIdx].StartTimeStamp = nTimeStamp;
                m_vecShipAreaInfo[nFindIdx].bIsInArea = true;
            }
            else
            {
                ShipAreaInfo AreaTmp;

                AreaTmp.nId = nAreaIdx;
                strcpy(AreaTmp.strName, vecRegInfo[i][3].c_str());
                AreaTmp.StartTimeStamp = nTimeStamp;
                AreaTmp.bIsInArea = true;

                m_vecShipAreaInfo.push_back(AreaTmp);

                if (m_vecShipAreaInfo.size() > 1000)
                    printf("m_vecShipAreaInfo.size()>1000\n");
            }

            printf("-------------area:%d,IN\n", nAreaIdx);
            m_LogFile.TraceInfo("-------------area:%d,IN\n", nAreaIdx);
        }
        else if (!strcmp(vecRegInfo[i][1].c_str(), "OUT")) //出某个区域
        {
            if (bFindFlag) //找到则更新，找不到则不处理
            {
                m_vecShipAreaInfo[nFindIdx].EndTimeStamp = nTimeStamp;
                m_vecShipAreaInfo[nFindIdx].bIsInArea = false;
            }

            printf("------------------area:%d,OUT\n", nAreaIdx);
            m_LogFile.TraceInfo("------------------area:%d,OUT\n", nAreaIdx);
        }
    }

    //清除已经out的区域,清除StartTimeStamp超过10天的区域
    std::vector<ShipAreaInfo>::iterator itArea;
    for (itArea = m_vecShipAreaInfo.begin(); itArea != m_vecShipAreaInfo.end();)
    {
        if (!itArea->bIsInArea /*|| GetTimeStamp() - itArea->StartTimeStamp > 10 * 24 * 3600*/) //???暂时不能删除，模拟时会把所有模拟历史数据删掉，无法测试
        {
            itArea = m_vecShipAreaInfo.erase(itArea);
        }
        else
            itArea++;
    }

    if (m_vecShipAreaInfo.empty()) //在普通区域
    {
        m_bInNormalArea = true;
        m_nShipAreaId = 0;
        if (atoi(m_vecAlarmRules[0][1].c_str())==1)
            funGroup3->captain_in_position  = true;
        else
            funGroup3->captain_in_position  = false;
        funGroup2->num_thresh = atoi(m_vecAlarmRules[0][2].c_str()); //人数不足报警阈值
        funGroup4->look_out_interval = atoi(m_vecAlarmRules[0][3].c_str()) * 60; // 瞭望间隔
        // m_nTimeLen.bNeedJudgePirate = atoi(m_vecAlarmRules[0][4].c_str()); 海盗刺
    }
    else //在特殊区域
    {
        m_bInNormalArea = false;
        m_nShipAreaId = m_vecShipAreaInfo[0].nId; //船舶所在区域，随机选其一，用于传给服务软件

        for (auto & i : m_vecShipAreaInfo)
        {
            if (!strcmp(m_vecAlarmRules[i.nId][1].c_str(), "1")) //需要检测船长是否在岗
            {
                funGroup3->captain_in_position = true;
                // todo: 应当传入船长连续在岗最小时间要求
            }

            int nPersonNum = atoi(m_vecAlarmRules[i.nId][2].c_str()); //本区域驾驶台人数阈值
            if (nPersonNum < funGroup2->num_thresh)                              //找到驾驶台人数最小值
            {
                funGroup2->num_thresh = nPersonNum;
            }

            int nLookTimeLen = atoi(m_vecAlarmRules[i.nId][3].c_str()) * 60; //瞭望
            if (nLookTimeLen < funGroup4->look_out_interval)                                       //找到驾驶台人数最小值
            {
                funGroup4->look_out_interval = nLookTimeLen;
            }

            if (!strcmp(m_vecAlarmRules[i.nId][4].c_str(), "1")) //需要检测防海盗刺
            {
                funGroup1->anti_pirate_thorn = true;
            }
        }
    }
    //m_nTimeLen.nBridgeOutNumberTimes = 0; //驾驶台人数小于阈值的次数清零
    //m_NoPersonHis[0].m_nNoPersonStart; //无瞭望清零
}






