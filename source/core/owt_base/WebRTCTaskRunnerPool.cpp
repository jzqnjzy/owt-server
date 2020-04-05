// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "WebRTCTaskRunnerPool.h"

namespace owt_base {

WebRTCTaskRunnerPool::WebRTCTaskRunnerPool(unsigned int poolSize)
{
    for(unsigned int i = 0; i < poolSize; ++i){
        taskRunners_.push_back(std::make_shared<WebRTCTaskRunner>("WebRTCTaskRunnerPool"));
    }
    m_moduleNum = 0;
}

WebRTCTaskRunnerPool::~WebRTCTaskRunnerPool()
{
}

void WebRTCTaskRunnerPool::Start()
{
    for (auto taskRunner : taskRunners_){
        taskRunner->Start();
    }
}

void WebRTCTaskRunnerPool::Stop()
{
    for (auto taskRunner : taskRunners_){
        taskRunner->Stop();
    }
}

std::shared_ptr<WebRTCTaskRunner> WebRTCTaskRunnerPool::getLessUsedTaskRunner()
{
    std::shared_ptr<WebRTCTaskRunner> ret = taskRunners_.front();
    for (auto taskRunner : taskRunners_){
        if(taskRunner->GetModuleNum() < ret->GetModuleNum()){
            ret = taskRunner;
        }
    }
    return ret;
}

} /* namespace mcu */
