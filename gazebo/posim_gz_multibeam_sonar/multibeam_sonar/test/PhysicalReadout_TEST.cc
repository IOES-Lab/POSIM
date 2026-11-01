// SPDX-License-Identifier: Apache-2.0
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
    input.maxDistance = 15;
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
  Require(
    output.nBeams == input.nBeams && output.nFreq == input.nFreq &&
      output.beamSpectrum.size() == size_t(input.nBeams) * input.nFreq,
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
}  // namespace

int main(int argc, char ** argv)
{
  try
  {
    const std::string name = argc > 1 ? argv[1] : "cpu";
    const bool radixOnly = argc > 2 && std::string(argv[2]) == "radix2";
    auto backend = CreateComputeBackend(name);
    Require(backend && std::string(backend->Name()) == name, "requested backend unavailable");
    Require(backend->Initialize(SonarComputeInput{}), "initialization failed");
    Geometry();
    Controls(*backend);
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
      if (radixOnly && frequencies != 64)
      {
        continue;
      }
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
    if (radixOnly)
    {
      Scene scene(5, 3, 63, 1);
      SonarComputeOutput rejected;
      Require(!backend->Compute(scene.input, rejected), "unqualified radix-2 odd length accepted");
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
