
//--------------------------------------������˵����-------------------------------------------
//		����˵������OpenCV3������š�OpenCV2���鱾����ʾ������20
//		����������ǰ�󱳾�����
//		�������ò���ϵͳ�� Windows 7 64bit
//		��������IDE�汾��Visual Studio 2013
//		��������OpenCV�汾��	3.0
//		2017��5�� Revised by @�Ž���
//------------------------------------------------------------------------------------------------

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"//background_segm�������ָ�
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <opencv2/tracking.hpp>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

extern vector<Vec4f> boundingBoxInf;//���ڴ洢ɸѡ������boundingBox��Ϣ��Vec4f�ĵ�һ������Ԫ��Ϊ��Χ�����Ͻ����꣬�ڶ�������Ԫ�ش����Χ�еĳ��Ϳ�
//extern vector<Vec4f> preciseboundingBoxInf;//����ȷ�����ڴ洢ɸѡ������boundingBox��Ϣ��Vec4f�ĵ�һ������Ԫ��Ϊ��Χ�����Ͻ����꣬�ڶ�������Ԫ�ش����Χ�еĳ��Ϳ�
vector<Vec4f> finalBoundingBoxInf;//���ڴ洢����ɸѡ������boundingBox��Ϣ��Vec4f�ĵ�һ������Ԫ��Ϊ��Χ�����Ͻ����꣬�ڶ�������Ԫ�ش����Χ�еĳ��Ϳ�
extern int dealMat(Mat img);
extern  void drawBoundingBox(Mat src);
extern Mat denoise(Mat fgmask);
//extern Mat deShadow(Mat fgmask, Mat img, double rgbDegreeThreshold);
//int g_int_fTau = 72, g_int_maxfTau = 100;
//int g_frames;
//int g_varThreshold, g_maxVarThreshold = 100;
Mat out, img, fgmask;//, fgimg, grayimg, graybgimg;
Ptr<BackgroundSubtractorMOG2> bg_model = createBackgroundSubtractorMOG2();//����OpenCV3��д��,��Ptr<����>���ȼ��ڡ�����*��
VideoCapture cap;//��ʵ������Ƶ����
//cap.open(0);


//--------------------------------------��help( )������--------------------------------------
//		 ���������һЩ������Ϣ
//----------------------------------------------------------------------------------------------
static void help()
{
	printf("\n\n\t�˳���չʾ����Ƶǰ�󱳾�����ķ���,����cvUpdateBGStatModel()����.\n"
		"\n\n\t�������Ȼᡰѧϰ��������Ȼ����зָ\n"
		"\n\n\t�����ù���Space���ո���й����л���\n\n");
}
//void on_TrackbarForVarThreshold(int)
//{
//	bg_model->setVarThreshold((double)g_varThreshold);
//	cout << "VarThreshold = " << bg_model->getVarThreshold() << endl;
//}
//void on_TrackbarForShadowDetect(int)
//{
//	bg_model->setShadowThreshold((double)g_int_fTau / 100);
//	cout << "ShadowValue = " << bg_model->getShadowThreshold() << endl;
//}
//void on_TrackbarForChangFrame(int)
//{
//	cap.set(CAP_PROP_POS_MSEC, (double)(g_frames * 100));
//	imshow("ǰ����ֵ��ͼ�񣨽���ȥ��Ӱ��", out);
//}

