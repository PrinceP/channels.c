#include "channels.cpp"


void producer_main(Channel<int>& chan, int val, int producer_id, int queue_id) {
    while (val >= 0) {
        chan.put(queue_id, val);
        std::cout << "Produced " << val << " by producer " << producer_id << " to queue " << queue_id << std::endl;
        val--;
    }
}

void consumer_main(Channel<int>& chan, int queue_id) {
    bool running = true;
    while (running) {
        auto val = chan.next(queue_id);
        if (!val) {
            running = false;
            continue;
        }

        std::cout << "Consumed " << *val << " from queue " << queue_id << std::endl;
    };
}

int main()
{
	Channel<int> channel_with_vector(30, 3);

    std::thread producer_thread1(producer_main, std::ref(channel_with_vector), 10, 1, 0);
    std::thread producer_thread2(producer_main, std::ref(channel_with_vector), 10, 2, 1);

    std::thread consumer_thread1(consumer_main, std::ref(channel_with_vector), 0);
    std::thread consumer_thread2(consumer_main, std::ref(channel_with_vector), 1);

    producer_thread1.join();
    producer_thread2.join();
    channel_with_vector.close();

    consumer_thread1.join();
    consumer_thread2.join();
}