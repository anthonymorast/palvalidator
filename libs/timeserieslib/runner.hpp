#ifndef RUNNER_HPP
#define RUNNER_HPP

#include <future>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <type_traits>
#include <utility>

//extracts the number of cpus as reported by std::hardware_concurrency,
//eventually overriden by environment variable ncpu
// run as: ncpu=7 ./PalValidator BP_R0_5_Simpler_Config.txt 300 2
std::size_t getNCpus();

///////////////////////////////////////
/// \brief The runner struct
/// it sets up a thread pool for parallelizing a computation
/// for localized short runs, use it to start a pool
/// for running inside a loop, prefer to use a pool started
/// by runner& getRunner() which implements a singleton and can reuse the pool for subsequent calls

struct runner
{
    //constructor. nthreads==0 is for sequential run,
    //nthreads<2 is adjusted to 2, anything else is taken literally
    runner(std::size_t nthreads=getNCpus());
    //non-copyable object
    runner(const runner&) = delete;
    runner& operator=(const runner&) = delete;
    //destructor. tries to stop the pool and waits for threads to end
    ~runner();
    //tries to stop the threads in the pool
    void stop();

    // method to submit job to thread pool. reports back exceptions through a future
    template<typename F,typename ...Args>
        std::future<void> post(F f, Args&&...xargs)
        {
              using R = void;
              auto promise = std::make_shared<std::promise<R>>();
              std::future<R> res = promise->get_future();
              ios.post([ promise=std::move(promise)
                       , task=boost::bind<R>(std::move(f), std::forward<Args>(xargs)...)](){
                    try
                    {
                        task();
                    }
                    catch(std::exception const&e)
                    {
                        promise->set_exception(std::current_exception());
                        return;
                    }
                    promise->set_value();
                });

            return std::move(res);
        }

private:
    boost::asio::io_service ios;
    std::shared_ptr<boost::asio::io_service::work> work;
    boost::thread_group pool;

    //worker method of thread pool
    void run();
};

//Meyer's singleton for runner
//returns a reference to the constructed-once thread pool
//which can be subsequently reused
//and then cleaned-up at the end of the program
runner& getRunner();
#endif // RUNNER_HPP
