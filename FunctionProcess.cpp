//
// Created by xlj on 2022/2/28.
//

#include "FunctionProcess.h"

// 全局变量定义
vector<bool> CALLBACKABLE = {true,true, true};
char EVENTIMAGEPATH[256]{};
int FPS = 8;
map<int, vector<char *>> MAPTYPECAMID{};
map<int, vector<int>> MAPTYPEFUN{};
int LEVEL[FUN_NUM] = {2, 1, 1, 2, 2, 2, 3, 3, 1, 2, 0, 0};
int FRENQUENCY[FUN_NUM] = {10, 5, 5, 1, 30, 30, 5, 5, 60, 1, 10, 1}; //单位：秒

bool BaseFunProcess::Initial(vector<FunIni> &funInis, CallbackInit callbackInit) {
    // 设置回调
    SetCallBack(callbackInit.alarmcallback, callbackInit.infocallback, callbackInit.algdetcallback,callbackInit.userdata);

    for (auto & funIni : funInis) {
        // funCamid记录 {'0':{camid1, camid2}, '1':{camid3, camid4}}
        int funid = funIni.funid;
        if (mapFunCam.find(funid)!= mapFunCam.end()){
            mapFunCam[funid].push_back(funIni.camid);
        }else{
            vector<char *> camids;
            camids.push_back(funIni.camid);
            mapFunCam.insert(make_pair(funid, camids));
        }

        // 身份识别、安全帽工作服检测场景需要跟踪id
        if(mapCamTrackid.find(funIni.camid) != mapCamTrackid.end() and (funid==4 or funid==5
                                                                        or funid==11))
            mapCamTrackid[funIni.camid] = {};

        // 记录每个camera的ROI
        if (funIni.iAeraType != 0)
            mapCamRoi[make_pair(funIni.camid, funIni.iAeraType)] = funIni.areaRois;

        // 记录每个功能下的cameraid
        if(mapTypeCam.find(funIni.iCamType) != mapTypeCam.end())
            mapTypeCam[funIni.iCamType] = {funIni.camid};
        else
            mapTypeCam[funIni.iCamType].push_back(funIni.camid);
    }
    return true;
}

void BaseFunProcess::SetCallBack(LPAlgAlarmResCallBack lpAlarmRes, LPAlgInfoResCallBack lpInfoRes,
                              LPAlgReDetCallBack lpReDetRes,void *userdata){
    m_AlarmRes = lpAlarmRes;
    m_InfoRes = lpInfoRes;
    m_AlgDet = lpReDetRes;
    m_userData = userdata;
}


bool FunGroup1::Initial(vector<FunIni> &funInis, CallbackInit callbackInit) {
    // 设置回调
    SetCallBack(callbackInit.alarmcallback, callbackInit.infocallback, callbackInit.algdetcallback,callbackInit.userdata);

    AlarmStatus alarmStatus;
    for (auto & funIni : funInis) {
        // funCamid记录 {'0':{camid1, camid2}, '1':{camid3, camid4}}
        int funid = funIni.funid;
        // 身份识别、安全帽工作服检测场景需要跟踪id
        if(mapCamTrackid.find(funIni.camid) != mapCamTrackid.end() and (funid==4 or funid==5
                                                                        or funid==11))
            mapCamTrackid[funIni.camid].reserve(1000);

        // 记录每个camera的ROI
        if (funIni.iAeraType != 0)
            mapCamRoi[make_pair(funIni.camid, funIni.iAeraType)] = funIni.areaRois;
        for (int i = 0; i < 6; ++i) {
            if (thisfuns[i] == funid){
                strcpy(alarmStatus.camid, funIni.camid);
                alarmStatus.funid = funIni.funid;
                alarmStatus.reportStatus = 0;
                memcpy(alarmStatus.alarm.szCamID, funIni.camid, sizeof(char )*64);
                memcpy(alarmStatus.alarm.szDevId, funIni.szDevId, sizeof(char )*32);
                alarmStatus.alarm.iType = funid;
                alarmStatus.alarm.iDetArea = funIni.iCamType;
                alarmStatus.alarm.ilevel = LEVEL[funid];
                if (funid == 3){
                    alarmStatus.oriMat.reserve(3);
                    alarmStatus.alarm.iPicNum = 3;
                }
                else{
                    alarmStatus.oriMat.reserve(1);
                    alarmStatus.alarm.iPicNum = 1;
                }
                mapFunAlarms[i][funIni.camid] = alarmStatus;

            }
        }
    }
}

