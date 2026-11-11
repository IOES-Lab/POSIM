/*

 * Authors:
 *   Helena Moyen helenamoyen@gmail.com
 *   Woen-Sug Choi woensug.choi@gmail.com
 *
 * Copyright (C) 2025 Open Source Robotics Foundation
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
 *
 */

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SVD>
#include "PhysicalPulse.hh"

#include <gz/common/Console.hh>
#include <gz/common/Event.hh>
#include <gz/common/Profiler.hh>
#include <gz/math/Helpers.hh>

#include <gz/msgs/float_v.pb.h>
#include <gz/msgs/image.pb.h>
#include <gz/msgs/PointCloudPackedUtils.hh>
#include <gz/msgs/Utility.hh>

#include <gz/math/TimeVaryingVolumetricGrid.hh>

#include <gz/rendering/Camera.hh>
#include <gz/rendering/DepthCamera.hh>
#include <gz/rendering/GpuRays.hh>
#include <gz/rendering/RayQuery.hh>
#include <gz/rendering/RenderEngine.hh>
#ifdef WITH_OGRE2
#include <Compositor/OgreCompositorManager2.h>
#include <OgreRectangle2D.h>
#include <OgreRoot.h>
#endif

#include <gz/sensors/GaussianNoiseModel.hh>
#include <gz/sensors/Manager.hh>
#include <gz/sensors/Noise.hh>
#include <gz/sensors/RenderingEvents.hh>
#include <gz/sensors/SensorTypes.hh>
#include "MultibeamSonarSensor.hh"
#include "RangeSampling.hh"
#include "RaySurfaceNormals.hh"
#include "SonarLog.hh"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <geometry_msgs/msg/vector3.hpp>
#include <marine_acoustic_msgs/msg/ping_info.hpp>
#include <marine_acoustic_msgs/msg/sonar_image_data.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <sensor_msgs/image_encodings.hpp>

namespace gz
{
namespace sensors
{
namespace
{

/// \brief A time-varying vector field built on
/// per-axis time-varying volumetric data grids
///
/// \see gz::math::InMemoryTimeVaryingVolumetricGrid
template <typename T, typename V = T, typename P = T>
class InMemoryTimeVaryingVectorField
{
public:
  using SessionT = gz::math::InMemorySession<T, P>;

public:
  using GridT = gz::math::InMemoryTimeVaryingVolumetricGrid<T, V, P>;

  /// \brief Default constructor.
public:
  InMemoryTimeVaryingVectorField() = default;

  /// \brief Constructor
  /// \param[in] _xData X-axis volumetric data grid.
  /// \param[in] _yData Y-axis volumetric data grid.
  /// \param[in] _zData Z-axis volumetric data grid.
public:
  explicit InMemoryTimeVaryingVectorField(
    const GridT * _xData, const GridT * _yData, const GridT * _zData)
  : xData(_xData), yData(_yData), zData(_zData)
  {
    if (this->xData)
    {
      this->xSession = this->xData->CreateSession();
    }
    if (this->yData)
    {
      this->ySession = this->yData->CreateSession();
    }
    if (this->zData)
    {
      this->zSession = this->zData->CreateSession();
    }
  }

  /// \brief Advance vector field in time.
  /// \param[in] _now Time to step data grids to.
public:
  void StepTo(const std::chrono::steady_clock::duration & _now)
  {
    const T now = std::chrono::duration<T>(_now).count();
    if (this->xData && this->xSession)
    {
      this->xSession = this->xData->StepTo(this->xSession.value(), now);
    }
    if (this->yData && this->ySession)
    {
      this->ySession = this->yData->StepTo(this->ySession.value(), now);
    }
    if (this->zData && this->zSession)
    {
      this->zSession = this->zData->StepTo(this->zSession.value(), now);
    }
  }

  /// \brief Look up vector field value, interpolating data grids.
  /// \param[in] _pos Vector field argument.
  /// \return vector field value at `_pos`
public:
  gz::math::Vector3<V> LookUp(const gz::math::Vector3<P> & _pos)
  {
    auto outcome = gz::math::Vector3<V>::Zero;
    if (this->xData && this->xSession)
    {
      const auto interpolation = this->xData->LookUp(this->xSession.value(), _pos);
      outcome.X(interpolation.value_or(0.));
    }
    if (this->yData && this->ySession)
    {
      const auto interpolation = this->yData->LookUp(this->ySession.value(), _pos);
      outcome.Y(interpolation.value_or(0.));
    }
    if (this->zData && this->zSession)
    {
      const auto interpolation = this->zData->LookUp(this->zSession.value(), _pos);
      outcome.Z(interpolation.value_or(0.));
    }
    return outcome;
  }

  /// \brief Session for x-axis volumetric data grid, if any.
private:
  std::optional<SessionT> xSession{std::nullopt};

  /// \brief Session for y-axis volumetric data grid, if any.
private:
  std::optional<SessionT> ySession{std::nullopt};

  /// \brief Session for z-axis volumetric data grid, if any.
private:
  std::optional<SessionT> zSession{std::nullopt};

  /// \brief X-axis volumetric data grid, if any.
private:
  const GridT * xData{nullptr};

  /// \brief Y-axis volumetric data grid, if any.
private:
  const GridT * yData{nullptr};

