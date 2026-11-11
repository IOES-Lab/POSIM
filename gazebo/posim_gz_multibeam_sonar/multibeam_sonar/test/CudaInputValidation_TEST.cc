#include "CudaInputValidation.hh"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
using gz::sensors::SonarComputeInput;
using NpsGazeboSonar::CudaImageStorageMatches;
using NpsGazeboSonar::TryCudaCalculation;

void Require(bool condition, const char * message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

struct Fixture
{
  cv::Mat depth{2, 3, CV_32FC1, cv::Scalar(1)};
  cv::Mat normals{2, 3, CV_32FC3, cv::Scalar(0, 0, 1)};
  cv::Mat reflectivity{2, 3, CV_32FC1, cv::Scalar(1)};
  float window[4] = {1, 1, 1, 1};
  float correction[3][3] = {};
  float * correctionRows[3] = {correction[0], correction[1], correction[2]};
  SonarComputeInput input;

  Fixture()
  {
    input.depthImage = &depth;
    input.normalImage = &normals;
    input.reflectivityImage = &reflectivity;
    input.nBeams = 3;
    input.nRays = 2;
    input.nFreq = 4;
    input.raySkips = 1;
    input.window = window;
    input.beamCorrector = correctionRows;
    input.elevation_angles = {0, 1};
  }
};

void RequireRejected(const SonarComputeInput & input)
{
  int calls = 0;
  Require(!TryCudaCalculation(input, [&] { ++calls; }), "invalid input returned success");
  Require(calls == 0, "invalid input reached CUDA calculation");
}
}  // namespace

