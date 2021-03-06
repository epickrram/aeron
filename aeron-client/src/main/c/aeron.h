/*
 * Copyright 2014-2020 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AERON_H
#define AERON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#define AERON_NULL_VALUE (-1)

typedef struct aeron_context_stct aeron_context_t;
typedef struct aeron_stct aeron_t;
typedef struct aeron_buffer_claim_stct aeron_buffer_claim_t;
typedef struct aeron_publication_stct aeron_publication_t;
typedef struct aeron_exclusive_publication_stct aeron_exclusive_publication_t;
typedef struct aeron_header_stct aeron_header_t;
typedef struct aeron_subscription_stct aeron_subscription_t;
typedef struct aeron_image_stct aeron_image_t;
typedef struct aeron_counter_stct aeron_counter_t;

typedef struct aeron_counters_reader_stct aeron_counters_reader_t;

/**
 * Environment variables and functions used for setting values of an aeron_context_t.
 */

/**
 * Function name to call on start of each agent.
 */
#define AERON_AGENT_ON_START_FUNCTION_ENV_VAR "AERON_AGENT_ON_START_FUNCTION"

typedef void (*aeron_agent_on_start_func_t)(void *state, const char *role_name);

int aeron_context_set_agent_on_start_function(
    aeron_context_t *context, aeron_agent_on_start_func_t value, void *state);
aeron_agent_on_start_func_t aeron_context_get_agent_on_start_function(aeron_context_t *context);
void *aeron_context_get_agent_on_start_state(aeron_context_t *context);

typedef void (*aeron_on_new_publication_t)(
    void *clientd, const char *channel, int32_t stream_id, int32_t session_id, int64_t correlation_id);
typedef void (*aeron_on_new_subscription_t)(
    void *clientd, const char *channel, int32_t stream_id, int64_t correlation_id);

typedef void (*aeron_on_available_image_t)(void *clientd, aeron_image_t *image);
typedef void (*aeron_on_unavailable_image_t)(void *clientd, aeron_image_t *image);

typedef void (*aeron_on_available_counter_t)(
    void *clientd, aeron_counters_reader_t *counters_reader, int64_t registration_id, int32_t counter_id);
typedef void (*aeron_on_unavailable_counter_t)(
    void *clientd, aeron_counters_reader_t *counters_reader, int64_t registration_id, int32_t counter_id);

typedef void (*aeron_on_close_client_t)(void *clientd);

typedef void (*aeron_error_handler_t)(void *clientd, int errcode, const char *message);

/**
 * Create a aeron_context_t struct and initialize with default values.
 *
 * @param context to create and initialize
 * @return 0 for success and -1 for error.
 */
int aeron_context_init(aeron_context_t **context);

/**
 * Close and delete aeron_context_t struct.
 *
 * @param context to close and delete
 * @return 0 for success and -1 for error.
 */
int aeron_context_close(aeron_context_t *context);

/**
 * Create a aeron_t client struct and initialize from the aeron_context_t struct.
 *
 * The given aeron_context_t struct will be used exclusively by the client. Do not reuse between clients.
 *
 * @param aeron  client to create and initialize.
 * @param context to use for initialization.
 * @return 0 for success and -1 for error.
 */
int aeron_init(aeron_t **client, aeron_context_t *context);

/**
 * Start an aeron_t. This may spawn a thread for the Client Conductor.
 *
 * @param client to start.
 * @param manual_main_loop to be called by the caller for the Conductor do_work cycle.
 * @return 0 for success and -1 for error.
 */
int aeron_start(aeron_t *client, bool manual_main_loop);

/**
 * Call the Conductor main do_work duty cycle once.
 *
 * Client must have been created with manual_main_loop set to true.
 *
 * @param client to call do_work duty cycle on.
 * @return 0 for success and -1 for error.
 */
int aeron_main_do_work(aeron_t *client);

/**
 * Call the Conductor Idle Strategy.
 *
 * @param client to idle.
 * @param work_count to pass to idle strategy.
 */
void aeron_main_idle_strategy(aeron_t *client, int work_count);

/**
 * Close and delete aeron_t struct.
 *
 * @param client to close and delete
 * @return 0 for success and -1 for error.
 */
int aeron_close(aeron_t *client);

/*
 * Aeron API functions
 */

void aeron_print_counters(aeron_t *client, void (*stream_out)(const char *));
aeron_context_t *aeron_context(aeron_t *client);
int64_t aeron_client_id(aeron_t *client);
int64_t aeron_next_correlation_id(aeron_t *client);

int64_t aeron_add_publication(aeron_t *client, const char *uri);
int aeron_find_publication(aeron_publication_t **publication, aeron_t *client, int64_t registration_id);

