#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include "WebRTCTaskRunnerPool.h"

using v8::Local;
using v8::Value;
using v8::Function;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Exception;

Nan::Persistent<Function> WebRTCTaskRunnerPool::constructor;

WebRTCTaskRunnerPool::WebRTCTaskRunnerPool() {
}

WebRTCTaskRunnerPool::~WebRTCTaskRunnerPool() {
}

NAN_MODULE_INIT(WebRTCTaskRunnerPool::Init) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("WebRTCTaskRunnerPool").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "close", close);
  Nan::SetPrototypeMethod(tpl, "start", start);

  constructor.Reset(tpl->GetFunction());
  Nan::Set(target, Nan::New("WebRTCTaskRunnerPool").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(WebRTCTaskRunnerPool::New) {
  if (info.Length() < 1) {
    Nan::ThrowError("Wrong number of arguments");
  }

  unsigned int num_workers = info[0]->IntegerValue();

  WebRTCTaskRunnerPool* obj = new WebRTCTaskRunnerPool();
  obj->me.reset(new owt_base::WebRTCTaskRunnerPool(num_workers));

  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(WebRTCTaskRunnerPool::close) {
  WebRTCTaskRunnerPool* obj = Nan::ObjectWrap::Unwrap<WebRTCTaskRunnerPool>(info.Holder());

  obj->me->Stop();
}

NAN_METHOD(WebRTCTaskRunnerPool::start) {
  WebRTCTaskRunnerPool* obj = Nan::ObjectWrap::Unwrap<WebRTCTaskRunnerPool>(info.Holder());

  obj->me->Start();
}