int main()
{
  try
  {
    Fixture fixture;
    int calls = 0;
    Require(TryCudaCalculation(fixture.input, [&] { ++calls; }), "valid continuous input");
    Require(calls == 1, "valid input should reach calculation once");

    cv::Mat sensorDepth(32, 32, CV_32FC1, cv::Scalar(1));
    cv::Mat sensorNormals(32, 32, CV_32FC3, cv::Scalar(0, 0, 1));
    cv::Mat sensorReflectivity(32, 32, CV_32FC1, cv::Scalar(1));
    std::vector<float> sensorWindow(400, 1);
    std::vector<float> sensorCorrection(32 * 32, 0);
    std::vector<float *> sensorRows(32);
    SonarComputeInput sensor = fixture.input;
    for (std::size_t beam = 0; beam < sensorRows.size(); ++beam)
    {
      sensorRows[beam] = sensorCorrection.data() + beam * 32;
    }
    sensor.depthImage = &sensorDepth;
    sensor.normalImage = &sensorNormals;
    sensor.reflectivityImage = &sensorReflectivity;
    sensor.nBeams = sensor.nRays = 32;
    sensor.nFreq = 400;
    sensor.raySkips = 10;
    sensor.window = sensorWindow.data();
    sensor.beamCorrector = sensorRows.data();
    sensor.elevation_angles.resize(32);
    Require(TryCudaCalculation(sensor, [] {}), "fixed 32/32/10/400 sensor buffers rejected");

    for (int value : {0, -1, 1, std::numeric_limits<int>::min()})
    {
      auto input = fixture.input;
      input.nBeams = value;
      RequireRejected(input);
      input = fixture.input;
      input.nRays = value;
      RequireRejected(input);
    }
    for (int value : {0, -1, std::numeric_limits<int>::min()})
    {
      auto input = fixture.input;
      input.nFreq = value;
      RequireRejected(input);
      input = fixture.input;
      input.raySkips = value;
      RequireRejected(input);
    }
    auto input = fixture.input;
    input.nFreq = std::numeric_limits<int>::max();
    RequireRejected(input);
    input = fixture.input;
    input.nBeams = 46341;
    input.nFreq = 1;
    input.raySkips = 2;
    RequireRejected(input);

    for (int skip : {2, 3, std::numeric_limits<int>::max()})
    {
      input = fixture.input;
      input.raySkips = skip;
      Require(TryCudaCalculation(input, [] {}), "skip >= rays remains supported");
    }
    for (std::size_t count : {std::size_t(0), std::size_t(1)})
    {
      input = fixture.input;
      input.elevation_angles.resize(count);
      RequireRejected(input);
    }
    input = fixture.input;
    input.elevation_angles.push_back(2);
    Require(TryCudaCalculation(input, [] {}), "extra elevation entries are harmless");
    input = fixture.input;
    input.depthImage = nullptr;
    RequireRejected(input);
    input = fixture.input;
    input.normalImage = nullptr;
    RequireRejected(input);
    input = fixture.input;
    input.reflectivityImage = nullptr;
    RequireRejected(input);
    input = fixture.input;
    input.window = nullptr;
    RequireRejected(input);
    input = fixture.input;
    input.beamCorrector = nullptr;
    RequireRejected(input);
    fixture.correctionRows[2] = nullptr;
    RequireRejected(fixture.input);
    fixture.correctionRows[2] = fixture.correction[2];

    cv::Mat empty;
    cv::Mat wrongShape(3, 3, CV_32FC1);
    cv::Mat wrongType(2, 3, CV_64FC1);
    const int dimensions[] = {2, 3, 1};
    cv::Mat wrongDimensions(3, dimensions, CV_32FC1);
    for (const cv::Mat * image : {&empty, &wrongShape, &wrongType, &wrongDimensions})
    {
      input = fixture.input;
      input.depthImage = image;
      RequireRejected(input);
    }
    input = fixture.input;
    input.normalImage = &fixture.depth;
    RequireRejected(input);
    input = fixture.input;
    input.reflectivityImage = &fixture.normals;
    RequireRejected(input);

    cv::Mat backing(3, 4, CV_32FC1);
    cv::Mat bottomRight = backing(cv::Rect(1, 1, 3, 2));
    Require(!bottomRight.isContinuous(), "multirow ROI must have padding");
    Require(
      bottomRight.step * bottomRight.rows >
        static_cast<std::size_t>(bottomRight.dataend - bottomRight.data),
      "ROI must reproduce the original copy overread");
    input = fixture.input;
    input.depthImage = &bottomRight;
    RequireRejected(input);

    cv::Mat lastRow = backing(cv::Rect(1, 2, 3, 1));
    Require(lastRow.isContinuous(), "one-row ROI reports continuous despite larger step");
    Require(!CudaImageStorageMatches(lastRow, 3, 1, CV_32FC1), "single-row tail overread");
    std::vector<float> paddedStorage(7);
    cv::Mat padded(2, 3, CV_32FC1, paddedStorage.data(), 4 * sizeof(float));
    input.depthImage = &padded;
    RequireRejected(input);

    cv::Mat fullWidthBacking(3, 3, CV_32FC1);
    cv::Mat packedRoi = fullWidthBacking(cv::Rect(0, 1, 3, 2));
    input.depthImage = &packedRoi;
    Require(TryCudaCalculation(input, [] {}), "safe tightly packed ROI rejected");
    cv::Mat cloned = bottomRight.clone();
    input.depthImage = &cloned;
    Require(TryCudaCalculation(input, [] {}), "packed clone rejected");

    cv::Mat normalBacking(3, 4, CV_32FC3);
    cv::Mat normalRoi = normalBacking(cv::Rect(1, 1, 3, 2));
    input = fixture.input;
    input.normalImage = &normalRoi;
    RequireRejected(input);
    input = fixture.input;
    input.reflectivityImage = &bottomRight;
    RequireRejected(input);

    calls = 0;
    Require(
      !TryCudaCalculation(
        fixture.input,
        [&]
        {
          ++calls;
          throw std::invalid_argument("wrapper validation");
        }),
      "wrapper invalid_argument escaped");
    Require(calls == 1, "wrapper called more than once");
    Require(
      !TryCudaCalculation(fixture.input, [] { throw std::overflow_error("wrapper bounds"); }),
      "wrapper overflow_error escaped");
    bool unexpectedPropagated = false;
    try
    {
      TryCudaCalculation(fixture.input, [] { throw std::runtime_error("unexpected error"); });
    }
    catch (const std::runtime_error &)
    {
      unexpectedPropagated = true;
    }
    Require(unexpectedPropagated, "unexpected errors must not be silently swallowed");
    std::cout << "PASS: CUDA host input gate, no-call rejection, ROI bounds and clean failure\n";
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
