#include <atomic>
#include <cstddef>
#include <vector>
#include <stdexcept>

constexpr size_t  CACHE_LINE_SIZE = 64;

template <typename T, size_t Capacity>
class SPSCQueue{

    static_assert(((Capacity != 0) && (Capacity & (Capacity - 1)) == 0), "Capacity must be a power of 2");

    public:
        SPSCQueue() : write_index_(0), read_index_(0) {}

        bool push(const T& item){
            //memory_order_relaxed just read it cpu can reorder instructions
            const size_t current_write = write_index_.load(std::memory_order_relaxed);

            // memory_order_acquire makes sure that any code below this line is executed by CPU only after this line is executed
            const size_t current_read = read_index_.load(std::memory_order_acquire);

            if(current_write - current_read == Capacity){
                return false;
            }

            buffer_[current_write & (Capacity - 1)] = item;
            
            // Publish the new write index to the Consumer.
            // memory_order_release ensures the data write above is visible in memory BEFORE the write_index_ update is visible.
            write_index_.store(current_write + 1, std::memory_order_release);
            return true;
        }

        bool pop(T& item){
            const size_t current_read = read_index_.load(std::memory_order_relaxed);

            // memory_order_acquire pairs with the Producer's release.
            // It guarantees we see the Producer's data write if we see the updated write_index_.
            const size_t current_write = write_index_.load(std::memory_order_acquire);


            if(current_read == current_write){
                return false;
            }

            item = buffer_[current_read & (Capacity - 1)];

            // Publish the new read index to the Producer
            read_index_.store(current_read + 1, std::memory_order_release);
            return true;
        }


    private:

        alignas(CACHE_LINE_SIZE) std::atomic<size_t> write_index_;
        alignas(CACHE_LINE_SIZE) std::atomic<size_t> read_index_;

        T buffer_[Capacity];
};