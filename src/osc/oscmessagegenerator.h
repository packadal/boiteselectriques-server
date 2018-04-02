#pragma once
#include <osc/OscOutboundPacketStream.h>
#include <string>
#include <vector>

namespace osc {
class MessageGenerator {
 public:
  MessageGenerator() = default;
  MessageGenerator(unsigned int c) : buffer(c), p(buffer.data(), c) {}

  template <typename... T>
  MessageGenerator(const std::string& name, const T&... args) {
    operator()(name, args...);
  }

  template <typename... T>
  MessageGenerator(unsigned int c, const std::string& name, const T&... args)
      : buffer(c), p(buffer.data(), c) {
    operator()(name, args...);
  }

  template <typename... T>
  const osc::OutboundPacketStream& operator()(const std::string& name,
                                              const T&... args) {
    p.Clear();
    p << osc::BeginBundleImmediate << osc::BeginMessage(name.c_str());
    subfunc(args...);

    return p;
  }

  const osc::OutboundPacketStream& stream() const { return p; }

 private:
  void subfunc() { p << osc::EndMessage << osc::EndBundle; }

  template <typename Arg1, typename... Args>
  void subfunc(const Arg1& arg1, const Args&... args) {
    p << arg1;
    subfunc(args...);
  }

  std::vector<char> buffer{std::vector<char>(4096)};
  osc::OutboundPacketStream p{
      osc::OutboundPacketStream(buffer.data(), buffer.size())};
};
}  // namespace osc