  /// \brief Z-axis volumetric data grid, if any.
private:
  const GridT * zData{nullptr};
};

/// \brief Copy an 8-bit OpenCV image into a ROS image message.
///
/// The sonar only publishes BGR8 and RGB8 visualization images here. Keeping
/// this small conversion local avoids requiring cv_bridge for a byte-for-byte
/// copy and prevents OpenCV major-version coupling at runtime.
void CopyImageToMessage(
  const cv::Mat & _image, const std::string & _encoding, sensor_msgs::msg::Image & _message)
{
  _message.height = static_cast<uint32_t>(_image.rows);
  _message.width = static_cast<uint32_t>(_image.cols);
  _message.encoding = _encoding;
  _message.is_bigendian = false;
  _message.step = static_cast<uint32_t>(_image.cols * _image.elemSize());

  const auto dataSize = static_cast<size_t>(_message.step) * _message.height;
  _message.data.resize(dataSize);
  if (dataSize == 0)
  {
    return;
  }

  if (_image.isContinuous())
  {
    std::memcpy(_message.data.data(), _image.data, dataSize);
    return;
  }

  for (int row = 0; row < _image.rows; ++row)
  {
    std::memcpy(
      _message.data.data() + static_cast<size_t>(row) * _message.step, _image.ptr(row),
      _message.step);
  }
}

}  // namespace

using namespace gz::msgs;

//////////////////////////////////////////////////
MultibeamSonarSensor::MultibeamSonarSensor() : dataPtr(new Implementation()) {}

//////////////////////////////////////////////////
MultibeamSonarSensor::~MultibeamSonarSensor()
{
  this->dataPtr->sceneChangeConnection.reset();
  this->dataPtr->Shutdown(this);
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::Implementation::Shutdown(MultibeamSonarSensor * _sensor)
{
  // Stop receiving frames, then finish the worker before releasing its inputs.
  this->rayConnection.reset();
  {
    std::lock_guard<std::mutex> lk(this->computeMutex_);
    this->stopThread_ = true;
  }
  this->computeCV_.notify_one();
  if (this->computeThread_.joinable())
  {
    this->computeThread_.join();
  }

  std::lock_guard<std::mutex> rayLock(this->rayMutex);
  this->initialized = false;
  this->newFrameReady_ = false;
  this->publishingPointCloud = false;
  this->computeBackend.reset();
  if (this->raySensor && _sensor->Scene() && _sensor->Scene()->IsInitialized())
  {
    _sensor->Scene()->DestroySensor(this->raySensor);
  }
  this->raySensor.reset();
  if (this->depthSensor && _sensor->Scene() && _sensor->Scene()->IsInitialized())
  {
#ifdef WITH_OGRE2
    if (_sensor->Scene()->Engine()->Name() == "ogre2")
    {
      // Ogre 2.3's shared fullscreen objects retain the last compositor pass's
      // datablock after its workspace is removed. Unlink it on the render
      // thread before DestroySensor releases the depth camera's materials.
      // Each surviving compositor pass restores its own binding on execution.
      auto * compositor = Ogre::Root::getSingleton().getCompositorManager2();
      compositor->getSharedFullscreenQuad()->_setNullDatablock();
      compositor->getSharedFullscreenTriangle()->_setNullDatablock();
    }
#endif
    _sensor->Scene()->DestroySensor(this->depthSensor);
  }
  this->depthSensor.reset();
  if (this->ros_executor_ && this->ros_node_)
  {
    this->ros_executor_->remove_node(this->ros_node_);
  }
  this->sonarImageRawPub.reset();
  this->sonarImagePub.reset();
  this->normalImagePub.reset();
  this->ros_executor_.reset();
  this->ros_node_.reset();
  if (this->ros_context_)
  {
    this->ros_context_->shutdown("Multibeam sonar sensor stopped");
    this->ros_context_.reset();
  }
  this->writeLog.close();
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::Reset()
{
  this->dataPtr->Shutdown(this);
  auto replacement = std::make_unique<Implementation>();
  // Keep the logical sensor identity and configuration. All renderer, backend,
  // publisher, calibration and frame-buffer state belongs to one scene only.
  replacement->sensorSdf = this->dataPtr->sensorSdf;
  replacement->entityId = this->dataPtr->entityId;
  replacement->worldState = this->dataPtr->worldState;
  replacement->frameCounter = this->dataPtr->frameCounter;
  replacement->sceneChangeConnection = std::move(this->dataPtr->sceneChangeConnection);
  this->dataPtr.swap(replacement);
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Load(const sdf::Sensor & _sdf)
{
  this->dataPtr->sceneChangeConnection.reset();
  this->Reset();
  this->dataPtr->sensorSdf.reset();
  if (!gz::sensors::RenderingSensor::Load(_sdf))
  {
    return false;
  }

  // Check if this sensor is of the right type
  if (_sdf.Type() != sdf::SensorType::CUSTOM)
  {
    gzerr << "Expected [" << this->Name() << "] sensor to be "
          << "a Multibeam Sonar but found a " << _sdf.TypeStr() << "." << std::endl;
    return false;
  }

  sdf::ElementPtr elem = _sdf.Element();
  if (!elem->HasAttribute("gz:type"))
  {
    gzerr << "Missing 'gz:type' attribute "
          << "for sensor [" << this->Name() << "]. "
          << "Aborting load." << std::endl;
    return false;
  }
  const auto type = elem->Get<std::string>("gz:type");
  if (type != "multibeam_sonar")
  {
    gzerr << "Expected sensor [" << this->Name() << "] to be a "
          << "multibeam_sonar but it is of '" << type << "' type. Aborting load." << std::endl;
    return false;
  }
  if (!elem->HasElement("gz:multibeam_sonar"))
  {
    gzerr << "Missing 'gz:multibeam_sonar' configuration for "
          << "sensor [" << this->Name() << "]. "
          << "Aborting load." << std::endl;
    return false;
  }
  this->dataPtr->sensorSdf = elem->GetElement("gz:multibeam_sonar");
  // Setup sensors
  if (this->Scene())
  {
    if (!this->dataPtr->Initialize(this))
    {
      gzerr << "Failed to setup [" << this->Name() << "] sensor. " << std::endl;
      return false;
    }
  }

  gzmsg << "Loaded [" << this->Name() << "] Multibeam Sonar sensor." << std::endl;
  this->dataPtr->sceneChangeConnection = gz::sensors::RenderingEvents::ConnectSceneChangeCallback(
    std::bind(&MultibeamSonarSensor::SetScene, this, std::placeholders::_1));

  return true;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Load(sdf::ElementPtr _sdf)
{
  sdf::Sensor sdfSensor;
  sdfSensor.Load(_sdf);
  return this->Load(sdfSensor);
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Implementation::Initialize(MultibeamSonarSensor * _sensor)
{
  if (this->initialized)
  {
    return true;
  }
  if (!_sensor->Scene() || !this->sensorSdf)
  {
    return false;
  }
  // Align packed point fields to memory boundaries.
  msgs::InitPointCloudPacked(
    this->pointMsg, _sensor->FrameId(), true,
    {{"xyz", msgs::PointCloudPacked::Field::FLOAT32},
     {"intensity", msgs::PointCloudPacked::Field::FLOAT32},
     {"ring", msgs::PointCloudPacked::Field::UINT16}});

  gzmsg << "Initializing [" << _sensor->Name() << "] sensor." << std::endl;

  if (!this->InitializeBeamArrangement(_sensor))
  {
    gzerr << "Failed to initialize beam arrangement for "
          << "[" << _sensor->Name() << "] sensor." << std::endl;
    this->Shutdown(_sensor);
    return false;
  }

  gz::math::Pose3d referenceFrameTransform =
    this->sensorSdf->Get<gz::math::Pose3d>("reference_frame", gz::math::Pose3d{}).first;

  this->referenceFrameRotation = referenceFrameTransform.Rot().Inverse();

  // Each initialization owns a new worker; no pending frame crosses scenes.
  {
    std::lock_guard<std::mutex> lk(this->computeMutex_);
    this->stopThread_ = false;
    this->newFrameReady_ = false;
  }
  // Spawn background sonar compute thread now that all setup is complete.
  this->computeThread_ = std::thread(
    [this]()
    {
      while (true)
      {
        std::unique_lock<std::mutex> lk(this->computeMutex_);
        this->computeCV_.wait(lk, [this] { return this->newFrameReady_ || this->stopThread_; });
        if (this->stopThread_)
        {
          break;
        }
        this->newFrameReady_ = false;
        lk.unlock();
        this->ComputeSonarImage();
      }
    });

  gzmsg << "Initialized [" << _sensor->Name() << "] sensor." << std::endl;
  this->initialized = true;
  return true;
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Implementation::InitializeBeamArrangement(MultibeamSonarSensor * _sensor)
{
  this->beams.clear();
  this->beamTargets.clear();

  // Backend selection uses the complete readout prototype below. Auto mode
  // must test the requested acoustic model, not just GPU availability.
  const char * backendEnv = std::getenv("POSIM_SONAR_COMPUTE_BACKEND");
  this->requestedBackend = backendEnv ? backendEnv : "auto";

  // Read ray definition from SDF
  sdf::ElementPtr rayElement = this->sensorSdf->GetElement("ray");
  sdf::ElementPtr sensorElement = this->sensorSdf->GetElement("spec");

  if (!rayElement)
  {
    gzerr << "No beam properties(format of GPU Ray) specified for "
          << "[" << _sensor->Name() << "] sensor" << std::endl;
    return false;
  }

  if (!sensorElement)
  {
    gzerr << "No sonar properties(format of GPU Ray) specified for "
          << "[" << _sensor->Name() << "] sensor" << std::endl;
    return false;
  }

  const bool useDegrees = rayElement->Get("degrees", false).first;
  const gz::math::Angle angleUnit = useDegrees ? GZ_DTOR(1.) : 1.;

  // -------- Assign ray sensor properties
  sdf::ElementPtr rangeElement = rayElement->GetElement("range");
  this->minimumRange = rangeElement->Get<double>("min", 0.1).first;
  gzmsg << "Setting minimum range to " << this->minimumRange << " m for [" << _sensor->Name()
        << "] sensor." << std::endl;
  this->maximumRange = rangeElement->Get<double>("max", 5.).first;
  gzmsg << "Setting maximum range to " << this->maximumRange << " m for [" << _sensor->Name()
        << "] sensor." << std::endl;

  // Read sonar properties from model.sdf

  this->verticalFOV = sensorElement->Get<double>("verticalFOV", 10).first;
  gzmsg << "verticalFOV: " << this->verticalFOV << " degrees" << std::endl;
  this->sonarFreq = sensorElement->Get<double>("sonarFreq", 900e3).first;
  gzmsg << "sonarFreq: " << this->sonarFreq << " Hz" << std::endl;

  this->bandwidth = sensorElement->Get<double>("bandwidth", 29.5e6).first;
  gzmsg << "bandwidth: " << this->bandwidth << " Hz" << std::endl;

  this->soundSpeed = sensorElement->Get<double>("soundSpeed", 1500).first;
  gzmsg << "soundSpeed: " << this->soundSpeed << " m/s" << std::endl;

  this->maxDistance = sensorElement->Get<double>("maxDistance", 60).first;
  gzmsg << "maxDistance: " << this->maxDistance << " meters" << std::endl;

  this->sourceLevel = sensorElement->Get<double>("sourceLevel", 220).first;
  gzmsg << "sourceLevel: " << this->sourceLevel << " dB" << std::endl;

  this->raySkips = sensorElement->Get<int>("raySkips", 10).first;
  gzmsg << "raySkips: " << this->raySkips << std::endl;

  this->plotScaler = sensorElement->Get<float>("plotScaler", 10).first;
  gzmsg << "plotScaler: " << this->plotScaler << std::endl;

  this->sensorGain = sensorElement->Get<float>("sensorGain", 0.02).first;
  gzmsg << "sensorGain: " << this->sensorGain << std::endl;

  this->debugFlag = sensorElement->Get<bool>("debugFlag", false).first;
  gzmsg << "Debug: " << this->debugFlag << std::endl;

  this->blazingFlag = sensorElement->Get<bool>("blazingSonarImage", false).first;
  gzmsg << "BlazingSonarImage: " << this->blazingFlag << std::endl;

  this->pointCloudTopicName =
    sensorElement->Get<std::string>("pointCloudTopicName", "point_cloud").first;
  gzmsg << "pointCloudTopicName: " << this->pointCloudTopicName << std::endl;

  this->sonarImageRawTopicName =
    sensorElement->Get<std::string>("sonarImageRawTopicName", "sonar_image_raw").first;
  gzmsg << "sonarImageRawTopicName: " << this->sonarImageRawTopicName << std::endl;

  this->sonarImageTopicName =
    sensorElement->Get<std::string>("sonarImageTopicName", "sonar_image").first;
  gzmsg << "sonarImageTopicName: " << this->sonarImageTopicName << std::endl;

  // NOTE: frameName is the TF frame name for published sonar messages (frame_id in ROS headers).
  // This is explicitly configured in SDF <frameName> rather than derived from Gazebo's
  // internal sensor frame ID. Must match TF tree for RViz transforms to work correctly.
  // Verify in SDF files that <frameName> matches the expected TF frame name.
  this->frameName =
    sensorElement->Get<std::string>("frameName", "forward_sonar_optical_link").first;
  gzmsg << "frameName: " << this->frameName << std::endl;

  // ROS Initialization

  // Gazebo owns process signals. Keep this sensor's ROS lifetime independent
  // from the global context and from other plugins' signal handlers. In
  // particular, shutdown must not invalidate the context during node creation.
  this->ros_context_ = std::make_shared<rclcpp::Context>();
  rclcpp::InitOptions initOptions;
  initOptions.shutdown_on_signal = false;
  this->ros_context_->init(0, nullptr, initOptions);
  rclcpp::NodeOptions nodeOptions;
  nodeOptions.context(this->ros_context_);
  this->ros_node_ = std::make_shared<rclcpp::Node>("multibeam_sonar_node", nodeOptions);
  rclcpp::ExecutorOptions executorOptions;
  executorOptions.context = this->ros_context_;
  this->ros_executor_ =
    std::make_shared<rclcpp::executors::SingleThreadedExecutor>(executorOptions);
  this->ros_executor_->add_node(this->ros_node_);

  // Create the point cloud publisher
  this->pointPub = this->node.Advertise<gz::msgs::PointCloudPacked>(
    _sensor->Topic() + "/" + this->pointCloudTopicName);
  this->pointFloatPub =
    this->node.Advertise<gz::msgs::Float_V>(_sensor->Topic() + "/point_cloud_float_vector");

  this->sonarImageRawPub =
    this->ros_node_->create_publisher<marine_acoustic_msgs::msg::ProjectedSonarImage>(
      _sensor->Topic() + "/" + this->sonarImageRawTopicName, rclcpp::SystemDefaultsQoS());

  this->sonarImagePub = this->ros_node_->create_publisher<sensor_msgs::msg::Image>(
    _sensor->Topic() + "/" + this->sonarImageTopicName, rclcpp::SystemDefaultsQoS());

  this->normalImagePub = this->ros_node_->create_publisher<sensor_msgs::msg::Image>(
    _sensor->Topic() + "/normal_image", rclcpp::SystemDefaultsQoS());

  // Configure skips
  if (this->raySkips == 0)
  {
    this->raySkips = 1;
    gzmsg << "raySkips was 0, setting to 1." << std::endl;
  }

  this->writeLogFlag = sensorElement->Get<bool>("writeLog", false).first;

  if (this->writeLogFlag)
  {
    const int interval = sensorElement->Get<int>("writeFrameInterval", 10).first;
    this->writeInterval = std::max(1, interval);
    if (interval < 1)
    {
      gzwarn << "writeFrameInterval must be positive; using 1." << std::endl;
    }
    try
    {
      this->logDirectory =
        posim::sonar::CreateLogDirectory(std::filesystem::current_path()).string();
      RCLCPP_INFO_STREAM(
        this->ros_node_->get_logger(), "Sonar CSV directory: " << this->logDirectory
                                                               << ", recording every "
                                                               << this->writeInterval << " frames");
    }
    catch (const std::exception & error)
    {
      this->writeLogFlag = false;
      gzerr << "Sonar CSV logging disabled: " << error.what() << std::endl;
    }
  }

  sdf::ElementPtr horizontalElement = rayElement->GetElement("scan")->GetElement("horizontal");
  const double horizAngleMin = horizontalElement->Get<double>("min_angle", -M_PI / 4.0).first;
  const double horizAngleMax = horizontalElement->Get<double>("max_angle", M_PI / 4.0).first;

  sdf::ElementPtr verticalElement = rayElement->GetElement("scan")->GetElement("vertical");
  const double verticalAngleMin = verticalElement->Get<double>("min_angle", -M_PI / 8.0).first;
  const double verticalAngleMax = verticalElement->Get<double>("max_angle", M_PI / 8.0).first;

  const int beamCount = horizontalElement->Get<int>("beams", 256).first;
  const int rayCount = verticalElement->Get<int>("rays", 3).first;
  const int horizontalSamples = horizontalElement->Get<int>("samples", beamCount).first;
  // Full power FWHM in radians, independent of the scan's degrees attribute.
  // Zero/missing retains the explicitly reported legacy grid-coupled model.
  const double receiveBeamWidth = sensorElement->Get<double>("receiveBeamWidth", 0.).first;
  this->receiveGain = sensorElement->Get<float>("receiveGain", 1.f).first;
  if (
    beamCount < 2 || horizontalSamples < 2 || rayCount < 2 || !std::isfinite(receiveBeamWidth) ||
    receiveBeamWidth < 0 || receiveBeamWidth >= M_PI || !std::isfinite(this->receiveGain) ||
    this->receiveGain < 0 || this->raySkips < 1 ||
    (receiveBeamWidth == 0 && horizontalSamples != beamCount))
  {
    gzerr << "Invalid sonar sampling/readout: counts must be >= 2, stride >= 1; independent "
             "horizontal samples require a positive receiveBeamWidth (power FWHM, radians)."
          << std::endl;
    return false;
  }

  // Compute FOVs
  this->hFOV = horizAngleMax - horizAngleMin;  // Horizontal Field of View
  gzmsg << "HFOV :" << this->hFOV << std::endl;

  this->vFOV = verticalAngleMax - verticalAngleMin;  // Vertical Field of View
  gzmsg << "VFOV :" << this->vFOV << std::endl;

  // Ensure FOV is always in radians
  if (useDegrees == true)
  {
    this->hFOV = this->hFOV * (M_PI / 180.0);
    this->vFOV = this->vFOV * (M_PI / 180.0);
  }

  // Gazebo debug output
  gzmsg << "Debug Info: FOV Calculations" << std::endl;
  gzmsg << "horizAngleMax: " << horizAngleMax << ", horizAngleMin: " << horizAngleMin
        << ", hFOV (radians): " << this->hFOV << std::endl;
  gzmsg << "verticalAngleMax: " << verticalAngleMax << ", verticalAngleMin: " << verticalAngleMin
        << ", vFOV (radians): " << this->vFOV << std::endl;

  // ---- Construct AcousticBeam
  // Initialize beamId
  int beamId = 0;

  // Iterate through the rays
  for (int v = 0; receiveBeamWidth == 0 && v < rayCount; ++v)
  {
    for (int h = 0; h < horizontalSamples; ++h)
    {
      // Calculate beam angles
      gz::math::Angle beamApertureAngle =
        gz::math::Angle(verticalAngleMax - verticalAngleMin) * angleUnit;
      gz::math::Angle beamRotationAngle =
        gz::math::Angle(
          horizAngleMin + (h * (horizAngleMax - horizAngleMin) / (horizontalSamples - 1))) *
        angleUnit;
      gz::math::Angle beamTiltAngle =
        gz::math::Angle(
          verticalAngleMin + (v * (verticalAngleMax - verticalAngleMin) / (rayCount - 1))) *
        angleUnit;

      // Normalize angles
      beamApertureAngle = beamApertureAngle.Normalized();
      beamRotationAngle = beamRotationAngle.Normalized();
      beamTiltAngle = beamTiltAngle.Normalized();

      // Build acoustic beam
      this->beams.push_back(
        AcousticBeam{beamId, beamApertureAngle, beamRotationAngle, beamTiltAngle});

      // Increment beamId
      ++beamId;
    }
  }

  // Add as many (still null) targets as beams
  this->beamTargets.resize(this->beams.size());

  // Aggregate all beams' footprint in spherical coordinates into one
  AxisAlignedPatch2d beamsSphericalFootprint;
  for (const auto & beam : this->beams)
  {
    beamsSphericalFootprint.Merge(beam.SphericalFootprint());
  }
  // Rendering sensors' FOV must be symmetric about its main axis
  beamsSphericalFootprint.Merge(beamsSphericalFootprint.Flip());

  // Scan limits describe the complete imaging fan. AcousticBeam footprints
  // include an aperture around each ray and must not enlarge that fan.
  const double scanHorizontalMin = horizAngleMin * angleUnit.Radian();
  const double scanHorizontalMax = horizAngleMax * angleUnit.Radian();
  const double scanVerticalMin = verticalAngleMin * angleUnit.Radian();
  const double scanVerticalMax = verticalAngleMax * angleUnit.Radian();
  this->nBeams = beamCount;
  this->nRays = rayCount;
  this->ray_nElevationRays = this->nRays;
  this->ray_nAzimuthRays = 1;
  const auto horizontalRayCount = horizontalSamples;
  const auto verticalRayCount = rayCount;
  try
  {
    this->receiveAngles = AngularCenters(scanHorizontalMin, scanHorizontalMax, beamCount);
    this->physicalReadout.reset();
    this->perspectiveGeometry.reset();
    if (receiveBeamWidth > 0)
    {
      if (
        std::abs(scanHorizontalMin + scanHorizontalMax) > 1e-6 ||
        std::abs(scanVerticalMin + scanVerticalMax) > 1e-6 || !std::isfinite(this->minimumRange) ||
        this->minimumRange <= 0 || !std::isfinite(this->maximumRange) ||
        this->maximumRange <= this->minimumRange)
      {
        throw std::invalid_argument(
          "perspective sonar requires a symmetric fan and valid range limits");
      }
      this->perspectiveGeometry =
        std::make_unique<const PerspectiveSonarGeometry>(MakePerspectiveSonarGeometry(
          horizontalSamples, rayCount, scanHorizontalMax, scanVerticalMax, this->raySkips,
          this->receiveAngles, receiveBeamWidth));
      const auto & grid = *this->perspectiveGeometry;
      this->azimuth_angles = grid.azimuth;
      this->elevation_angles = grid.elevation[horizontalSamples / 2];
      this->physicalReadout = std::make_unique<const AcousticReadout>(grid.readout);
      this->depthSensor = _sensor->Scene()->CreateDepthCamera(_sensor->Name() + "_depth_sensor");
      if (!this->depthSensor)
      {
        throw std::invalid_argument("renderer cannot create direct sonar geometry");
      }
      this->depthSensor->SetLocalPose(_sensor->Pose());
      this->depthSensor->SetImageWidth(horizontalSamples);
      this->depthSensor->SetImageHeight(rayCount);
      this->depthSensor->SetHFOV(this->hFOV);
      this->depthSensor->SetAspectRatio(grid.horizontalTangent / grid.verticalTangent);
      this->depthSensor->SetAntiAliasing(0);
      // Camera clipping is axial; the callback applies radial sonar limits.
      this->depthSensor->SetNearClipPlane(
        this->minimumRange / std::sqrt(
                               1. + grid.horizontalTangent * grid.horizontalTangent +
                               grid.verticalTangent * grid.verticalTangent));
      this->depthSensor->SetFarClipPlane(this->maximumRange);
    }
    else
    {
      this->azimuth_angles =
        AngularCenters(scanHorizontalMin, scanHorizontalMax, horizontalSamples);
      this->elevation_angles = AngularCenters(scanVerticalMin, scanVerticalMax, rayCount);
      this->raySensor = _sensor->Scene()->CreateGpuRays(_sensor->Name() + "_ray_sensor");
      if (!this->raySensor)
      {
        throw std::invalid_argument("renderer cannot create sonar GpuRays");
      }
      this->raySensor->SetLocalPose(_sensor->Pose());
      this->raySensor->SetNearClipPlane(this->minimumRange);
      this->raySensor->SetFarClipPlane(this->maximumRange);
      this->raySensor->SetClamp(false);
      this->raySensor->SetAngleMin(scanHorizontalMin);
      this->raySensor->SetAngleMax(scanHorizontalMax);
      this->raySensor->SetVerticalAngleMin(scanVerticalMin);
      this->raySensor->SetVerticalAngleMax(scanVerticalMax);
      this->raySensor->SetRayCount(horizontalSamples);
      this->raySensor->SetVerticalRayCount(rayCount);
      if (
        this->raySensor->RangeCount() != static_cast<unsigned>(horizontalSamples) ||
        this->raySensor->VerticalRangeCount() != static_cast<unsigned>(rayCount))
      {
        throw std::invalid_argument("renderer dimensions differ from sonar geometry");
      }
    }
  }
  catch (const std::invalid_argument & error)
  {
    gzerr << "Invalid sonar readout: " << error.what() << std::endl;
    return false;
  }
  gzmsg << "Sonar receive model: "
        << (this->physicalReadout ? "perspective-area-sinc-v1" : "legacy-grid") << "; geometry "
        << horizontalSamples << " x " << rayCount << "; receivers " << this->nBeams
        << "; power FWHM " << receiveBeamWidth << " rad; pressure gain " << this->receiveGain
        << std::endl;

  auto & intrinsics = this->raySensorIntrinsics;
  intrinsics.offset.X(scanHorizontalMin);
  intrinsics.offset.Y(scanVerticalMin);
  intrinsics.step.X(this->hFOV / (horizontalRayCount - 1));
  intrinsics.step.Y(this->vFOV / (verticalRayCount - 1));

  // Pre-compute scan indices covered by beam spherical
  // footprints for speed during scan iteration
  this->beamScanPatches.clear();
  for (const auto & beam : this->beams)
  {
    this->beamScanPatches.push_back(
      AxisAlignedPatch2i{(beam.SphericalFootprint() - intrinsics.offset) / intrinsics.step});
  }

  // Keep RGB-only decorative geometry out of acoustic range measurements when
  // the world author assigns it a dedicated visibility category.
  const auto visibilityMask =
    sensorElement->Get<unsigned int>("visibilityMask", GZ_VISIBILITY_ALL).first;
  gzmsg << "Acoustic visibility mask: " << visibilityMask << std::endl;
  this->pointMsg.set_width(horizontalSamples);
  this->pointMsg.set_height(rayCount);
  this->pointMsg.set_row_step(this->pointMsg.point_step() * this->pointMsg.width());
  if (this->depthSensor)
  {
    this->depthSensor->SetVisibilityMask(visibilityMask);
    _sensor->AddSensor(this->depthSensor);
    this->rayConnection = this->depthSensor->ConnectNewRgbPointCloud(std::bind(
      &MultibeamSonarSensor::Implementation::OnNewDepthFrame, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
  }
  else
  {
    this->raySensor->SetVisibilityMask(visibilityMask);
    _sensor->AddSensor(this->raySensor);
    this->rayConnection = this->raySensor->ConnectNewGpuRaysFrame(std::bind(
      &MultibeamSonarSensor::Implementation::OnNewFrame, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
  }

  // Transmission path properties (typical model used here)
  // More sophisticated model by Francois-Garrison model is available
  this->absorption = 0.0354;  // [dB/m]
  this->attenuation = this->absorption * log(10) / 20.0;

  // Published range interval and internal pulse quadrature are independent.
  const float delta_t = 1.0 / this->bandwidth;
  PhysicalPulse pulse;
  if (this->physicalReadout)
  {
    try
    {
      pulse = MakePhysicalPulse(this->maxDistance, this->soundSpeed, this->bandwidth);
    }
    catch (const std::invalid_argument & error)
    {
      gzerr << error.what() << std::endl;
      return false;
    }
    this->nFreq = static_cast<int>(pulse.ranges.size());
    this->spectrumSamples = static_cast<int>(pulse.spectrum.size());
  }
  else
  {
    this->nFreq = RangeSampleCount(this->maxDistance, this->soundSpeed, this->bandwidth);
    this->spectrumSamples = this->nFreq;
  }
  if (this->nFreq < 2)
  {
    gzerr << "Invalid or unrepresentable sonar range sampling parameters." << std::endl;
    return false;
  }
  const int nTime = this->nFreq;
  this->rangeVector.resize(nTime);
  for (int i = 0; i < nTime; i++)
  {
    this->rangeVector[i] =
      this->physicalReadout ? pulse.ranges[i] : delta_t * i * this->soundSpeed / 2.0;
  }

  // Print sonar calculation settings
  RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "==================================================");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "============   SONAR PLUGIN LOADED   =============");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "==================================================");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "============       RAY VERSION       =============");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "==================================================");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "Maximum view range  [m] = " << this->maxDistance);
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(),
    "Range sample spacing [m] = " << this->soundSpeed / (2.0 * this->bandwidth));
  RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "# of Beams = " << this->nBeams);
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "# of Rays / Beam (Elevation, Azimuth) = ("
                                     << this->ray_nElevationRays << ", " << this->ray_nAzimuthRays
                                     << ")");
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "Calculation skips (Elevation) = " << this->raySkips);
  RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "# of Time data / Beam = " << this->nFreq);
  RCLCPP_INFO_STREAM(
    this->ros_node_->get_logger(), "==================================================");
  RCLCPP_INFO_STREAM(this->ros_node_->get_logger(), "");