// 人数相关功能
bool FunGroup2::Initial(vector<FunIni> &funInis, CallbackInit callbackInit) {
    // 设置回调
    SetCallBack(callbackInit.alarmcallback, callbackInit.infocallback, callbackInit.algdetcallback,callbackInit.userdata);

    AlarmStatus alarmStatus;
    InfoStatus infoStatus;
    
    for (auto & funIni : funInis) {
        // funCamid记录 {'0':{camid1, camid2}, '1':{camid3, camid4}}
        int funid = funIni.funid;

        // 记录每个camera的ROI
        if (funIni.iAeraType != 0 and mapCamRoi.find(make_pair(funIni.camid, funIni.iAeraType))== mapCamRoi.end()){
            mapCamRoi[make_pair(funIni.camid, funIni.iAeraType)] = funIni.areaRois;
            // 区域的最小外接矩形状
            int min_x = funIni.areaRois[0].x;
            int min_y = funIni.areaRois[1].y;
            int max_x = min_x;
            int max_y = min_y;
            for (int i = 1; i < funIni.areaRois.size(); ++i) {
                min_x = funIni.areaRois[i].x < min_x?funIni.areaRois[i].x : min_x;
                min_y = funIni.areaRois[i].y < min_y?funIni.areaRois[i].y : min_y;
                max_x = funIni.areaRois[i].x > max_x?funIni.areaRois[i].x : max_x;
                max_y = funIni.areaRois[i].y > max_y?funIni.areaRois[i].y : max_y;
            }
            // 多边形区域的最小外接矩形
            mapCamRoiMinRect[make_pair(funIni.camid, funIni.iAeraType)] =
                    cv::Rect_<int>(min_x, min_y, max_x-min_x, max_y-min_y);
            if((max_x-min_x)*(max_y-min_y)>maxArea)
                strcpy(mainCam, funIni.camid);
        }

        if (funid == 0 and mapPersonAlarm.find(funIni.iCamType)==mapPersonAlarm.end()){
            strcpy(alarmStatus.camid, funIni.camid);
            alarmStatus.funid = funIni.funid;
            alarmStatus.reportStatus = 0;
            alarmStatus.alarm.iType = funid;
            memcpy(alarmStatus.alarm.szCamID, funIni.camid, sizeof(char )*64);
            memcpy(alarmStatus.alarm.szDevId, funIni.szDevId, sizeof(char )*32);
            alarmStatus.alarm.iDetArea = funIni.iCamType;
            alarmStatus.alarm.ilevel = LEVEL[funid];
            alarmStatus.alarm.iPicNum = 3;
            alarmStatus.oriMat.reserve(3);
            mapPersonAlarm[funIni.iCamType] = alarmStatus;
        }
        if (funid == 10){
            strcpy( infoStatus.camid, funIni.camid);
            infoStatus.funid = funIni.funid;
            infoStatus.reportStatus = 0;
            infoStatus.info.iType = funid;
            memcpy(infoStatus.info.szCamID, funIni.camid, sizeof(char )*64);
            memcpy(infoStatus.info.szDevId, funIni.szDevId, sizeof(char )*32);
            infoStatus.info.iDetArea = funIni.iCamType;
            mapCountInfo[funIni.camid] = infoStatus;
        }

        // 关联相机
        if (mapCamRel.find(funIni.iCamType) != mapCamRel.end())
            mapCamRel[funIni.iCamType] = {funIni.camid};
        else
            mapCamRel[funIni.iCamType].push_back(funIni.camid);
    }

}

// 身份识别相关
bool FunGroup3::Initial(vector<FunIni> &funInis, CallbackInit callbackInit) {
    // 设置回调
    SetCallBack(callbackInit.alarmcallback, callbackInit.infocallback, callbackInit.algdetcallback,callbackInit.userdata);

    AlarmStatus alarmStatus;
    InfoStatus infoStatus;
    for (auto & funIni : funInis) {
        // funCamid记录 {'0':{camid1, camid2}, '1':{camid3, camid4}}
        int funid = funIni.funid;

        // 身份识别、安全帽工作服检测场景需要跟踪id
        if(mapCamTrackid.find(funIni.camid) != mapCamTrackid.end() and (funid==4 or funid==5
                                                                        or funid==11))
            mapCamTrackid[funIni.camid].reserve(1000);

        // 记录每个camera的ROI
        if (funIni.iAeraType != 0)
            mapCamRoi[make_pair(funIni.camid, funIni.iAeraType)] = funIni.areaRois;

        if (funid == 2){
            // 船长缺岗
            strcpy(alarmStatus.camid, funIni.camid);
            alarmStatus.funid = funIni.funid;
            alarmStatus.reportStatus = 0;
            alarmStatus.alarm.iType = funid;
            memcpy(alarmStatus.alarm.szCamID, funIni.camid, sizeof(char )*64);
            memcpy(alarmStatus.alarm.szDevId, funIni.szDevId, sizeof(char )*32);
            alarmStatus.alarm.iDetArea = funIni.iCamType;
            alarmStatus.alarm.ilevel = LEVEL[funid];
            alarmStatus.alarm.iPicNum = 3;
            alarmStatus.oriMat.reserve(3);
            mapCaptainAlarm[funIni.camid] = alarmStatus;

            CaptainCams.push_back(funIni.camid);
        }
        if (funid == 11){
            // 人脸识别结果上报
            strcpy( infoStatus.camid, funIni.camid);
            infoStatus.funid = funIni.funid;
            infoStatus.reportStatus = 0;
            infoStatus.info.iType = funid;
            memcpy(infoStatus.info.szCamID, funIni.camid, sizeof(char )*64);
            memcpy(infoStatus.info.szDevId, funIni.szDevId, sizeof(char )*32);
            infoStatus.info.iDetArea = funIni.iCamType;
            mapIdInfo[funIni.camid] = infoStatus;
        }
    }
}

bool FunGroup4::Initial(vector<FunIni> &funInis, CallbackInit callbackInit) {
    // 设置回调
    SetCallBack(callbackInit.alarmcallback, callbackInit.infocallback, callbackInit.algdetcallback,callbackInit.userdata);

    AlarmStatus alarmStatus;
    for (auto & funIni : funInis) {
        // funCamid记录 {'0':{camid1, camid2}, '1':{camid3, camid4}}
        int funid = funIni.funid;

        // 记录每个camera的ROI
        if (funIni.iAeraType != 0)
            mapCamRoi[make_pair(funIni.camid, funIni.iAeraType)] = funIni.areaRois;

        if (funid == 1){
            strcpy(alarmStatus.camid, funIni.camid);
            alarmStatus.funid = funIni.funid;
            alarmStatus.reportStatus = 0;
            alarmStatus.alarm.iType = funid;
            memcpy(alarmStatus.alarm.szCamID, funIni.camid, sizeof(char )*64);
            memcpy(alarmStatus.alarm.szDevId, funIni.szDevId, sizeof(char )*32);
            alarmStatus.alarm.iDetArea = funIni.iCamType;
            alarmStatus.alarm.ilevel = LEVEL[funid];
            alarmStatus.alarm.iPicNum = 3;
            alarmStatus.oriMat.reserve(3);
            mapLookoutAlarm[funIni.camid] = alarmStatus;

            if (mapLookout.find(funIni.iCamType) != mapLookout.end())
                mapLookout[funIni.iCamType] = {funIni.camid};
            else
                mapLookout[funIni.iCamType].push_back(funIni.camid);
        }

        if (funid == 8){
            strcpy(alarmStatus.camid, funIni.camid);
            alarmStatus.funid = funIni.funid;
            alarmStatus.reportStatus = 0;
            alarmStatus.alarm.iType = funid;
            memcpy(alarmStatus.alarm.szCamID, funIni.camid, sizeof(char )*64);
            memcpy(alarmStatus.alarm.szDevId, funIni.szDevId, sizeof(char )*32);
            alarmStatus.alarm.iDetArea = funIni.iCamType;
            alarmStatus.alarm.ilevel = LEVEL[funid];
            alarmStatus.alarm.iPicNum = 1;
            alarmStatus.oriMat.reserve(1);
            mapFireAlarm[funIni.camid] = alarmStatus;

        }
    }
}

