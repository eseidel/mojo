// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/base/file_stream_context.h"

#include <windows.h>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/metrics/histogram.h"
#include "base/profiler/scoped_tracker.h"
#include "base/task_runner.h"
#include "base/threading/worker_pool.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"

namespace net {

namespace {

void SetOffset(OVERLAPPED* overlapped, const LARGE_INTEGER& offset) {
  overlapped->Offset = offset.LowPart;
  overlapped->OffsetHigh = offset.HighPart;
}

void IncrementOffset(OVERLAPPED* overlapped, DWORD count) {
  LARGE_INTEGER offset;
  offset.LowPart = overlapped->Offset;
  offset.HighPart = overlapped->OffsetHigh;
  offset.QuadPart += static_cast<LONGLONG>(count);
  SetOffset(overlapped, offset);
}

}  // namespace

FileStream::Context::Context(const scoped_refptr<base::TaskRunner>& task_runner)
    : io_context_(),
      async_in_progress_(false),
      orphaned_(false),
      task_runner_(task_runner),
      weak_ptr_factory_(this) {
  io_context_.handler = this;
  memset(&io_context_.overlapped, 0, sizeof(io_context_.overlapped));
}

FileStream::Context::Context(base::File file,
                             const scoped_refptr<base::TaskRunner>& task_runner)
    : io_context_(),
      file_(file.Pass()),
      async_in_progress_(false),
      orphaned_(false),
      task_runner_(task_runner),
      weak_ptr_factory_(this) {
  io_context_.handler = this;
  memset(&io_context_.overlapped, 0, sizeof(io_context_.overlapped));
  if (file_.IsValid()) {
    // TODO(hashimoto): Check that file_ is async.
    OnFileOpened();
  }
}

FileStream::Context::~Context() {
}

int FileStream::Context::Read(IOBuffer* buf,
                              int buf_len,
                              const CompletionCallback& callback) {
  // TODO(vadimt): Remove ScopedTracker below once crbug.com/423948 is fixed.
  tracked_objects::ScopedTracker tracking_profile(
      FROM_HERE_WITH_EXPLICIT_FUNCTION("423948 FileStream::Context::Read"));

  DCHECK(!async_in_progress_);

  IOCompletionIsPending(callback, buf);

  base::WorkerPool::PostTask(
      FROM_HERE,
      base::Bind(&FileStream::Context::ReadAsync,
                 weak_ptr_factory_.GetWeakPtr(), file_.GetPlatformFile(),
                 make_scoped_refptr(buf), buf_len, &io_context_.overlapped,
                 base::MessageLoop::current()->message_loop_proxy()),
      false);

  return ERR_IO_PENDING;
}

int FileStream::Context::Write(IOBuffer* buf,
                               int buf_len,
                               const CompletionCallback& callback) {
  DWORD bytes_written = 0;
  if (!WriteFile(file_.GetPlatformFile(), buf->data(), buf_len,
                 &bytes_written, &io_context_.overlapped)) {
    IOResult error = IOResult::FromOSError(GetLastError());
    if (error.os_error == ERROR_IO_PENDING)
      IOCompletionIsPending(callback, buf);
    else
      LOG(WARNING) << "WriteFile failed: " << error.os_error;
    return static_cast<int>(error.result);
  }

  IOCompletionIsPending(callback, buf);
  return ERR_IO_PENDING;
}

FileStream::Context::IOResult FileStream::Context::SeekFileImpl(
    base::File::Whence whence,
    int64 offset) {
  LARGE_INTEGER result;
  result.QuadPart = file_.Seek(whence, offset);
  if (result.QuadPart >= 0) {
    SetOffset(&io_context_.overlapped, result);
    return IOResult(result.QuadPart, 0);
  }

  return IOResult::FromOSError(GetLastError());
}

void FileStream::Context::OnFileOpened() {
  base::MessageLoopForIO::current()->RegisterIOHandler(file_.GetPlatformFile(),
                                                       this);
}

void FileStream::Context::IOCompletionIsPending(
    const CompletionCallback& callback,
    IOBuffer* buf) {
  DCHECK(callback_.is_null());
  callback_ = callback;
  in_flight_buf_ = buf;  // Hold until the async operation ends.
  async_in_progress_ = true;
}

void FileStream::Context::OnIOCompleted(
    base::MessageLoopForIO::IOContext* context,
    DWORD bytes_read,
    DWORD error) {
  DCHECK_EQ(&io_context_, context);
  DCHECK(!callback_.is_null());
  DCHECK(async_in_progress_);

  async_in_progress_ = false;
  if (orphaned_) {
    callback_.Reset();
    in_flight_buf_ = NULL;
    CloseAndDelete();
    return;
  }

  int result;
  if (error == ERROR_HANDLE_EOF) {
    result = 0;
  } else if (error) {
    IOResult error_result = IOResult::FromOSError(error);
    result = static_cast<int>(error_result.result);
  } else {
    result = bytes_read;
    IncrementOffset(&io_context_.overlapped, bytes_read);
  }

  CompletionCallback temp_callback = callback_;
  callback_.Reset();
  scoped_refptr<IOBuffer> temp_buf = in_flight_buf_;
  in_flight_buf_ = NULL;
  temp_callback.Run(result);
}

// static
void FileStream::Context::ReadAsync(
    const base::WeakPtr<FileStream::Context>& context,
    HANDLE file,
    scoped_refptr<net::IOBuffer> buf,
    int buf_len,
    OVERLAPPED* overlapped,
    scoped_refptr<base::MessageLoopProxy> origin_thread_loop) {
  DWORD bytes_read = 0;
  if (!ReadFile(file, buf->data(), buf_len, &bytes_read, overlapped)) {
    origin_thread_loop->PostTask(
        FROM_HERE, base::Bind(&FileStream::Context::ReadAsyncResult, context,
                              ::GetLastError()));
  }
}

void FileStream::Context::ReadAsyncResult(DWORD os_error) {
  IOResult error = IOResult::FromOSError(os_error);
  if (error.os_error == ERROR_HANDLE_EOF) {
    // Report EOF by returning 0 bytes read.
    OnIOCompleted(&io_context_, 0, error.os_error);
  } else if (error.os_error != ERROR_IO_PENDING) {
    // We don't need to inform the caller about ERROR_PENDING_IO as that was
    // already done when the ReadFile call was queued to the worker pool.
    if (error.os_error)
      LOG(WARNING) << "ReadFile failed: " << error.os_error;
    OnIOCompleted(&io_context_, 0, error.os_error);
  }
}

}  // namespace net
