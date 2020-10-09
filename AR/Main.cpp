// ErweiterteRealitt.cpp : Defines the entry point for the console application.
//

#include <opencv2\opencv.hpp>
#include <iostream>
#include <Windows.h>
using namespace std;
using namespace cv;


Mat frame;
int level, level2;

static void on_trackbar(int, void*)
{
	//threshold(frame, frame, 10, level, THRESH_TRIANGLE);
}

int subpixSampleSafe(const cv::Mat &pSrc, const cv::Point2f &p)
{
	int x = int(floorf(p.x));
	int y = int(floorf(p.y));

	if (x < 0 || x >= pSrc.cols - 1 ||
		y < 0 || y >= pSrc.rows - 1)
		return 127;

	int dx = int(256 * (p.x - floorf(p.x)));
	int dy = int(256 * (p.y - floorf(p.y)));

	unsigned char* i = (unsigned char*)((pSrc.data + y * pSrc.step) + x);
	int a = i[0] + ((dx * (i[1] - i[0])) >> 8);
	i += pSrc.step;
	int b = i[0] + ((dx * (i[1] - i[0])) >> 8);
	return a + ((dy * (b - a)) >> 8);
}

int main()
{
	VideoCapture cap = VideoCapture(0);

	for (;;)
	{
		// wait for a new frame from camera and store it into 'frame'
		cap.read(frame);
		
		// check if we succeeded
		if (frame.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		}
		Mat canny_output;
		cvtColor(frame, canny_output, CV_BGR2GRAY);
		//createTrackbar("treshold max", "Live", &level,1000,on_trackbar);
		threshold(canny_output, canny_output, 1, 200, THRESH_OTSU);
		
		
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		//Canny(canny_output, canny_output, 100, 100 * 2, 3);
		// Find contours
		findContours(canny_output, contours, RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

		vector<vector<Point> > contours_poly(contours.size());
		vector<Rect> boundRect(contours.size());
		vector<float>radius(contours.size());

		for (int i = 0; i < contours.size(); i++)
		{
			approxPolyDP(contours[i], contours_poly[i],arcLength(contours[i],true) * 0.02, true);
			boundRect[i] = boundingRect(contours_poly[i]);
			Scalar colour;
			if (contours_poly[i].size() == 4)
				colour = Scalar(0, 0, 255);
			else
			{
				continue;
			}
			if (boundRect[i].height<20 || boundRect[i].width<20 || boundRect[i].width > frame.cols - 10 || boundRect[i].height > frame.rows - 10) {
				continue;
			}
			Point const* points(&contours_poly[i][0]);
			int n_points(static_cast<int>(contours_poly[i].size()));
			//linien werden gezeichnet
			polylines(frame,&points,&n_points,1,true,colour,4);

			for (int i = 0; i < n_points; ++i)
			{
				//ausßen Kreise
				circle(frame, points[i], 3, CV_RGB(0, 255, 0), -1);

				double dx = (double)(points[(i + 1) % 4].x - points[i].x) / 7.0;
				double dy = (double)(points[(i + 1) % 4].y - points[i].y) / 7.0;

				for (int j = 1; j < 7; ++j)
				{
					double px = (double)points[i].x + (double)j*dx;
					double py = (double)points[i].y + (double)j*dy;

					cv::Point p;
					p.x = (int)px;
					p.y = (int)py;
					//blaue Punkte
					circle(frame, p, 2, CV_RGB(0, 0, 255), -1);
				}
				for (int i = 0; i<4; ++i) {
						// Stripe size
						int stripeLength = (int)(0.8*sqrt(dx*dx + dy * dy));
					if (stripeLength < 5)
						stripeLength = 5;
					cv::Size stripeSize;
					stripeSize.width = 3;
					stripeSize.height = stripeLength;

					// Direction vectors
					cv::Point2f stripeVecX;
					cv::Point2f stripeVecY;
					double diffLength = sqrt(dx*dx + dy * dy);
					stripeVecX.x = dx / diffLength;
					stripeVecX.y = dy / diffLength;
					stripeVecY.x = stripeVecX.y;
					stripeVecY.y = -stripeVecX.x;

				}
			}
			
		}

		// Draw contours
		Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
		/*for (int i = 0; i< contours.size(); i++)
		{
			Scalar color = Scalar(0,0,255);
			if (boundRect[i].height <= 30 || boundRect[i].width <= 30 
				|| !(boundRect[i].width >= boundRect[i].height - 30 && boundRect[i].width <= boundRect[i].height + 30))
				continue;
			rectangle(frame, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
		}*/
		imshow("Contours", drawing);
		imshow("Contours2", frame);
		// show live and wait for a key with timeout long enough to show images
		

		if (waitKey(10) >= 0) {

			if (GetAsyncKeyState(VK_ESCAPE))
				break;
			continue;
		}
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}