#/*
 * The MIT License
 * Copyright (c) 2011 Clark Gaebel <cg.wowus.cg@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include "pipe.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define MUST_SENTINEL __attribute__((sentinel))
#else
#define MUST_SENTINEL
#endif

/*
 * A function that can be used for processing a pipe.
 *
 * elem_in  - an array of elements to process
 * count    - the number of elements in `elem_in'
 * elem_out - The producer that may be pushed into to continue the chain
 * aux      - auxilary data previously passed to pipe_connect.
 */
typedef void (*pipe_processor_t)(const void* /* elem_in */,
                                 size_t      /* count */,
                                 producer_t* /* elem_out */,
                                 void*       /* aux */
                                );

typedef struct {
    producer_t* in;
    consumer_t* out;
} pipeline_t;

/*
 * Creates a pipeline wrapping an existing pipe.
 */
pipeline_t pipe_simple_pipeline(pipe_t* p);

/*
 * Creates a pipeline with multiple instances of the same function working on
 * the same queue. Whenever elements are pushed into the pipeline, the
 * instances will work in parallel to process them.
 *
 * `proc' is run in `instances' many threads, each one being passed _the same_
 * `aux'. It is highly recommended to avoid as much shared state as possible,
 * as resource management can get very annoying with this setup and any
 * synchronization will eat away at any concurrency gained by using the
 * pipeline in the first place.
 */
pipeline_t pipe_parallel(size_t           instances,
                         size_t           bufsize,
                         size_t           in_size,
                         pipe_processor_t proc,
                         void*            aux,
                         size_t           out_size);

/*
 * A pipeline consists of a list of functions and pipes. As data is recieved in
 * one end, it is processed by each of the pipes and pushed into the other end.
 * Each stage's processing is done in a seperate thread. The last parameter must
 * be NULL (in place of a pipe_processor_t) if you want to have a consumer_t
 * returned, or 0 (in place of a sizeof()) if you don't want or need a consumer_t.
 * If the last parameter replaces a sizeof(), the return value's `c' member will
 * be NULL.
 *
 * The first parameter determines how much buffering the pipeline will do. It
 * has a unit of "elements", not "bytes". If this parameter is zero, the
 * implementation will pick a "nice" (but generic) value for batch processing.
 * If you want, you can set it to 1 for no buffering at all, or a high value for
 * more buffering.
 *
 * When passing NULL `aux' pointers to your functors, you MUST cast them to
 * void* to maintain 64-bit compatibility. The C standard only requires NULL to
 * be defined as 0, so will be cast to a 32-bit wide int. This will destroy
 * alignment since pipe_pipeline looks for a void* in that space.
 *
 * Sample:
 *  pipeline_t p = pipe_pipeline(0, sizeof(int),
 *                               &int_to_float,       &i2f_data,   sizeof(float),
 *                               &float_to_garbage,   &f2g_data,   sizeof(garbage),
 *                               &garbage_to_awesome, (void*)NULL, sizeof(awesome),
 *                               (void*)NULL
 *                              );
 *
 *  // Now push all your ints into p.p ...
 *
 *  pipe_producer_free(p.p);
 *
 *  // Now pop all your awesome out of p.c ...
 *
 *  pipe_consumer_free(p.c);
 *
 *  NOTE: All the functions must be of type pipe_processor_t. This call will
 *  return a pipeline which takes the type specified by the first parameter
 *  [int] and returns the last type [awesome] (or NULL if the last vararg was a
 *  function).
 */
pipeline_t MUST_SENTINEL pipe_pipeline(size_t bufsize, size_t first_size, ...);

#undef MUST_SENTINEL

#ifdef __cplusplus
}
#endif

/* vim: set et ts=4 sw=4 softtabstop=4 textwidth=80: */
