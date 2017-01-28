#include "HazeRemover.h"
#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;
using cv::Mat;

void guiedFilter(Mat I, Mat P,Mat& transmissionRate,int radius, double epsilon);

struct Node 
{ 
	int value; 
	int index; 
}; 

bool compare(Node a, Node b) 
{ 
	return a.value>b.value;
} 

void computeRGBMinImage(Mat img, Mat& RGBMinImage)
{
	//spilt RGB to 3 single channel component;
	vector<Mat> imgComponent;
	split(img,imgComponent);
	cv::min(imgComponent[0],imgComponent[1],RGBMinImage);
	cv::min(RGBMinImage,imgComponent[2],RGBMinImage);
}

void borderExtension(Mat img, Mat &borderExtImg,int borderSize)
{
	copyMakeBorder(img,borderExtImg,borderSize,borderSize,borderSize,borderSize,cv::BORDER_REPLICATE);
}

void HazeRemover::process(Mat img)
{
	img.copyTo(mImg);
	int width = mImg.cols;
	int height = mImg.rows;

	mDarkChannel.create(height,width,CV_8UC1);
	memset(mDarkChannel.data,0,sizeof(uint8_t)*width*height);
	computeDarkChannel();
	computeAtmosphereLight();
	estimateTransmissionRate();
	computeTransmissionRate();
	computeDehazeImg();

	//cv::imshow("darkChannel",mDarkChannel);
	//cv::waitKey();
}

void HazeRemover::computeDehazeImg()
{
	int width = mImg.cols;
	int height = mImg.rows;

	mDehazeImg.create(height,width,CV_8UC3);
	memset(mDehazeImg.data,0,sizeof(uint8_t)*width*height*3);

	double atmosphereLightB = mAtmosphereLight[0];
	double atmosphereLightG = mAtmosphereLight[1];
	double atmosphereLightR = mAtmosphereLight[2];
	for (int i = 0; i < width*height; i++)
	{
		double transmissionRate = ((float*)mTransmissionRate.data)[i];
		if (transmissionRate < mParameter.mTransmissionThre)
			transmissionRate = mParameter.mTransmissionThre;

		mDehazeImg.data[i*3]   = cv::saturate_cast<uint8_t>(((float)mImg.data[i*3]   - atmosphereLightB)/transmissionRate + atmosphereLightB);
		mDehazeImg.data[i*3+1] = cv::saturate_cast<uint8_t>(((float)mImg.data[i*3+1] - atmosphereLightG)/transmissionRate + atmosphereLightG);
		mDehazeImg.data[i*3+2] = cv::saturate_cast<uint8_t>(((float)mImg.data[i*3+2] - atmosphereLightR)/transmissionRate + atmosphereLightR);

	}

	//imshow("dehaze",mDehazeImg);
	//imshow("origin",mImg);
}

void HazeRemover::estimateTransmissionRate()
{
	int width = mImg.cols;
	int height = mImg.rows;
	// 先忽略对A的计算
	mTransmissionRateEst.create(height,width,CV_32FC1);
	Mat darkChannelFloat;
	mDarkChannel.convertTo(darkChannelFloat,CV_32FC1);

	//mTransmissionRateEst = 1 - mParameter.mOmega*darkChannelFloat/255;

	float Amax = mAtmosphereLight[0];
	for (int i = 1; i < 3; i++)
	{
		if (Amax < mAtmosphereLight[i]) 
			Amax = mAtmosphereLight[i];
	}

	for (int u = 0; u < width; u++)
	{
		for (int v = 0; v < height; v++)
		{
			float darkChannelElement = darkChannelFloat.at<float>(v,u);
			mTransmissionRateEst.at<float>(v,u) = max(0.0 ,1 - mParameter.mOmega*darkChannelElement/Amax);
		}
	}

	//Mat temp = mTransmissionRateEst * 255;
	//Mat transmissionRateEstGray;
	//temp.convertTo(transmissionRateEstGray,CV_8UC1);
	//imshow("trans",transmissionRateEstGray);
}

