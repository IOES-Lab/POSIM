/*
 * Copyright (C) 2024 Rakesh Vivekanandan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "posim_gz_sensor_plugins/UnderwaterCamera.hh"
#include <cmath>
#include <cstdlib>
#include <gz/common/Console.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Util.hh>
#include <gz/sim/components/Camera.hh>
#include <gz/sim/components/ParentEntity.hh>
#include <gz/sim/components/RgbdCamera.hh>
#include <gz/sim/components/Sensor.hh>
#include <gz/transport/Node.hh>
#include <limits>
#include <new>
#include <rclcpp/expand_topic_or_service_name.hpp>
#include <sdf/Camera.hh>
#include "gz/plugin/Register.hh"
#include "gz/sim/EntityComponentManager.hh"
#include "gz/sim/components/Link.hh"
#include "gz/sim/components/Model.hh"
#include "gz/sim/components/Name.hh"
#include "gz/sim/components/World.hh"

GZ_ADD_PLUGIN(
  posim_gz_sensor_plugins::UnderwaterCamera, gz::sim::System,
  posim_gz_sensor_plugins::UnderwaterCamera::ISystemConfigure,
  posim_gz_sensor_plugins::UnderwaterCamera::ISystemPostUpdate)

namespace posim_gz_sensor_plugins
{

struct UnderwaterCamera::PrivateData
{
  // Add any private data members here.
  gz::sim::Model model;
  std::string linkName;

  std::mutex mutex_;
  gz::transport::Node gz_node;
  std::string topic;
  std::string image_topic;
  std::string depth_image_topic;
  std::string simulated_image_topic;
  std::string camera_info_topic;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub;

  /// \brief Width of the image.
  unsigned int width;

  /// \brief Height of the image.
  unsigned int height;

  /// \brief Camera intrinsics.
  double fx;
  double fy;
  double cx;
  double cy;
  double skew;

  /// \brief Temporarily store pointer to previous depth image.
  gz::msgs::Image lastDepth;

  /// \brief Latest simulated image.
  gz::msgs::Image lastImage;

  /// \brief Depth to range lookup table (LUT)
  std::unique_ptr<float[]> depth2rangeLUT;

  /// \brief Attenuation constants per channel (RGB)
  float attenuation[3];

  /// \brief Background constants per channel (RGB)
  unsigned char background[3];

  bool firstImage = true;

  float min_range;
  float max_range;
};

struct UnderwaterCamera::CallbackState
{
  std::mutex mutex;
  UnderwaterCamera * owner = nullptr;
};

UnderwaterCamera::UnderwaterCamera()
: dataPtr(std::make_unique<PrivateData>()), callbackState(std::make_shared<CallbackState>())
{
  this->callbackState->owner = this;
}

UnderwaterCamera::~UnderwaterCamera()
{
  // Drain an active callback before freeing the LUT, and make already-queued
  // callbacks harmless. Do not hold this lock while calling transport APIs.
  {
    std::lock_guard<std::mutex> lock(this->callbackState->mutex);
    this->callbackState->owner = nullptr;
  }
  if (!this->dataPtr->image_topic.empty())
  {
    this->dataPtr->gz_node.Unsubscribe(this->dataPtr->image_topic);
  }
  if (!this->dataPtr->depth_image_topic.empty())
  {
    this->dataPtr->gz_node.Unsubscribe(this->dataPtr->depth_image_topic);
  }
}

void UnderwaterCamera::Configure(
  const gz::sim::Entity & _entity, const std::shared_ptr<const sdf::Element> & _sdf,
  gz::sim::EntityComponentManager & _ecm, gz::sim::EventManager & _eventManager)
{
  gzdbg << "posim_gz_sensor_plugins::UnderwaterCamera::Configure on entity: " << _entity
        << std::endl;

  auto rgbdCamera = _ecm.Component<gz::sim::components::RgbdCamera>(_entity);
  if (!_sdf || !rgbdCamera)
  {
    gzerr << "UnderwaterCamera requires plugin settings on an rgbd_camera sensor. "
          << "Failed to initialize." << std::endl;
    return;
  }

  auto sensorSdf = rgbdCamera->Data();
  this->dataPtr->topic = sensorSdf.Topic();

  if (sensorSdf.CameraSensor() == nullptr)
  {
    gzerr << "Attempting to a load an RGBD Camera sensor, but received "
          << "a null sensor." << std::endl;
    return;
  }

  if (this->dataPtr->topic.empty())
  {
    // Gazebo's RGBD topic already includes the world and sensor scope.
    this->dataPtr->topic = gz::sim::topicFromScopedName(_entity, _ecm, false);
  }
  if (this->dataPtr->topic.empty())
  {
    gzerr << "Unable to determine the RGBD camera topic." << std::endl;
    return;
  }

  // SDF names may contain characters that ROS node names do not accept.
  const std::string rosNodeName = "underwater_camera_" + std::to_string(_entity) + "_node";
  this->dataPtr->image_topic = this->dataPtr->topic + "/image";
  this->dataPtr->depth_image_topic = this->dataPtr->topic + "/depth_image";
  try
  {
    this->dataPtr->simulated_image_topic = rclcpp::expand_topic_or_service_name(
      this->dataPtr->topic + "/simulated_image", rosNodeName, "/");
  }
  catch (const rclcpp::exceptions::InvalidTopicNameError & error)
  {
    gzerr << "Invalid underwater camera output topic: " << error.what() << std::endl;
    return;
  }

  sdf::Camera * cameraSdf = sensorSdf.CameraSensor();

  // get camera intrinsics
  this->dataPtr->width = cameraSdf->ImageWidth();
  this->dataPtr->height = cameraSdf->ImageHeight();

  const uint64_t pixels = static_cast<uint64_t>(this->dataPtr->width) * this->dataPtr->height;
  if (
    pixels == 0 || this->dataPtr->width > std::numeric_limits<int>::max() ||
    this->dataPtr->height > std::numeric_limits<int>::max() ||
    pixels > std::numeric_limits<uint32_t>::max() / 3 ||
    pixels > std::numeric_limits<size_t>::max() / sizeof(float))
  {
    gzerr << "Camera dimensions must fit OpenCV and a serialized ROS BGR image." << std::endl;
    return;
  }

  const double nearClip = cameraSdf->NearClip();
  const double farClip = cameraSdf->FarClip();
  if (
    !std::isfinite(nearClip) || !std::isfinite(farClip) || nearClip <= 0 || farClip <= nearClip ||
    farClip > std::numeric_limits<float>::max() || static_cast<float>(nearClip) <= 0 ||
    static_cast<float>(farClip) <= static_cast<float>(nearClip))
  {
    gzerr << "Camera clipping distances must be finite, with 0 < near < far." << std::endl;
    return;
  }
  this->dataPtr->min_range = static_cast<float>(nearClip);
  this->dataPtr->max_range = static_cast<float>(farClip);

  this->dataPtr->skew = 0;
  // Match Gazebo's projection precedence. Unspecified lens values in the SDF
  // DOM are fixed defaults, not the intrinsics of a resized or wider camera.
  if (cameraSdf->HasLensProjection())
  {
    this->dataPtr->fx = cameraSdf->LensProjectionFx();
    this->dataPtr->fy = cameraSdf->LensProjectionFy();
    this->dataPtr->cx = cameraSdf->LensProjectionCx();
    this->dataPtr->cy = cameraSdf->LensProjectionCy();
  }
  else if (cameraSdf->HasLensIntrinsics())
  {
    this->dataPtr->fx = cameraSdf->LensIntrinsicsFx();
    this->dataPtr->fy = cameraSdf->LensIntrinsicsFy();
    this->dataPtr->cx = cameraSdf->LensIntrinsicsCx();
    this->dataPtr->cy = cameraSdf->LensIntrinsicsCy();
    this->dataPtr->skew = cameraSdf->LensIntrinsicsSkew();
  }
  else
  {
    const double hfov = cameraSdf->HorizontalFov().Radian();
    if (!std::isfinite(hfov) || hfov <= 0 || hfov >= GZ_PI)
    {
      gzerr << "Camera horizontal field of view must be between 0 and pi radians." << std::endl;
      return;
    }
    this->dataPtr->fx = this->dataPtr->width / (2.0 * std::tan(hfov / 2.0));
    this->dataPtr->fy = this->dataPtr->fx;
    this->dataPtr->cx = this->dataPtr->width / 2.0;
    this->dataPtr->cy = this->dataPtr->height / 2.0;
  }
  if (
    !std::isfinite(this->dataPtr->fx) || !std::isfinite(this->dataPtr->fy) ||
    !std::isfinite(this->dataPtr->cx) || !std::isfinite(this->dataPtr->cy) ||
    !std::isfinite(this->dataPtr->skew) || this->dataPtr->fx <= 0 || this->dataPtr->fy <= 0)
  {
    gzerr << "Camera intrinsics must be finite, with positive focal lengths." << std::endl;
    return;
  }

  bool parsed = true;
  const auto parameter = [&](const std::string & key, auto defaultValue)
  {
    sdf::Errors errors;
    const auto value = _sdf->Get<decltype(defaultValue)>(errors, key, defaultValue);
    parsed = parsed && errors.empty() && (!_sdf->HasElement(key) || value.second);
    if (!errors.empty())
    {
      gzerr << "Invalid camera parameter <" << key << ">." << std::endl;
    }
    return value.first;
  };
  const char * channels[] = {"B", "G", "R"};
  for (int channel = 0; channel < 3; ++channel)
  {
    const float attenuation = parameter(std::string("attenuation") + channels[channel], 1.f / 30.f);
    const int background = parameter(std::string("background") + channels[channel], 0);
    if (
      !parsed || !std::isfinite(attenuation) || attenuation < 0 || background < 0 ||
      background > 255)
    {
      gzerr << "Camera attenuation must be finite and nonnegative; background must be 0-255."
            << std::endl;
      return;
    }
    this->dataPtr->attenuation[channel] = attenuation;
    this->dataPtr->background[channel] = static_cast<unsigned char>(background);
  }

  gzmsg << "Camera intrinsics: fx=" << this->dataPtr->fx << ", fy=" << this->dataPtr->fy
        << ", cx=" << this->dataPtr->cx << ", cy=" << this->dataPtr->cy << std::endl;

  gzmsg << "Image size: width=" << this->dataPtr->width << ", height=" << this->dataPtr->height
        << std::endl;

  gzmsg << "Min range: " << this->dataPtr->min_range << ", Max range: " << this->dataPtr->max_range
        << std::endl;

  std::unique_ptr<float[]> lut;
  try
  {
    lut = std::make_unique<float[]>(static_cast<size_t>(pixels));
  }
  catch (const std::bad_alloc &)
  {
    gzerr << "Unable to allocate the camera depth-to-range table." << std::endl;
    return;
  }
  float * lutPtr = lut.get();

  // Fill depth2range LUT
  for (unsigned int v = 0; v < this->dataPtr->height; v++)
  {
    double y_z = (v - this->dataPtr->cy) / this->dataPtr->fy;
    for (unsigned int u = 0; u < this->dataPtr->width; u++)
    {
      double x_z = (u - this->dataPtr->cx - this->dataPtr->skew * y_z) / this->dataPtr->fx;
      // Precompute the per-pixel factor in the following formula:
      // range = || (x, y, z) ||_2
      // range = || z * (x/z, y/z, 1.0) ||_2
      // range = z * || (x/z, y/z, 1.0) ||_2
      const double factor = std::hypot(std::hypot(x_z, y_z), 1.0);
      if (!std::isfinite(factor) || factor > std::numeric_limits<float>::max())
      {
        gzerr << "Camera intrinsics produce an unrepresentable depth-to-range factor." << std::endl;
        return;
      }
      *(lutPtr++) = static_cast<float>(factor);
    }
  }
  this->dataPtr->depth2rangeLUT = std::move(lut);

  if (!rclcpp::ok())
  {
    // Gazebo owns process signals. A plugin must not replace its handlers or
    // invalidate ROS while Gazebo is still running update/transport callbacks.
    rclcpp::init(0, nullptr, rclcpp::InitOptions(), rclcpp::SignalHandlerOptions::None);
  }
  this->ros_node_ = std::make_shared<rclcpp::Node>(rosNodeName);

  // Construct the publisher before making callbacks visible to transport.
  this->dataPtr->image_pub = this->ros_node_->create_publisher<sensor_msgs::msg::Image>(
    this->dataPtr->simulated_image_topic, 1);

  // Capture the lifetime gate, not an unguarded `this` pointer.
  const auto state = this->callbackState;
  std::function<void(const gz::msgs::Image &)> camera_callback =
    [state](const gz::msgs::Image & image)
  {
    std::lock_guard<std::mutex> lock(state->mutex);
    if (state->owner)
    {
      state->owner->CameraCallback(image);
    }
  };

  this->dataPtr->gz_node.Subscribe(this->dataPtr->image_topic, camera_callback);

  // Gazebo depth image subscriber
  std::function<void(const gz::msgs::Image &)> depth_callback =
    [state](const gz::msgs::Image & image)
  {
    std::lock_guard<std::mutex> lock(state->mutex);
    if (state->owner)
    {
      state->owner->DepthImageCallback(image);
    }
  };

  this->dataPtr->gz_node.Subscribe(this->dataPtr->depth_image_topic, depth_callback);
}

cv::Mat UnderwaterCamera::ConvertGazeboToOpenCV(const gz::msgs::Image & gz_image)
{
  int cv_type;
  switch (gz_image.pixel_format_type())
  {
    case gz::msgs::PixelFormatType::RGB_INT8:
      cv_type = CV_8UC3;
      break;
    case gz::msgs::PixelFormatType::RGBA_INT8:
      cv_type = CV_8UC4;
      break;
    case gz::msgs::PixelFormatType::BGR_INT8:
      cv_type = CV_8UC3;
      break;
    case gz::msgs::PixelFormatType::L_INT8:  // MONO8
      cv_type = CV_8UC1;
      break;
    case gz::msgs::PixelFormatType::R_FLOAT32:  // DEPTH32F
      cv_type = CV_32FC1;
      break;
    default:
      return {};
  }

  const size_t rowBytes = static_cast<size_t>(gz_image.width()) * CV_ELEM_SIZE(cv_type);
  const size_t stride = gz_image.step();
  if (
    gz_image.width() == 0 || gz_image.height() == 0 ||
    gz_image.width() > static_cast<unsigned int>(std::numeric_limits<int>::max()) ||
    gz_image.height() > static_cast<unsigned int>(std::numeric_limits<int>::max()) ||
    stride < rowBytes || stride % CV_ELEM_SIZE1(cv_type) != 0 ||
    static_cast<uint64_t>(stride) * gz_image.height() > gz_image.data().size())
  {
    return {};
  }

  // Respect Gazebo's row stride, including padded depth images.
  cv::Mat cv_image(
    gz_image.height(), gz_image.width(), cv_type,
    const_cast<void *>(reinterpret_cast<const void *>(gz_image.data().data())), stride);

  // Optionally convert color space if needed (e.g., RGB to BGR)
  if (gz_image.pixel_format_type() == gz::msgs::PixelFormatType::RGB_INT8)
  {
    cv::Mat converted;
    cv::cvtColor(cv_image, converted, cv::COLOR_RGB2BGR);
    return converted;
  }
  else if (gz_image.pixel_format_type() == gz::msgs::PixelFormatType::RGBA_INT8)
  {
    cv::Mat converted;
    cv::cvtColor(cv_image, converted, cv::COLOR_RGBA2BGRA);
    return converted;
  }

  return cv_image;
}

void UnderwaterCamera::CameraCallback(const gz::msgs::Image & msg)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex_);

  if (!this->ros_node_ || !rclcpp::ok(this->ros_node_->get_node_base_interface()->get_context()))
  {
    return;
  }
  if (!this->dataPtr->depth2rangeLUT)
  {
    gzerr << "Depth2range LUT not initialized" << std::endl;
    return;
  }
  else
  {
    gzmsg << "posim_gz_sensor_plugins::UnderwaterCamera::CameraCallback" << std::endl;

    if (this->dataPtr->firstImage)
    {
      this->dataPtr->lastImage = msg;
      this->dataPtr->firstImage = false;
    }
    else
    {
      // RGB and depth arrive independently. Do not index a missing, truncated,
      // or differently-sized depth frame (including during startup/teardown).
      const auto & depth = this->dataPtr->lastDepth;
      if (
        depth.pixel_format_type() != gz::msgs::PixelFormatType::R_FLOAT32 ||
        depth.width() != this->dataPtr->width || depth.height() != this->dataPtr->height ||
        msg.width() != this->dataPtr->width || msg.height() != this->dataPtr->height)
      {
        return;
      }
      // Convert Gazebo image to OpenCV image
      cv::Mat image = this->ConvertGazeboToOpenCV(msg);

      // Convert depth image to OpenCV image using the ConvertGazeboToOpenCV function
      cv::Mat depth_image = this->ConvertGazeboToOpenCV(this->dataPtr->lastDepth);

      if (image.empty() || image.type() != CV_8UC3 || depth_image.empty())
      {
        return;
      }

      // Own the output buffer instead of writing through a protobuf const view.
      cv::Mat output_image(image.rows, image.cols, CV_8UC3);

      // Simulate underwater
      cv::Mat simulated_image = this->SimulateUnderwater(image, depth_image, output_image);

      // Publish simulated image
      sensor_msgs::msg::Image ros_image;
      // Preserve the sensor acquisition time and frame for ROS synchronization.
      ros_image.header.stamp.sec = msg.header().stamp().sec();
      ros_image.header.stamp.nanosec = msg.header().stamp().nsec();
      for (const auto & entry : msg.header().data())
      {
        if (entry.key() == "frame_id" && entry.value_size() > 0)
        {
          ros_image.header.frame_id = entry.value(0);
          // Match ros_gz_bridge's ROS frame convention for scoped Gazebo names.
          std::size_t separator = 0;
          while ((separator = ros_image.header.frame_id.find("::", separator)) != std::string::npos)
          {
            ros_image.header.frame_id.replace(separator, 2, "/");
            ++separator;
          }
          break;
        }
      }
      ros_image.height = msg.height();
      ros_image.width = msg.width();
      ros_image.encoding = "bgr8";
      ros_image.is_bigendian = false;
      ros_image.step = msg.width() * 3;
      ros_image.data = std::vector<unsigned char>(
        simulated_image.data,
        simulated_image.data + simulated_image.total() * simulated_image.elemSize());

      this->dataPtr->image_pub->publish(ros_image);

      // Store the current image for the next iteration
      this->dataPtr->lastImage = msg;
    }
  }
}

void UnderwaterCamera::DepthImageCallback(const gz::msgs::Image & msg)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex_);

  gzmsg << "posim_gz_sensor_plugins::UnderwaterCamera::DepthImageCallback" << std::endl;

  this->dataPtr->lastDepth = msg;
}

cv::Mat UnderwaterCamera::SimulateUnderwater(
  const cv::Mat & _inputImage, const cv::Mat & _inputDepth, cv::Mat & _outputImage)
{
  // The scene renderer already applies optics to the raw RGB topic. Preserve
  // this compatibility output without attenuating the same pixels twice.
  if (std::getenv("POSIM_UNDERWATER_PLUGIN"))
  {
    _inputImage.copyTo(_outputImage);
    return _outputImage;
  }
  const float * lutPtr = this->dataPtr->depth2rangeLUT.get();
  for (unsigned int row = 0; row < this->dataPtr->height; row++)
  {
    const cv::Vec3b * inrow = _inputImage.ptr<cv::Vec3b>(row);
    const float * depthrow = _inputDepth.ptr<float>(row);
    cv::Vec3b * outrow = _outputImage.ptr<cv::Vec3b>(row);

    for (int col = 0; col < this->dataPtr->width; col++)
    {
      // Convert depth to range using the depth2range LUT
      float r = *(lutPtr++) * depthrow[col];

      const cv::Vec3b & in = inrow[col];
      cv::Vec3b & out = outrow[col];

      // NaN is an invalid/no-return depth, not a valid byte intensity.
      // Treat it like a far return; signed infinities retain the clipping below.
      if (std::isnan(r))
      {
        r = this->dataPtr->max_range;
      }
      if (r < this->dataPtr->min_range)
      {
        r = this->dataPtr->min_range;
      }
      else if (r > this->dataPtr->max_range)
      {
        r = this->dataPtr->max_range;
      }

      for (int c = 0; c < 3; c++)
      {
        // Approximate intensity as proportional to irradiance.
        float e = std::exp(-r * this->dataPtr->attenuation[c]);
        out[c] = e * in[c] + (1.0f - e) * this->dataPtr->background[c];
      }
    }
  }
  return _outputImage;
}

void UnderwaterCamera::PostUpdate(
  const gz::sim::UpdateInfo & _info, const gz::sim::EntityComponentManager & _ecm)
{
  if (!this->ros_node_)
  {
    return;
  }
  const auto context = this->ros_node_->get_node_base_interface()->get_context();
  if (!rclcpp::ok(context))
  {
    return;
  }
  if (!_info.paused)
  {
    try
    {
      rclcpp::spin_some(this->ros_node_);
    }
    catch (const rclcpp::exceptions::RCLError &)
    {
      // Another embedded ROS component may shut down the shared context.
      // Do not mask an error while the context is still valid.
      if (rclcpp::ok(context))
      {
        throw;
      }
      return;
    }

    if (_info.iterations % 1000 == 0)
    {
      gzmsg << "posim_gz_sensor_plugins::UnderwaterCamera::PostUpdate" << std::endl;
    }
  }
}

}  // namespace posim_gz_sensor_plugins