void FunGroup4::SendMrcnnResult(DeepLearnResult mrcnnRes) {

}

void FunGroup4::SendAlgInput(AlgInput algInput) {
    BaseFunProcess::SendAlgInput(algInput);
}

void FunGroup1::SendAlgInput(AlgInput algInput) {
    if (algInput.funid == 3 and roiround_on)
        roiround(algInput);
    else if (algInput.funid == 4 and helmatcloth_on)
        helmat(algInput);
    else if (algInput.funid == 5 and helmatcloth_on)
        workcloth(algInput);
    else if (algInput.funid == 6 or algInput.funid==7 and smokephone_on)
        smoke_phone(algInput);
    else if (algInput.funid == 9)
        roiInvade(algInput);
}

void FunGroup1::helmat(const AlgInput& algInput) {
    // 可能需要二次验证，先初始化一个结构体
    ImgUnit imgUnit;
    auto iter = mapFunAlarms[1].find(algInput.camid);
    if(iter != mapFunAlarms[1].end()){
        if (iter->second.reportStatus == 0){ // 当前该功能下相机报警状态是没有报警的状态，才取进行下一步验证
            iter->second.alarm.iRegionType = algInput.iRegionType;
            auto iter_track = mapCamTrackid.find(algInput.camid);
            if (iter_track->second.size() >= 1000) //清空id缓存区域
                iter_track->second.clear();
            for (auto & detre : algInput.detres) {
                if (detre.iType==3 and
                    std::find(iter_track->second.begin(), iter_track->second.end(), detre.iObjectId)==iter_track->second.end()){
                    // 有头这个类且这个类是一个新的id
                    imgUnit.rctTgt.push_back(detre.rctTgt);
                    iter_track->second.push_back(detre.iObjectId); // 需要记录跟踪id，避免重复对用一个人进行检测
                }
            }
            if (!imgUnit.rctTgt.empty()){
                // 修改功能此camere的状态
                iter->second.reportStatus = 2; // 有图待验证
                iter->second.oriMat.push_back(algInput.image);
                iter->second.alarm.iTimeStamp = algInput.iTimeStamp;
                memcpy(iter->second.alarm.szTime, algInput.szTime, sizeof(char)*32);

                // 传出回调
                memcpy(imgUnit.szTime, algInput.szTime, sizeof(char)*32);
                imgUnit.iTimeStamp = algInput.iTimeStamp;
                imgUnit.iDetType = 5;
                imgUnit.imgMat = algInput.image;
                memcpy(imgUnit.szCamID, algInput.camid, sizeof(char)*64);
                CONVAR.wait(MUTEX, []() { return CALLBACKABLE[2]; });
                CALLBACKABLE[2] = false;
                m_AlgDet(imgUnit, m_userData); // 回调出去，需要进行安全帽分类验证
                CONVAR.notify_one();
            }
        }

    }else
        m_LogFile.TraceWarning("Camid: %s没有初始化安全帽检测功能",algInput.camid);

}

void FunGroup1::workcloth(const AlgInput& algInput) {
    ImgUnit imgUnit;
    auto iter = mapFunAlarms[2].find(algInput.camid);
    if(iter != mapFunAlarms[2].end()){
        if (iter->second.reportStatus == 0){
            iter->second.alarm.iRegionType = algInput.iRegionType;
            auto iter_track = mapCamTrackid.find(algInput.camid);
            if (iter_track->second.size() >= 1000)
                iter_track->second.clear();
            for (auto & detre : algInput.detres) {
                if (detre.iType==2 and
                    std::find(iter_track->second.begin(), iter_track->second.end(), detre.iObjectId)==iter_track->second.end()){
                    // 有头这个类且这个类是一个新的id
                    imgUnit.rctTgt.push_back(detre.rctTgt);
                    iter_track->second.push_back(detre.iObjectId);  // 需要记录跟踪id，避免重复对用一个人进行检测
                }
            }
            if (!imgUnit.rctTgt.empty()){
                // 修改功能此camere的状态
                iter->second.reportStatus = 2; // 有图待验证
                iter->second.oriMat.push_back(algInput.image);
                iter->second.alarm.iTimeStamp = algInput.iTimeStamp;
                memcpy(iter->second.alarm.szTime, algInput.szTime, sizeof(char)*32);

                // 传出回调
                memcpy(imgUnit.szTime, algInput.szTime, sizeof(char)*32);
                imgUnit.iTimeStamp = algInput.iTimeStamp;
                imgUnit.iDetType = 6;
                imgUnit.imgMat = algInput.image;
                memcpy(imgUnit.szCamID, algInput.camid, sizeof(char)*64);
                CONVAR.wait(MUTEX, []() { return CALLBACKABLE[2]; });
                CALLBACKABLE[2] = false;
                m_AlgDet(imgUnit, m_userData); // 回调出去，需要进行安全帽分类验证
                CONVAR.notify_one();
            }
        }

    }else
        m_LogFile.TraceWarning("Camid: %s没有初始化安全帽检测功能", algInput.camid);
}

