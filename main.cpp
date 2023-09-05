#include "channels.cpp"


void producer_main(Channel<std::string>& chan, int val, int producer_id, int queue_id) {
	while (val >= 0) {
		std::string temp = std::to_string(queue_id)+"-"+std::to_string(val);
		chan.put(queue_id, temp);
		std::cout << "Produced " << temp << " by producer " << producer_id << " to queue " << queue_id << std::endl;
		val--;
	}
}

void consumer_main(Channel<std::string>& chan, int queue_id) {
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

	std::vector<Channel<std::string>*> channels;
	int CHANNELS = 100;

	for(int ch=0; ch < CHANNELS; ch++){
		Channel<std::string>* channel_with_vector1 = new Channel<std::string>(30, ch + 3);
		channels.push_back(channel_with_vector1);
	}


	std::vector<std::thread> thVec;
	for(int ch=0; ch < CHANNELS; ch++){
		Channel<std::string>& channel_with_vector = *channels[ch];
		std::thread producer_thread1(producer_main, std::ref(channel_with_vector), 10, ch+1, ch);
		std::thread consumer_thread1(consumer_main, std::ref(channel_with_vector), ch);

		thVec.emplace_back(std::move(producer_thread1));
		thVec.emplace_back(std::move(consumer_thread1));

	}

	int temp = 0;
	for(int ch=0; ch < CHANNELS; ch++){
		Channel<std::string>& channel_with_vector = *channels[ch];
		thVec[temp].join();
		channel_with_vector.close();
		thVec[temp+1].join();
		temp += 2;
		delete channels[ch];
	}

	thVec.clear();
	channels.clear();

}



// int main()
// {
// 	Channel<int> channel_with_vector(30, 3);
// 	std::vector<Channel<int>> channels;

// 	//TODO: Make it work
// 	// Create a vector of channels
// 	std::vector<Channel<int>> channels;

// 	// Push a copy of channel_with_vector into channels
// 	channels.push_back(channel_with_vector);  // This will invoke the copy constructor


// 	std::thread producer_thread1(producer_main, std::ref(channel_with_vector), 10, 1, 0);
// 	std::thread producer_thread2(producer_main, std::ref(channel_with_vector), 10, 2, 1);

// 	std::thread consumer_thread1(consumer_main, std::ref(channel_with_vector), 0);
// 	std::thread consumer_thread2(consumer_main, std::ref(channel_with_vector), 1);

// 	producer_thread1.join();
// 	producer_thread2.join();
// 	channel_with_vector.close();

// 	consumer_thread1.join();
// 	consumer_thread2.join();
// }