  // -- Pre calculations for sonar -- //

  this->window.resize(this->spectrumSamples);
  if (this->physicalReadout)
  {
    std::copy(pulse.spectrum.begin(), pulse.spectrum.end(), this->window.begin());
    gzmsg << "Physical unit-peak Hamming pulse: " << this->nFreq << " output ranges, "
          << this->spectrumSamples << " midpoint frequency samples including guard." << std::endl;
  }
  else
  {
    float windowSum = 0;
    for (int f = 0; f < this->nFreq; ++f)
    {
      this->window[f] = 0.54 - 0.46 * cos(2.0 * M_PI * (f + 1) / this->nFreq);
      windowSum += pow(this->window[f], 2.0);
    }
    for (int f = 0; f < this->nFreq; ++f)
    {
      this->window[f] /= sqrt(windowSum);
    }
  }

  // Only the legacy model uses the square grid-coupled corrector.
  this->beamCorrector.clear();
  this->beamCorrectorStorage.clear();
  this->beamCorrectorSum = 0.0;
  if (!this->physicalReadout)
  {
    this->beamCorrectorStorage.resize(static_cast<size_t>(this->nBeams) * this->nBeams);
    this->beamCorrector.resize(this->nBeams);
    for (int i = 0; i < this->nBeams; i++)
    {
      this->beamCorrector[i] = this->beamCorrectorStorage.data() + i * this->nBeams;
    }
  }

