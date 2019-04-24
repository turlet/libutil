#ifndef NUL_BUFFER_POOL_H_
#define NUL_BUFFER_POOL_H_
#include "buffer.hpp"
#include <deque>
#include <memory>
#include <algorithm>
#include <cassert>

namespace nul {
  class BufferPool {
    public:
      BufferPool(std::size_t maxBufferSize, std::size_t maxBufferCount) :
        maxBufferSize_(maxBufferSize), maxBufferCount_(maxBufferCount) {
        assert(maxBufferCount > 0);
        for (std::size_t i = 0; i < maxBufferCount; ++i) {
          freeBuffers_.push_back(std::make_unique<Buffer>(maxBufferSize));
        }
      }
      virtual ~BufferPool() = default;

      std::unique_ptr<Buffer> requestBuffer(std::size_t size) {
        if (!freeBuffers_.empty()) {
          auto freeBufIt = std::find_if(
            freeBuffers_.begin(), freeBuffers_.end(), [size](auto &buf) {
              return buf->getCapacity() >= size;
            });
          if (freeBufIt != freeBuffers_.end()) {
            auto freeBuf = std::move(*freeBufIt);
            freeBuffers_.erase(freeBufIt);
            return freeBuf;
          }
        }
        return std::make_unique<Buffer>(size);
      }

      void returnBuffer(std::unique_ptr<Buffer> &&data) {
        if (data->getCapacity() <= maxBufferSize_ &&
            freeBuffers_.size() < maxBufferCount_) {
          freeBuffers_.push_back(std::move(data));
        }
      }

      std::unique_ptr<Buffer> assembleDataBuffer(
        const char *data, std::size_t dataLen) {
        auto dataBuf = requestBuffer(dataLen);
        dataBuf->assign(data, dataLen);
        return dataBuf;
      }

      std::size_t getTotalBufferCount() const {
        return freeBuffers_.size();
      }

      uint64_t getTotalBufferSize() const {
        uint64_t size = 0;
        for (auto &b : freeBuffers_) {
          size += b->getCapacity();
        }
        return size;
      }

    private:
      std::deque<std::unique_ptr<Buffer>> freeBuffers_;
      std::size_t maxBufferSize_;
      std::size_t maxBufferCount_;
  };
} /* end of namspace: nul */

#endif /* end of include guard: NUL_BUFFER_POOL_H_ */
