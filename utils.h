//
// Created by xlj on 2022/2/23.
//

#ifndef CABINBEHAVIORPERCEPTION_UTILS_H
#define CABINBEHAVIORPERCEPTION_UTILS_H
#include "vector"
#include "opencv2/opencv.hpp"
using namespace std;

namespace algprocess{
    // 判断点是否在区域内
    bool IsInRoi(const vector<cv::Point_<int>>& roi, const cv::Point_<int>& point);

//输入为2020-11-24 17:07:32格式时间，输出为时间戳
    time_t Time2StampTime(const std::string& strTime, int iType);

    bool display(cv::Mat& image, vector<cv::Rect_<int>> rects, string &txt);

}
#endif //CABINBEHAVIORPERCEPTION_UTILS_H
