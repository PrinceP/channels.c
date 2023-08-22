#include "channels.cpp"


void producer_main(Channel<int>& chan, int val, int producer_id) {
	while (val >= 0) {
		chan.put(val, val);
		std::cout << "Produced " << val << " by " << producer_id << std::endl;
		val--;
	}
}

void consumer_main(Channel<int>& chan, int consumer_id) {
	bool running = true;
	while (running) {
		auto val = chan.next();
		if (!val) {
			running = false;
			continue;
		}

		std::cout << "Consumed " << *val << " by " << consumer_id << std::endl;
	};
}


int main()
{
	Channel<int> channel;

	std::thread producer_thread1(producer_main, std::ref(channel), 10, 1);
	std::thread producer_thread2(producer_main, std::ref(channel), 10, 2);
	
	std::thread consumer_thread(consumer_main, std::ref(channel), 1);
	
	producer_thread1.join();
	producer_thread2.join();
	channel.close();
	
	consumer_thread.join();
	

	return 0;
}