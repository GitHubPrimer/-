#include "opencv2/opencv.hpp"
#include <iostream>
using namespace std;
using namespace cv;
vector<Vec4f> boundingBoxInf;//���ڴ洢һ֡��ɸѡ������boundingBox��Ϣ��Vec4f�ĵ�һ������Ԫ��Ϊ��Χ�����Ͻ����꣬�ڶ�������Ԫ�ش����Χ�еĳ��Ϳ�
//vector<Vec4f> preciseboundingBoxInf;//����ȷ�����ڴ洢һ֡��ɸѡ������boundingBox��Ϣ��Vec4f�ĵ�һ������Ԫ��Ϊ��Χ�����Ͻ����꣬�ڶ�������Ԫ�ش����Χ�еĳ��Ϳ�
void drawBoundingBox(Mat src)
{
	boundingBoxInf.clear();//���vector���顣
	//����һЩ����
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;


	// �ҳ�����
	findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	// ����αƽ����� + ��ȡ���κ�Բ�α߽��
	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	//vector<Point2f>center(contours.size());
	//vector<float>radius(contours.size());

	//һ��ѭ�����������в��֣����б���������ĵĲ���
	for (unsigned int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);//��ָ�����ȱƽ���������� 
		boundRect[i] = boundingRect(Mat(contours_poly[i]));//����㼯�������棨up-right�����α߽�
	}

	// ���ƶ�������� + ��Χ�ľ��ο� + Բ�ο�
	double area = (double)(src.cols * src.rows);
	double minArea = area / 500;
	double maxArea = area / 50;
	for (int unsigned i = 0; i<contours.size(); i++)
	{
		double boundingWidth = (double)(double)(boundRect[i].br().x - boundRect[i].tl().x);
		double boundingHeight = (double)(boundRect[i].br().y - boundRect[i].tl().y);
		double boundingboxArea = boundingWidth * boundingHeight;
		double boxRatio = boundingHeight / boundingWidth;
		if (boundingboxArea >  minArea && boundingboxArea <  maxArea && boxRatio > 1.5 && boxRatio < 4)//ȥ�����ŶȲ��ߵ�boundingBox
		{
			/*Vec4f bB0(boundRect[i].tl().x, boundRect[i].tl().y, boundingWidth, boundingHeight);
			preciseboundingBoxInf.push_back(bB0);*/
			int temp_x = boundRect[i].tl().x, temp_y = boundRect[i].tl().y;//temp_x�����Χ�����Ͻ�x���꣬temp_y�����Χ�����Ͻ�y����
			if (temp_x - boundingWidth / 2 > 0)
				temp_x = temp_x - boundingWidth / 2;
			if (temp_x + boundingWidth * 2 < src.cols)//��չ���Χ���ұ߽�δԽ�磬ע��Mat�����ԭ�㣨0��0��λ��ͼ�����Ͻǣ�
				boundingWidth = (int)(boundingWidth * 2);
			if (temp_y - boundingHeight / 2 > 0)
				temp_y = temp_y - boundingHeight / 2;
			if (temp_y + boundingHeight * 2 < src.rows)//��չ���Χ���±߽�δԽ��
				boundingHeight = (int)(boundingHeight * 2);
			Vec4f bB(temp_x, temp_y, boundingWidth, boundingHeight);//����չ��İ�Χ����Ϣ����bB��
			boundingBoxInf.push_back(bB);
		}
	}
	//for (int i = 0; i < boundingBoxInf.size(); i++)
	//{
	//	cout << "x = " << boundingBoxInf[i][0] << ",y = " << boundingBoxInf[i][1] << ",length = " << boundingBoxInf[i][2] << ",wide = " << boundingBoxInf[i][3] << endl;
	//}

}