void FunGroup1::smoke_phone(const AlgInput& algInput) {
    ImgUnit imgUnit;
    auto iter = mapFunAlarms[3].find(algInput.camid);
    if (algInput.funid == 7)
        iter = mapFunAlarms[4].find(algInput.camid);

    if ((algInput.funid==6 and iter != mapFunAlarms[3].end())
    or (algInput.funid==7 and iter != mapFunAlarms[4].end())){
        if (iter->second.reportStatus == 0){
            iter->second.alarm.iRegionType = algInput.iRegionType;
            for (auto & detre : algInput.detres)
                if (detre.iType==2)
                    // 有人这个类
                    imgUnit.rctTgt.push_back(detre.rctTgt);
            if (!imgUnit.rctTgt.empty()){
                iter->second.reportStatus = 2; // 有图待验证
                iter->second.oriMat.push_back(algInput.image);
                iter->second.alarm.iTimeStamp = algInput.iTimeStamp;
                memcpy(iter->second.alarm.szTime, algInput.szTime, sizeof(char)*32);

                // 传出回调
                memcpy(imgUnit.szTime, algInput.szTime, sizeof(char)*32);
                imgUnit.iTimeStamp = algInput.iTimeStamp;
                if (algInput.funid == 6)
                    imgUnit.iDetType = 3;
                else
                    imgUnit.iDetType = 4;
                imgUnit.imgMat = algInput.image;
                memcpy(imgUnit.szCamID, algInput.camid, sizeof(char)*64);
                CONVAR.wait(MUTEX, []() { return CALLBACKABLE[2]; });
                CALLBACKABLE[2] = false;
                m_AlgDet(imgUnit, m_userData); // 回调出去，需要进行安全帽分类验证
                CONVAR.notify_one();
            }
        }
    }else{
        if (algInput.funid == 6)
            m_LogFile.TraceWarning("Camid: %s没有初始化打电话检测功能", algInput.camid);
        else
            m_LogFile.TraceWarning("Camid: %s没有初始化抽烟检测功能", algInput.camid);
    }
}

void FunGroup1::roiround(AlgInput algInput) {
    auto iter = mapFunAlarms[0].find(algInput.camid);
    bool meet = false;
    if (iter != mapFunAlarms[0].end()){
        iter->second.alarm.iRegionType = algInput.iRegionType;
        for (int i = 0; i < algInput.detres.size(); ++i) {
            if (algInput.detres[i].iType == 2){
                // 有人这个类，判断是否在巡逻区域
                auto areaROI = mapCamRoi.find(make_pair(algInput.camid, 2))->second;
                auto rect = algInput.detres[i].rctTgt;
                cv::Point_<int> middle_point(rect.x+rect.width/2, rect.y+rect.height/2);
                if(IsInRoi(areaROI, middle_point)){
                    meet = true;
                    break;
                }
            }
        }
        if (meet){
            iter->second.reportStatus = 0;
            RoiWaitTime = 0;
            iter->second.oriMat.clear();
        }else{
            if (RoiWaitTime == 0){
                iter->second.oriMat.push_back(algInput.image); // 周期开始图片
                szTimeRoiRound.push_back(algInput.szTime);
            }
            RoiWaitTime += FRENQUENCY[3];
        }
        if (int(roiround_thresh*60/RoiWaitTime)==2 and iter->second.oriMat.size()==1){
            // 周期中间那张图片
            iter->second.oriMat.push_back(algInput.image);
            szTimeRoiRound.push_back(algInput.szTime);
        }else if (RoiWaitTime>=roiround_thresh*60){
            // 周期结束的一张图片
            iter->second.oriMat.push_back(algInput.image);
            iter->second.alarm.iTimeStamp = algInput.iTimeStamp;
            szTimeRoiRound.push_back(algInput.szTime);
            assert(iter->second.oriMat.size()==3);
            assert(szTimeRoiRound.size() == 3);
            for (int i = 0; i < 3; ++i) {
                string camidS = iter->second.camid;
                string image_save_path = EVENTIMAGEPATH+ camidS + "_" + szTimeRoiRound[i]+ "_NoRoiRound.jpg";
                image_save_path.copy(iter->second.alarm.szImgPath[i], 128, 0);
                cv::imwrite(image_save_path, iter->second.oriMat[i]);
            }
            CONVAR.wait(MUTEX, []() { return CALLBACKABLE[0]; });
            CALLBACKABLE[0] = false;
            m_AlarmRes(iter->second.alarm, m_userData);
            CONVAR.notify_one();
            iter->second.reportStatus = 0;
            iter->second.oriMat.clear();
            szTimeRoiRound.clear();
            RoiWaitTime = 0;
        }
    }else
        m_LogFile.TraceWarning("Camid: %s没有初始化区域巡逻功能", algInput.camid);
}

void FunGroup1::SendYoloV5Result(YoloV5Result yolov5Res, int iDpType) {
    bool isAlarm = false;
    int index = 3;
    string txt = "Phone";
    if (iDpType == 4){
        index = 4;
        txt = "Smoke";
    }
    auto iter = mapFunAlarms[index].find(yolov5Res.szCamID);
    if (iter != mapFunAlarms[index].end()){
        if (iter->second.reportStatus==2 and iter->second.alarm.iTimeStamp==yolov5Res.iTimeStamp){
            // 是这一帧上待验证的结果
            // todo: 当等待二次验证时间过长，应该丢弃这个一帧，将iter->second.reportStatus==0
            for (const auto& res:yolov5Res.yolov5Res) {
                if (!res.tgtRes.empty()){
                    for (auto& tgtres: res.tgtRes) {
                        if (tgtres.iType == 1){
                            isAlarm = true;
                            iter->second.alarm.rctTgt.push_back(res.rctTgt);
                        }
                    }
                }
            }
            if (isAlarm){
                display(iter->second.oriMat[0], iter->second.alarm.rctTgt, txt);
                string camidS = iter->second.camid;
                string image_save_path = EVENTIMAGEPATH+camidS + "_" + iter->second.alarm.szTime + "_"+txt+".jpg";
                cv::imwrite(image_save_path, iter->second.oriMat[0]);
                image_save_path.copy(iter->second.alarm.szImgPath[0], 128, 0);
                CONVAR.wait(MUTEX, []() { return CALLBACKABLE[0]; });
                CALLBACKABLE[0] = false;
                m_AlarmRes(iter->second.alarm, m_userData);
                CONVAR.notify_one();
                iter->second.reportStatus = 0;
                iter->second.oriMat.clear();
            }
        }
    }
 }