  this->constMu = true;
  this->mu = 1e-3;

  // Probe the actual geometry/output/transform shape before starting the
  // compute thread. These zero-return images live through backend selection.
  const cv::Mat probeDepth = cv::Mat::zeros(this->nRays, horizontalSamples, CV_32FC1);
  const cv::Mat probeNormal = cv::Mat::zeros(this->nRays, horizontalSamples, CV_32FC3);
  const cv::Mat probeReflectivity = cv::Mat::zeros(this->nRays, horizontalSamples, CV_32FC1);
  SonarComputeInput prototype;
  prototype.depthImage = &probeDepth;
  prototype.normalImage = &probeNormal;
  prototype.reflectivityImage = &probeReflectivity;
  prototype.physicalReadout = this->physicalReadout.get();
  prototype.receiveGain = this->receiveGain;
  prototype.nBeams = this->nBeams;
  prototype.nRays = this->nRays;
  prototype.nFreq = this->spectrumSamples;
  prototype.nOutputRanges = this->physicalReadout ? this->nFreq : 0;
  prototype.raySkips = this->raySkips;
  prototype.hFOV = this->hFOV;
  prototype.vFOV = this->vFOV;
  prototype.maxDistance = this->maxDistance;
  prototype.soundSpeed = this->soundSpeed;
  prototype.attenuation = this->attenuation;
  prototype.sensorGain = this->sensorGain;
  prototype.debugFlag = this->debugFlag;
  prototype.blazingFlag = this->blazingFlag;
  prototype.window = this->window.data();
  prototype.rangeVector = this->rangeVector.data();
  prototype.beamCorrector = this->beamCorrector.data();
  prototype.beamCorrectorSum = this->beamCorrectorSum;
  prototype.sourceLevel = this->sourceLevel;
  prototype.bandwidth = this->bandwidth;
  prototype.sonarFreq = this->sonarFreq;
  prototype.elevation_angles = this->elevation_angles;
  prototype.seed = this->sonarSeed;

