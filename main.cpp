#include <iostream>
#include <cv.h>
#include <highgui.h>

#include "HazeRemover.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		cerr << "usage: ./hazeRemovalBin [image_directory]/[image_name].[image_file_format]" << endl;
		return 0;
	}

	string imgFile = argv[1];
	Mat img = imread(imgFile);
	imshow("oringal img",img);

	HazeRemover hr;
	hr.process(img);
	imshow("result",hr.mDehazeImg);
	
	waitKey();
	return 0;
}
