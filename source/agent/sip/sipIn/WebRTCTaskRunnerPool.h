#ifndef ERIZOAPI_WebRTCTaskRunnerPool_H_
#define ERIZOAPI_WebRTCTaskRunnerPool_H_


#include <nan.h>
#include <WebRTCTaskRunnerPool.h>


/*
 * Wrapper class of owt_base::WebRTCTaskRunnerPool
 *
 * Represents a OneToMany connection.
 * Receives media from one publisher and retransmits it to every subscriber.
 */
class WebRTCTaskRunnerPool : public Nan::ObjectWrap {
 public:
    static NAN_MODULE_INIT(Init);
    std::unique_ptr<owt_base::WebRTCTaskRunnerPool> me;

 private:
    WebRTCTaskRunnerPool();
    ~WebRTCTaskRunnerPool();

    /*
     * Constructor.
     * Constructs a ThreadPool
     */
    static NAN_METHOD(New);
    /*
     * Closes the ThreadPool.
     * The object cannot be used after this call
     */
    static NAN_METHOD(close);
    /*
     * Starts all workers in the ThreadPool
     */
    static NAN_METHOD(start);

    static Nan::Persistent<v8::Function> constructor;
};

#endif  // ERIZOAPI_WebRTCTaskRunnerPool_H_
