#include "channels.cpp"

void producer_main(Channel<int>& chan, int val) {
    while (val >= 0) {
        chan.put(val);
        std::cout << "Produced " << val << std::endl;
        val--;
    }
}

void consumer_main(Channel<int>& chan) {
    bool running = true;
    while (running) {
        auto val = chan.next();
        if (!val) {
            running = false;
            continue;
        }

        std::cout << "Consumed " << *val << std::endl;
    };
}

int main()
{
    Channel<int> channel;

    std::thread producer_thread(producer_main, std::ref(channel), 13);
    std::thread consumer_thread(consumer_main, std::ref(channel));
    
    producer_thread.join();
    channel.close();

    consumer_thread.join();

    return 0;
}