int64_t aeron_add_exclusive_publication(aeron_t *client, const char *uri);
int aeron_find_exclusive_publication(
    aeron_exclusive_publication_t **publication, aeron_t *client, int64_t registration_id);

int64_t aeron_add_subscription(
    aeron_t *client,
    const char *uri,
    aeron_on_available_image_t on_available_image_handler,
    void *on_available_image_clientd,
    aeron_on_unavailable_image_t on_unavailable_image_handler,
    void *on_unavailable_image_clientd);
int aeron_find_subscription(aeron_subscription_t **subscription, aeron_t *client, int64_t registration_id);

aeron_counters_reader_t *aeron_counters_reader(aeron_t *client);

int64_t aeron_add_counter(
    aeron_t *client,
    int32_t type_id,
    const uint8_t *key_buffer,
    size_t key_buffer_length,
    const char *label_buffer,
    size_t label_buffer_length);
int aeron_find_counter(aeron_counter_t **counter, aeron_t *client, int64_t registration_id);

int aeron_add_available_counter_handler(aeron_t *client, aeron_on_available_counter_t handler, void *clientd);
int aeron_remove_available_counter_handler(aeron_t *client, aeron_on_available_counter_t handler, void *clientd);
int aeron_add_unavailable_counter_handler(aeron_t *client, aeron_on_unavailable_counter_t handler, void *clientd);
int aeron_remove_unavailable_counter_handler(aeron_t *client, aeron_on_unavailable_counter_t handler, void *clientd);

int aeron_add_close_handler(aeron_t *client, aeron_on_close_client_t handler, void *clientd);
int aeron_remove_close_handler(aeron_t *client, aeron_on_close_client_t handler, void *clientd);

/*
 * Publication functions
 */

#define AERON_PUBLICATION_NOT_CONNECTED (-1L)
#define AERON_PUBLICATION_BACK_PRESSURED (-2L)
#define AERON_PUBLICATION_ADMIN_ACTION (-3L)
#define AERON_PUBLICATION_CLOSED (-4L)
#define AERON_PUBLICATION_MAX_POSITION_EXCEEDED (-5L)

typedef int64_t (*aeron_reserved_value_supplier_t)(void *clientd, uint8_t *buffer, size_t frame_length);
typedef struct aeron_iovec_stct
{
    uint8_t *iov_base;
    size_t iov_len;
}
aeron_iovec_t;

int64_t aeron_publication_offer(
    aeron_publication_t *publication,
    uint8_t *buffer,
    size_t length,
    aeron_reserved_value_supplier_t reserved_value_supplier,
    void *clientd);

int64_t aeron_publication_offerv(
    aeron_publication_t *publication,
    aeron_iovec_t *iov,
    size_t iovcnt,
    aeron_reserved_value_supplier_t reserved_value_supplier,
    void *clientd);

int64_t aeron_publication_try_claim(
    aeron_publication_t *publication,
    size_t length,
    aeron_buffer_claim_t *buffer_claim);

int aeron_publication_close(aeron_publication_t *publication);

/*
 * Exclusive Publication functions
 */

int64_t aeron_exclusive_publication_offer(
    aeron_exclusive_publication_t *publication,
    uint8_t *buffer,
    size_t length,
    aeron_reserved_value_supplier_t reserved_value_supplier);

int64_t aeron_exclusive_publication_offerv(
    aeron_exclusive_publication_t *publication,
    aeron_iovec_t *iov,
    size_t iovcnt,
    aeron_reserved_value_supplier_t reserved_value_supplier);

int64_t aeron_exclusive_publication_try_claim(
    aeron_exclusive_publication_t *publication,
    size_t length,
    aeron_buffer_claim_t *buffer_claim);

int aeron_exclusive_publication_close(aeron_exclusive_publication_t *publication);

/*
 * Subscription functions
 */

typedef void (*aeron_fragment_handler_t)(
    void *clientd, uint8_t *buffer, size_t offset, size_t length, aeron_header_t *header);

typedef enum aeron_controlled_fragment_handler_action_en
{
    AERON_ACTION_ABORT, AERON_ACTION_BREAK, AERON_ACTION_COMMIT, AERON_ACTION_CONTINUE
}
aeron_controlled_fragment_handler_action_t;
typedef aeron_controlled_fragment_handler_action_t (*aeron_controlled_fragment_handler_t)(
    void *clientd, uint8_t *buffer, size_t offset, size_t length, aeron_header_t *header);

typedef void (*aeron_block_handler_t)(
    void *clientd, uint8_t *buffer, size_t offset, size_t length, int32_t session_id, int32_t term_id);

