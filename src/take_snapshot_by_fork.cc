#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <node.h>
#include <v8-profiler.h>
#include <chrono>

using namespace std::chrono;

namespace addon {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Context;
using v8::Value;

// https://github.com/bnoordhuis/node-heapdump
class FileOutputStream : public v8::OutputStream {
 public:
  explicit FileOutputStream(FILE* stream) : stream_(stream) {}

  virtual int GetChunkSize() {
    return 65536;  // big chunks == faster
  }

  virtual void EndOfStream() {}

  virtual WriteResult WriteAsciiChunk(char* data, int size) {
    const size_t len = static_cast<size_t>(size);
    size_t off = 0;
    while (off < len && !feof(stream_) && !ferror(stream_))
      off += fwrite(data + off, 1, len - off, stream_);
 
    return off == len ? kContinue : kAbort;
  }

 private:
  FILE* stream_;
};

void TakeSnapshotByFork(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    String::Utf8Value filename(isolate, args[0]);
    high_resolution_clock::time_point fork_t1 = high_resolution_clock::now();
    pid_t pid = fork();

    switch (pid) {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0: {
        FILE* fp = fopen(*filename, "w");
        if (fp == NULL) {
          perror("fopen");
          exit(EXIT_FAILURE);
        }
        high_resolution_clock::time_point take_snapshot_t1 = high_resolution_clock::now();
        const v8::HeapSnapshot* const snapshot = isolate->GetHeapProfiler()->TakeHeapSnapshot();
        FileOutputStream stream(fp);
        snapshot->Serialize(&stream, v8::HeapSnapshot::kJSON);
        high_resolution_clock::time_point take_snapshot_t2 = high_resolution_clock::now();
        duration<double, std::milli> time_span = take_snapshot_t2 - take_snapshot_t1;
        std::cout << "taking snapshot cost " << time_span.count() << " milliseconds."<<std::endl;
        std::cout <<"take snapshot done !\n"<<std::endl;
        fclose(fp);
        const_cast<v8::HeapSnapshot*>(snapshot)->Delete();
        exit(EXIT_SUCCESS);
    }
    default:
        high_resolution_clock::time_point fork_t2 = high_resolution_clock::now();
        duration<double, std::milli> time_span = fork_t2 - fork_t1;
        std::cout << "fork cost " << time_span.count() << " milliseconds."<<std::endl;
        break;
    }
}

void Initialize(Local<Object> exports, Local<Value> module, Local<Context> context) {
  NODE_SET_METHOD(exports, "takeSnapshotByFork", TakeSnapshotByFork);
}

NODE_MODULE_CONTEXT_AWARE(NODE_GYP_MODULE_NAME, Initialize)

} 
