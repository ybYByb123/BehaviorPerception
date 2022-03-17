//
// Created by xlj on 2022/2/23.
//

#include "utils.h"

bool IsInRoi(const vector<cv::Point_<int>> &roi, const cv::Point_<int> &point) {
    if (cv::pointPolygonTest(roi, point, false) > 0)
        return true;
    else
        return false;
}


//输入为2020-11-24 17:07:32格式时间，输出为时间戳
time_t Time2StampTime(const std::string &strTime, int iType) {
    struct tm tm{};
    memset(&tm, 0, sizeof(tm));
    if (iType == 0 || iType == 3 || iType == 5) {
        tm.tm_year = atoi(strTime.substr(0, 4).c_str()) - 1900;
        tm.tm_mon = atoi(strTime.substr(5, 2).c_str()) - 1;
        tm.tm_mday = atoi(strTime.substr(8, 2).c_str());
        tm.tm_hour = atoi(strTime.substr(11, 2).c_str());
        tm.tm_min = atoi(strTime.substr(14, 2).c_str());
        tm.tm_sec = atoi(strTime.substr(17, 2).c_str());
    } else if (iType == 1 || iType == 2) {
        tm.tm_year = atoi(strTime.substr(0, 4).c_str()) - 1900;
        tm.tm_mon = atoi(strTime.substr(4, 2).c_str()) - 1;
        tm.tm_mday = atoi(strTime.substr(6, 2).c_str());
        tm.tm_hour = atoi(strTime.substr(8, 2).c_str());
        tm.tm_min = atoi(strTime.substr(10, 2).c_str());
        tm.tm_sec = atoi(strTime.substr(12, 2).c_str());
    }

    return mktime(&tm);
}

bool display(cv::Mat &image, vector<cv::Rect_<int>> rects, string &txt){
    for (const auto& rect: rects) {
        cv::rectangle(image, rect, cv::Scalar_<int>(0, 0, 255), 2);
    }
}