void FunGroup1::SendClassifierResult(ClassResult& classRes) {
    bool isAlarm = false;
    int index = 1;
    string txt= "Nohelmat";
    if(classRes.iType == 6){
        index = 2;
        txt = "NoWorkCloth";
    }

    auto iter = mapFunAlarms[index].find(classRes.szCamID);
    if (iter != mapFunAlarms[index].end()){
        if (iter->second.reportStatus==2 and iter->second.alarm.iTimeStamp==classRes.iTimeStamp){
            // 是这一帧上待验证的结果
            // todo: 当等待二次验证时间过长，应该丢弃这个一帧，将iter->second.reportStatus==0
            for (const auto& classinfo:classRes.classInfo) {
                if (!classinfo.bRet) {
                    isAlarm = true;
                    iter->second.alarm.rctTgt.push_back(classinfo.vecTgt);
                }
            }
            if (isAlarm){
                // 有安全帽的报警
                // 将框show在原图上并保存，上传报警
                display(iter->second.oriMat[0], iter->second.alarm.rctTgt, txt);
                string camidS = iter->second.camid;
                string image_save_path = EVENTIMAGEPATH+camidS + "_" + iter->second.alarm.szTime + "_"+txt+".jpg";
                cv::imwrite(image_save_path, iter->second.oriMat[0]);
                image_save_path.copy(iter->second.alarm.szImgPath[0], 128, 0);
                CONVAR.wait(MUTEX, []() { return CALLBACKABLE[0]; });
                CALLBACKABLE[0] = false;
                m_AlarmRes(iter->second.alarm, m_userData); // 回调出去，需要进行安全帽分类验证
                CONVAR.notify_one();
                iter->second.reportStatus = 0;
                iter->second.oriMat.clear();
                iter->second.alarm.rctTgt.clear();
                isAlarm = false;
            }
        }
    }

    /*
    if (classRes.iType == 6){
        // 在工作服分类结果
        auto iter = mapFunAlarms[2].find(classRes.szCamID);
        if (iter != mapFunAlarms[2].end()){
            if (iter->second.reportStatus==2 and iter->second.alarm.iTimeStamp==classRes.iTimeStamp){
                // 是这一帧上待验证的结果
                // todo: 当等待二次验证时间过长，应该丢弃这个一帧，将iter->second.reportStatus==0
                for (const auto& classinfo:classRes.classInfo) {
                    if (!classinfo.bRet) {
                        isAlarm = true;
                        iter->second.alarm.rctTgt.push_back(classinfo.vecTgt);
                    }
                }
                if (isAlarm){
                    // 有安全帽的报警
                    // 将框show在原图上并保存，上传报警
                    display(iter->second.oriMat[0], iter->second.alarm.rctTgt, (string &) "No workcloth");
                    string camidS = iter->second.camid;
                    string image_save_path = EVENTIMAGEPATH+camidS + "_" + iter->second.alarm.szTime + "_NoWorkCloth.jpg";
                    cv::imwrite(image_save_path, iter->second.oriMat[0]);
                    image_save_path.copy(iter->second.alarm.szImgPath[0], 128, 0);
                    CONVAR.wait(MUTEX, []() { return CALLBACKABLE[0]; });
                    CALLBACKABLE[0] = false;
                    m_AlarmRes(iter->second.alarm, nullptr); // 回调出去，需要进行安全帽分类验证
                    CONVAR.notify_one();
                    iter->second.reportStatus = 0;
                    iter->second.oriMat.clear();
                }
            }
        }
    }*/
}

void FunGroup1::roiInvade(AlgInput algInput) {
    // todo:实现区域入侵报警
    auto iter = mapFunAlarms[5].find(algInput.camid);
    auto iter_roi = mapCamRoi.find(make_pair(algInput.camid, 2));
    if (iter != mapFunAlarms[5].end()){
        for(auto &dpres: algInput.detres){
            if (dpres.iType == 2){
                // todo:应当根据摄像头角度确定根据哪个点，确定是否在区域内
                // 右下角的点
                cv::Point_<int> right_down(dpres.rctTgt.x+dpres.rctTgt.width, dpres.rctTgt.y+dpres.rctTgt.height);
                if (IsInRoi(iter_roi->second, right_down)){
                    // 有人入侵区域，产生报警
                    iter->second.alarm.rctTgt.push_back(dpres.rctTgt);
                }
            }
        }
        if (!iter->second.alarm.rctTgt.empty()){
            iter->second.alarm.iTimeStamp = algInput.iTimeStamp;
            iter->second.alarm.iRegionType = algInput.iRegionType;
            strcpy(iter->second.alarm.szTime, algInput.szTime);
            string camidS = iter->second.camid;
            string image_save_path = EVENTIMAGEPATH+ camidS + "_" + algInput.szTime+ "_RoiInvade.jpg";
            image_save_path.copy(iter->second.alarm.szImgPath[0], 128, 0);
            display(algInput.image, iter->second.alarm.rctTgt, (string &) "RoiInvade");
            cv::imwrite(image_save_path, algInput.image);
            CONVAR.wait(MUTEX, []() { return CALLBACKABLE[0]; });
            CALLBACKABLE[0] = false;
            m_AlarmRes(iter->second.alarm, m_userData);
            CONVAR.notify_one();
            iter->second.reportStatus = 0;
            iter->second.alarm.rctTgt.clear();
        }
    }else
        m_LogFile.TraceWarning("Camid: %s没有初始化区域入侵功能", algInput.camid);

}

