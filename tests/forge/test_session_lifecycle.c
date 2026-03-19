/*
 * test_session_lifecycle.c
 *
 * Verify the stub session lifecycle:
 * - forge_session_create on a stub artifact returns FORGE_OK and a live handle.
 * - forge_session_reset succeeds on the live handle.
 * - forge_session_reset and forge_session_free with NULL return INVALID_HANDLE.
 * - forge_last_error_string is non-NULL after a failure.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/breadboard_api.h"
#include "../../include/forge_api.h"
#include "../../include/strata_placeholder_artifact.h"

static void
fill_stub_artifact(unsigned char *buffer, unsigned int target_backend_id)
{
    StrataPlaceholderAdmissionInfo admission_info;
    size_t out_size;

    if (!strata_placeholder_expected_admission_info(
        STRATA_PLACEHOLDER_PAYLOAD_BASELINE,
        &admission_info))
    {
        fprintf(stderr, "FAIL: could not build baseline placeholder admission info\n");
        exit(1);
    }

    (void)strata_placeholder_artifact_write(
        buffer,
        strata_placeholder_artifact_size(),
        target_backend_id,
        STRATA_PLACEHOLDER_PAYLOAD_BASELINE,
        &admission_info,
        &out_size);
}

static int
exercise_real_fast_session_lifecycle(ForgeBackendId backend_id)
{
    BreadboardModule *module = NULL;
    BreadboardArtifactDraft *draft = NULL;
    BreadboardCompileOptions options;
    BreadboardDescriptorSpec input_a = { 410u, "real_a", 1u };
    BreadboardDescriptorSpec output_y = { 412u, "real_y", 1u };
    BreadboardComponentSpec buffer = { 510u, "BUF", false };
    BreadboardExecutableConnectionSpec exec_in = {
        { BREADBOARD_ENDPOINT_MODULE_INPUT_SOURCE, 410u, 0u, 0u },
        { BREADBOARD_ENDPOINT_COMPONENT_INPUT_SINK, 0u, 510u, 0u }
    };
    BreadboardExecutableConnectionSpec exec_out = {
        { BREADBOARD_ENDPOINT_COMPONENT_OUTPUT_SOURCE, 0u, 510u, 0u },
        { BREADBOARD_ENDPOINT_MODULE_OUTPUT_SINK, 412u, 0u, 0u }
    };
    unsigned char *bytes = NULL;
    size_t size = 0u;
    ForgeArtifact *artifact = NULL;
    ForgeSession *session = NULL;
    ForgeSessionInfo session_info;
    ForgeResult result;
    BreadboardResult breadboard_result;

    memset(&options, 0, sizeof(options));
    options.allow_placeholders = false;
    options.deny_approximation = false;
    options.strict_projection = false;
    options.require_real_executable = true;

    if (breadboard_module_create(&module) != BREADBOARD_OK ||
        breadboard_module_set_target(module, BREADBOARD_TARGET_FAST_4STATE) != BREADBOARD_OK ||
        breadboard_module_add_input_descriptor(module, &input_a) != BREADBOARD_OK ||
        breadboard_module_add_output_descriptor(module, &output_y) != BREADBOARD_OK ||
        breadboard_module_add_component_instance(module, &buffer) != BREADBOARD_OK ||
        breadboard_module_add_executable_connection(module, &exec_in) != BREADBOARD_OK ||
        breadboard_module_add_executable_connection(module, &exec_out) != BREADBOARD_OK)
    {
        fprintf(stderr, "FAIL: could not build real FAST_4STATE module\n");
        breadboard_module_free(module);
        return 1;
    }

    breadboard_result = breadboard_module_compile(module, &options, &draft);
    if (breadboard_result != BREADBOARD_OK || !draft)
    {
        fprintf(stderr, "FAIL: could not compile real FAST_4STATE draft: %d\n",
            (int)breadboard_result);
        breadboard_module_free(module);
        return 1;
    }

    breadboard_result = breadboard_artifact_draft_export_fast_size(draft, &size);
    if (breadboard_result != BREADBOARD_OK)
    {
        fprintf(stderr, "FAIL: could not size real FAST_4STATE draft\n");
        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
        return 1;
    }

    bytes = (unsigned char *)malloc(size);
    if (!bytes)
    {
        fprintf(stderr, "FAIL: could not allocate real FAST_4STATE bytes\n");
        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
        return 1;
    }

    breadboard_result = breadboard_artifact_draft_export_fast(draft, bytes, size, &size);
    if (breadboard_result != BREADBOARD_OK)
    {
        fprintf(stderr, "FAIL: could not export real FAST_4STATE draft\n");
        free(bytes);
        breadboard_artifact_draft_free(draft);
        breadboard_module_free(module);
        return 1;
    }

    breadboard_artifact_draft_free(draft);
    breadboard_module_free(module);

    result = forge_artifact_load(backend_id, bytes, size, &artifact);
    free(bytes);
    bytes = NULL;
    if (result != FORGE_OK || !artifact)
    {
        fprintf(stderr, "FAIL: real FAST_4STATE artifact should load, got %d\n",
            (int)result);
        return 1;
    }

    result = forge_session_create(artifact, &session);
    if (result != FORGE_OK || !session)
    {
        fprintf(stderr, "FAIL: real FAST_4STATE session should create, got %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_info(session, &session_info);
    if (result != FORGE_OK ||
        session_info.backend_id != backend_id ||
        session_info.lifecycle_state != FORGE_SESSION_STATE_READY ||
        session_info.placeholder_state != 0u)
    {
        fprintf(stderr, "FAIL: real FAST_4STATE session info mismatch\n");
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_reset(session);
    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: real FAST_4STATE session reset failed: %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_session_info(session, &session_info);
    if (result != FORGE_OK ||
        session_info.backend_id != backend_id ||
        session_info.lifecycle_state != FORGE_SESSION_STATE_READY ||
        session_info.placeholder_state != 0u)
    {
        fprintf(stderr, "FAIL: real FAST_4STATE session info after reset mismatch\n");
        forge_session_free(session);
        forge_artifact_unload(artifact);
        return 1;
    }

    {
        ForgeSignalValue real_inputs[1] = { { 410u, FORGE_LOGIC_1 } };
        ForgeSignalValue real_outputs[1];
        ForgeSignalValue bad_inputs[1] = { { 999u, FORGE_LOGIC_1 } };
        ForgeSignalValue bad_value_inputs[1] = { { 410u, (ForgeLogicValue)99 } };

        result = forge_apply_inputs(session, real_inputs, 1u);
        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: real FAST_4STATE apply inputs failed: %d\n",
                (int)result);
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_step(session, 1u);
        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: real FAST_4STATE step failed: %d\n",
                (int)result);
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_read_outputs(session, real_outputs, 1u);
        if (result != FORGE_OK ||
            real_outputs[0].signal_id != 412u ||
            real_outputs[0].value != FORGE_LOGIC_1)
        {
            fprintf(stderr, "FAIL: real FAST_4STATE output read mismatch\n");
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_step(session, 1u);
        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: real FAST_4STATE repeated step failed: %d\n",
                (int)result);
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_read_outputs(session, real_outputs, 1u);
        if (result != FORGE_OK ||
            real_outputs[0].signal_id != 412u ||
            real_outputs[0].value != FORGE_LOGIC_1)
        {
            fprintf(stderr, "FAIL: real FAST_4STATE repeated output read mismatch\n");
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_apply_inputs(session, bad_inputs, 1u);
        if (result != FORGE_ERR_OUT_OF_BOUNDS)
        {
            fprintf(stderr, "FAIL: real FAST_4STATE invalid input id should fail, got %d\n",
                (int)result);
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_apply_inputs(session, bad_value_inputs, 1u);
        if (result != FORGE_ERR_INVALID_ARGUMENT)
        {
            fprintf(stderr, "FAIL: real FAST_4STATE invalid input value should fail, got %d\n",
                (int)result);
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_apply_inputs(NULL, real_inputs, 1u);
        if (result != FORGE_ERR_INVALID_HANDLE)
        {
            fprintf(stderr, "FAIL: forge_apply_inputs(NULL, ...) expected INVALID_HANDLE, got %d\n",
                (int)result);
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_step(NULL, 1u);
        if (result != FORGE_ERR_INVALID_HANDLE)
        {
            fprintf(stderr, "FAIL: forge_step(NULL, ...) expected INVALID_HANDLE, got %d\n",
                (int)result);
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }

        result = forge_read_outputs(NULL, real_outputs, 1u);
        if (result != FORGE_ERR_INVALID_HANDLE)
        {
            fprintf(stderr, "FAIL: forge_read_outputs(NULL, ...) expected INVALID_HANDLE, got %d\n",
                (int)result);
            forge_session_free(session);
            forge_artifact_unload(artifact);
            return 1;
        }
    }

    result = forge_session_free(session);
    session = NULL;
    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: real FAST_4STATE session free failed: %d\n",
            (int)result);
        forge_artifact_unload(artifact);
        return 1;
    }

    result = forge_artifact_unload(artifact);
    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: real FAST_4STATE artifact unload failed: %d\n",
            (int)result);
        return 1;
    }

    return 0;
}

int main(void)
{
    ForgeBackendId  id      = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact  *art     = NULL;
    ForgeSession   *session = NULL;
    ForgeSessionInfo session_info;
    ForgeResult     result;
    unsigned char   artifact_bytes[strata_placeholder_artifact_size()];

    result = forge_backend_id_at(0, &id);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not get backend id at index 0: %d\n", (int)result);
        return 1;
    }

    fill_stub_artifact(artifact_bytes, (unsigned int)id);

    /* Load a stub artifact so we have a valid handle to pass. */
    result = forge_artifact_load(id, artifact_bytes, sizeof(artifact_bytes), &art);

    if (result != FORGE_OK || !art)
    {
        fprintf(stderr, "FAIL: could not load stub artifact: %d\n", (int)result);
        return 1;
    }

    /* Session creation now succeeds for the stub lifecycle path. */
    result = forge_session_create(art, &session);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_session_create expected FORGE_OK, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    if (session == NULL)
    {
        fprintf(stderr,
            "FAIL: session handle should be non-NULL after FORGE_OK\n");
        forge_artifact_unload(art);
        return 1;
    }

    result = forge_session_info(session, &session_info);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_session_info expected FORGE_OK, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(art);
        return 1;
    }

    if (session_info.backend_id != id ||
        session_info.lifecycle_state != FORGE_SESSION_STATE_READY ||
        session_info.placeholder_state != 0)
    {
        fprintf(stderr, "FAIL: forge_session_info returned unexpected metadata\n");
        forge_session_free(session);
        forge_artifact_unload(art);
        return 1;
    }

    result = forge_session_reset(session);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_session_reset expected FORGE_OK, got %d\n",
            (int)result);
        forge_session_free(session);
        forge_artifact_unload(art);
        return 1;
    }

    result = forge_session_free(session);

    if (result != FORGE_OK)
    {
        fprintf(stderr,
            "FAIL: forge_session_free expected FORGE_OK, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    session = NULL;

    /* Null-handle paths must still return INVALID_HANDLE. */
    result = forge_session_reset(NULL);

    if (result != FORGE_ERR_INVALID_HANDLE)
    {
        fprintf(stderr,
            "FAIL: forge_session_reset(NULL) expected FORGE_ERR_INVALID_HANDLE, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    result = forge_session_free(NULL);

    if (result != FORGE_ERR_INVALID_HANDLE)
    {
        fprintf(stderr,
            "FAIL: forge_session_free(NULL) expected FORGE_ERR_INVALID_HANDLE, got %d\n",
            (int)result);
        forge_artifact_unload(art);
        return 1;
    }

    if (!forge_last_error_string())
    {
        fprintf(stderr, "FAIL: forge_last_error_string returned NULL after failure\n");
        forge_artifact_unload(art);
        return 1;
    }

    /* Clean up artifact after session release. */
    result = forge_artifact_unload(art);

    if (result != FORGE_OK)
    {
        fprintf(stderr, "FAIL: forge_artifact_unload returned %d\n", (int)result);
        return 1;
    }

    if (exercise_real_fast_session_lifecycle(id) != 0)
    {
        return 1;
    }

    printf("PASS: test_session_lifecycle\n");
    return 0;
}
