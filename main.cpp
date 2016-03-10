#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;					
using namespace std;

#define SQR(n) (n)*(n)
#define FRAME_DELAY 5				// control the delay of surprise action

int is_surprise = 0;
int radius_slider;				
int radius_slider_sqr;
const int radius_slider_max = 444; // greater that possible, actually the largest radius is sqrt(3)*256
const char* window = "Project 4";	
Point current_pixel;
Mat dst;
list<Mat> prevFrame;				// store previous frames for show them with a delay

void init_gui();										// GUI initialization
void onMouse( int event, int x, int y, int, void* );	// callback function of on mouse event
void onTrackbar(int, void*);							// callback function of on trackbar event
void updateImage();										// ...
void surprise(int, void*);								// callback function of on surprise trackbar event
void updateSurpriseImage();								// ...

int main(int argc, char** argv)
{
	int act;
	current_pixel.x = 0;
	current_pixel.y = 0;

	cout << "1 -- Static\n" << "2 -- Web camera" << endl;
	cin >> act;

	if ( act == 1 ) {
		dst = imread("balls.jpg");
		if (dst.empty()) 
		{
			cout << "Cannot load image!" << endl;
			return -1;
		}
		current_pixel.x = dst.cols/2;
		current_pixel.y = dst.rows/2;

		init_gui();

		waitKey();
		destroyAllWindows();
	}

	if (act == 2) {
		VideoCapture vCapture(0); // 0 means open default web camera

		if (!vCapture.isOpened()) {
			cerr << "Cannot open webcam" << endl;
			return -1;
		}

		vCapture >> dst;

		prevFrame.push_back(dst.clone()); // saving frame, it will be used for surprise action

		cout << "Capturing ..." << endl;

		current_pixel.x = dst.cols/2;	// default current pixel position, the color is selected according to the color of this pixel
		current_pixel.y = dst.rows/2;	// the center of the image

		init_gui();

		for(;;) //infinitely capturing frames
		{
			vCapture >> dst;

			if( dst.empty() )
				break;
			prevFrame.push_back(dst.clone()); // saving frame, it will be used for surprise action

			flip(dst, dst, 1); // flipping the image, such that it will look more like a mirror

			if (!is_surprise){ 
				updateImage();
			}else{
				updateSurpriseImage();
			}
			if(waitKey(30) >= 0) break;
		}
		destroyAllWindows();
		return 0;
	}

	return 0;
}

void init_gui()
{
	namedWindow(window, CV_WINDOW_AUTOSIZE); // create a resizable windows 
	imshow(window, dst);					

	createTrackbar("radius", window, &radius_slider, radius_slider_max, onTrackbar); // trackbar for radius
	setTrackbarPos("radius", window, radius_slider_max / 8);						// setting default value
	createTrackbar("surprise", window, &is_surprise, 1, surprise);					// trackbar for surprise action
	setMouseCallback(window, onMouse, NULL);										// attaching callback for on mouse event, needed for pixel selection
}

void onMouse( int event, int x, int y, int, void* )
{
	// only left button event, ignore other mouse buttons
	if( event != EVENT_LBUTTONDOWN )	
		return;
	current_pixel.x = y;
	current_pixel.y = x;
	updateImage();
}

void onTrackbar(int, void*)
{
	radius_slider_sqr = SQR(radius_slider);	
	updateImage();
}

void updateImage()
{
	int i, j;
	Mat work = dst.clone();		// hard copy of matrix
	Point3_<uchar>* p;			// pointer for accessing pixel BGR values
	Point3_<uchar>* cur_color = work.ptr<Point3_<uchar> >(current_pixel.x, current_pixel.y); // value of the selected pixel
	

	int grayColor;
	// walk through the matrix
	for(i=0; i<work.rows; i++){
		for(j=0; j<work.cols; j++){
			p = work.ptr<Point3_<uchar> >(i,j);
			if ( (SQR(p->x - cur_color->x) + SQR(p->y - cur_color->y) + SQR(p->z - cur_color->z)) >= radius_slider_sqr){
				grayColor = (p->x + p->y + p->z)/3;
				p->x = p->y = p->z = grayColor;
			}
		}
	}

	// remove too old frame from the memory, needed for surprise action
	if (prevFrame.size() > FRAME_DELAY){
		prevFrame.pop_front();
	}

	imshow(window, work);
}

void surprise(int, void*)
{
	;// nothing here since createTrackbar handles the change of is_surprise variable
}


void updateSurpriseImage()
{
	Mat work;
	Mat tmp[3];
	Mat result = Mat::zeros(prevFrame.back().rows, prevFrame.back().cols, CV_8UC3); // the result matrix full of zeros such that we can easily add
	auto iterator = prevFrame.begin();

	if (prevFrame.size() > FRAME_DELAY){ // if there is not enough frames just show image
		for(int i=2;i>=0;i--){
			split(*iterator, tmp);
			// BGR
			// filling other channels with zeros
			tmp[i] = Mat::zeros(tmp[i].rows, tmp[i].cols, CV_8UC1); // Set red channel 0
			if (i < 2){
				tmp[i+1] = Mat::zeros(tmp[i+1].rows, tmp[i+1].cols, CV_8UC1);
			} else {
				tmp[0] = Mat::zeros(tmp[0].rows, tmp[0].cols, CV_8UC1);
			}

			merge(tmp, 3, work);
			for(int j=0;j<FRAME_DELAY/3;j++){
				iterator++;
			}
			add(result, work, result);
		}
		flip(result, result, 1);
		prevFrame.pop_front();
		imshow(window, result);
	} else {
		imshow(window, dst);
	}

}
