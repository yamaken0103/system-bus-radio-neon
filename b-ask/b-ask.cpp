#include <iostream>
#include <iomanip>
#include <chrono>
#include <arm_neon.h>
#include <stdlib.h>
#include <inttypes.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <signal.h>

std::mutex m ;
std::condition_variable cv ;
std::chrono::high_resolution_clock::time_point end ;
std::chrono::high_resolution_clock::time_point reset ;

int32x4_t va;
std::int32_t a ;
std::int32_t * ptr;
std::int32_t n = 2500;
std::int64_t size = sizeof(a)*n;
std::int32_t limit = n-4;
//std::int32_t data_bit[] = {1, 1, 1, 1, 1, 0, 1, 0};
std::int32_t data_bit[] = {1, 0, 1, 0};     //square audio

void inline sig_handler(int sign) {
    free(ptr);
    std::cout << "\nReceived signal. aborting." << std::endl ;
    exit(-1);
}

void inline boost_song() {
    using namespace std::chrono ;

    int i{0} ;
    while( true ) {
        std::unique_lock<std::mutex> lk{m} ;
        cv.wait( lk ) ;

        while( high_resolution_clock::now() < end ) {
            int32_t var[4] = { *(ptr + i), *(ptr + i + 1), *(ptr + i + 2), *(ptr + i + 3) };
            va = vld1q_s32(var);
            i++;
            if(i==limit) i=0;
        }
        
        std::this_thread::sleep_until( reset ) ;
    }
}

int init_memory(void) {
    ptr = (int32_t *)malloc(size);
    if( ptr == NULL ){
        std::cout << "Malloc Error" << std::endl;
        return -1;
    }
    for(int i=0; i<=n; i++){
        ptr[i] = i;
    }
    return 0;
}

void send_data(float time) {
    using namespace std::chrono ;

    seconds const sec{1} ;
    nanoseconds const nsec{ sec } ;
    using rep = nanoseconds::rep ;
    auto nsec_per_sec = nsec.count() ;

    for(int32_t d : data_bit){
        auto start = high_resolution_clock::now() ;
        auto end = start + nanoseconds( static_cast<rep>(time * nsec_per_sec) ) ;

        if( d == 1 ){
            while (high_resolution_clock::now() < end) {
                cv.notify_all() ;
                std::this_thread::sleep_until( end ) ;
            }
        }
        else{
            std::this_thread::sleep_until( end ) ;
        }
    }
}

int main(){
    signal(SIGINT, sig_handler);

    init_memory();
    for ( unsigned i = 0 ; i < std::thread::hardware_concurrency() ; ++i ) {
        std::thread t( boost_song ) ;
        t.detach() ;
    }
    while(1){
        //std::cout << "send data {1,0,0,1}" <<std::endl;
        //send_data(0.000374111485223/2);
        send_data(0.000425713069391/2);
        //std::cout << "complete" <<std::endl;
/*/TEST
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.400, 1/(2093*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.790, 1/(2673*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.790, 1/(2349*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(3136*2));
        send_data(0.790, 1/(3136*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.400, 1/(2093*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.400, 1/(2673*2));
        send_data(0.400, 1/(2349*2));
        send_data(0.790, 1/(2093*2));
//END::TEST */
    }
    free(ptr);
    return 0;
}