void FunGroup2::SendAlgInput(AlgInput algInput) {
    auto iter = mapCountInfo.find(algInput.camid);
    if (iter != mapCountInfo.end()){
        int person_count = 0;
        int head_count = 0;
        // 拷贝当前信息
        strcpy(iter->second.info.szTime, algInput.szTime);
        iter->second.info.iRegionType = algInput.iRegionType;
        iter->second.info.iTimeStamp = algInput.iTimeStamp;

        // 找到这个相机下的人数统计区域
//        auto iter_roirect = mapCamRoiMinRect.find(make_pair(algInput.camid, 1));
        auto iter_roi = mapCamRoi.find(make_pair(algInput.camid, 1));

        // 遍历主检测起结果统计人数
        if (!algInput.detres.empty()){
            for (auto &detre: algInput.detres) {
                cv::Point_<int> middle(detre.rctTgt.x+detre.rctTgt.width/2, detre.rctTgt.y+detre.rctTgt.height/2);
                if (detre.iType==2){
                    if (iter_roi != mapCamRoi.end()){
                        if (IsInRoi(iter_roi->second, middle)){
                            person_count += 1;
                            iter->second.info.rctTgt.push_back(detre.rctTgt);
                        }
                    }else{
                        person_count += 1;
                        iter->second.info.rctTgt.push_back(detre.rctTgt);
                    }
                }
                /*统计头的数量
                if (detre.iType==3){
                    if (iter_roi != mapCamRoi.end()){
                        if (IsInRoi(iter_roi->second, middle)){
                            head_count += 1;
                        }
                    }else head_count += 1;
                }
            }*/
            }
        }

        // 记录当前人数
        iter->second.info.personNum = person_count;
        iter->second.reportStatus = 2;
        iter->second.oriMat = algInput.image.clone();

        // 找到这个区域里其他相机下的状态和参数
        auto iter_cams = mapCamRel.find(iter->second.info.iDetArea);
        vector<bool> all_equal; // 记录同区域相机下人数是否相同
        all_equal.push_back(true); // 自己与自己是true
        vector<bool> all_below; // 记录每个相机是否小于人数报警阈值
        if (iter->second.info.personNum < num_thresh)
            all_below.push_back(true);
        for (auto cam: iter_cams->second) {
            if (cam != algInput.camid){
                auto iter_caminfo = mapCountInfo.find(cam);
                if (iter_caminfo->second.reportStatus == 2){ // 当前相机也是等待上报状态
                    if(std::abs(iter->second.info.iTimeStamp - iter_caminfo->second.info.iTimeStamp)<2000){
                        // 是两秒内的不同相机下的图片
                        if (iter_caminfo->second.info.personNum == iter->second.info.personNum){
                            // 两个相机下人数相同
                            all_equal.push_back(true);
                        }else
                            all_equal.push_back(false);
                        if (iter_caminfo->second.info.personNum<num_thresh)
                            // 循环判断每个相机下的人数是不是都小于报警阈值
                            all_below.push_back(true);
                        else
                            all_below.push_back(false);
                    }else{
                        //两个相机下的图片帧已经相差两秒以上，则不能做为一起判断的标准, 退出循环
                        iter_caminfo->second.reportStatus = 0;
                        iter_caminfo->second.statusTime = 0;
                        iter_caminfo->second.info.rctTgt.clear();
                        break;
                    }
                }else if (iter_caminfo->second.reportStatus == 0){
                    // 当前相机还在等待下一帧的输入,退出当前循环，等待所有相关相机都获得最新的人数
                    break;
                }
            }
        }

        // 人数不足报警，只有驾驶室驾驶台有
        if (iter_roi != mapCamRoi.end()){
            // 更新船只航行区域
            mapPersonAlarm[1].alarm.iRegionType = algInput.iRegionType;
            // 每个相机下的人数都统计到了，且都大于人数报警阈值，则清空周期内报警
            if (all_below.size() == iter_cams->second.size()){
                if (std::find(all_below.begin(), all_below.end(), true) == all_below.end()){
                    // 所有相机下的人数都高于人数报警阈值，报警记录清零
                    PersonLackTime = 0;
                    szTimePersonLack.clear();
                    mapPersonAlarm[1].reportStatus = 0;
                    mapPersonAlarm[1].oriMat.clear();
                }
                if(std::find(all_below.begin(), all_below.end(), false)==all_below.end()){
                    // 所有相机下的人数都低于人数报警阈值，不需要二次验证，将记录为周期内的一次报警
                    if (mapPersonAlarm[1].reportStatus == 0){
                        // 周期内首次
                        szTimePersonLack.push_back(algInput.szTime);
                        mapPersonAlarm[1].oriMat.push_back(algInput.image);
                        mapPersonAlarm[1].reportStatus = 2; // 等待周期内其他状态
                    }else{
                        PersonLackTime +=FRENQUENCY[0];
                    }
                    if (int(alarm_time*60/PersonLackTime)==2 and mapPersonAlarm[1].oriMat.size() == 1){
                        // 周期中
                        mapPersonAlarm[1].oriMat.push_back(algInput.image);
                        szTimePersonLack.push_back(algInput.szTime);
                    }
                    if (PersonLackTime>=int(alarm_time*60)){
                        // 周期结束
                        mapPersonAlarm[1].oriMat.push_back(algInput.image);
                        szTimePersonLack.push_back(algInput.szTime);
                        for (int i = 0; i < 3; ++i) {
                            string camidS = iter->second.camid;
                            string image_save_path = EVENTIMAGEPATH+ camidS + "_" + szTimePersonLack[i]+ "_LackPerson.jpg";
                            image_save_path.copy(mapPersonAlarm[1].alarm.szImgPath[i], 128, 0);
                            cv::imwrite(image_save_path, mapPersonAlarm[1].oriMat[i]);
                            strcpy(mapPersonAlarm[1].alarm.szCamID, algInput.camid);
                            mapPersonAlarm[1].alarm.iTimeStamp = algInput.iTimeStamp;
                            strcpy(mapPersonAlarm[1].alarm.szTime, algInput.szTime); // todo:alarm.rctTgt没有，alarm.szDevid错乱
                            // 推送报警消息
                            CONVAR.wait(MUTEX, []() { return CALLBACKABLE[0]; });
                            CALLBACKABLE[0] = false;
                            m_AlarmRes(mapPersonAlarm[1].alarm, m_userData);
                            CONVAR.notify_one();
                            // 清空报警消息，等待下一次周期开始
                            mapPersonAlarm[1].reportStatus = 0;
                            mapPersonAlarm[1].oriMat.clear();
                            szTimePersonLack.clear();
                            PersonLackTime = 0;
                        }
                    }
                }
            }
        }

        // 人数统计，直接上报和需要maskrcnn确定的情况
        if (all_equal.size() == iter_cams->second.size()){
            // 统计到所有相机的人数
            if (std::find(all_equal.begin(), all_equal.end(), false) == all_equal.end()){
                // 每个相机下的人数全部都是相同的，不需要二次验证，即是最终人数，上报消息
                // 保存图像
//                display(algInput.image, iter->second.info.rctTgt, (string &) "person_count");
                string camidS = iter->second.camid;
                string image_save_path = EVENTIMAGEPATH+camidS + "_" + iter->second.info.szTime + "_"+"person_count.jpg";
                image_save_path.copy(iter->second.info.szImgPath, 128, 0);
                CONVAR.wait(MUTEX, []() { return CALLBACKABLE[1]; });
                CALLBACKABLE[1] = false;
                m_InfoRes(iter->second.info, m_userData);
                CONVAR.notify_one();
                for (auto cam:iter_cams->second) {
                    // 该区域下所有相机中存储内容都清零
                    auto iter_caminfo = mapCountInfo.find(cam);
                    iter_caminfo->second.reportStatus = 0;
                    iter_caminfo->second.info.rctTgt.clear();
                    iter_caminfo->second.statusTime = 0;
                }
            }else{
                // 所有区域内相机的人数都初步统计到，但是人数不一致
                // todo:2022-3-14选择ROI区域最大的那个相机做为参考，进入二次验证
                ImgUnit imgUnit;
                if (iter->second.info.iDetArea == 1){
                    // 是驾驶室内的人数统计，则找到最大的驾驶台区域
                    auto iter_roirect = mapCamRoiMinRect.find(make_pair(mainCam, 1));
                    iter = mapCountInfo.find(mainCam);
                    // 截取最大驾驶台区域，进行二次验证
                    imgUnit.imgMat = iter->second.oriMat(iter_roirect->second);
                }else
                    // 集控室，直接获取原图
                    imgUnit.imgMat = iter->second.oriMat;

                strcpy(imgUnit.szCamID, iter->second.camid);
                strcpy(imgUnit.szTime, iter->second.info.szTime);
                imgUnit.iTimeStamp = iter->second.info.iTimeStamp;
                imgUnit.iDetType = 2; // maskrcnn人校正
                // 推送消息
                CONVAR.wait(MUTEX, []() { return CALLBACKABLE[2]; });
                CALLBACKABLE[2] = false;
                m_AlgDet(imgUnit, m_userData);
                CALLBACKABLE[2] = true;
                CONVAR.notify_one();
            }
        }
    }
}

