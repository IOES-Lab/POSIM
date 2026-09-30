#include <chrono>
#include <future>
#include <iostream>
#include <mavconn/io_context_runner.hpp>
#include <memory>
#include <stdexcept>

struct Completion
{
  std::promise<void> done;
  ~Completion() { done.set_value(); }
};

int main()
{
  for (int i = 0; i < 100; ++i)
  {
    auto owner = std::make_shared<mavconn::IoContextRunner>();
    auto * io = &owner->io();
    std::promise<void> release;
    auto gate = release.get_future().share();
    auto completion = std::make_shared<Completion>();
    auto done = completion->done.get_future();
    asio::post(
      *io,
      [owned = owner, gate]() mutable
      {
        gate.wait();
        owned->shutdown_owned();
        owned.reset();
      });
    owner->start([io, completion]() { io->run(); });
    completion.reset();
    owner.reset();
    release.set_value();
    if (done.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
      throw std::runtime_error("I/O worker did not finish after self-close");
    }
  }
  std::cout << "100 self-close/destruction trials completed" << std::endl;
}
