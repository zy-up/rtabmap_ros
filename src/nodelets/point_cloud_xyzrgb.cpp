/*
Copyright (c) 2010-2016, Mathieu Labbe - IntRoLab - Universite de Sherbrooke
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Universite de Sherbrooke nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <nodelet/nodelet.h>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>

#include <rtabmap_ros/MsgConversion.h>

#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/CameraInfo.h>

#include <stereo_msgs/DisparityImage.h>

#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>

#include <image_geometry/pinhole_camera_model.h>
#include <image_geometry/stereo_camera_model.h>

#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/sync_policies/exact_time.h>
#include <message_filters/subscriber.h>

#include <cv_bridge/cv_bridge.h>
#include <opencv2/highgui/highgui.hpp>

#include "rtabmap/core/util2d.h"
#include "rtabmap/core/util3d.h"
#include "rtabmap/core/util3d_filtering.h"
#include "rtabmap/core/util3d_surface.h"
#include "rtabmap/utilite/UConversion.h"
#include "rtabmap/utilite/UStl.h"

namespace rtabmap_ros
{

// 从RGBD中返回RGB点云  创建类，并继承Nodelet类
class PointCloudXYZRGB : public nodelet::Nodelet
{
public:
	// 构造函数，参数初始化
	PointCloudXYZRGB() :
		maxDepth_(0.0),
		minDepth_(0.0),
		voxelSize_(0.0),
		decimation_(1),
		noiseFilterRadius_(0.0),
		noiseFilterMinNeighbors_(5),
		normalK_(0),
		normalRadius_(0.0),
		filterNaNs_(false),
		approxSyncDepth_(0),
		approxSyncDisparity_(0),
		approxSyncStereo_(0),
		exactSyncDepth_(0),
		exactSyncDisparity_(0),
		exactSyncStereo_(0)
	{}

	// 析构函数
	virtual ~PointCloudXYZRGB()
	{
		if(approxSyncDepth_)
			delete approxSyncDepth_;
		if(approxSyncDisparity_)
			delete approxSyncDisparity_;
		if(approxSyncStereo_)
			delete approxSyncStereo_;
		if(exactSyncDepth_)
			delete exactSyncDepth_;
		if(exactSyncDisparity_)
			delete exactSyncDisparity_;
		if(exactSyncStereo_)
			delete exactSyncStereo_;
	}

private:
	// 加载nodelet的初始函数
	virtual void onInit()
	{
		// 获取共有、私有句柄
		ros::NodeHandle & nh = getNodeHandle();
		ros::NodeHandle & pnh = getPrivateNodeHandle();

		// 队列大小
		int queueSize = 10;
		// 消息同步
		bool approxSync = true;
		// 设置兴趣区域
		std::string roiStr;
		// 消息同步时的最大时间间隔
		double approxSyncMaxInterval = 0.0;
		pnh.param("approx_sync", approxSync, approxSync);
		pnh.param("approx_sync_max_interval", approxSyncMaxInterval, approxSyncMaxInterval);
		pnh.param("queue_size", queueSize, queueSize);
		pnh.param("max_depth", maxDepth_, maxDepth_);
		pnh.param("min_depth", minDepth_, minDepth_);
		pnh.param("voxel_size", voxelSize_, voxelSize_);
		pnh.param("decimation", decimation_, decimation_);
		pnh.param("noise_filter_radius", noiseFilterRadius_, noiseFilterRadius_);
		pnh.param("noise_filter_min_neighbors", noiseFilterMinNeighbors_, noiseFilterMinNeighbors_);
		pnh.param("normal_k", normalK_, normalK_);
		pnh.param("normal_radius", normalRadius_, normalRadius_);
		pnh.param("filter_nans", filterNaNs_, filterNaNs_);
		pnh.param("roi_ratios", roiStr, roiStr);

		//parse roi (region of interest) 
		roiRatios_.resize(4, 0);
		if(!roiStr.empty())
		{
			std::list<std::string> strValues = uSplit(roiStr, ' ');
			if(strValues.size() != 4)
			{
				ROS_ERROR("The number of values must be 4 (\"roi_ratios\"=\"%s\")", roiStr.c_str());
			}
			else
			{
				std::vector<float> tmpValues(4);
				unsigned int i=0;
				for(std::list<std::string>::iterator jter = strValues.begin(); jter!=strValues.end(); ++jter)
				{
					tmpValues[i] = uStr2Float(*jter);
					++i;
				}

				if(tmpValues[0] >= 0 && tmpValues[0] < 1 && tmpValues[0] < 1.0f-tmpValues[1] &&
					tmpValues[1] >= 0 && tmpValues[1] < 1 && tmpValues[1] < 1.0f-tmpValues[0] &&
					tmpValues[2] >= 0 && tmpValues[2] < 1 && tmpValues[2] < 1.0f-tmpValues[3] &&
					tmpValues[3] >= 0 && tmpValues[3] < 1 && tmpValues[3] < 1.0f-tmpValues[2])
				{
					roiRatios_ = tmpValues;
				}
				else
				{
					ROS_ERROR("The roi ratios are not valid (\"roi_ratios\"=\"%s\")", roiStr.c_str());
				}
			}
		}

		// StereoBM parameters 双目相机参数读取
		stereoBMParameters_ = rtabmap::Parameters::getDefaultParameters("StereoBM");
		for(rtabmap::ParametersMap::iterator iter=stereoBMParameters_.begin(); iter!=stereoBMParameters_.end(); ++iter)
		{
			std::string vStr;
			bool vBool;
			int vInt;
			double vDouble;
			if(pnh.getParam(iter->first, vStr))
			{
				NODELET_INFO("point_cloud_xyzrgb: Setting parameter \"%s\"=\"%s\"", iter->first.c_str(), vStr.c_str());
				iter->second = vStr;
			}
			else if(pnh.getParam(iter->first, vBool))
			{
				NODELET_INFO("point_cloud_xyzrgb: Setting parameter \"%s\"=\"%s\"", iter->first.c_str(), uBool2Str(vBool).c_str());
				iter->second = uBool2Str(vBool);
			}
			else if(pnh.getParam(iter->first, vDouble))
			{
				NODELET_INFO("point_cloud_xyzrgb: Setting parameter \"%s\"=\"%s\"", iter->first.c_str(), uNumber2Str(vDouble).c_str());
				iter->second = uNumber2Str(vDouble);
			}
			else if(pnh.getParam(iter->first, vInt))
			{
				NODELET_INFO("point_cloud_xyzrgb: Setting parameter \"%s\"=\"%s\"", iter->first.c_str(), uNumber2Str(vInt).c_str());
				iter->second = uNumber2Str(vInt);
			}
		}

		NODELET_INFO("Approximate time sync = %s", approxSync?"true":"false");

		// 设置点云的发布参数，消息名为cloud，不排队
		cloudPub_ = nh.advertise<sensor_msgs::PointCloud2>("cloud", 1);
		// 如果订阅RGBD图像，因为时间戳已经对齐了，所以不用管同步的事
		rgbdImageSub_ = nh.subscribe("rgbd_image", 1, &PointCloudXYZRGB::rgbdImageCallback, this);

		// 如果开启同步检测，则需要对时间戳进行检查时间同步性
		if(approxSync)
		{
			approxSyncDepth_ = new message_filters::Synchronizer<MyApproxSyncDepthPolicy>(MyApproxSyncDepthPolicy(queueSize), imageSub_, imageDepthSub_, cameraInfoSub_);
			if(approxSyncMaxInterval > 0.0)
				approxSyncDepth_->setMaxIntervalDuration(ros::Duration(approxSyncMaxInterval));
			approxSyncDepth_->registerCallback(boost::bind(&PointCloudXYZRGB::depthCallback, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

			approxSyncDisparity_ = new message_filters::Synchronizer<MyApproxSyncDisparityPolicy>(MyApproxSyncDisparityPolicy(queueSize), imageLeft_, imageDisparitySub_, cameraInfoLeft_);
			if(approxSyncMaxInterval > 0.0)
				approxSyncDisparity_->setMaxIntervalDuration(ros::Duration(approxSyncMaxInterval));
			approxSyncDisparity_->registerCallback(boost::bind(&PointCloudXYZRGB::disparityCallback, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

			approxSyncStereo_ = new message_filters::Synchronizer<MyApproxSyncStereoPolicy>(MyApproxSyncStereoPolicy(queueSize), imageLeft_, imageRight_, cameraInfoLeft_, cameraInfoRight_);
			if(approxSyncMaxInterval > 0.0)
				approxSyncStereo_->setMaxIntervalDuration(ros::Duration(approxSyncMaxInterval));
			approxSyncStereo_->registerCallback(boost::bind(&PointCloudXYZRGB::stereoCallback, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4));
		}
		else
		{
			exactSyncDepth_ = new message_filters::Synchronizer<MyExactSyncDepthPolicy>(MyExactSyncDepthPolicy(queueSize), imageSub_, imageDepthSub_, cameraInfoSub_);
			exactSyncDepth_->registerCallback(boost::bind(&PointCloudXYZRGB::depthCallback, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

			exactSyncDisparity_ = new message_filters::Synchronizer<MyExactSyncDisparityPolicy>(MyExactSyncDisparityPolicy(queueSize), imageLeft_, imageDisparitySub_, cameraInfoLeft_);
			exactSyncDisparity_->registerCallback(boost::bind(&PointCloudXYZRGB::disparityCallback, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

			exactSyncStereo_ = new message_filters::Synchronizer<MyExactSyncStereoPolicy>(MyExactSyncStereoPolicy(queueSize), imageLeft_, imageRight_, cameraInfoLeft_, cameraInfoRight_);
			exactSyncStereo_->registerCallback(boost::bind(&PointCloudXYZRGB::stereoCallback, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4));
		}

		// RGBD相机部分
		ros::NodeHandle rgb_nh(nh, "rgb");
		ros::NodeHandle depth_nh(nh, "depth");
		ros::NodeHandle rgb_pnh(pnh, "rgb");
		ros::NodeHandle depth_pnh(pnh, "depth");
		image_transport::ImageTransport rgb_it(rgb_nh);
		image_transport::ImageTransport depth_it(depth_nh);
		image_transport::TransportHints hintsRgb("raw", ros::TransportHints(), rgb_pnh);
		image_transport::TransportHints hintsDepth("raw", ros::TransportHints(), depth_pnh);

		// 图片订阅、深度图、相机内参
		imageSub_.subscribe(rgb_it, rgb_nh.resolveName("image"), 1, hintsRgb);
		imageDepthSub_.subscribe(depth_it, depth_nh.resolveName("image"), 1, hintsDepth);
		cameraInfoSub_.subscribe(rgb_nh, "camera_info", 1);

		// 双目相机部分
		ros::NodeHandle left_nh(nh, "left");
		ros::NodeHandle right_nh(nh, "right");
		ros::NodeHandle left_pnh(pnh, "left");
		ros::NodeHandle right_pnh(pnh, "right");
		image_transport::ImageTransport left_it(left_nh);
		image_transport::ImageTransport right_it(right_nh);
		image_transport::TransportHints hintsLeft("raw", ros::TransportHints(), left_pnh);
		image_transport::TransportHints hintsRight("raw", ros::TransportHints(), right_pnh);

		// 视差图、左右目相机、以及双目相机信息
		imageDisparitySub_.subscribe(nh, "disparity", 1);
		imageLeft_.subscribe(left_it, left_nh.resolveName("image"), 1, hintsLeft);
		imageRight_.subscribe(right_it, right_nh.resolveName("image"), 1, hintsRight);
		cameraInfoLeft_.subscribe(left_nh, "camera_info", 1);
		cameraInfoRight_.subscribe(right_nh, "camera_info", 1);
	}

	// 深度图订阅反馈函数
	void depthCallback(
			  const sensor_msgs::ImageConstPtr& image,
			  const sensor_msgs::ImageConstPtr& imageDepth,
			  const sensor_msgs::CameraInfoConstPtr& cameraInfo)
	{
		if(!(image->encoding.compare(sensor_msgs::image_encodings::TYPE_8UC1) ==0 ||
			image->encoding.compare(sensor_msgs::image_encodings::MONO8) ==0 ||
			image->encoding.compare(sensor_msgs::image_encodings::MONO16) == 0 ||
			image->encoding.compare(sensor_msgs::image_encodings::BGR8) == 0 ||
			image->encoding.compare(sensor_msgs::image_encodings::RGB8) == 0 ||
			image->encoding.compare(sensor_msgs::image_encodings::BGRA8) == 0 ||
			image->encoding.compare(sensor_msgs::image_encodings::RGBA8) == 0 ||
			image->encoding.compare(sensor_msgs::image_encodings::BAYER_GRBG8) == 0) ||
		   !(imageDepth->encoding.compare(sensor_msgs::image_encodings::TYPE_16UC1)==0 ||
			 imageDepth->encoding.compare(sensor_msgs::image_encodings::TYPE_32FC1)==0 ||
			 imageDepth->encoding.compare(sensor_msgs::image_encodings::MONO16)==0))
		{
			NODELET_ERROR("Input type must be image=mono8,mono16,rgb8,bgr8 and image_depth=32FC1,16UC1,mono16");
			return;
		}

		if(cloudPub_.getNumSubscribers())
		{
			ros::WallTime time = ros::WallTime::now();

			cv_bridge::CvImageConstPtr imagePtr;
			if(image->encoding.compare(sensor_msgs::image_encodings::TYPE_8UC1)==0)
			{
				imagePtr = cv_bridge::toCvShare(image);
			}
			else if(image->encoding.compare(sensor_msgs::image_encodings::MONO8) == 0 ||
					image->encoding.compare(sensor_msgs::image_encodings::MONO16) == 0)
			{
				imagePtr = cv_bridge::toCvShare(image, "mono8");
			}
			else
			{
				imagePtr = cv_bridge::toCvShare(image, "bgr8");
			}

			cv_bridge::CvImageConstPtr imageDepthPtr = cv_bridge::toCvShare(imageDepth);

			rtabmap::CameraModel model = cameraModelFromROS(*cameraInfo);

			pcl::PointCloud<pcl::PointXYZRGB>::Ptr pclCloud;

			cv::Mat rgb = imagePtr->image;
			cv::Mat depth = imageDepthPtr->image;
			if( roiRatios_.size() == 4 &&
				((roiRatios_[0] > 0.0f && roiRatios_[0] <= 1.0f) ||
				 (roiRatios_[1] > 0.0f && roiRatios_[1] <= 1.0f) ||
				 (roiRatios_[2] > 0.0f && roiRatios_[2] <= 1.0f) ||
				 (roiRatios_[3] > 0.0f && roiRatios_[3] <= 1.0f)))
			{
				cv::Rect roiDepth = rtabmap::util2d::computeRoi(depth, roiRatios_);
				cv::Rect roiRgb = rtabmap::util2d::computeRoi(rgb, roiRatios_);
				if(	roiDepth.width%decimation_==0 &&
					roiDepth.height%decimation_==0 &&
					roiRgb.width%decimation_==0 &&
					roiRgb.height%decimation_==0)
				{
					depth = cv::Mat(depth, roiDepth);
					rgb = cv::Mat(rgb, roiRgb);
					model = model.roi(roiRgb);
				}
				else
				{
					NODELET_ERROR("Cannot apply ROI ratios [%f,%f,%f,%f] because resulting "
						  "dimension (depth=%dx%d rgb=%dx%d) cannot be divided exactly "
						  "by decimation parameter (%d). Ignoring ROI ratios...",
						  roiRatios_[0],
						  roiRatios_[1],
						  roiRatios_[2],
						  roiRatios_[3],
						  roiDepth.width,
						  roiDepth.height,
						  roiRgb.width,
						  roiRgb.height,
						  decimation_);
				}
			}

			pcl::IndicesPtr indices(new std::vector<int>);
			pclCloud = rtabmap::util3d::cloudFromDepthRGB(
					rgb,
					depth,
					model,
					decimation_,
					maxDepth_,
					minDepth_,
					indices.get());


			processAndPublish(pclCloud, indices, imagePtr->header);

			NODELET_DEBUG("point_cloud_xyzrgb from RGB-D time = %f s", (ros::WallTime::now() - time).toSec());
		}
	}

	// 视差图订阅反馈函数
	void disparityCallback(
			const sensor_msgs::ImageConstPtr& image,
			const stereo_msgs::DisparityImageConstPtr& imageDisparity,
			const sensor_msgs::CameraInfoConstPtr& cameraInfo)
	{
		cv_bridge::CvImageConstPtr imagePtr;
		if(image->encoding.compare(sensor_msgs::image_encodings::TYPE_8UC1)==0)
		{
			imagePtr = cv_bridge::toCvShare(image);
		}
		else if(image->encoding.compare(sensor_msgs::image_encodings::MONO8) == 0 ||
				image->encoding.compare(sensor_msgs::image_encodings::MONO16) == 0)
		{
			imagePtr = cv_bridge::toCvShare(image, "mono8");
		}
		else
		{
			imagePtr = cv_bridge::toCvShare(image, "bgr8");
		}

		if(imageDisparity->image.encoding.compare(sensor_msgs::image_encodings::TYPE_32FC1) !=0 &&
		   imageDisparity->image.encoding.compare(sensor_msgs::image_encodings::TYPE_16SC1) !=0)
		{
			NODELET_ERROR("Input type must be disparity=32FC1 or 16SC1");
			return;
		}

		cv::Mat disparity;
		if(imageDisparity->image.encoding.compare(sensor_msgs::image_encodings::TYPE_32FC1) == 0)
		{
			disparity = cv::Mat(imageDisparity->image.height, imageDisparity->image.width, CV_32FC1, const_cast<uchar*>(imageDisparity->image.data.data()));
		}
		else
		{
			disparity = cv::Mat(imageDisparity->image.height, imageDisparity->image.width, CV_16SC1, const_cast<uchar*>(imageDisparity->image.data.data()));
		}

		if(cloudPub_.getNumSubscribers())
		{
			ros::WallTime time = ros::WallTime::now();

			cv::Rect roi = rtabmap::util2d::computeRoi(disparity, roiRatios_);

			pcl::PointCloud<pcl::PointXYZRGB>::Ptr pclCloud;
			rtabmap::CameraModel leftModel = rtabmap_ros::cameraModelFromROS(*cameraInfo);
			UASSERT(disparity.cols == leftModel.imageWidth() && disparity.rows == leftModel.imageHeight());
			UASSERT(imagePtr->image.cols == leftModel.imageWidth() && imagePtr->image.rows == leftModel.imageHeight());
			rtabmap::StereoCameraModel stereoModel(imageDisparity->f, imageDisparity->f, leftModel.cx()-roiRatios_[0]*double(disparity.cols), leftModel.cy()-roiRatios_[2]*double(disparity.rows), imageDisparity->T);
			pcl::IndicesPtr indices(new std::vector<int>);
			pclCloud = rtabmap::util3d::cloudFromDisparityRGB(
					cv::Mat(imagePtr->image, roi),
					cv::Mat(disparity, roi),
					stereoModel,
					decimation_,
					maxDepth_,
					minDepth_,
					indices.get());

			processAndPublish(pclCloud, indices, imageDisparity->header);

			NODELET_DEBUG("point_cloud_xyzrgb from disparity time = %f s", (ros::WallTime::now() - time).toSec());
		}
	}

	// 双目相机订阅反馈函数
	void stereoCallback(const sensor_msgs::ImageConstPtr& imageLeft,
			const sensor_msgs::ImageConstPtr& imageRight,
			const sensor_msgs::CameraInfoConstPtr& camInfoLeft,
			const sensor_msgs::CameraInfoConstPtr& camInfoRight)
	{
		if(!(imageLeft->encoding.compare(sensor_msgs::image_encodings::MONO8) == 0 ||
				imageLeft->encoding.compare(sensor_msgs::image_encodings::MONO16) == 0 ||
				imageLeft->encoding.compare(sensor_msgs::image_encodings::BGR8) == 0 ||
				imageLeft->encoding.compare(sensor_msgs::image_encodings::RGB8) == 0 ||
				imageLeft->encoding.compare(sensor_msgs::image_encodings::BGRA8) == 0 ||
				imageLeft->encoding.compare(sensor_msgs::image_encodings::RGBA8) == 0) ||
			!(imageRight->encoding.compare(sensor_msgs::image_encodings::MONO8) == 0 ||
				imageRight->encoding.compare(sensor_msgs::image_encodings::MONO16) == 0 ||
				imageRight->encoding.compare(sensor_msgs::image_encodings::BGR8) == 0 ||
				imageRight->encoding.compare(sensor_msgs::image_encodings::RGB8) == 0 ||
				imageRight->encoding.compare(sensor_msgs::image_encodings::BGRA8) == 0 ||
				imageRight->encoding.compare(sensor_msgs::image_encodings::RGBA8) == 0))
		{
			NODELET_ERROR("Input type must be image=mono8,mono16,rgb8,bgr8,rgba8,bgra8 (enc=%s)", imageLeft->encoding.c_str());
			return;
		}

		if(cloudPub_.getNumSubscribers())
		{
			ros::WallTime time = ros::WallTime::now();

			cv_bridge::CvImageConstPtr ptrLeftImage, ptrRightImage;
			if(imageLeft->encoding.compare(sensor_msgs::image_encodings::MONO8) == 0 ||
				imageLeft->encoding.compare(sensor_msgs::image_encodings::MONO16) == 0)
			{
				ptrLeftImage = cv_bridge::toCvShare(imageLeft, "mono8");
			}
			else
			{
				ptrLeftImage = cv_bridge::toCvShare(imageLeft, "bgr8");
			}
			ptrRightImage = cv_bridge::toCvShare(imageRight, "mono8");

			if(roiRatios_[0]!=0.0f || roiRatios_[1]!=0.0f || roiRatios_[2]!=0.0f || roiRatios_[3]!=0.0f)
			{
				ROS_WARN("\"roi_ratios\" set but ignored for stereo images.");
			}

			pcl::PointCloud<pcl::PointXYZRGB>::Ptr pclCloud;
			pcl::IndicesPtr indices(new std::vector<int>);
			pclCloud = rtabmap::util3d::cloudFromStereoImages(
					ptrLeftImage->image,
					ptrRightImage->image,
					rtabmap_ros::stereoCameraModelFromROS(*camInfoLeft, *camInfoRight),
					decimation_,
					maxDepth_,
					minDepth_,
					indices.get(),
					stereoBMParameters_);

			processAndPublish(pclCloud, indices, imageLeft->header);

			NODELET_DEBUG("point_cloud_xyzrgb from stereo time = %f s", (ros::WallTime::now() - time).toSec());
		}
	}

	// RGBD相机订阅反馈函数
	void rgbdImageCallback(const rtabmap_ros::RGBDImageConstPtr & image)
	{	
		// 如果没有订阅者，就不会产生点云
		if(cloudPub_.getNumSubscribers())
		{
			// 获取当前墙上时间（即当前时间）
			ros::WallTime time = ros::WallTime::now();
			// 将ROS中的RGBD图像，转化为rtabmap所需格式
			rtabmap::SensorData data = rtabmap_ros::rgbdImageFromROS(image);

			pcl::PointCloud<pcl::PointXYZRGB>::Ptr pclCloud;
			pcl::IndicesPtr indices(new std::vector<int>);
			if(data.isValid())
			{
				pclCloud = rtabmap::util3d::cloudRGBFromSensorData(
						data,
						decimation_,
						maxDepth_,
						minDepth_,
						indices.get(),
						stereoBMParameters_,
						roiRatios_);

				processAndPublish(pclCloud, indices, image->header);
			}

			NODELET_DEBUG("point_cloud_xyzrgb from rgbd_image time = %f s", (ros::WallTime::now() - time).toSec());
		}
	}

	// 在以上的反馈函数收到图像后，调用当前函数，利用PCL生成RGB点云（包含法向量）并发布
	void processAndPublish(pcl::PointCloud<pcl::PointXYZRGB>::Ptr & pclCloud, pcl::IndicesPtr & indices, const std_msgs::Header & header)
	{
		if(indices->size() && voxelSize_ > 0.0)
		{
			pclCloud = rtabmap::util3d::voxelize(pclCloud, indices, voxelSize_);
			pclCloud->is_dense = true;
		}

		// Do radius filtering after voxel filtering ( a lot faster)
		if(!pclCloud->empty() && (pclCloud->is_dense || !indices->empty()) && noiseFilterRadius_ > 0.0 && noiseFilterMinNeighbors_ > 0)
		{
			if(pclCloud->is_dense)
			{
				indices = rtabmap::util3d::radiusFiltering(pclCloud, noiseFilterRadius_, noiseFilterMinNeighbors_);
			}
			else
			{
				indices = rtabmap::util3d::radiusFiltering(pclCloud, indices, noiseFilterRadius_, noiseFilterMinNeighbors_);
			}
			pcl::PointCloud<pcl::PointXYZRGB>::Ptr tmp(new pcl::PointCloud<pcl::PointXYZRGB>);
			pcl::copyPointCloud(*pclCloud, *indices, *tmp);
			pclCloud = tmp;
		}

		sensor_msgs::PointCloud2 rosCloud;
		if(!pclCloud->empty() && (pclCloud->is_dense || !indices->empty()) && (normalK_ > 0 || normalRadius_ > 0.0f))
		{
			//compute normals
			pcl::PointCloud<pcl::Normal>::Ptr normals = rtabmap::util3d::computeNormals(pclCloud, normalK_, normalRadius_);
			pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr pclCloudNormal(new pcl::PointCloud<pcl::PointXYZRGBNormal>);
			pcl::concatenateFields(*pclCloud, *normals, *pclCloudNormal);
			if(filterNaNs_)
			{
				pclCloudNormal = rtabmap::util3d::removeNaNNormalsFromPointCloud(pclCloudNormal);
			}
			pcl::toROSMsg(*pclCloudNormal, rosCloud);
		}
		else
		{
			if(filterNaNs_ && !pclCloud->is_dense)
			{
				pclCloud = rtabmap::util3d::removeNaNFromPointCloud(pclCloud);
			}
			pcl::toROSMsg(*pclCloud, rosCloud);
		}
		rosCloud.header.stamp = header.stamp;
		rosCloud.header.frame_id = header.frame_id;

		//publish the message
		cloudPub_.publish(rosCloud);
	}

private:

	double maxDepth_;
	double minDepth_;
	double voxelSize_;
	int decimation_;
	double noiseFilterRadius_;
	int noiseFilterMinNeighbors_;
	int normalK_;
	double normalRadius_;
	bool filterNaNs_;
	std::vector<float> roiRatios_;
	rtabmap::ParametersMap stereoBMParameters_;

	ros::Publisher cloudPub_;

	ros::Subscriber rgbdImageSub_;

	image_transport::SubscriberFilter imageSub_;
	image_transport::SubscriberFilter imageDepthSub_;
	message_filters::Subscriber<sensor_msgs::CameraInfo> cameraInfoSub_;

	message_filters::Subscriber<stereo_msgs::DisparityImage> imageDisparitySub_;

	image_transport::SubscriberFilter imageLeft_;
	image_transport::SubscriberFilter imageRight_;
	message_filters::Subscriber<sensor_msgs::CameraInfo> cameraInfoLeft_;
	message_filters::Subscriber<sensor_msgs::CameraInfo> cameraInfoRight_;

	typedef message_filters::sync_policies::ApproximateTime<sensor_msgs::Image, sensor_msgs::Image, sensor_msgs::CameraInfo> MyApproxSyncDepthPolicy;
	message_filters::Synchronizer<MyApproxSyncDepthPolicy> * approxSyncDepth_;

	typedef message_filters::sync_policies::ApproximateTime<sensor_msgs::Image, stereo_msgs::DisparityImage, sensor_msgs::CameraInfo> MyApproxSyncDisparityPolicy;
	message_filters::Synchronizer<MyApproxSyncDisparityPolicy> * approxSyncDisparity_;

	typedef message_filters::sync_policies::ApproximateTime<sensor_msgs::Image, sensor_msgs::Image, sensor_msgs::CameraInfo, sensor_msgs::CameraInfo> MyApproxSyncStereoPolicy;
	message_filters::Synchronizer<MyApproxSyncStereoPolicy> * approxSyncStereo_;

	typedef message_filters::sync_policies::ExactTime<sensor_msgs::Image, sensor_msgs::Image, sensor_msgs::CameraInfo> MyExactSyncDepthPolicy;
	message_filters::Synchronizer<MyExactSyncDepthPolicy> * exactSyncDepth_;

	typedef message_filters::sync_policies::ExactTime<sensor_msgs::Image, stereo_msgs::DisparityImage, sensor_msgs::CameraInfo> MyExactSyncDisparityPolicy;
	message_filters::Synchronizer<MyExactSyncDisparityPolicy> * exactSyncDisparity_;

	typedef message_filters::sync_policies::ExactTime<sensor_msgs::Image, sensor_msgs::Image, sensor_msgs::CameraInfo, sensor_msgs::CameraInfo> MyExactSyncStereoPolicy;
	message_filters::Synchronizer<MyExactSyncStereoPolicy> * exactSyncStereo_;
};
// 提示编译器 PointCloudXYZRGB 是一个插件
PLUGINLIB_EXPORT_CLASS(rtabmap_ros::PointCloudXYZRGB, nodelet::Nodelet);
}