void FunGroup2::SendMrcnnResult(DeepLearnResult mrcnnRes) {
    // 获取mrcnn对人数统计的结果
    auto iter = mapCountInfo.find(mrcnnRes.szCamID);
    if (iter != mapCountInfo.end() and mrcnnRes.iTimeStamp == iter->second.info.iTimeStamp){
        int person_num=0;
        for(const auto& dpRes:mrcnnRes.vDpResult){

            if (dpRes.iType == 0)
                person_num += 1;
        }
        iter->second.info.personNum = person_num;
        // 实时消息上报
        string camidS = iter->second.camid;
        string image_save_path = EVENTIMAGEPATH+camidS + "_" + iter->second.info.szTime + "_"+"person_count.jpg";
        image_save_path.copy(iter->second.info.szImgPath, 128, 0);
        CONVAR.wait(MUTEX, []() { return CALLBACKABLE[1]; });
        CALLBACKABLE[1] = false;
        m_InfoRes(iter->second.info, m_userData);
        CONVAR.notify_one();

        auto iter_cams = mapCamRel.find(iter->second.info.iDetArea);
        for (auto cam:iter_cams->second) {
            // 该区域下所有相机中存储内容都清零
            auto iter_caminfo = mapCountInfo.find(cam);
            iter_caminfo->second.reportStatus = 0;
            iter_caminfo->second.info.rctTgt.clear();
            iter_caminfo->second.statusTime = 0;
        }
        if (iter->second.info.iDetArea == 1){
            if (person_num>num_thresh){
                PersonLackTime = 0;
                szTimePersonLack.clear();
                mapPersonAlarm[1].reportStatus = 0;
                mapPersonAlarm[1].oriMat.clear();
            }else{
                // 所有相机下的人数都低于人数报警阈值，不需要二次验证，将记录为周期内的一次报警
                if (mapPersonAlarm[1].reportStatus == 0){
                    // 周期内首次
                    szTimePersonLack.push_back(iter->second.info.szTime);
                    mapPersonAlarm[1].oriMat.push_back(iter->second.oriMat);
                    mapPersonAlarm[1].reportStatus = 2; // 等待周期内其他状态
                }else{
                    PersonLackTime +=FRENQUENCY[0];
                }
                if (int(alarm_time*60/PersonLackTime)==2 and mapPersonAlarm[1].oriMat.size() == 1){
                    // 周期中
                    mapPersonAlarm[1].oriMat.push_back(iter->second.oriMat);
                    szTimePersonLack.push_back(iter->second.info.szTime);
                }
                if (PersonLackTime>=alarm_time*60){
                    // 周期结束
                    mapPersonAlarm[1].oriMat.push_back(iter->second.oriMat);
                    szTimePersonLack.push_back(iter->second.info.szTime);
                    for (int i = 0; i < 3; ++i) {
                        string camidS = iter->second.camid;
                        string image_save_path = EVENTIMAGEPATH+ camidS + "_" + szTimePersonLack[i]+ "_LackPerson.jpg";
                        image_save_path.copy(mapPersonAlarm[1].alarm.szImgPath[i], 128, 0);
                        cv::imwrite(image_save_path, mapPersonAlarm[1].oriMat[i]);
                        strcpy(mapPersonAlarm[1].alarm.szCamID, iter->second.camid);
                        mapPersonAlarm[1].alarm.iTimeStamp = iter->second.info.iTimeStamp;
                        strcpy(mapPersonAlarm[1].alarm.szTime, iter->second.info.szTime); // todo:alarm.rctTgt没有，alarm.szDevid错乱
                        // 推送报警消息
                        CONVAR.wait(MUTEX, []() { return CALLBACKABLE[0]; });
                        CALLBACKABLE[0] = false;
                        m_AlarmRes(mapPersonAlarm[1].alarm, m_userData);
                        CONVAR.notify_one();
                        // 清空报警消息，等待下一次周期开始
                        mapPersonAlarm[1].reportStatus = 0;
                        mapPersonAlarm[1].oriMat.clear();
                        szTimePersonLack.clear();
                        PersonLackTime = 0;
                    }
                }
            }
        }
    }else
        m_LogFile.TraceWarning("Cam%s 没有要求在 %s时刻进行maskrcnn人数进行二次验证");
}