  this->computeBackend = CreateComputeBackend(this->requestedBackend, prototype);
  if (!this->computeBackend)
  {
    gzerr << "Sonar backend [" << this->requestedBackend << "] cannot initialize the configured "
          << (this->physicalReadout ? "area-sinc-v1" : "legacy-grid") << " readout ("
          << this->spectrumSamples << " frequencies)." << std::endl;
    return false;
  }
  gzmsg << "Using sonar compute backend: " << this->computeBackend->Name() << std::endl;

  return true;
}

//////////////////////////////////////////////////
std::vector<gz::rendering::SensorPtr> MultibeamSonarSensor::RenderingSensors() const
{
  if (this->dataPtr->depthSensor)
  {
    return {this->dataPtr->depthSensor};
  }
  if (!this->dataPtr->raySensor)
  {
    return {};
  }
  return {this->dataPtr->raySensor};
}

// Simplified function to only process point cloud
void MultibeamSonarSensor::Implementation::OnNewFrame(
  const float * _scan, unsigned int _width, unsigned int _height, unsigned int _channels,
  const std::string & /*_format*/)
{
  if (!this->ros_node_ || !rclcpp::ok(this->ros_node_->get_node_base_interface()->get_context()))
  {
    return;
  }
  // Lock the ray buffer for thread safety
  std::lock_guard<std::mutex> lock(this->rayMutex);

  // A dimension mismatch must not resize the renderer's immutable acoustic
  // model or overrun the frame buffer while the compute thread uses a snapshot.
  if (
    _width != this->azimuth_angles.size() || _height != this->elevation_angles.size() ||
    _channels != this->kChannelCount)
  {
    gzerr << "Sonar render frame dimensions differ from the initialized readout." << std::endl;
    return;
  }

  // Total number of points in the scan
  unsigned int samples = _width * _height * _channels;
  unsigned int rayBufferSize = samples * sizeof(float);

  this->lastMeasurementTime = static_cast<double>(this->renderStamp) * 1e-9;

  // Allocate memory for the ray buffer if not already allocated
  if (this->rayBuffer.size() != samples)
  {
    this->rayBuffer.resize(samples);
  }

  // Copy the incoming scan data into the ray buffer
  memcpy(this->rayBuffer.data(), _scan, rayBufferSize);

  // Fill point cloud with the ray buffer
  this->FillPointCloudMsg(this->rayBuffer.data());

  // Signals the background compute thread to process the new frame
  if (this->pointCloudImage.size().width != 0)
  {
    {
      std::lock_guard<std::mutex> lk(this->computeMutex_);
      this->newFrameReady_ = true;
    }
    this->computeCV_.notify_one();
  }
}

void MultibeamSonarSensor::Implementation::OnNewDepthFrame(
  const float * _scan, unsigned int _width, unsigned int _height, unsigned int _channels,
  const std::string & /*_format*/)
{
  std::lock_guard<std::mutex> lock(this->rayMutex);
  if (
    !this->perspectiveGeometry || _channels != 4 ||
    _width != static_cast<unsigned>(this->perspectiveGeometry->columns) ||
    _height != static_cast<unsigned>(this->perspectiveGeometry->rows))
  {
    gzerr << "Direct sonar geometry dimensions differ from the initialized readout." << std::endl;
    return;
  }
  cv::Mat points, ranges;
  ReadPerspectiveSonarFrame(
    _scan, *this->perspectiveGeometry, this->minimumRange,
    std::min(this->maximumRange, this->maxDistance), points, ranges);
  std::string * buffer = this->pointMsg.mutable_data();
  buffer->assign(static_cast<size_t>(this->pointMsg.row_step()) * _height, '\0');
  bool dense = true;
  for (unsigned r = 0; r < _height; ++r)
  {
    for (unsigned c = 0; c < _width; ++c)
    {
      const auto p = points.at<cv::Vec3f>(r, c);
      dense = dense && std::isfinite(p[0]);
      char * destination = buffer->data() + static_cast<size_t>(r) * this->pointMsg.row_step() +
                           c * this->pointMsg.point_step();
      for (int k = 0; k < 3; ++k)
      {
        std::memcpy(destination + this->pointMsg.field(k).offset(), &p[k], sizeof(float));
      }
      // Geometry has no measured acoustic intensity; RGB is never used as one.
      const uint16_t ring = r;
      std::memcpy(destination + this->pointMsg.field(4).offset(), &ring, sizeof(ring));
    }
  }
  this->pointMsg.set_is_dense(dense);
  {
    std::lock_guard<std::mutex> snapshotLock(this->lock_);
    this->pointCloudImage = std::move(ranges);
    this->measuredPoints = std::move(points);
    this->physicalSnapshotStamp = this->physicalRenderStamp;
  }
  {
    std::lock_guard<std::mutex> computeLock(this->computeMutex_);
    this->newFrameReady_ = true;
  }
  this->computeCV_.notify_one();
}

/////////////////////////////////////////////////
void MultibeamSonarSensor::SetScene(gz::rendering::ScenePtr _scene)
{
  if (this->Scene() == _scene)
  {
    return;
  }
  this->Reset();
  RenderingSensor::SetScene(_scene);
  if (_scene && this->dataPtr->sensorSdf && !this->dataPtr->Initialize(this))
  {
    gzerr << "Failed to initialize [" << this->Name() << "] sensor." << std::endl;
  }
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::SetWorldState(const WorldState & _state)
{
  this->dataPtr->worldState = &_state;
}
//////////////////////////////////////////////////
void MultibeamSonarSensor::SetEnvironmentalData(const EnvironmentalData & _data) {}

//////////////////////////////////////////////////
void MultibeamSonarSensor::SetEntity(uint64_t _entityId) { this->dataPtr->entityId = _entityId; }

//////////////////////////////////////////////////
bool MultibeamSonarSensor::Update(const std::chrono::steady_clock::duration & _now)
{
  GZ_PROFILE("MultibeamSonarSensor::Update");
  if (!this->dataPtr->initialized || this->dataPtr->entityId == 0)
  {
    gzerr << "Not initialized, update ignored." << std::endl;
    return false;
  }

  if (!this->dataPtr->raySensor && !this->dataPtr->depthSensor)
  {
    gzerr << "Geometry sensor for Multibeam Sonar does not exist.\n";
    return false;
  }

  if (this->dataPtr->pointPub.HasConnections())
  {
    this->dataPtr->publishingPointCloud = true;
  }

  // Capture simulation time before the render callback produces this frame.
  {
    std::lock_guard<std::mutex> lock(this->dataPtr->rayMutex);
    this->dataPtr->renderStamp = std::chrono::duration_cast<std::chrono::nanoseconds>(_now).count();
    this->dataPtr->physicalRenderStamp = this->dataPtr->renderStamp;
  }
  // Generate sensor data
  this->Render();

  return true;
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::PostUpdate(const std::chrono::steady_clock::duration & _now)
{
  GZ_PROFILE("MultibeamSonarSensor::PostUpdate");

  if (!this->dataPtr->worldState)
  {
    gzwarn << "No world state available, "
           << "cannot estimate velocities." << std::endl;
    return;
  }
  if (this->dataPtr->ros_executor_)
  {
    this->dataPtr->ros_executor_->spin_some();
  }

  if (this->dataPtr->publishingPointCloud)
  {
    // Set the time stamp
    *this->dataPtr->pointMsg.mutable_header()->mutable_stamp() = msgs::Convert(_now);
    // Set frame_id
    for (auto i = 0; i < this->dataPtr->pointMsg.mutable_header()->data_size(); ++i)
    {
      if (
        this->dataPtr->pointMsg.mutable_header()->data(i).key() == "frame_id" &&
        this->dataPtr->pointMsg.mutable_header()->data(i).value_size() > 0)
      {
        this->dataPtr->pointMsg.mutable_header()->mutable_data(i)->set_value(0, this->FrameId());
      }
    }

    // For the point cloud visualization in gazebo
    // https://github.com/gazebosim/gz-gui/pull/346
    {
      this->AddSequence(this->dataPtr->pointMsg.mutable_header());
      GZ_PROFILE("MultibeamSonarSensor::Update Publish point cloud");
      this->dataPtr->pointPub.Publish(this->dataPtr->pointMsg);
    }
  }
}

//////////////////////////////////////////////////
bool MultibeamSonarSensor::HasConnections() const
{
  return this->dataPtr->pointPub && this->dataPtr->pointPub.HasConnections();
}

//////////////////////////////////////////////////
void MultibeamSonarSensor::Implementation::FillPointCloudMsg(const float * _rayBuffer)
{
  uint32_t width = this->pointMsg.width();
  uint32_t height = this->pointMsg.height();
  unsigned int channels = 3;

  float angleStep = (this->raySensor->AngleMax() - this->raySensor->AngleMin()).Radian() /
                    (this->raySensor->RangeCount() - 1);

  float verticleAngleStep =
    (this->raySensor->VerticalAngleMax() - this->raySensor->VerticalAngleMin()).Radian() /
    (this->raySensor->VerticalRangeCount() - 1);

  // Angles of ray currently processing, azimuth is horizontal, inclination
  // is vertical
  float inclination = this->raySensor->VerticalAngleMin().Radian();

  std::string * msgBuffer = this->pointMsg.mutable_data();
  msgBuffer->resize(this->pointMsg.row_step() * this->pointMsg.height());
  char * msgBufferIndex = msgBuffer->data();
  // Set Pointcloud as dense. Change if invalid points are found.
  bool isDense{true};
  // Iterate over scan and populate point cloud
  for (uint32_t j = 0; j < height; ++j)
  {
    float azimuth = this->raySensor->AngleMin().Radian();

    for (uint32_t i = 0; i < width; ++i)
    {
      // Index of current point, and the depth value at that point
      auto index = j * width * channels + i * channels;
      float depth = _rayBuffer[index];
      // Validate Depth/Radius and update pointcloud density flag
      if (isDense)
      {
        isDense = !(gz::math::isnan(depth) || std::isinf(depth));
      }

      float intensity = _rayBuffer[index + 1];
      uint16_t ring = j;

      int fieldIndex = 0;

      // Convert spherical coordinates to Cartesian for pointcloud
      // See https://en.wikipedia.org/wiki/Spherical_coordinate_system
      *reinterpret_cast<float *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        depth * std::cos(inclination) * std::cos(azimuth);

      *reinterpret_cast<float *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        depth * std::cos(inclination) * std::sin(azimuth);

      *reinterpret_cast<float *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        depth * std::sin(inclination);

      // Intensity
      *reinterpret_cast<float *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        intensity;

      // Ring
      *reinterpret_cast<uint16_t *>(msgBufferIndex + this->pointMsg.field(fieldIndex++).offset()) =
        ring;

      // Move the index to the next point.
      msgBufferIndex += this->pointMsg.point_step();

      azimuth += angleStep;
    }
    inclination += verticleAngleStep;
  }
  this->pointMsg.set_is_dense(isDense);

  // After filling pointMsg, compute pointCloudImage as well
  std::lock_guard<std::mutex> lock(this->lock_);
  this->snapshotStamp = this->renderStamp;
  this->pointCloudImage.create(height, width, CV_32FC1);
  cv::MatIterator_<float> iter_image = this->pointCloudImage.begin<float>();

  for (uint32_t j = 0; j < height; ++j)
  {
    for (uint32_t i = 0; i < width; ++i, ++iter_image)
    {
      // Index in _rayBuffer
      auto index = j * width * channels + i * channels;
      float depth = _rayBuffer[index];

      float range = std::isfinite(depth) ? depth : 100000.0f;
      *iter_image = range;

      if (!std::isfinite(*iter_image) || std::isnan(*iter_image))
      {
        *iter_image = 100000.0f;
      }
    }
  }
}

cv::Mat MultibeamSonarSensor::Implementation::ComputeNormalImage(cv::Mat & depth)
{
  return gz::sensors::RaySurfaceNormals(
    depth, this->azimuth_angles, this->elevation_angles, this->maxDistance);
}

// Precalculation of corrector sonar calculation
void MultibeamSonarSensor::Implementation::ComputeCorrector(int _snapshotWidth, int _nBeams)
{
  if (_snapshotWidth <= 1 || _nBeams <= 0)
  {
    return;
  }

  double hPixelSize = this->hFOV / (_snapshotWidth - 1);

  // Beam culling correction precalculation
  for (int beam = 0; beam < _nBeams; beam++)
  {
    for (int beam_other = 0; beam_other < _nBeams; beam_other++)
    {
      float azimuthBeamPattern = unnormalized_sinc(
        M_PI * 0.884 / hPixelSize *
        sin(this->azimuth_angles[beam] - this->azimuth_angles[beam_other]));
      this->beamCorrector[beam][beam_other] = abs(azimuthBeamPattern);
      this->beamCorrectorSum += pow(azimuthBeamPattern, 2);
    }
  }
  this->beamCorrectorSum = sqrt(this->beamCorrectorSum);
}

// Compute sonar image from point cloud
void MultibeamSonarSensor::Implementation::ComputeSonarImage()

{
  // Snapshot the depth image quickly, then release the lock so the render
  // thread can fill the next frame without waiting for GPU compute.
  cv::Mat depthSnapshot, pointsSnapshot;
  // Capture the timestamp and image dimensions while holding the lock so that
  // all published messages share one coherent stamp matching when the data was
  // recorded -- mismatched stamps are the primary cause of RViz flicker.
  rclcpp::Time capturedStamp;
  {
    std::lock_guard<std::mutex> snapshot_lk(this->lock_);
    if (this->pointCloudImage.empty())
    {
      return;
    }
    depthSnapshot = this->pointCloudImage.clone();
    capturedStamp = rclcpp::Time(this->snapshotStamp, RCL_ROS_TIME);
    if (this->perspectiveGeometry)
    {
      pointsSnapshot = this->measuredPoints.clone();
      capturedStamp = rclcpp::Time(this->physicalSnapshotStamp, RCL_ROS_TIME);
    }

    if (!this->physicalReadout && this->beamCorrectorSum == 0)
    {
      ComputeCorrector(depthSnapshot.cols, this->nBeams);
    }

    if (this->reflectivityImage.rows == 0)
    {
      this->reflectivityImage =
        cv::Mat(depthSnapshot.rows, depthSnapshot.cols, CV_32FC1, cv::Scalar(this->mu));
    }
  }
  // lock_ released -> render thread can proceed immediately.

  // NOTE: reflectivityImage is used here without a lock.
  // This is safe for now because only the compute thread writes to it
  // and the render thread only reads it. If the render thread ever starts modifying
  // reflectivityImage, this will cause a data race.
  // TODO: Either guarantee that reflectivityImage stays read-only
  // after initialization, or add proper locking around the compute step.

  // Use snapshot dimensions from here on to avoid racing with the render thread
  // that may be writing pointMsg concurrently.
  const int snapWidth = depthSnapshot.cols;
  const int snapHeight = depthSnapshot.rows;

  double vPixelSize = this->vFOV / (snapHeight - 1);
  double hPixelSize = this->hFOV / (snapWidth - 1);

  cv::Mat normal_image = this->perspectiveGeometry
                           ? CartesianSurfaceNormals(
                               pointsSnapshot, depthSnapshot,
                               this->perspectiveGeometry->normalCalibration, this->maxDistance)
                           : this->ComputeNormalImage(depthSnapshot);

  auto start = std::chrono::high_resolution_clock::now();

  // ------------------------------------------------//
  // --------      Sonar calculations       -------- //
  // ------------------------------------------------//

  SonarComputeInput input;
  input.depthImage = &depthSnapshot;
  input.physicalReadout = this->physicalReadout.get();
  input.receiveGain = this->receiveGain;
  input.normalImage = &normal_image;
  input.reflectivityImage = &this->reflectivityImage;
  input.nBeams = this->nBeams;
  input.nRays = this->nRays;
  input.nFreq = this->spectrumSamples;
  input.nOutputRanges = this->physicalReadout ? this->nFreq : 0;
  input.raySkips = this->raySkips;
  input.hFOV = this->hFOV;
  input.vFOV = this->vFOV;
  input.maxDistance = this->maxDistance;
  input.soundSpeed = this->soundSpeed;
  input.attenuation = this->attenuation;
  input.sensorGain = this->sensorGain;
  input.debugFlag = this->debugFlag;
  input.blazingFlag = this->blazingFlag;
  input.window = this->window.data();
  input.rangeVector = this->rangeVector.data();
  input.beamCorrector = this->beamCorrector.data();
  input.beamCorrectorSum = this->beamCorrectorSum;
  input.sourceLevel = this->sourceLevel;
  input.bandwidth = this->bandwidth;
  input.sonarFreq = this->sonarFreq;
  input.elevation_angles = this->elevation_angles;
  input.frameIndex = this->frameCounter++;
  input.seed = this->sonarSeed;

  SonarComputeOutput backendOutput;
  if (!this->computeBackend->Compute(input, backendOutput))
  {
    RCLCPP_ERROR_STREAM(
      this->ros_node_->get_logger(),
      "Sonar compute backend failed: " << this->computeBackend->Name());
    return;
  }

  CArray2D P_Beams(CArray(this->nFreq), this->nBeams);
  for (int beam = 0; beam < this->nBeams; ++beam)
  {
    for (int freq = 0; freq < this->nFreq; ++freq)
    {
      P_Beams[beam][freq] = backendOutput.At(beam, freq);
    }
  }

  // For calc time measure
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  if (debugFlag)
  {
    double timeMs = duration.count() / 1000.0;
    RCLCPP_INFO_STREAM(
      this->ros_node_->get_logger(), this->computeBackend->Name()
                                       << " Sonar Frame Calc Time " << std::fixed
                                       << std::setprecision(3) << timeMs << " [ms]\n");
  }

  // CSV log write stream
  // Each cols corresponds to each beams
  if (this->writeLogFlag)
  {
    this->writeCounter = this->writeCounter + 1;
    if (this->writeCounter == 1 || this->writeCounter % this->writeInterval == 0)
    {
      const double time = capturedStamp.seconds();
      std::stringstream filename;
      filename << this->logDirectory << "/SonarRawData_" << std::setw(6) << std::setfill('0')
               << this->writeNumber << ".csv";
      writeLog.open(filename.str().c_str(), std::ios_base::app);
      filename.clear();
      writeLog << "# Raw Sonar Data Log (Row: beams, Col: time series data)\n";
      writeLog << "# First column is range vector\n";
      writeLog << "#  nBeams : " << this->nBeams << "\n";
      writeLog << "# Simulation time : " << time << "\n";
      for (size_t i = 0; i < P_Beams[0].size(); i++)
      {
        // writing range vector at first column
        writeLog << this->rangeVector[i];
        for (size_t b = 0; b < this->nBeams; b++)
        {
          writeLog << ',';
          posim::sonar::WriteComplexCsvValue(writeLog, P_Beams[b][i]);
        }
        writeLog << "\n";
      }
      writeLog.close();

      // write beam (azimuth) angles
      if (this->writeNumber == 1)
      {
        std::stringstream filename_angle;
        filename_angle << this->logDirectory << "/SonarRawData_beam_angles.csv";
        writeLog.open(filename_angle.str().c_str(), std::ios_base::app);
        filename_angle.clear();
        writeLog << "# Raw Sonar Data Log \n";
        writeLog << "# Beam (azimuth) angles of rays\n";
        writeLog << "#  nBeams : " << nBeams << "\n";
        writeLog << "# Simulation time : " << time << "\n";
        for (size_t i = 0; i < this->receiveAngles.size(); i++)
        {
          writeLog << this->receiveAngles[i] << "\n";
        }
        writeLog.close();
      }

      this->writeNumber = this->writeNumber + 1;
    }
  }

  // Use the single timestamp captured at snapshot time for all publications.
  // This ensures sonarRawDataMsg, sonarImgMsg, and normalImgMsg all carry the
  // same stamp - the sim time when the depth frame was actually recorded -
  // so RViz TF lookups succeed and images don't flicker.

  // FEEDBACK_NEEDED/TODO: Solved?
  // Using frameName (from SDF <frameName> tag) as the ROS frame_id.
  // This is user-friendly but requires the SDF value to match an actual TF frame exactly.
  // The old approach used frameId (Gazebo's internal sensor ID) which was more likely
  // to match the TF tree automatically.
  // If TF lookup errors appear in RViz, shall revert back to frameId.
  this->sonarRawDataMsg = marine_acoustic_msgs::msg::ProjectedSonarImage();

  this->sonarRawDataMsg.header.frame_id = this->frameId;

  this->sonarRawDataMsg.header.frame_id = this->frameName;

  this->sonarRawDataMsg.header.stamp.sec = static_cast<int32_t>(capturedStamp.seconds());
  this->sonarRawDataMsg.header.stamp.nanosec =
    static_cast<uint32_t>(capturedStamp.nanoseconds() % 1000000000);

  marine_acoustic_msgs::msg::PingInfo ping_info_msg_;

  ping_info_msg_.frequency = this->sonarFreq;
  ping_info_msg_.sound_speed = this->soundSpeed;
  for (size_t beam = 0; beam < this->nBeams; beam++)
  {
    ping_info_msg_.rx_beamwidths.push_back(
      this->physicalReadout ? this->physicalReadout->powerBeamwidth
                            : static_cast<float>(hFOV / (this->nBeams - 1)));
    ping_info_msg_.tx_beamwidths.push_back(static_cast<float>(vFOV));
  }

  this->sonarRawDataMsg.ping_info = ping_info_msg_;

  for (size_t beam = 0; beam < this->nBeams; beam++)
  {
    geometry_msgs::msg::Vector3 beam_direction;
    beam_direction.x = cos(this->receiveAngles[beam]);
    beam_direction.y = sin(this->receiveAngles[beam]);
    beam_direction.z = 0.0;
    this->sonarRawDataMsg.beam_directions.push_back(beam_direction);
  }

  std::vector<float> ranges;
  for (size_t i = 0; i < P_Beams[0].size(); i++)
  {
    ranges.push_back(rangeVector[i]);
  }

  this->sonarRawDataMsg.ranges = ranges;
  marine_acoustic_msgs::msg::SonarImageData sonar_image_data;
  sonar_image_data.is_bigendian = false;
  sonar_image_data.dtype = marine_acoustic_msgs::msg::SonarImageData::DTYPE_FLOAT32;
  sonar_image_data.beam_count = this->nBeams;
  // this->sonar_image_raw_msg_.data_size = 1;  // sizeof(float) * nFreq * nBeams;
  std::vector<float> intensities;
  // int Intensity[this->nBeams][this->nFreq];

  for (size_t r = 0; r < P_Beams[0].size(); r++)
  {
    for (size_t beam = 0; beam < this->nBeams; beam++)
    {
      float power_dB = 20.0f * log10(std::abs(P_Beams[beam][r]));
      intensities.push_back(power_dB);
    }
  }
  sonar_image_data.data.resize(intensities.size() * sizeof(float));
  memcpy(sonar_image_data.data.data(), intensities.data(), intensities.size() * sizeof(float));
  this->sonarRawDataMsg.image = sonar_image_data;
  this->sonarImageRawPub->publish(this->sonarRawDataMsg);

  // Fan coordinates are metric (one range bin per pixel), not beam-index
  // coordinates. Keep the full lateral extent and the zero-range origin inside
  // the raster, independently of the number of horizontal beams.
  const unsigned int radius = this->nFreq;
  cv::Mat Intensity_image = cv::Mat::zeros(cv::Size(2 * radius + 1, radius + 1), CV_8UC1);

  const float rangeMax = this->maxDistance;
  const float rangeRes = ranges[1] - ranges[0];
  const int nEffectiveRanges = ceil(rangeMax / rangeRes);
  const cv::Point origin(radius, radius);
  const float binThickness = 2 * ceil(radius / nEffectiveRanges);

  struct BearingEntry
  {
    float begin, center, end;
    BearingEntry(float b, float c, float e) : begin(b), center(c), end(e) { ; }
  };

  std::vector<BearingEntry> angles;
  angles.reserve(this->nBeams);

  for (int b = 0; b < this->nBeams; ++b)
  {
    const float center = this->receiveAngles[b];
    float begin = 0.0, end = 0.0;
    if (b == 0)
    {
      end = (this->receiveAngles[b + 1] + center) / 2.0;
      begin = 2 * center - end;
    }
    else if (b == this->nBeams - 1)
    {
      begin = angles[b - 1].end;
      end = 2 * center - begin;
    }
    else
    {
      begin = angles[b - 1].end;
      end = (this->receiveAngles[b + 1] + center) / 2.0;
    }
    angles.push_back(BearingEntry(begin, center, end));
  }

  const float ThetaShift = 1.5 * M_PI;

  float max_power = 0.0f;

  for (size_t r = 0; r < P_Beams[0].size(); ++r)
  {
    for (size_t b = 0; b < this->nBeams; ++b)
    {
      float power = std::abs(P_Beams[b][r]);
      if (power > max_power)
      {
        max_power = power;
      }
    }
  }

  float max_dB = 20.0f * log10(max_power + 1e-10f);
  float min_dB = max_dB - 60.0f;

  // With no returns, the per-bin and maximum powers share the epsilon floor.
  // Normalizing it would draw a false full-white fan. Keep the black image
  // instead; the raw -inf samples and positive-return rendering are unchanged.
  for (int r = 0; max_power > 0.0f && r < ranges.size(); ++r)
  {
    // The last bin center may lie just beyond maxDistance. Render its
    // overlapping half-cell at the outer edge without relabeling raw ranges.
    if (ranges[r] - 0.5f * rangeRes > rangeMax)
    {
      continue;
    }
    for (int b = 0; b < this->nBeams; ++b)
    {
      const float range = ranges[r];
      const float power = std::abs(P_Beams[this->nBeams - 1 - b][r]);
      // No echo must stay black, including a frame with no finite returns.
      // The logarithm epsilon otherwise maps an all-zero frame to full white.
      const float power_dB =
        power > 0.0f && std::isfinite(power) ? 20.0f * log10(power + 1e-10f) : min_dB;
      int intensity = static_cast<int>(255.0f * (power_dB - min_dB) / (max_dB - min_dB));
      const float begin = angles[b].begin + ThetaShift, end = angles[b].end + ThetaShift;
      const float rad = static_cast<float>(radius) * std::min(range, rangeMax) / rangeMax;
      // Assume angles are in image frame x-right, y-down
      cv::ellipse(
        Intensity_image, origin, cv::Size(rad, rad), 0.0, begin * 180.0 / M_PI, end * 180.0 / M_PI,
        intensity, binThickness);
    }
  }

  cv::Mat Itensity_image_color;
  cv::applyColorMap(Intensity_image, Itensity_image_color, cv::COLORMAP_HOT);

  this->sonarImgMsg.header.frame_id = this->frameName;
  this->sonarImgMsg.header.stamp.sec = static_cast<int32_t>(capturedStamp.seconds());
  this->sonarImgMsg.header.stamp.nanosec =
    static_cast<uint32_t>(capturedStamp.nanoseconds() % 1000000000);

  CopyImageToMessage(Itensity_image_color, sensor_msgs::image_encodings::BGR8, this->sonarImgMsg);
  this->sonarImagePub->publish(this->sonarImgMsg);

  this->normalImgMsg.header.frame_id = this->frameName;
  this->normalImgMsg.header.stamp.sec = static_cast<int32_t>(capturedStamp.seconds());
  this->normalImgMsg.header.stamp.nanosec =
    static_cast<uint32_t>(capturedStamp.nanoseconds() % 1000000000);

  cv::Mat normal_image8;
  normal_image.convertTo(normal_image8, CV_8UC3, 255.0);
  CopyImageToMessage(normal_image8, sensor_msgs::image_encodings::RGB8, this->normalImgMsg);
  this->normalImagePub->publish(this->normalImgMsg);

  // ---------------------------------------- End of sonar calculation
}

}  // namespace sensors
}  // namespace gz
