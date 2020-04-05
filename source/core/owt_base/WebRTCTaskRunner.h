// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WebRTCTaskRunner_h
#define WebRTCTaskRunner_h

#include <webrtc/base/location.h>
#include <webrtc/modules/utility/include/process_thread.h>

namespace owt_base {

/**
 * This thread is now responsible for running the non critical process for each modules.
 * It's now simply a wrapper of the webrtc ProcessThread, and we need it because it's shared
 * by different objects and we want to manage its lifetime automatically.
 */
class WebRTCTaskRunner {
public:
    WebRTCTaskRunner(const char* task_name);
    ~WebRTCTaskRunner();

    void Start();
    void Stop();
    void RegisterModule(webrtc::Module*);
    void DeRegisterModule(webrtc::Module*);

    webrtc::ProcessThread* unwrap();

    unsigned int GetModuleNum();

private:
    std::unique_ptr<webrtc::ProcessThread> m_processThread;
    int m_moduleNum;
};

inline WebRTCTaskRunner::WebRTCTaskRunner(const char* task_name)
    : m_processThread(webrtc::ProcessThread::Create(task_name))
{
    m_moduleNum = 0;
}

inline WebRTCTaskRunner::~WebRTCTaskRunner()
{
}

inline void WebRTCTaskRunner::Start()
{
    m_processThread->Start();
}

inline void WebRTCTaskRunner::Stop()
{
    m_processThread->Stop();
}

inline unsigned int WebRTCTaskRunner::GetModuleNum()
{
    return m_moduleNum;
}

inline void WebRTCTaskRunner::RegisterModule(webrtc::Module* module)
{
    m_processThread->RegisterModule(module, RTC_FROM_HERE);
    ++m_moduleNum;
}

inline void WebRTCTaskRunner::DeRegisterModule(webrtc::Module* module)
{
    m_processThread->DeRegisterModule(module);
    --m_moduleNum;
}

inline webrtc::ProcessThread* WebRTCTaskRunner::unwrap()
{
    return m_processThread.get();
}

} /* namespace owt_base */

#endif /* WebRTCTaskRunner_h */