int aeron_subscription_poll(
    aeron_subscription_t *subscription, aeron_fragment_handler_t handler, void *clientd, int fragment_limit);
int aeron_subscription_controlled_poll(
    aeron_subscription_t *subscription, aeron_controlled_fragment_handler_t handler, void *clientd, int fragment_limit);
int aeron_subscription_block_poll(
    aeron_subscription_t *subscription, aeron_block_handler_t handler, void *clientd, size_t block_length_limit);

aeron_image_t *aeron_subscription_image_by_session_id(aeron_subscription_t *subscription, int32_t session_id);

int aeron_subscription_close(aeron_subscription_t *subscription);

/*
 * Image functions
 */

int aeron_image_poll(aeron_image_t *image, aeron_fragment_handler_t handler, void *clientd, int fragment_limit);
int aeron_image_controlled_poll(
    aeron_image_t *image, aeron_controlled_fragment_handler_t handler, void *clientd, int fragment_limit);
int aeron_image_bounded_poll(
    aeron_image_t *image, aeron_fragment_handler_t handler, void *clientd, int64_t limit_position, int fragment_limit);
int aeron_image_bounded_controlled_poll(
    aeron_image_t *image, aeron_controlled_fragment_handler_t handler,
    void *clientd, int64_t limit_position, int fragment_limit);
int64_t aeron_image_controlled_peek(
    aeron_image_t *image, aeron_controlled_fragment_handler_t handker, void *clientd, int64_t limit_position);
int aeron_image_block_poll(
    aeron_image_t *image, aeron_block_handler_t handler, void *clientd, size_t block_length_limit);

/*
 * Counter functions
 */

int aeron_counter_close(aeron_counter_t *counter);

/**
 * Return full version and build string.
 *
 * @return full version and build string.
 */
const char *aeron_version_full();

/**
 * Return major version number.
 *
 * @return major version number.
 */
int aeron_version_major();

/**
 * Return minor version number.
 *
 * @return minor version number.
 */
int aeron_version_minor();

/**
 * Return patch version number.
 *
 * @return patch version number.
 */
int aeron_version_patch();


/**
 * Clock function used by aeron.
 */
typedef int64_t (*aeron_clock_func_t)();

/**
 * Return time in nanoseconds for machine. Is not wall clock time.
 *
 * @return nanoseconds since epoch for machine.
 */
int64_t aeron_nano_clock();

/**
 * Return time in milliseconds since epoch. Is wall clock time.
 *
 * @return milliseconds since epoch.
 */
int64_t aeron_epoch_clock();

/**
 * Function to return logging information.
 */
typedef void (*aeron_log_func_t)(const char *);

/**
 * Determine if an aeron driver is using a given aeron directory.
 *
 * @param dirname  for aeron directory
 * @param timeout_ms  to use to determine activity for aeron directory
 * @param log_func to call during activity check to log diagnostic information.
 * @return true for active driver or false for no active driver.
 */
bool aeron_is_driver_active(const char *dirname, int64_t timeout_ms, aeron_log_func_t log_func);

/**
 * Load properties from a string containing name=value pairs and set appropriate environment variables for the
 * process so that subsequent calls to aeron_driver_context_init will use those values.
 *
 * @param buffer containing properties and values.
 * @return 0 for success and -1 for error.
 */
int aeron_properties_buffer_load(const char *buffer);

/**
 * Load properties file and set appropriate environment variables for the process so that subsequent
 * calls to aeron_driver_context_init will use those values.
 *
 * @param filename to load.
 * @return 0 for success and -1 for error.
 */
int aeron_properties_file_load(const char *filename);

/**
 * Load properties from HTTP URL and set environment variables for the process so that subsequent
 * calls to aeron_driver_context_init will use those values.
 *
 * @param url to attempt to retrieve and load.
 * @return 0 for success and -1 for error.
 */
int aeron_properties_http_load(const char *url);

/**
 * Load properties based on URL or filename. If string contains file or http URL, it will attempt
 * to load properties from a file or http as indicated. If not a URL, then it will try to load the string
 * as a filename.
 *
 * @param url_or_filename to load properties from.
 * @return 0 for success and -1 for error.
 */
int aeron_properties_load(const char *url_or_filename);

/**
 * Return current aeron error code (errno) for calling thread.
 *
 * @return aeron error code for calling thread.
 */
int aeron_errcode();

/**
 * Return the current aeron error message for calling thread.
 *
 * @return aeron error message for calling thread.
 */
const char *aeron_errmsg();

#ifdef __cplusplus
}
#endif

#endif //AERON_H
