#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/objdetect/objdetect.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "cv.h"
#include "iostream"

#include <stdio.h>  
#include <string.h>  
#include <ctype.h>  
#include <opencv2/ml/ml.hpp>
using namespace cv;
using namespace std;
//�̳���CvSVM���࣬��Ϊ����setSVMDetector()���õ��ļ���Ӳ���ʱ����Ҫ�õ�ѵ���õ�SVM��decision_func������  
//��ͨ���鿴CvSVMԴ���֪decision_func������protected���ͱ������޷�ֱ�ӷ��ʵ���ֻ�ܼ̳�֮��ͨ����������  

int dealMat(Mat img)
{
	
	int flag = 0;
	//namedWindow("people detector");
	HOGDescriptor hog(Size(64, 128), Size(16, 16), Size(8, 8), Size(8, 8), 9);//HOG���������������HOG�����ӵ�
	//hog.setSVMDetector(HOGDescriptor::getDaimlerPeopleDetector())
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
	fflush(stdout);
	vector<Rect> found, found_filtered;
	//double t = (double)getTickCount();
	// run the detector with default parameters. to get a higher hit-rate  
	// (and more false alarms, respectively), decrease the hitThreshold and  
	// groupThreshold (set groupThreshold to 0 to turn off the grouping completely).  
	//hog.svmDetector
	//hog.detectMultiScale(img, found, 0, Size(2, 1), Size(8, 8), 1.05, 2);
	hog.detectMultiScale(img, found, 0, Size(6, 3), Size(4, 4), 1.05, 2);
	//t = (double)getTickCount() - t;
	//printf("tdetection time = %gms\n", t*1000. / cv::getTickFrequency());
	size_t i, j;
	if (found.size() > 0)
		flag = 1;
	for (i = 0; i < found.size(); i++) {
		Rect r = found[i];
		for (j = 0; j < found.size(); j++)
		if (j != i && (r & found[j]) == r)
			break;
		if (j == found.size())
			found_filtered.push_back(r);

	}
	for (i = 0; i < found_filtered.size(); i++) {
		Rect r = found_filtered[i];
		// the HOG detector returns slightly larger rectangles than the real objects.  
		// so we slightly shrink the rectangles to get a nicer output.  
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		rectangle(img, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
	}
	//imshow("people detector", img);
	//waitKey(20);
	return flag;
}