// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>

#include "include/libplatform/libplatform.h"
#include "include/v8-context.h"
#include "include/v8-exception.h"
#include "include/v8-function.h"
#include "include/v8-initialization.h"
#include "include/v8-isolate.h"
#include "include/v8-local-handle.h"
#include "include/v8-primitive.h"
#include "include/v8-script.h"

#define SRC "var COUNT= 5; function test() { return ['hello',  'world', ++COUNT].join(' '); }"

uint64_t
njs_time(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t) ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void
dump_stats(const char *name, uint64_t iters, uint64_t ns)
{
    struct rusage  usage;

    uint64_t total = njs_time() - ns, ms;

    ms = total / 1000000;
    ns = total % 1000000;

    printf("%s total: %lu.%06lums\n", name, ms, ns);

    uint64_t periter = total / iters;

    ms = periter / 1000000;
    ns = periter % 1000000;

    printf("%s per req: %lu.%06lums\n", name, ms, ns);

    getrusage(RUSAGE_SELF, &usage);

    uint64_t us = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec
      + usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;

    printf("%s %.3fμs per VM, %d times/s\n", name,
           (double) us / iters, (int) ((uint64_t) iters * 1000000 / us));
}


void
context_per_req(uint64_t iters)
{
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, SRC)
                                   .ToLocalChecked();
    v8::ScriptCompiler::Source script_source(source);
    v8::Local<v8::UnboundScript> unbound_script =
      v8::ScriptCompiler::CompileUnboundScript(isolate, &script_source).ToLocalChecked();
    v8::Persistent<v8::UnboundScript> persistent_script(isolate, unbound_script);

    uint64_t ns = njs_time();

    for (uint64_t i = 0; i < iters; i++) {
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> ctx = v8::Context::New(isolate);
        v8::Context::Scope context_scope(ctx);

        v8::Local<v8::Script> script = unbound_script->BindToCurrentContext();

        v8::TryCatch try_catch(ctx->GetIsolate());

        v8::Local<v8::Value> result;
        if (!script->Run(ctx).ToLocal(&result)) {
            v8::String::Utf8Value error(ctx->GetIsolate(), try_catch.Exception());
            printf("Thrown in global: %s\n", *error);
            return;
        }

        v8::Local<v8::Object> global = ctx->Global();
        v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, "test").ToLocalChecked();

        v8::Local<v8::Function> fun = global->Get(ctx, key).ToLocalChecked()->ToObject(ctx).
                                    ToLocalChecked().As<v8::Function>();

        if (!fun->Call(ctx, global, 0, nullptr).ToLocal(&result)) {
            v8::String::Utf8Value error(ctx->GetIsolate(), try_catch.Exception());
            printf("Thrown in call: %s\n", *error);
            return;
        }

        v8::String::Utf8Value utf8(isolate, result);
        //printf("%s\n", *utf8);
    }

    dump_stats("create context per req", iters, ns);

    delete create_params.array_buffer_allocator;
}

void
shared_context(uint64_t iters)
{
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    uint64_t ns = njs_time();

    v8::Isolate* isolate = v8::Isolate::New(create_params);

    {
      v8::Isolate::Scope isolate_scope(isolate);

      v8::HandleScope handle_scope(isolate);

      v8::Local<v8::Context> ctx = v8::Context::New(isolate);

      v8::Context::Scope context_scope(ctx);

      {
        v8::Local<v8::String> source =
            v8::String::NewFromUtf8Literal(isolate, SRC);

        v8::Local<v8::Script> script =
            v8::Script::Compile(ctx, source).ToLocalChecked();

        v8::TryCatch try_catch(ctx->GetIsolate());

        v8::Local<v8::Value> result;
        if (!script->Run(ctx).ToLocal(&result)) {
            v8::String::Utf8Value error(ctx->GetIsolate(), try_catch.Exception());
            printf("Thrown in global: %s\n", *error);
            return;
        }

        for (uint64_t i = 0; i < iters; i++) {
            v8::Local<v8::Object> global = ctx->Global();
            v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, "test").ToLocalChecked();

            v8::Local<v8::Function> fun = global->Get(ctx, key).ToLocalChecked()->ToObject(ctx).
                                        ToLocalChecked().As<v8::Function>();

            if (!fun->Call(ctx, global, 0, nullptr).ToLocal(&result)) {
                v8::String::Utf8Value error(ctx->GetIsolate(), try_catch.Exception());
                printf("Thrown in call: %s\n", *error);
                return;
            }

            v8::String::Utf8Value utf8(isolate, result);
            //printf("%s\n", *utf8);
        }
      }
    }

    isolate->Dispose();

    dump_stats("shared context", iters, ns);

    delete create_params.array_buffer_allocator;
}


int
main(int argc, char* argv[])
{
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  uint64_t iters = 50000;

  shared_context(iters);
  context_per_req(iters);

  v8::V8::Dispose();
  v8::V8::DisposePlatform();

  return 0;
}