int RectOverlap(Rect2d r, Rect2d rOther)
{
	int x0 = std::max(r.x, rOther.x);
	int x1 = std::min(r.x + r.width, rOther.x + rOther.width);
	int y0 = std::max(r.y, rOther.y);
	int y1 = std::min(r.y + r.height, rOther.y + rOther.height);

	if (x0 >= x1 || y0 >= y1) return 0.f;

	float areaInt = (x1 - x0)*(y1 - y0);
	float overlap = areaInt / ((float)r.width*r.height + (float)rOther.width*rOther.height - areaInt);
	if (overlap > 0.3)return 1;
	else
		return 0;
}
int  JudgeSameBBox(Rect2d b1, Rect2d b2)
{
	int b1c_x = b1.x + b1.width / 2;
	int b1c_y = b1.y + b1.height / 2;
	int b2c_x = b2.x + b2.width / 2;
	int b2c_y = b2.y + b2.height / 2;
	int disc = abs(b1c_x - b2c_x) + abs(b1c_y - b2c_y);
	int dis = abs(b1.x - b2.x) + abs(b1.y - b2.y);
	if ((disc < 15) || (dis<15))return 1;
	else
		return 0;
}
int compareHist(Rect2d b1, Rect2d b2, Mat frame)
{

	if (b1.x < 0 || b1.y < 0 || b2.x < 0 || b2.y < 0)return 0;
	int r_x1 = b1.x + b1.width;
	int r_y1 = b1.y + b1.height;
	int r_x2 = b2.x + b2.width;
	int r_y2 = b2.y + b2.height;
	if (r_x1 > frame.cols || r_x2 > frame.cols || r_y1>frame.rows || r_y2>frame.cols)return 0;
	Mat src1, src2, gray1, gray2;
	//src1 = imread(argv[1]);
	//src2 = imread(argv[2]);
	src1 = frame(b1);
	src2 = frame(b2);
	cvtColor(src1, gray1, CV_BGR2GRAY);
	cvtColor(src2, gray2, CV_BGR2GRAY);

	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	int channels[] = { 0 };

	Mat hist1, hist2;
	calcHist(&gray1, 1, channels, Mat(), hist1, 1, &histSize, &histRange);
	calcHist(&gray2, 1, channels, Mat(), hist2, 1, &histSize, &histRange);

	//��أ�CV_COMP_CORREL      
	//������CV_COMP_CHISQR  
	//ֱ��ͼ�ཻ��CV_COMP_INTERSECT  
	//Bhattacharyya���룺CV_COMP_BHATTACHARYYA  
	double diff = compareHist(hist1, hist2, CV_COMP_BHATTACHARYYA);
	if (diff > 0.5)return 0;
	else return 1;

	//imshow("img1", img1);
}
//-----------------------------------��main( )������--------------------------------------------
//		����������̨Ӧ�ó������ں��������ǵĳ�������￪ʼ
//------------------------------------------------------------------------------------------------

