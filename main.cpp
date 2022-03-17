#include <iostream>
#include "FunctionProcess.h"
#include <thread>

void GetFunResultsCallBack(int iReDet, int iCamID, int iType, char *szResult, cv::Mat &imgMat, void *userData){
    cout<<"iRedet:"<<iReDet<<endl;
    cout<<"iCamID:"<<iCamID<<endl;
    cout<<"iType:"<<iType<<endl;
    CALLBACKABLE = true;
}

void thread1(){
    vector<FunIni *> funInis;
    BaseFunProcess baseFunProcess;
    baseFunProcess.Initial(funInis, &GetFunResultsCallBack, nullptr);
    AlgInput algInput;
    for (int i = 0; i < 5; ++i) {
        baseFunProcess.SendAlgInput(&algInput);
    }
}

void thread2(){
    HelmatWorkCloth helmatWorkCloth{&GetFunResultsCallBack, nullptr};
    AlgInput algInput;
    for (int i = 0; i < 5; ++i) {
        helmatWorkCloth.SendAlgInput(&algInput);
    }
}
int main() {
    thread th1(thread1);
    thread th2(thread2);
    th1.join();
    th2.join();
    return 0;
}
