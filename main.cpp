// Multithreaded Packet Processing Simulator
// Author: Jannatun Nur Zinia
// Description:
// This program simulates a packet processing pipeline using
// a producer-consumer architecture with multiple worker threads.
//
// Concepts demonstrated:
// - Multithreading using std::thread
// - Synchronization with mutex and condition_variable
// - Thread-safe queue handling
// - Performance measurement (throughput)

#include <iostream>
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

using namespace std;

// Packet structure
struct Packet {
    int id;
    Packet(int i) : id(i) {}
};

// Shared Queue
queue<Packet> packetQueue;

// Synchronization
mutex mtx;
condition_variable cv;

// Control flags
atomic<bool> finished(false);

// Statistics
atomic<int> processedPackets(0);

// Packet Generator
void generatePackets(int totalPackets) {
    for (int i = 1; i <= totalPackets; i++) {
        this_thread::sleep_for(chrono::milliseconds(50));

        unique_lock<mutex> lock(mtx);
        packetQueue.push(Packet(i));

        cout << "[Generator] Created Packet " << i << endl;

        lock.unlock();
        cv.notify_one();
    }

    finished = true;
    cv.notify_all();
}

// Packet Processing Worker
void worker(int workerID) {
    while (true) {
        unique_lock<mutex> lock(mtx);

        cv.wait(lock, [] {
            return !packetQueue.empty() || finished;
        });

        if (packetQueue.empty() && finished)
            break;

        Packet pkt = packetQueue.front();
        packetQueue.pop();

        lock.unlock();

        cout << "[Worker " << workerID
             << "] Processing Packet "
             << pkt.id << endl;

        this_thread::sleep_for(chrono::milliseconds(100));

        processedPackets++;
    }
}

// Main Function
int main() {

    const int TOTAL_PACKETS = 20;
    const int WORKER_THREADS = 4;

    auto startTime = chrono::high_resolution_clock::now();

    // Start packet generator
    thread generator(generatePackets, TOTAL_PACKETS);

    // Worker thread pool
    vector<thread> workers;

    for (int i = 1; i <= WORKER_THREADS; i++) {
        workers.push_back(thread(worker, i));
    }

    generator.join();

    for (auto &t : workers) {
        t.join();
    }

    auto endTime = chrono::high_resolution_clock::now();

    chrono::duration<double> elapsed = endTime - startTime;

    double throughput = processedPackets.load() / elapsed.count();

    cout << "\n---- Statistics ----\n";
    cout << "Total Packets Processed: "
         << processedPackets << endl;

    cout << "Time Taken: "
         << elapsed.count()
         << " seconds\n";

    cout << "Throughput: "
         << throughput
         << " packets/sec\n";

    return 0;
}
