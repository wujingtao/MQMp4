#ifndef FASTER_RCNN_HPP
#define FASTER_RCNN_HPP
#include <stdio.h>  // for snprintf
#include <string>
#include <vector>
#include <math.h>
#include <fstream>
#include <boost/python.hpp>
#include "caffe/caffe.hpp"
#include "nms/gpu_nms.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace caffe;
using namespace std;


#define max(a, b) (((a)>(b)) ? (a) :(b))
#define min(a, b) (((a)<(b)) ? (a) :(b))


#define		EV_ERR_SUCCESS 0
//矩形定义
typedef struct {
	int x;
	int y;
	int w;
	int h;
}EVRect;

typedef struct
{
	unsigned char * imagedata;			//图像矩阵指针
	int width;							//图像宽度
	int height;							//图像高度
	int widthStep;						//每行像素的字节数
	int type;							//图像类型
	char useROI;						//感兴趣区域表示
    EVRect mROI;					//感兴趣区域
}EVImage;
class Detector;

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  EV_InitCarDetector
 *  Description:  Load the model file and weights file ,set GPUID
 * =====================================================================================
 */
int EV_InitCarDetector(const char * model_file,const  char * weights_file, const int GPUID ,Detector * &handle);
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  EV_ReleaseCarDetector
 *  Description:  Release required resource
 * =====================================================================================
 */
int EV_ReleaseCarDetector();
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Detector
 *  Description:  Detect Car and return detection result
 * =====================================================================================
 */

int EV_A_GetCarRect(const EVImage * image, int &max_ret_num, EVRect * rect, Detector * &handle);

//background and car
const int class_num=2;

/*
 * ===  Class  ======================================================================
 *         Name:  Detector
 *  Description:  FasterRCNN CXX Detector
 * =====================================================================================
 */
class Detector {
public:
    Detector(const string& model_file, const string& weights_file, const int GPUID);
    void Detect(cv::Mat & cv_img, vector<cv::Rect> & detection_result ,vector<float>& objectScore);
	void bbox_transform_inv(const int num, const float* box_deltas, const float* pred_cls, float* boxes, float* pred, int img_height, int img_width);
	void vis_detections(cv::Mat image, int* keep, int num_out, float* sorted_pred_cls, float CONF_THRESH);
	void boxes_sort(int num, const float* pred, float* sorted_pred);

private:
    std::shared_ptr<Net<float> > net_;
	Detector(){}
};

//Using for box sort
struct Info
{
	float score;
	const float* head;
};
extern bool compare(const Info& Info1, const Info& Info2);


#endif
