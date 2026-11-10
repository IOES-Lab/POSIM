// SPDX-License-Identifier: Apache-2.0
#include "PerspectiveSonarGeometry.hh"
#include "PhysicalPulse.hh"
#include "PhysicalSonarInput.hh"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>

using namespace gz::sensors;
namespace
{
void Require(bool condition, const std::string & message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

double Energy(const SonarComputeOutput & value)
{
  double sum = 0;
  for (auto sample : value.beamSpectrum)
  {
    Require(std::isfinite(sample.real()) && std::isfinite(sample.imag()), "nonfinite output");
    sum += std::norm(std::complex<double>(sample));
  }
  return sum;
}

void Geometry()
{
  for (int horizontal : {1, 5, 17, 65})
  {
    for (int vertical : {1, 6, 7})
    {
      for (int stride : {1, 2, 4, 8})
      {
        auto model = MakeAcousticReadout(
          AngularCenters(-.3, .3, horizontal), AngularCenters(-.1, .1, vertical), -.3, .3, -.1, .1,
          stride, {0}, .04);
        const double area =
          std::accumulate(model.angularAreas.begin(), model.angularAreas.end(), 0.);
        Require(std::abs(area / (.6 * 2 * std::sin(.1)) - 1) < 2e-7, "solid angle not conserved");
        Require(
          model.elevationIndices.front() == 0 && model.elevationIndices.back() == vertical - 1,
          "endpoint omitted by stride");
      }
    }
  }
  auto beam = MakeAcousticReadout({0.f, .02f, .08f}, {0.f}, -.1, .1, -.1, .1, 1, {0.f}, .04);
  Require(std::abs(beam.pressureResponse[0] - 1) < 1e-7, "on-axis pressure gain");
  Require(
    std::abs(beam.pressureResponse[1] * beam.pressureResponse[1] - .5) < 1e-6,
    "specified synthetic half-power width");
  Require(beam.pressureResponse[2] < 0, "side-lobe phase lost");
}

struct Scene
{
  cv::Mat depth, normal, reflectivity;
  AcousticReadout model;
  std::vector<float> window;
  SonarComputeInput input;
  Scene(int columns, int receivers, int frequencies, int stride)
  : depth(7, columns, CV_32F),
    normal(7, columns, CV_32FC3),
    reflectivity(7, columns, CV_32F),
    model(MakeAcousticReadout(
      AngularCenters(-.3, .3, columns), AngularCenters(-.1, .1, 7), -.3, .3, -.1, .1, stride,
      AngularCenters(-.25, .25, receivers), .08)),
    window(frequencies)
  {
    for (int r = 0; r < 7; ++r)
    {
      for (int c = 0; c < columns; ++c)
      {
        depth.at<float>(r, c) = 4.f + .13f * r + .01f * c;
        normal.at<cv::Vec3f>(r, c) = {0, 0, .8f};
        reflectivity.at<float>(r, c) = .3f + .03f * ((r + c) % 5);
      }
    }
    for (int f = 0; f < frequencies; ++f)
    {
      window[f] =
        (.6f - .4f * std::cos(2.f * float(M_PI) * f / frequencies)) / std::sqrt(float(frequencies));
    }
    input.depthImage = &depth;
    input.normalImage = &normal;
    input.reflectivityImage = &reflectivity;
    input.nBeams = receivers;
    input.nRays = 7;
    input.nFreq = frequencies;
    input.maxDistance = 14;  // below the shortest control transform period
    input.soundSpeed = 1500;
    input.bandwidth = 3200;
    input.sourceLevel = 220;
    input.attenuation = .02;
    input.physicalReadout = &model;
    input.window = window.data();
    input.seed = 73;
    input.frameIndex = (uint64_t(1) << 33) + 20;
  }
};

SonarComputeOutput Compute(ComputeBackend & backend, const SonarComputeInput & input)
{
  SonarComputeOutput output;
  Require(backend.Compute(input, output), "compute rejected valid physical input");
  const int count = input.nOutputRanges ? input.nOutputRanges : input.nFreq;
  Require(
    output.nBeams == input.nBeams && output.nFreq == count &&
      output.beamSpectrum.size() == size_t(input.nBeams) * count,
    "wrong receiver shape");
  Energy(output);
  return output;
}

void Controls(ComputeBackend & backend)
{
  Scene scene(5, 3, 64, 4);
  scene.depth.setTo(0);
  scene.depth.at<float>(6, 2) = 5;  // last endpoint is selected despite stride 4
  auto base = Compute(backend, scene.input);
  const double energy = Energy(base);
  Require(energy > 0, "non-divisible endpoint lost");
  scene.input.receiveGain = 2;
  Require(std::abs(Energy(Compute(backend, scene.input)) / energy - 4) < .003, "receive gain");
  scene.input.receiveGain = 1;
  scene.depth.setTo(0);
  scene.depth.at<float>(3, 2) = 5;
  Require(Energy(Compute(backend, scene.input)) == 0, "unselected elevation row contributed");
  scene.depth.setTo(std::numeric_limits<float>::quiet_NaN());
  Require(Energy(Compute(backend, scene.input)) == 0, "no-return NaN contributed");
  scene.depth.at<float>(6, 2) = 5;
  scene.window.assign(64, 0);
  scene.input.window = scene.window.data();
  Require(Energy(Compute(backend, scene.input)) == 0, "zero frequency window ignored");
  scene.model.angularAreas[0] = -1;
  SonarComputeOutput rejected;
  Require(!backend.Compute(scene.input, rejected), "negative solid angle accepted");
  scene.model.angularAreas[0] = 0;
  scene.model.pressureResponse[0] = std::numeric_limits<float>::infinity();
  Require(!backend.Compute(scene.input, rejected), "nonfinite response accepted");
  scene.model.pressureResponse[0] = 0;
  scene.model.elevationIndices.back() = 7;
  Require(!backend.Compute(scene.input, rejected), "invalid selected row accepted");
}
void PulseControls(ComputeBackend & backend)
{
  auto pulse = MakePhysicalPulse(15, 1500, 29900);
  Require(pulse.ranges.size() == 599 && pulse.spectrum.size() == 2048, "pulse sampling plan");
  Require(std::abs(pulse.ranges.back() - 15) < 1e-6, "far endpoint absent");
  Require(
    std::abs(
      std::accumulate(pulse.spectrum.begin(), pulse.spectrum.end(), 0.) / pulse.spectrum.size() -
      1) < 1e-7,
    "pulse peak depends on transform length");
  for (double invalid :
       {0., -1., std::numeric_limits<double>::infinity(), std::numeric_limits<double>::max()})
  {
    bool rejected = false;
    try
    {
      MakePhysicalPulse(invalid, 1500, 29900);
    }
    catch (const std::invalid_argument &)
    {
      rejected = true;
    }
    Require(rejected, "unsupported pulse accepted");
  }
  Scene scene(5, 3, 2048, 1);
  scene.depth.setTo(0);
  scene.input.bandwidth = 29900;
  scene.input.maxDistance = 15;
  scene.input.nOutputRanges = 599;
  scene.input.window = pulse.spectrum.data();
  for (float range : {14.99f, 15.f})
  {
    scene.depth.at<float>(3, 2) = range;
    auto result = Compute(backend, scene.input);
    double nearPower = 0;
    for (int beam = 0; beam < result.nBeams; ++beam)
    {
      for (int r = 0; r < 20; ++r)
      {
        nearPower += std::norm(std::complex<double>(result.At(beam, r)));
      }
    }
    Require(nearPower / Energy(result) < 1e-5, "far pulse wrapped into near range");
  }
  scene.depth.at<float>(3, 2) = 4;
  const auto original = Compute(backend, scene.input);
  auto longer = MakePhysicalPulse(30, 1500, 29900);
  scene.input.nFreq = static_cast<int>(longer.spectrum.size());
  scene.input.window = longer.spectrum.data();
  scene.input.maxDistance = 30;
  // Compare the identical complete original output interval, no peak fitting.
  const auto extended = Compute(backend, scene.input);
  double difference = 0;
  for (size_t i = 0; i < original.beamSpectrum.size(); ++i)
  {
    difference += std::norm(
      std::complex<double>(original.beamSpectrum[i]) -
      std::complex<double>(extended.beamSpectrum[i]));
  }
  Require(std::sqrt(difference / Energy(original)) < 3e-4, "record length changed pulse");
  SonarComputeOutput rejected;
  scene.input.maxDistance =
    scene.input.nFreq * scene.input.soundSpeed / (2 * scene.input.bandwidth);
  Require(!backend.Compute(scene.input, rejected), "period boundary accepted");
  scene.input.maxDistance = 30;
  scene.input.nOutputRanges = scene.input.nFreq / 2 + 1;
  Require(!backend.Compute(scene.input, rejected), "missing pulse guard accepted");
  scene.input.nOutputRanges = -1;
  Require(!backend.Compute(scene.input, rejected), "negative output count accepted");
}

// Keep the geometry, random realization and pulse fixed while varying only
// receive gain. A nonzero CPU return must not disappear at a fixed-point floor.
void WeakReturns(ComputeBackend & backend)
{
  for (int columns : {1, 17})
  {
    Scene scene(columns, 3, 64, 1);
    for (bool sparse : {true, false})
    {
      scene.depth.setTo(5);
      if (sparse)
      {
        scene.depth.setTo(0);
        scene.depth.at<float>(3, columns / 2) = 5;
      }
      for (float gain : {1.f, .1f, .01f, .001f, .0001f, 1e-5f, 1e-6f, 1e-7f, 0.f, 1.f})
      {
        scene.input.receiveGain = gain;
        SonarComputeOutput reference;
        Require(ComputePhysicalSonarCpu(scene.input, reference), "weak-return CPU reference");
        auto actual = Compute(backend, scene.input);
        if (gain == 0)
        {
          Require(Energy(actual) == 0, "zero gain retained a previous spectrum");
          continue;
        }
        Require(Energy(reference) > 0 && Energy(actual) > 0, "weak return erased");
        double difference = 0;
        for (size_t i = 0; i < actual.beamSpectrum.size(); ++i)
        {
          difference += std::norm(
            std::complex<double>(actual.beamSpectrum[i]) -
            std::complex<double>(reference.beamSpectrum[i]));
        }
        Require(
          std::sqrt(difference / Energy(reference)) < 3e-4,
          "weak-return CPU/GPU relative error exceeds physical tolerance");
      }
      // Low reflectivity is not equivalent to moving receiveGain after a
      // quantized reduction: the scattering sum itself must remain floating point.
      scene.input.receiveGain = 1;
      scene.reflectivity.setTo(1e-16f);
      SonarComputeOutput reference;
      Require(ComputePhysicalSonarCpu(scene.input, reference), "low-reflectivity CPU reference");
      auto actual = Compute(backend, scene.input);
      double difference = 0;
      for (size_t i = 0; i < actual.beamSpectrum.size(); ++i)
      {
        difference += std::norm(
          std::complex<double>(actual.beamSpectrum[i]) -
          std::complex<double>(reference.beamSpectrum[i]));
      }
      Require(Energy(reference) > 0 && Energy(actual) > 0, "low reflectivity erased");
      Require(std::sqrt(difference / Energy(reference)) < 3e-4, "low-reflectivity parity");
      scene.reflectivity.setTo(.3f);
    }
  }
}

void LargeRanges(ComputeBackend & backend)
{
  // Physical output includes its far endpoint and a complete guard interval.
  // These are calculation controls, not scene-level ranging calibration.
  for (int ranges : {4096, 4097, 8193})
  {
    const auto pulse = MakePhysicalPulse(ranges - 1, 1500, 750);
    Require(pulse.ranges.size() == size_t(ranges), "large physical range grid truncated");
    Require(pulse.ranges.back() == ranges - 1, "large physical endpoint absent");
    Require(pulse.spectrum.size() >= 2 * size_t(ranges), "large physical pulse guard absent");
    Scene scene(5, 3, static_cast<int>(pulse.spectrum.size()), 4);
    scene.window = pulse.spectrum;
    scene.input.window = scene.window.data();
    scene.input.bandwidth = 750;
    scene.input.maxDistance = ranges - 1;
    scene.input.nOutputRanges = ranges;
    scene.depth.setTo(0);
    scene.depth.at<float>(6, 2) = 20;
    SonarComputeOutput reference;
    Require(ComputePhysicalSonarCpu(scene.input, reference), "large physical CPU rejected");
    const auto actual = Compute(backend, scene.input);
    double difference = 0;
    for (size_t i = 0; i < actual.beamSpectrum.size(); ++i)
    {
      difference += std::norm(
        std::complex<double>(actual.beamSpectrum[i]) -
        std::complex<double>(reference.beamSpectrum[i]));
    }
    Require(
      std::sqrt(difference / Energy(reference)) < 3e-4, "large physical CPU/GPU signal differs");
  }
  for (int frequencies : {4097, 8193})
  {
    Scene scene(5, 3, frequencies, 4);
    SonarComputeOutput reference;
    Require(ComputePhysicalSonarCpu(scene.input, reference), "large odd CPU rejected");
    const auto actual = Compute(backend, scene.input);
    double difference = 0;
    for (size_t i = 0; i < actual.beamSpectrum.size(); ++i)
    {
      difference += std::norm(
        std::complex<double>(actual.beamSpectrum[i]) -
        std::complex<double>(reference.beamSpectrum[i]));
    }
    Require(
      std::sqrt(difference / Energy(reference)) < 3e-4,
      "large odd physical CPU/GPU signal differs");
  }
}

void Selection(const std::string & name)
{
  Scene prototype(17, 256, 64, 4);
  Require(bool(CreateComputeBackend("cpu", prototype.input)), "physical CPU selection failed");
  Require(!CreateComputeBackend("cuda", prototype.input), "unsupported physical CUDA selected");
  auto bad = prototype.input;
  bad.nBeams = 17;
  Require(!CreateComputeBackend("cpu", bad), "malformed prototype accepted by CPU selection");
  if (name == "wgpu")
  {
    Require(bool(CreateComputeBackend("wgpu", prototype.input)), "physical WGPU probe failed");
    Require(!CreateComputeBackend("wgpu", bad), "malformed prototype accepted by WGPU selection");
    Scene odd(17, 256, 63, 4);
    auto explicitGpu = CreateComputeBackend("wgpu", odd.input);
    Require(bool(explicitGpu), "exact-length capability not reflected at selection");
    auto automatic = CreateComputeBackend("auto", odd.input);
    Require(
      automatic && std::string(automatic->Name()) == "wgpu",
      "auto selected an incompatible physical model");
    Compute(*automatic, odd.input);
  }
}

void PerspectiveInput(ComputeBackend & backend)
{
  Scene scene(17, 3, 64, 1);
  const auto grid =
    MakePerspectiveSonarGeometry(17, 7, M_PI / 3, M_PI / 18, 2, {-.25f, 0.f, .25f}, .08);
  scene.model = grid.readout;
  SonarComputeOutput reference;
  Require(ComputePhysicalSonarCpu(scene.input, reference), "perspective CPU input rejected");
  const auto output = Compute(backend, scene.input);
  double difference = 0.;
  for (size_t i = 0; i < output.beamSpectrum.size(); ++i)
  {
    difference += std::norm(
      std::complex<double>(output.beamSpectrum[i]) -
      std::complex<double>(reference.beamSpectrum[i]));
  }
  Require(
    std::sqrt(difference / Energy(reference)) < 3e-4, "CPU/GPU perspective cell response differs");
}
}  // namespace

int main(int argc, char ** argv)
{
  try
  {
    const std::string name = argc > 1 ? argv[1] : "cpu";
    auto backend = CreateComputeBackend(name);
    Require(backend && std::string(backend->Name()) == name, "requested backend unavailable");
    Require(backend->Initialize(SonarComputeInput{}), "initialization failed");
    Geometry();
    Selection(name);
    PerspectiveInput(*backend);
    Controls(*backend);
    PulseControls(*backend);
    WeakReturns(*backend);
    LargeRanges(*backend);
    std::ofstream arrays;
    if (argc > 3)
    {
      arrays.open(argv[3]);
      Require(bool(arrays), "cannot preserve complete comparison");
      arrays << "geometry,receivers,frequencies,stride,index,cpu_re,cpu_im,actual_re,actual_im\n";
      arrays << std::setprecision(9);
    }
    double maximum = 0;
    int cases = 0;
    for (int frequencies : {64, 63, 65, 599})
    {
      // Include larger/smaller output dimensions and cache reuse when only output count changes.
      for (int columns : {5, 17})
      {
        for (int receivers : {3, 19, 1, 3})
        {
          for (int stride : {1, 4})
          {
            Scene scene(columns, receivers, frequencies, stride);
            SonarComputeOutput reference;
            Require(ComputePhysicalSonarCpu(scene.input, reference), "CPU reference failed");
            auto actual = Compute(*backend, scene.input);
            double difference = 0;
            for (size_t i = 0; i < actual.beamSpectrum.size(); ++i)
            {
              difference += std::norm(
                std::complex<double>(actual.beamSpectrum[i]) -
                std::complex<double>(reference.beamSpectrum[i]));
              if (arrays.is_open())
              {
                arrays << columns << ',' << receivers << ',' << frequencies << ',' << stride << ','
                       << i << ',' << reference.beamSpectrum[i].real() << ','
                       << reference.beamSpectrum[i].imag() << ',' << actual.beamSpectrum[i].real()
                       << ',' << actual.beamSpectrum[i].imag() << '\n';
              }
            }
            const double error = std::sqrt(difference / Energy(reference));
            Require(
              error < 3e-4, "CPU/GPU physical signal relative error " + std::to_string(error));
            maximum = std::max(maximum, error);
            ++cases;
          }
        }
      }
    }
    std::cout << "PASS " << name << " physical readout: " << cases
              << " rectangular cases; max relative complex error=" << std::setprecision(9)
              << maximum << '\n';
  }
  catch (const std::exception & error)
  {
    std::cerr << "FAIL " << error.what() << '\n';
    return 1;
  }
}
