#ifndef HAZEREMOVER_H
#define HAZEREMOVER_H

#include <cv.h>
#include <highgui.h>

struct DehazeParameter
{
	int mDrkChnWindowRadius;
	int mAtmosphereThreshold;
	double mOmega;
	int mGuideFilterRadius;
	double mEpsilon;
	double mTransmissionThre;
	DehazeParameter()
	{
		mDrkChnWindowRadius = 15;
		mAtmosphereThreshold = 220;
		mOmega = 0.95;
		mGuideFilterRadius = 120;
		mEpsilon = 0.001;
		mTransmissionThre = 0.1;
	}
};

class HazeRemover
{
public:
	DehazeParameter mParameter;
	cv::Mat mImg;			//Original Image
	cv::Mat mDarkChannel;		
	cv::Mat mTransmissionRateEst;   
	cv::Mat mTransmissionRate;      
	cv::Mat mDehazeImg;			
	cv::Vec3d mAtmosphereLight;     
	void process(cv::Mat img);

private:
	void computeDarkChannel(); 
	void computeAtmosphereLight();
	void estimateTransmissionRate();
	void computeDehazeImg();
	void computeTransmissionRate();
};

#endif