void HazeRemover::computeAtmosphereLight()
{
	int numOfCandidate = 0.001 * (float)mDarkChannel.rows* (float)mDarkChannel.cols;

	const int length = mDarkChannel.rows * mDarkChannel.cols;

	Node *darkChannelWithInd = new Node[length];
	for (int i = 0; i < length; i++)
	{
		darkChannelWithInd[i].index = i;
		darkChannelWithInd[i].value = mDarkChannel.data[i];
	}

	std::sort(darkChannelWithInd,darkChannelWithInd+length,compare);

	cv::Vec3d accumulate;
	accumulate[0] = 0;
	accumulate[1] = 0;
	accumulate[2] = 0;

	for (int i = 0; i < numOfCandidate; i++)
	{
		int ind = darkChannelWithInd[i].index*3;
		accumulate[0] += mImg.data[ind];
		accumulate[1] += mImg.data[ind+1];
		accumulate[2] += mImg.data[ind+2];
	}

	mAtmosphereLight[0] = accumulate[0]/numOfCandidate;
	mAtmosphereLight[1] = accumulate[1]/numOfCandidate;
	mAtmosphereLight[2] = accumulate[2]/numOfCandidate;

	int thershold = mParameter.mAtmosphereThreshold;
	mAtmosphereLight[0] = mAtmosphereLight[0] > thershold ? thershold : mAtmosphereLight[0];
	mAtmosphereLight[1] = mAtmosphereLight[1] > thershold ? thershold : mAtmosphereLight[1];
	mAtmosphereLight[2] = mAtmosphereLight[2] > thershold ? thershold : mAtmosphereLight[2];

	//cout<<mAtmosphereLight<<endl;
	delete []darkChannelWithInd;

}

void HazeRemover::computeDarkChannel()
{
	int width = mImg.cols;
	int height = mImg.rows;
	int windowSize = mParameter.mDrkChnWindowRadius *2+1;
	int radius = mParameter.mDrkChnWindowRadius;

	Mat RGBMinImage(height,width,CV_8UC1);
	computeRGBMinImage(mImg,RGBMinImage);
	//imshow("RGBMinImage",RGBMinImage);
	//cv::waitKey();

	Mat RGBMinImageWithBorder;
	borderExtension(RGBMinImage,RGBMinImageWithBorder,radius);
	//imshow("RGBMinImage",RGBMinImageWithBorder);
	//cv::waitKey();

	//with border
	for (int v = 0; v < height; v++)
	{
		uint8_t* currRowData = mDarkChannel.data + v * width;
		for (int u = 0; u < width; u++)
		{
			Mat currWindow = RGBMinImageWithBorder(cv::Rect(u,v,windowSize,windowSize));
			double minValInWindow;
			minMaxLoc(currWindow,&minValInWindow,NULL,NULL,NULL);  
			*(currRowData + u) = uint8_t(minValInWindow);
		}
	}
}

void HazeRemover::computeTransmissionRate()
{
	Mat imgGray,imgGrayFloat;
	cv::cvtColor(mImg,imgGray,CV_RGB2GRAY);
	imgGray.convertTo(imgGrayFloat,CV_32FC1);
	imgGrayFloat /= float(255); //[0,1]
	guiedFilter(imgGrayFloat,mTransmissionRateEst,mTransmissionRate,mParameter.mGuideFilterRadius,mParameter.mEpsilon);
}

//implementation of https://arxiv.org/pdf/1505.00996v1.pdf algorithm 1
void guiedFilter(Mat I, Mat P,Mat& transmissionRate,int radius, double epsilon)
{

	Mat meanI,meanP;
	Mat II,IP;
	Mat corrI,corrIP;
	Mat varI,covIP;
	//Mat meanIuint8,meanPuint8;
	int windowSize = radius *2 +1;
	//int windowSize = radius;

	cv::blur(I,meanI,cv::Size(windowSize,windowSize));
	cv::blur(P,meanP,cv::Size(windowSize,windowSize));

	II = I.mul(I);
	IP = I.mul(P);

	cv::blur(II,corrI,cv::Size(windowSize,windowSize));
	cv::blur(IP,corrIP,cv::Size(windowSize,windowSize));

	varI = corrI - meanI.mul(meanI);
	covIP = corrIP - meanI.mul(meanP);

	Mat a,b;
	a = covIP/(varI+epsilon);
	b = meanP - a.mul(meanI);

	Mat meanA,meanB;
	cv::blur(a,meanA,cv::Size(windowSize,windowSize));
	cv::blur(b,meanB,cv::Size(windowSize,windowSize));

	transmissionRate = meanA.mul(I) + meanB;
	//Mat temp = transmissionRate * 255;
	//Mat rate;
	//temp.convertTo(rate,CV_8UC1);
	//imshow("rate",rate);
}