int main(int argc, const char** argv)
{
	
	double t = (double)getTickCount();
	MultiTracker trackers("KCF");
	bg_model->setVarThreshold(20);
	bg_model->setHistory(150);//���ñ�����������,����ֵԽ����µ�Խ��
	bg_model->setShadowThreshold((double)0.72);
	cap.open("qingshi1.avi");//�ٽ���Ƶ�����ʼ��Ϊ1.avi��Ƶ
	int numFrames = (int)cap.get(CAP_PROP_FRAME_COUNT);
	cout << "numFrames = " << numFrames << endl;
	help();
	namedWindow("��Ƶ");
	//namedWindow("����");
	//namedWindow("ǰ����ֵ��ͼ�񣨽���ȥ��Ӱ��", 0);
	bool update_bg_model = true;

	if (!cap.isOpened())//�����Ƶû�б����򷵻�-1���˳�
	{
		printf("can not open camera or video file\n");
		system("pause");
		return -1;
	}
	
	
	vector<Rect2d> objectsold;

	//vector<Vec4f> finalin;
	int num = 0;//�������֡������
	for (;;)
	{
		cap >> img;
		if (img.data == NULL)break;
		Mat frame = img.clone();
		
		if (img.empty())
			break;
		/*cvCreateTrackbar("��Ӱ����", "ǰ����ֵ��ͼ�񣨽���ȥ��Ӱ��", &g_int_fTau, g_int_maxfTau, on_TrackbarForShadowDetect);
		cvCreateTrackbar("ǰ����ֵ", "ǰ����ֵ��ͼ�񣨽���ȥ��Ӱ��", &g_varThreshold, g_maxVarThreshold, on_TrackbarForVarThreshold);
		cvCreateTrackbar("֡��", "ǰ����ֵ��ͼ�񣨽���ȥ��Ӱ��", &g_frames, numFrames, on_TrackbarForChangFrame);*/
		//g_frames = (int)cap.get(CAP_PROP_POS_MSEC) / 100;
		/*if (fgimg.empty())
			fgimg.create(img.size(), img.type());*/

		//����ģ��
		bg_model->apply(img, fgmask, update_bg_model ? -1 : 0);//update_bg_model? -1:0����˼�ǵ�update_bg_model > 0��ʱ��update_bg_modelȡ-1����֮ȡ0.

		Mat bgimg;
		bg_model->getBackgroundImage(bgimg);
		//cvtColor(bgimg, graybgimg, COLOR_BGR2GRAY);

		for (int i = 0; i < fgmask.rows; i++)//��������ʶ��������ƶ���Ӱ
		{
			for (int j = 0; j < fgmask.cols; j++)
			{
				if (fgmask.at<uchar>(i, j) == 127)
				{
					fgmask.at<uchar>(i, j) = 0;
				}
			}
		}

		out = denoise(fgmask);//���õ���ǰ�����н��뼰��ʴ���ʹ���

		//fgimg = Scalar::all(0);//��fgimg������ȫ��ʼ��Ϊ0����ɫ��
		//img.copyTo(fgimg, out);//��imgͼ���еĶ�Ӧ��outͼ�����겻Ϊ0�����ص㿽��fgimg�еĶ�Ӧλ��

		//if (!bgimg.empty())
			//imshow("����", bgimg);
		//imshow("ǰ����ֵ��ͼ�񣨽���ȥ��Ӱ��", out);
		drawBoundingBox(out);//�Խ���ͼ����boungdingBox
		finalBoundingBoxInf.clear();
		for (int i = 0; i < boundingBoxInf.size(); i++)
		{
			Mat imageROI = img(Rect(boundingBoxInf[i][0], boundingBoxInf[i][1], boundingBoxInf[i][2], boundingBoxInf[i][3]));
			Mat postimageROI;
			resize(imageROI, postimageROI, Size(100, 200), (0, 0), (0, 0), 3);
			//cout << "size = " << postimageROI.size() << endl;
			//imshow("postimageROI", postimageROI);
			if (dealMat(postimageROI))//��ROI����ʶ��Ϊ���ˣ���
			{
				Vec4f temp(boundingBoxInf[i][0], boundingBoxInf[i][1], boundingBoxInf[i][2], boundingBoxInf[i][3]);
				//rectangle(img, Point(preciseboundingBoxInf[i][0], preciseboundingBoxInf[i][1]), Point(preciseboundingBoxInf[i][0] + preciseboundingBoxInf[i][2], preciseboundingBoxInf[i][1] + preciseboundingBoxInf[i][3]), Scalar(123, 0, 0), 2, 2, 0);//���ƾ���
				finalBoundingBoxInf.push_back(temp);
				//finalin.push_back(temp);
			}
		}
		//	trackers.update(frame);
		Rect2d BBox_T;

		if (trackers.objects.size() == 0)//��ʼ׷��Ŀ��Ϊ��
		{
			for (int i = 0; i < finalBoundingBoxInf.size(); i++)
			{
				Vec4f fb = finalBoundingBoxInf[i];
				BBox_T.x = fb[0]; BBox_T.y = fb[1]; BBox_T.width = fb[2]; BBox_T.height = fb[3];
				trackers.add(frame, BBox_T);
			}
			trackers.update(frame);
			for (unsigned i = 0; i < trackers.objects.size(); i++)
			{
				rectangle(frame, trackers.objects[i], Scalar(255, 0, 0), 2, 1);
			}

			imshow("KCF", frame);
		}
		else//�ǳ�ʼ״̬��׷��Ŀ�겻Ϊ��
		{
			num++;
			//trackers.update(frame);
			for (int i = 0; i < finalBoundingBoxInf.size(); i++)
			{
				Vec4f fb = finalBoundingBoxInf[i];
				BBox_T.x = fb[0]; BBox_T.y = fb[1]; BBox_T.width = fb[2]; BBox_T.height = fb[3];

				bool flag = false;
				for (int j = 0; j < trackers.objects.size(); j++)
				{
					int JT = compareHist(BBox_T, trackers.objects[j], frame);
					//int BT = JudgeSameBBox(BBox_T, trackers.objects[j]);
					int OT = RectOverlap(BBox_T, trackers.objects[j]);
					if (JT && OT)//���ж�Ϊ�ظ���Χ��
					{
						flag = false;
						break;
					}
					else flag = true;
				}
				bool flagT = false;
				for (int k = 0; k < trackers.objects.size(); k++)
				{
					int tb = compareHist(BBox_T, trackers.objects[k], frame);
					//int BT = JudgeSameBBox(BBox_T, trackers.objects[k]);
					int OT = RectOverlap(BBox_T, trackers.objects[k]);
					if (tb && OT)
					{
						flagT = false;
						break;
					}
					else
						flagT = true;

				}
				if (flag&&flagT)//��BBox_T��Ϊ�ظ���Χ��
				{
					trackers.add(frame, BBox_T);
				}
			}
			if (num % 3 == 0)
			{
				objectsold.clear();
				for (int i = 0; i < trackers.objects.size(); i++)
				{
					objectsold.push_back(trackers.objects[i]);
				}
				//MultiTracker pretrackers("KCF");
				//	pretrackers = trackers;
			}
			
			trackers.update(frame);
			
			cout << "Ŀ����1��" << trackers.objects.size() << endl;

			for (int i = 0; i < trackers.objects.size(); i++)//���������˵İ�Χ��
			{
				bool f1 = false;
				for (int j = 0; j < objectsold.size(); j++)
				{

					if ((trackers.objects[i].x == objectsold[j].x) && (trackers.objects[i].y == objectsold[j].y)
						&& (trackers.objects[i].width == objectsold[j].width))
					{
						trackers.objects[i].width = 0;
						trackers.objects[i].height = 0;
						//trackers.objects._Pop_back_n(trackers.objects[i])
					}

				}
			}
			
			vector<Rect2d> Ob_clear;
			for (int i = 0; i < trackers.objects.size(); i++)
			{
				if (trackers.objects[i].width == 0 && trackers.objects[i].height == 0)
				{
					trackers.trackerList.pop_back();
					continue;
				}
				Ob_clear.push_back(trackers.objects[i]);
			}
			trackers.objects.clear();
			for (int i = 0; i < Ob_clear.size(); i++)
			{
				trackers.objects.push_back(Ob_clear[i]);
			}
			cout << "Ŀ����2��" << trackers.objects.size() << endl;
			Ob_clear.clear();
			for (unsigned i = 0; i < trackers.objects.size(); i++)
			{
				rectangle(frame, trackers.objects[i], Scalar(255, 0, 0), 2, 1);
			}

			imshow("KCF", frame);
		}
	
		
		//waitKey(500);
		imshow("��Ƶ", img);


		char k = (char)waitKey(1);
		if (k == 27) break;
		if (k == ' ')
		{
			update_bg_model = !update_bg_model;
			if (update_bg_model)
				printf("\t>��������(Background update)�Ѵ�\n");
			else
				printf("\t>��������(Background update)�ѹر�\n");
		}
	}
	t = (double)getTickCount() - t;
	//printf("tdetection time = %gms\n", t*1000. / cv::getTickFrequency());
	printf("֡�� = %g/s\n", numFrames / (t*1000. / cv::getTickFrequency()/1000));
	system("pause");
	return 0;
}