void FunGroup3::SendAlgInput(AlgInput algInput) {
    auto iter = mapIdInfo.find(algInput.camid);
    iter->second.info.iRegionType = algInput.iRegionType;

    // 跟新船长缺席报警中，船只所在区域，因为只有在这里才传进来
    auto iter_captain = mapCaptainAlarm.find(algInput.camid);
    if (iter_captain != mapCaptainAlarm.end())
        iter_captain->second.alarm.iRegionType = algInput.iRegionType;

    auto iter_trackids = mapCamTrackid.find(algInput.camid);
    if(iter_trackids->second.size() >= 1000)
        iter_trackids->second.clear(); // 记录200次跟踪结果
    ImgUnit imgUnit;
    if (iter != mapIdInfo.end()){
        for(auto &dpres: algInput.detres)
            if (dpres.iType == 2 and
            std::find(iter_trackids->second.begin(), iter_trackids->second.end(), dpres.iObjectId)==iter_trackids->second.end()){
                // 主检测器检测到"人"类，且是新出现的跟踪id
                iter_trackids->second.push_back(dpres.iObjectId);
                imgUnit.rctTgt.push_back(dpres.rctTgt);
            }
        if (!imgUnit.rctTgt.empty()){
            iter->second.info.iTimeStamp = algInput.iTimeStamp;
            strcpy(iter->second.info.szTime, algInput.szTime);
            iter->second.reportStatus = 2;

            strcpy(imgUnit.szCamID, algInput.camid);
            strcpy(imgUnit.szTime, algInput.szTime);
            imgUnit.iTimeStamp = algInput.iTimeStamp;
            imgUnit.imgMat = algInput.image;
            imgUnit.iDetType = 7;
            // 推送消息，等待人脸识别结果
            CONVAR.wait(MUTEX, []() { return CALLBACKABLE[2]; });
            CALLBACKABLE[2] = false;
            m_AlgDet(imgUnit, m_userData);
            CALLBACKABLE[2] = true;
            CONVAR.notify_one();
        }
    }
}

void FunGroup3::SendFaceRecoRes(FaceRegResult faceRegResult) {
    auto iter = mapIdInfo.find(faceRegResult.szCamID);
    auto iter_captian = mapCaptainAlarm.find(faceRegResult.szCamID);
    bool meet_captain = false;
    if (iter->second.info.iTimeStamp == faceRegResult.iTimeStamp){
        // 是同一个时间戳下的二次验证结果
        if (!faceRegResult.regInfo.empty()){
            for(auto &reginfo:faceRegResult.regInfo){
                // 判断是否有船长
                if ( iter_captian != mapCaptainAlarm.end()
                and strcmp(reginfo.szPID, "船长") == 0){
                    auto iter_roi = mapCamRoi.find(make_pair(iter->second.camid, 1));
                    cv::Point_<int> middle(reginfo.rctTgt.x+reginfo.rctTgt.width/2, reginfo.rctTgt.y+reginfo.rctTgt.height/2);
                    if (IsInRoi(iter_roi->second, middle))
                        meet_captain = true;
                }
                iter->second.info.rctTgt.clear();
                iter->second.info.rctTgt.push_back(reginfo.rctTgt);
                strcpy(iter->second.info.szPID, reginfo.szPID);
                strcpy(iter->second.info.szPName, reginfo.szPName);
                cv::Mat image = iter->second.oriMat.clone();
                display(image, iter->second.info.rctTgt , reinterpret_cast<string &>(reginfo.szPName));
                string camidS = iter->second.camid;
                string image_save_path = EVENTIMAGEPATH+ camidS + "_" + iter->second.info.szTime+ "_NoRoiRound.jpg";
                image_save_path.copy(iter->second.info.szImgPath, 128, 0);
                cv::imwrite(image_save_path, image);
                // 推送人脸识别实时消息
                CONVAR.wait(MUTEX, []() { return CALLBACKABLE[1]; });
                CALLBACKABLE[1] = false;
                m_InfoRes(iter->second.info, m_userData);
                CALLBACKABLE[1] = true;
                CONVAR.notify_one();
                iter->second.reportStatus = 0;
            }
        }
    }
    // 船长缺席报警
    if (meet_captain){
        // 所有相机下记录的船长缺席状态都清零
        for(auto & cplack: mapCamCaptainLackTime)
            cplack.second = 0;
        for(auto &ctcplack: mapCamTimeCaptainLack)
            ctcplack.second.clear();
        for(auto &ca:mapCaptainAlarm){
            ca.second.reportStatus = 0;
            ca.second.oriMat.clear();
        }
    }else{
        auto camid = faceRegResult.szCamID;
        auto iter_camtime = mapCamTimeCaptainLack.find(camid);
        auto iter_camlack = mapCamCaptainLackTime.find(camid);
        if (iter_captian->second.reportStatus == 0){
            // 这个相机下第一次记录到船长缺席
            assert(iter_camtime->second.empty());
            iter_camtime->second.push_back(faceRegResult.szTime);
            iter_camlack->second = 0;
            iter_captian->second.reportStatus = 2;
            iter_captian->second.oriMat.push_back(iter->second.oriMat);
        }else{
            iter_camlack->second += FRENQUENCY[11];
            if (int(captian_alarm_time*60/iter_camlack->second)==2 and iter_captian->second.oriMat.size()==1){
                iter_captian->second.oriMat.push_back(iter->second.oriMat);
                iter_camtime->second.push_back(faceRegResult.szTime);
            }
            if (iter_camlack->second > captian_alarm_time*60 and iter_captian->second.oriMat.size() == 2){
                // 周期结束一张图
                iter_captian->second.oriMat.push_back(iter->second.oriMat);
                iter_camtime->second.push_back(faceRegResult.szTime);
                iter_captian->second.reportStatus = 1; // 状态置于等待报警状态
                strcpy(iter_captian->second.alarm.szTime, faceRegResult.szTime);
                iter_captian->second.alarm.iTimeStamp = faceRegResult.iTimeStamp;

                // todo: 联合多个相机判断船长是否缺席
            }
        }
    }
}



