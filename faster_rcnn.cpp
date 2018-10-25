#include <stdio.h>  // for snprintf
#include <string>
#include <vector>
#include "qmyglobal.h"
#include <QMutex>
#include <fstream>
#include <boost/python.hpp>
#include "caffe/caffe.hpp"
#include "nms/gpu_nms.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "faster_rcnn.h"

using namespace caffe;
using namespace std;

bool compare(const Info& Info1, const Info& Info2)
{
    return Info1.score > Info2.score;
}
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  EV_InitCarDetector
 *  Description:  Load the model file and weights file ,set GPUID
 * =====================================================================================
 */
int EV_InitCarDetector(const char * model_file, const  char * weights_file, const int GPUID , Detector * &handle){
    handle = new Detector(model_file, weights_file, GPUID);
    return EV_ERR_SUCCESS;
}
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  EV_ReleaseCarDetector
 *  Description:  Release required resource
 * =====================================================================================
 */
int EV_ReleaseCarDetector(){
     return EV_ERR_SUCCESS;
}
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Detector
 *  Description:  Detect Car and return detection result
 * =====================================================================================
 */
int EV_A_GetCarRect(const EVImage * image, int &max_ret_num, EVRect * rect, Detector * &handle){
    vector<cv::Rect>  detection_result;
    vector<float> detection_score;

	IplImage * img = cvCreateImage(cvSize(image->width, image->height), 8, 3);
	memcpy(img->imageData, image->imagedata, img->imageSize);

	cv::Mat inputImg = cv::cvarrToMat(img, true);
    handle->Detect(inputImg, detection_result,detection_score);

	for (int j = 0; j < detection_result.size(); j++)
	{
		if(j >= max_ret_num)
		{
			max_ret_num=j;
			break;
		}
		rect[j].x = detection_result[j].x ;
		rect[j].y = detection_result[j].y ;
		rect[j].w = detection_result[j].width ;
		rect[j].h = detection_result[j].height ;
	}
	cvReleaseImage(&img);
	inputImg.release();
    return EV_ERR_SUCCESS;
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Detector
 *  Description:  Load the model file and weights file
 * =====================================================================================
 */
//load modelfile and weights
Detector::Detector(const string& model_file, const string& weights_file, const int GPUID)
{
    Caffe::SetDevice(GPUID);
    Caffe::set_mode(Caffe::GPU);
    net_ = std::shared_ptr<Net<float> >(new Net<float>(model_file, caffe::TEST));
	net_->CopyTrainedLayersFrom(weights_file);
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Detect
 *  Description:  Perform detection operation
 *                 Warning the max input size should less than 1000*600
 * =====================================================================================
 */
//perform detection operation
//input image max size 1000*600
void Detector::Detect(cv::Mat & cv_img, vector<cv::Rect> & detection_result ,vector<float> & objectScore)
{
    objectScore.clear();
    float CONF_THRESH = 0.600;  //置信度阈值
    float NMS_THRESH = 0.100;   //非极大值抑制阈值
    const int  max_input_side=1000;
    const int  min_input_side=600;

	cv::Mat cv_new(cv_img.rows, cv_img.cols, CV_32FC3, cv::Scalar(0,0,0));
	if(cv_img.empty())
    {
        std::cout<<"Can not get the image"<<endl;
        return;
    }
    int max_side = max(cv_img.rows, cv_img.cols);  //分别求出图片宽和高的较大者
    int min_side = min(cv_img.rows, cv_img.cols);

    float max_side_scale = float(max_side) / float(max_input_side);  //分别求出缩放因子
    float min_side_scale = float(min_side) /float( min_input_side);
    float max_scale=max(max_side_scale, min_side_scale);

    float img_scale = 1;

    if(max_scale > 1)
    {
        img_scale = float(1) / max_scale;    //求出缩放因子
    }

	int height = int(cv_img.rows * img_scale);
	int width = int(cv_img.cols * img_scale);
	int num_out;
	cv::Mat cv_resized;

	float im_info[3];
	float data_buf[height*width*3];
	float *boxes = NULL;
	float *pred = NULL;
	float *pred_per_class = NULL;
	float *sorted_pred_cls = NULL;
	int *keep = NULL;
	const float* bbox_delt;
	const float* rois;
	const float* pred_cls;
	int num;

	for (int h = 0; h < cv_img.rows; ++h )
	{
		for (int w = 0; w < cv_img.cols; ++w)
		{
			cv_new.at<cv::Vec3f>(cv::Point(w, h))[0] = float(cv_img.at<cv::Vec3b>(cv::Point(w, h))[0])-float(102.9801);
			cv_new.at<cv::Vec3f>(cv::Point(w, h))[1] = float(cv_img.at<cv::Vec3b>(cv::Point(w, h))[1])-float(115.9465);
			cv_new.at<cv::Vec3f>(cv::Point(w, h))[2] = float(cv_img.at<cv::Vec3b>(cv::Point(w, h))[2])-float(122.7717);

		}
	}

	cv::resize(cv_new, cv_resized, cv::Size(width, height));
	im_info[0] = cv_resized.rows;
	im_info[1] = cv_resized.cols;
	im_info[2] = img_scale;

	for (int h = 0; h < height; ++h )
	{
		for (int w = 0; w < width; ++w)
		{
			data_buf[(0*height+h)*width+w] = float(cv_resized.at<cv::Vec3f>(cv::Point(w, h))[0]);
			data_buf[(1*height+h)*width+w] = float(cv_resized.at<cv::Vec3f>(cv::Point(w, h))[1]);
			data_buf[(2*height+h)*width+w] = float(cv_resized.at<cv::Vec3f>(cv::Point(w, h))[2]);
		}
	}

	net_->blob_by_name("data")->Reshape(1, 3, height, width);
	//net_->blob_by_name("data")->set_cpu_data(data_buf);
	Blob<float> * input_blobs= net_->input_blobs()[0];
    switch(Caffe::mode()){
    case Caffe::CPU:
        memcpy(input_blobs->mutable_cpu_data(), data_buf, sizeof(float) * input_blobs->count());
        break;
    case Caffe::GPU:
        caffe_gpu_memcpy(sizeof(float)* input_blobs->count(), data_buf, input_blobs->mutable_gpu_data());
        break;
    default:
        LOG(FATAL)<<"Unknow Caffe mode";
    }
	net_->blob_by_name("im_info")->set_cpu_data(im_info);
    caffeDeal_mutex.lock();
    net_->ForwardFrom(0);   //进行网络前向传播
    caffeDeal_mutex.unlock();
    bbox_delt = net_->blob_by_name("bbox_pred")->cpu_data(); //预测的矩形框 维度为 13949*84
    num = net_->blob_by_name("rois")->num();  //产生的 ROI 个数,比如为 13949个ROI

    rois = net_->blob_by_name("rois")->cpu_data(); //维度比如为：13949*5*1*1
    pred_cls = net_->blob_by_name("cls_prob")->cpu_data();

	boxes = new float[num*4];
	pred = new float[num*5*class_num];
	pred_per_class = new float[num*5];
	sorted_pred_cls = new float[num*5];
	keep = new int[num];

	for (int n = 0; n < num; n++)
	{
		for (int c = 0; c < 4; c++)
		{
			boxes[n*4+c] = rois[n*5+c+1] / img_scale;
		}
	}

	bbox_transform_inv(num, bbox_delt, pred_cls, boxes, pred, cv_img.rows, cv_img.cols);
	for (int i = 1; i < class_num; i ++)
	{
		for (int j = 0; j< num; j++)
		{
			for (int k=0; k<5; k++)
				pred_per_class[j*5+k] = pred[(i*num+j)*5+k];
		}
		boxes_sort(num, pred_per_class, sorted_pred_cls);
		_nms(keep, &num_out, sorted_pred_cls, num, 5, NMS_THRESH, 0);
        //for visualize only
		//vis_detections(cv_img, keep, num_out, sorted_pred_cls, CONF_THRESH);
	}

	int k=0;
    while(sorted_pred_cls[keep[k]*5+4]>=CONF_THRESH && k < num_out)
	{
		if(k>=num_out)
			break;
		//detection format x1 y1 width height
        detection_result.push_back(cv::Rect(sorted_pred_cls[keep[k]*5+0],
                                            sorted_pred_cls[keep[k]*5+1],
                                            sorted_pred_cls[keep[k]*5+2]-sorted_pred_cls[keep[k]*5+0],
                                            sorted_pred_cls[keep[k]*5+3]-sorted_pred_cls[keep[k]*5+1]));
        objectScore.push_back(sorted_pred_cls[keep[k]*5+4]);
        k++;
	}

    //cv::imwrite("vis.jpg",cv_img);
	delete []boxes;
	delete []pred;
	delete []pred_per_class;
	delete []keep;
	delete []sorted_pred_cls;

}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  vis_detections
 *  Description:  Visuallize the detection result
 * =====================================================================================
 */
void Detector::vis_detections(cv::Mat image, int* keep, int num_out, float* sorted_pred_cls, float CONF_THRESH)
{
	int i=0;
	while(sorted_pred_cls[keep[i]*5+4]>CONF_THRESH && i < num_out)
	{
		if(i>=num_out)
			return;
		cv::rectangle(image,cv::Point(sorted_pred_cls[keep[i]*5+0], sorted_pred_cls[keep[i]*5+1]),cv::Point(sorted_pred_cls[keep[i]*5+2], sorted_pred_cls[keep[i]*5+3]),cv::Scalar(255,0,0));
		i++;
	}
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  boxes_sort
 *  Description:  Sort the bounding box according score
 * =====================================================================================
 */
void Detector::boxes_sort(const int num, const float* pred, float* sorted_pred)
{
	vector<Info> my;
	Info tmp;
	for (int i = 0; i< num; i++)
	{
		tmp.score = pred[i*5 + 4];
		tmp.head = pred + i*5;
		my.push_back(tmp);
	}
   // std::sort(my.begin(), my.end(), compare);
	for (int i=0; i<num; i++)
	{
		for (int j=0; j<5; j++)
			sorted_pred[i*5+j] = my[i].head[j];
	}
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  bbox_transform_inv
 *  Description:  Compute bounding box regression value
 * =====================================================================================
 */
void Detector::bbox_transform_inv(int num, const float* box_deltas, const float* pred_cls, float* boxes, float* pred, int img_height, int img_width)
{
	float width, height, ctr_x, ctr_y, dx, dy, dw, dh, pred_ctr_x, pred_ctr_y, pred_w, pred_h;
	for(int i=0; i< num; i++)
	{
		width = boxes[i*4+2] - boxes[i*4+0] + 1.0;
		height = boxes[i*4+3] - boxes[i*4+1] + 1.0;
		ctr_x = boxes[i*4+0] + 0.5 * width;
		ctr_y = boxes[i*4+1] + 0.5 * height;
		for (int j=0; j< class_num; j++)
		{

			dx = box_deltas[(i*class_num+j)*4+0];
			dy = box_deltas[(i*class_num+j)*4+1];
			dw = box_deltas[(i*class_num+j)*4+2];
			dh = box_deltas[(i*class_num+j)*4+3];
			pred_ctr_x = ctr_x + width*dx;
			pred_ctr_y = ctr_y + height*dy;
			pred_w = width * exp(dw);
			pred_h = height * exp(dh);
			pred[(j*num+i)*5+0] = max(min(pred_ctr_x - 0.5* pred_w, img_width -1), 0);
			pred[(j*num+i)*5+1] = max(min(pred_ctr_y - 0.5* pred_h, img_height -1), 0);
			pred[(j*num+i)*5+2] = max(min(pred_ctr_x + 0.5* pred_w, img_width -1), 0);
			pred[(j*num+i)*5+3] = max(min(pred_ctr_y + 0.5* pred_h, img_height -1), 0);
			pred[(j*num+i)*5+4] = pred_cls[i*class_num+j];
		}
	}

}
