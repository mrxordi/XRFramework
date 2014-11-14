#pragma once

namespace XR {
  /**
   * Any class that inherits from NonCopyable will ... not be copyable (Duh!)
   */
  class NonCopyable {
    inline NonCopyable(const NonCopyable& ) {}
    inline NonCopyable& operator=(const NonCopyable& ) { return *this; }
  public:
    inline NonCopyable() {}
  };

}