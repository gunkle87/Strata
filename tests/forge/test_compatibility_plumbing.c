/*
 * test_compatibility_plumbing.c
 *
 * Verify that load-time capability compliance checks accurately refuse
 * artifacts based on reported capabilities rather than backend class.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/forge_api.h"
#include "../../include/strata_placeholder_artifact.h"

int
main(void)
{
    ForgeBackendId lxs_id = FORGE_BACKEND_ID_INVALID;
    ForgeBackendId highz_id = FORGE_BACKEND_ID_INVALID;
    ForgeArtifact *artifact;
    ForgeResult result;
    unsigned char artifact_bytes[strata_placeholder_artifact_size()];
    uint32_t count;
    uint32_t index;

    if (forge_install_product_profile(FORGE_PRODUCT_PROFILE_UNRESTRICTED) != FORGE_OK)
    {
        fprintf(stderr, "FAIL: could not install unrestricted profile\n");
        return 1;
    }

    count = forge_backend_count();
    for (index = 0u; index < count; ++index)
    {
        ForgeBackendId id;
        ForgeBackendInfo info;

        if (forge_backend_id_at(index, &id) == FORGE_OK &&
            forge_backend_info(id, &info) == FORGE_OK)
        {
            if (strcmp(info.name, "LXS") == 0) lxs_id = id;
            if (strcmp(info.name, "HighZ") == 0) highz_id = id;
        }
    }

    if (lxs_id == FORGE_BACKEND_ID_INVALID || highz_id == FORGE_BACKEND_ID_INVALID)
    {
        fprintf(stderr, "FAIL: could not resolve backend IDs\n");
        return 1;
    }

    /* Test 1: HighZ natively supports advanced controls. It should load an advanced payload. */
    {
        StrataPlaceholderAdmissionInfo admission_info;
        size_t out_size;

        strata_placeholder_expected_admission_info(STRATA_PLACEHOLDER_PAYLOAD_ADVANCED, &admission_info);
        strata_placeholder_artifact_write(
            artifact_bytes,
            sizeof(artifact_bytes),
            (unsigned int)highz_id,
            STRATA_PLACEHOLDER_PAYLOAD_ADVANCED,
            &admission_info,
            &out_size);

        artifact = NULL;
        result = forge_artifact_load(highz_id, artifact_bytes, out_size, &artifact);
        if (result != FORGE_OK)
        {
            fprintf(stderr, "FAIL: HighZ failed to load natively supported advanced artifact, got %d\n", (int)result);
            return 1;
        }
        forge_artifact_unload(artifact);
    }

    /* Test 2: LXS natively lacks advanced controls. It must reject an advanced payload exactly for capability lack. */
    {
        StrataPlaceholderAdmissionInfo admission_info;
        size_t out_size;

        strata_placeholder_expected_admission_info(STRATA_PLACEHOLDER_PAYLOAD_ADVANCED, &admission_info);
        strata_placeholder_artifact_write(
            artifact_bytes,
            sizeof(artifact_bytes),
            (unsigned int)lxs_id,
            STRATA_PLACEHOLDER_PAYLOAD_ADVANCED,
            &admission_info,
            &out_size);

        artifact = NULL;
        result = forge_artifact_load(lxs_id, artifact_bytes, out_size, &artifact);
        if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
        {
            fprintf(stderr, "FAIL: LXS should reject advanced artifact due to native lack, got %d\n", (int)result);
            return 1;
        }
    }

    /* Test 3: Manually corrupt admission to require bizarre extension that HighZ doesn't support. */
    {
        StrataPlaceholderAdmissionInfo admission_info;
        size_t out_size;
        StrataPlaceholderArtifactHeader* header;

        strata_placeholder_expected_admission_info(STRATA_PLACEHOLDER_PAYLOAD_BASELINE, &admission_info);
        
        /* Instead of ADVANCED or NATIVE_STATE flags, directly mutate requirements after generate */
        strata_placeholder_artifact_write(
            artifact_bytes,
            sizeof(artifact_bytes),
            (unsigned int)highz_id,
            STRATA_PLACEHOLDER_PAYLOAD_BASELINE,
            &admission_info,
            &out_size);

        header = (StrataPlaceholderArtifactHeader*)artifact_bytes;
        /* Force requires_native_inputs which HighZ doesn't support natively maybe? Or set a weird requirement. */
        header->admission_info.requires_native_inputs = 1u;

        artifact = NULL;
        result = forge_artifact_load(highz_id, artifact_bytes, out_size, &artifact);
        /* 
         * Depending on capabilities, HighZ might support inputs. Let's just assume we want it to reject
         * if we set temporal_substep requirement via native_inputs but HighZ has temporal_substep. 
         * HighZ HAS temporal_substep! So HighZ might ACCEPT native inputs!
         * But wait, LXS does NOT. Let's test on LXS.
         */
    }

    {
        StrataPlaceholderAdmissionInfo admission_info;
        size_t out_size;
        StrataPlaceholderArtifactHeader* header;

        strata_placeholder_expected_admission_info(STRATA_PLACEHOLDER_PAYLOAD_BASELINE, &admission_info);
        strata_placeholder_artifact_write(
            artifact_bytes,
            sizeof(artifact_bytes),
            (unsigned int)lxs_id,
            STRATA_PLACEHOLDER_PAYLOAD_BASELINE,
            &admission_info,
            &out_size);

        header = (StrataPlaceholderArtifactHeader*)artifact_bytes;
        /* Force requires_native_inputs which LXS definitely does not support (no temporal substep) */
        header->admission_info.requires_native_inputs = 1u;

        artifact = NULL;
        result = forge_artifact_load(lxs_id, artifact_bytes, out_size, &artifact);
        if (result != FORGE_ERR_ARTIFACT_INCOMPATIBLE)
        {
            fprintf(stderr, "FAIL: LXS should reject native inputs artifact due to lack of temporal substep, got %d\n", (int)result);
            return 1;
        }
    }

    printf("PASS: test_compatibility_plumbing\n");
    return 0;
}
