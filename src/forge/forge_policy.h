#ifndef FORGE_POLICY_H
#define FORGE_POLICY_H

#include <stdint.h>
#include "../../include/forge_api.h"
#include "../../include/forge_capabilities.h"
#include "../../include/forge_types.h"

enum
{
    FORGE_BACKEND_MASK_NONE  = 0u,
    FORGE_BACKEND_MASK_LXS   = 1u << 0,
    FORGE_BACKEND_MASK_HIGHZ = 1u << 1,
    FORGE_BACKEND_MASK_ALL   = FORGE_BACKEND_MASK_LXS | FORGE_BACKEND_MASK_HIGHZ
};

typedef enum ForgeProbeVisibilityClass
{
    FORGE_PROBE_VISIBILITY_INVALID            = 0,
    FORGE_PROBE_VISIBILITY_COMMON_PUBLIC      = 1,
    FORGE_PROBE_VISIBILITY_BACKEND_PUBLIC     = 2,
    FORGE_PROBE_VISIBILITY_BACKEND_PRIVILEGED = 3,
    FORGE_PROBE_VISIBILITY_INTERNAL_ONLY      = 4

}
ForgeProbeVisibilityClass;

#define FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC \
    (1u << (FORGE_PROBE_VISIBILITY_COMMON_PUBLIC - 1u))
#define FORGE_PROBE_VISIBILITY_MASK_BACKEND_PUBLIC \
    (1u << (FORGE_PROBE_VISIBILITY_BACKEND_PUBLIC - 1u))
#define FORGE_PROBE_VISIBILITY_MASK_BACKEND_PRIVILEGED \
    (1u << (FORGE_PROBE_VISIBILITY_BACKEND_PRIVILEGED - 1u))
#define FORGE_PROBE_VISIBILITY_MASK_INTERNAL_ONLY \
    (1u << (FORGE_PROBE_VISIBILITY_INTERNAL_ONLY - 1u))
#define FORGE_PROBE_VISIBILITY_MASK_ALL \
    (FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC | \
     FORGE_PROBE_VISIBILITY_MASK_BACKEND_PUBLIC | \
     FORGE_PROBE_VISIBILITY_MASK_BACKEND_PRIVILEGED | \
     FORGE_PROBE_VISIBILITY_MASK_INTERNAL_ONLY)

typedef struct ForgeBuildCapabilitySet
{
    uint32_t backend_mask;
    uint32_t extension_mask;
    uint32_t probe_visibility_mask;
    uint32_t allow_common_observation;
    uint32_t allow_native_state_read;
    uint32_t allow_common_inputs;
    uint32_t allow_native_inputs;
    uint32_t allow_common_probes;
    uint32_t allow_advanced_controls;

}
ForgeBuildCapabilitySet;

typedef struct ForgeProductExposureProfile
{
    uint32_t visible_backend_mask;
    uint32_t allowed_extension_mask;
    uint32_t visible_probe_class_mask;
    uint32_t allow_common_observation;
    uint32_t allow_native_state_read;
    uint32_t allow_common_inputs;
    uint32_t allow_native_inputs;
    uint32_t allow_common_probes;
    uint32_t allow_advanced_controls;

}
ForgeProductExposureProfile;

typedef struct ForgeSessionRestrictionProfile
{
    uint32_t visible_backend_mask;
    uint32_t allowed_extension_mask;
    uint32_t visible_probe_class_mask;
    uint32_t allow_common_observation;
    uint32_t allow_native_state_read;
    uint32_t allow_common_inputs;
    uint32_t allow_native_inputs;
    uint32_t allow_common_probes;
    uint32_t allow_advanced_controls;

}
ForgeSessionRestrictionProfile;

typedef struct ForgeEffectiveProfile
{
    uint32_t visible_backend_mask;
    uint32_t allowed_extension_mask;
    uint32_t visible_probe_class_mask;
    uint32_t allow_common_observation;
    uint32_t allow_native_state_read;
    uint32_t allow_common_inputs;
    uint32_t allow_native_inputs;
    uint32_t allow_common_probes;
    uint32_t allow_advanced_controls;

}
ForgeEffectiveProfile;

typedef struct ForgeInternalDescriptor
{
    ForgeDescriptor descriptor;
    ForgeProbeVisibilityClass probe_visibility_class;

}
ForgeInternalDescriptor;

void forge_policy_reset_defaults(void);
void forge_policy_get_installed_product_profile(ForgeProductExposureProfile *out_product);
void forge_policy_get_library_effective_profile(ForgeEffectiveProfile *out_effective);
void forge_policy_get_session_effective_profile(ForgeEffectiveProfile *out_effective);
void forge_policy_build_product_profile_kind(
    ForgeProductProfileKind profile_kind,
    ForgeProductExposureProfile *out_profile);
void forge_policy_build_session_profile_kind(
    ForgeSessionProfileKind profile_kind,
    ForgeSessionRestrictionProfile *out_profile);
uint32_t forge_policy_profile_is_valid(
    const ForgeBuildCapabilitySet *build,
    const ForgeProductExposureProfile *product);
uint32_t forge_policy_session_profile_is_valid(
    const ForgeBuildCapabilitySet *build,
    const ForgeProductExposureProfile *product,
    const ForgeSessionRestrictionProfile *session);
void forge_policy_get_build_capabilities(ForgeBuildCapabilitySet *out_build);
void forge_policy_install_product_profile(
    const ForgeProductExposureProfile *profile);
void forge_policy_install_session_profile(
    const ForgeSessionRestrictionProfile *profile);
uint32_t forge_policy_backend_visible(
    const ForgeEffectiveProfile *profile,
    ForgeBackendId backend_id);
uint32_t forge_policy_extension_allowed(
    const ForgeEffectiveProfile *profile,
    ForgeExtensionFamily extension_family);
uint32_t forge_policy_probe_visibility_allowed(
    const ForgeEffectiveProfile *profile,
    ForgeProbeVisibilityClass visibility_class);

#endif /* FORGE_POLICY_H */
