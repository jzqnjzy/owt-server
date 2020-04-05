// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WebRTCTaskRunnerPool_h
#define WebRTCTaskRunnerPool_h

#include <vector>
#include <webrtc/base/location.h>
#include <webrtc/modules/utility/include/process_thread.h>
#include "WebRTCTaskRunner.h"

namespace owt_base {

/**
 * This thread is now responsible for running the non critical process for each modules.
 * It's now simply a wrapper of the webrtc ProcessThread, and we need it because it's shared
 * by different objects and we want to manage its lifetime automatically.
 */
class WebRTCTaskRunnerPool {
public:
    WebRTCTaskRunnerPool(unsigned int poolSize);
    ~WebRTCTaskRunnerPool();

    void Start();
    void Stop();
    std::shared_ptr<WebRTCTaskRunner> getLessUsedTaskRunner();

private:
    std::vector<std::shared_ptr<WebRTCTaskRunner>> taskRunners_;
    int m_moduleNum;
};

} /* namespace owt_base */

#endif /* WebRTCTaskRunnerPool_h */
