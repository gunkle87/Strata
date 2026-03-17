#include <string.h>
#include "forge_policy.h"

static ForgeBuildCapabilitySet s_build_capabilities;
static ForgeProductExposureProfile s_product_profile;
static ForgeSessionRestrictionProfile s_session_profile;
static uint32_t s_policy_initialized = 0u;

static uint32_t
forge_extension_mask_for_family(ForgeExtensionFamily extension_family)
{
    switch (extension_family)
    {
        case FORGE_EXT_PERFORMANCE_PROFILE:
            return 1u << 0;
        case FORGE_EXT_TEMPORAL_CONTROL:
            return 1u << 1;
        case FORGE_EXT_NATIVE_STATE_READ:
            return 1u << 2;
        case FORGE_EXT_RUNTIME_DIAGNOSTICS:
            return 1u << 3;
        default:
            return 0u;
    }
}

static uint32_t
forge_backend_mask_for_id(ForgeBackendId backend_id)
{
    switch (backend_id)
    {
        case 1u:
            return FORGE_BACKEND_MASK_LXS;
        case 2u:
            return FORGE_BACKEND_MASK_HIGHZ;
        default:
            return FORGE_BACKEND_MASK_NONE;
    }
}

static void
forge_policy_set_defaults(void)
{
    memset(&s_build_capabilities, 0, sizeof(s_build_capabilities));
    memset(&s_product_profile, 0, sizeof(s_product_profile));
    memset(&s_session_profile, 0, sizeof(s_session_profile));

    s_build_capabilities.backend_mask = FORGE_BACKEND_MASK_ALL;
    s_build_capabilities.extension_mask =
        forge_extension_mask_for_family(FORGE_EXT_PERFORMANCE_PROFILE) |
        forge_extension_mask_for_family(FORGE_EXT_TEMPORAL_CONTROL) |
        forge_extension_mask_for_family(FORGE_EXT_NATIVE_STATE_READ) |
        forge_extension_mask_for_family(FORGE_EXT_RUNTIME_DIAGNOSTICS);
    s_build_capabilities.probe_visibility_mask = FORGE_PROBE_VISIBILITY_MASK_ALL;
    s_build_capabilities.allow_common_observation = 1u;
    s_build_capabilities.allow_native_state_read = 1u;
    s_build_capabilities.allow_common_inputs = 1u;
    s_build_capabilities.allow_native_inputs = 1u;
    s_build_capabilities.allow_common_probes = 1u;
    s_build_capabilities.allow_advanced_controls = 1u;

    s_product_profile.visible_backend_mask = s_build_capabilities.backend_mask;
    s_product_profile.allowed_extension_mask = s_build_capabilities.extension_mask;
    s_product_profile.visible_probe_class_mask =
        FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC |
        FORGE_PROBE_VISIBILITY_MASK_BACKEND_PUBLIC |
        FORGE_PROBE_VISIBILITY_MASK_BACKEND_PRIVILEGED;
    s_product_profile.allow_common_observation = 1u;
    s_product_profile.allow_native_state_read = 1u;
    s_product_profile.allow_common_inputs = 1u;
    s_product_profile.allow_native_inputs = 1u;
    s_product_profile.allow_common_probes = 1u;
    s_product_profile.allow_advanced_controls = 1u;

    s_session_profile.visible_backend_mask = FORGE_BACKEND_MASK_ALL;
    s_session_profile.allowed_extension_mask = s_build_capabilities.extension_mask;
    s_session_profile.visible_probe_class_mask = FORGE_PROBE_VISIBILITY_MASK_ALL;
    s_session_profile.allow_common_observation = 1u;
    s_session_profile.allow_native_state_read = 1u;
    s_session_profile.allow_common_inputs = 1u;
    s_session_profile.allow_native_inputs = 1u;
    s_session_profile.allow_common_probes = 1u;
    s_session_profile.allow_advanced_controls = 1u;

    s_policy_initialized = 1u;
}

static void
forge_policy_ensure_initialized(void)
{
    if (!s_policy_initialized)
    {
        forge_policy_set_defaults();
    }
}

static void
forge_policy_compute_effective_profile(
    const ForgeBuildCapabilitySet *build,
    const ForgeProductExposureProfile *product,
    const ForgeSessionRestrictionProfile *session,
    ForgeEffectiveProfile *out_effective)
{
    if (!build || !product || !session || !out_effective)
    {
        return;
    }

    out_effective->visible_backend_mask =
        build->backend_mask &
        product->visible_backend_mask &
        session->visible_backend_mask;

    out_effective->allowed_extension_mask =
        build->extension_mask &
        product->allowed_extension_mask &
        session->allowed_extension_mask;

    out_effective->visible_probe_class_mask =
        build->probe_visibility_mask &
        product->visible_probe_class_mask &
        session->visible_probe_class_mask;

    out_effective->allow_common_observation =
        build->allow_common_observation &&
        product->allow_common_observation &&
        session->allow_common_observation;

    out_effective->allow_native_state_read =
        build->allow_native_state_read &&
        product->allow_native_state_read &&
        session->allow_native_state_read;

    out_effective->allow_common_inputs =
        build->allow_common_inputs &&
        product->allow_common_inputs &&
        session->allow_common_inputs;

    out_effective->allow_native_inputs =
        build->allow_native_inputs &&
        product->allow_native_inputs &&
        session->allow_native_inputs;

    out_effective->allow_common_probes =
        build->allow_common_probes &&
        product->allow_common_probes &&
        session->allow_common_probes;

    out_effective->allow_advanced_controls =
        build->allow_advanced_controls &&
        product->allow_advanced_controls &&
        session->allow_advanced_controls;
}

void
forge_policy_reset_defaults(void)
{
    forge_policy_set_defaults();
}

void
forge_policy_get_build_capabilities(ForgeBuildCapabilitySet *out_build)
{
    forge_policy_ensure_initialized();

    if (out_build)
    {
        *out_build = s_build_capabilities;
    }
}

void
forge_policy_get_installed_product_profile(ForgeProductExposureProfile *out_product)
{
    forge_policy_ensure_initialized();

    if (out_product)
    {
        *out_product = s_product_profile;
    }
}

void
forge_policy_get_library_effective_profile(ForgeEffectiveProfile *out_effective)
{
    ForgeSessionRestrictionProfile no_session_narrowing;

    forge_policy_ensure_initialized();

    no_session_narrowing.visible_backend_mask = FORGE_BACKEND_MASK_ALL;
    no_session_narrowing.allowed_extension_mask = s_build_capabilities.extension_mask;
    no_session_narrowing.visible_probe_class_mask = FORGE_PROBE_VISIBILITY_MASK_ALL;
    no_session_narrowing.allow_common_observation = 1u;
    no_session_narrowing.allow_native_state_read = 1u;
    no_session_narrowing.allow_common_inputs = 1u;
    no_session_narrowing.allow_native_inputs = 1u;
    no_session_narrowing.allow_common_probes = 1u;
    no_session_narrowing.allow_advanced_controls = 1u;

    forge_policy_compute_effective_profile(
        &s_build_capabilities,
        &s_product_profile,
        &no_session_narrowing,
        out_effective);
}

void
forge_policy_get_session_effective_profile(ForgeEffectiveProfile *out_effective)
{
    forge_policy_ensure_initialized();

    forge_policy_compute_effective_profile(
        &s_build_capabilities,
        &s_product_profile,
        &s_session_profile,
        out_effective);
}

void
forge_policy_build_product_profile_kind(
    ForgeProductProfileKind profile_kind,
    ForgeProductExposureProfile *out_profile)
{
    forge_policy_ensure_initialized();

    if (!out_profile)
    {
        return;
    }

    *out_profile = s_product_profile;

    switch (profile_kind)
    {
        case FORGE_PRODUCT_PROFILE_UNRESTRICTED:
            out_profile->visible_backend_mask = s_build_capabilities.backend_mask;
            out_profile->allowed_extension_mask = s_build_capabilities.extension_mask;
            out_profile->visible_probe_class_mask =
                FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC |
                FORGE_PROBE_VISIBILITY_MASK_BACKEND_PUBLIC |
                FORGE_PROBE_VISIBILITY_MASK_BACKEND_PRIVILEGED;
            out_profile->allow_common_observation = 1u;
            out_profile->allow_native_state_read = 1u;
            out_profile->allow_common_inputs = 1u;
            out_profile->allow_native_inputs = 1u;
            out_profile->allow_common_probes = 1u;
            out_profile->allow_advanced_controls = 1u;
            break;
        case FORGE_PRODUCT_PROFILE_LXS_ONLY:
            out_profile->visible_backend_mask = FORGE_BACKEND_MASK_LXS;
            out_profile->allowed_extension_mask =
                forge_extension_mask_for_family(FORGE_EXT_PERFORMANCE_PROFILE) |
                forge_extension_mask_for_family(FORGE_EXT_RUNTIME_DIAGNOSTICS);
            out_profile->visible_probe_class_mask =
                FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC;
            out_profile->allow_common_observation = 1u;
            out_profile->allow_native_state_read = 0u;
            out_profile->allow_common_inputs = 1u;
            out_profile->allow_native_inputs = 0u;
            out_profile->allow_common_probes = 1u;
            out_profile->allow_advanced_controls = 0u;
            break;
        case FORGE_PRODUCT_PROFILE_COMMON_ONLY:
            out_profile->visible_backend_mask = s_build_capabilities.backend_mask;
            out_profile->allowed_extension_mask =
                forge_extension_mask_for_family(FORGE_EXT_RUNTIME_DIAGNOSTICS);
            out_profile->visible_probe_class_mask =
                FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC;
            out_profile->allow_common_observation = 1u;
            out_profile->allow_native_state_read = 0u;
            out_profile->allow_common_inputs = 1u;
            out_profile->allow_native_inputs = 0u;
            out_profile->allow_common_probes = 1u;
            out_profile->allow_advanced_controls = 0u;
            break;
        default:
            break;
    }
}

void
forge_policy_build_session_profile_kind(
    ForgeSessionProfileKind profile_kind,
    ForgeSessionRestrictionProfile *out_profile)
{
    forge_policy_ensure_initialized();

    if (!out_profile)
    {
        return;
    }

    out_profile->visible_backend_mask = FORGE_BACKEND_MASK_ALL;
    out_profile->allowed_extension_mask = s_build_capabilities.extension_mask;
    out_profile->visible_probe_class_mask =
        FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC |
        FORGE_PROBE_VISIBILITY_MASK_BACKEND_PUBLIC |
        FORGE_PROBE_VISIBILITY_MASK_BACKEND_PRIVILEGED;
    out_profile->allow_common_observation = 1u;
    out_profile->allow_native_state_read = 1u;
    out_profile->allow_common_inputs = 1u;
    out_profile->allow_native_inputs = 1u;
    out_profile->allow_common_probes = 1u;
    out_profile->allow_advanced_controls = 1u;

    switch (profile_kind)
    {
        case FORGE_SESSION_PROFILE_DEFAULT:
            break;
        case FORGE_SESSION_PROFILE_COMMON_ONLY:
            out_profile->visible_probe_class_mask =
                FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC;
            out_profile->allow_native_state_read = 0u;
            out_profile->allow_native_inputs = 0u;
            out_profile->allow_advanced_controls = 0u;
            break;
        case FORGE_SESSION_PROFILE_NO_PROBES:
            out_profile->visible_probe_class_mask = 0u;
            out_profile->allow_common_probes = 0u;
            break;
        default:
            break;
    }
}

uint32_t
forge_policy_profile_is_valid(
    const ForgeBuildCapabilitySet *build,
    const ForgeProductExposureProfile *product)
{
    if (!build || !product)
    {
        return 0u;
    }

    if ((product->visible_backend_mask & ~build->backend_mask) != 0u)
    {
        return 0u;
    }

    if ((product->allowed_extension_mask & ~build->extension_mask) != 0u)
    {
        return 0u;
    }

    if ((product->visible_probe_class_mask & ~build->probe_visibility_mask) != 0u)
    {
        return 0u;
    }

    return 1u;
}

uint32_t
forge_policy_session_profile_is_valid(
    const ForgeBuildCapabilitySet *build,
    const ForgeProductExposureProfile *product,
    const ForgeSessionRestrictionProfile *session)
{
    if (!build || !product || !session)
    {
        return 0u;
    }

    if ((session->visible_backend_mask & ~build->backend_mask) != 0u)
    {
        return 0u;
    }

    if ((session->visible_backend_mask & ~product->visible_backend_mask) != 0u)
    {
        return 0u;
    }

    if ((session->allowed_extension_mask & ~build->extension_mask) != 0u)
    {
        return 0u;
    }

    if ((session->allowed_extension_mask & ~product->allowed_extension_mask) != 0u)
    {
        return 0u;
    }

    if ((session->visible_probe_class_mask & ~build->probe_visibility_mask) != 0u)
    {
        return 0u;
    }

    if ((session->visible_probe_class_mask & ~product->visible_probe_class_mask) != 0u)
    {
        return 0u;
    }

    if (session->allow_common_observation > product->allow_common_observation ||
        session->allow_native_state_read > product->allow_native_state_read ||
        session->allow_common_inputs > product->allow_common_inputs ||
        session->allow_native_inputs > product->allow_native_inputs ||
        session->allow_common_probes > product->allow_common_probes ||
        session->allow_advanced_controls > product->allow_advanced_controls)
    {
        return 0u;
    }

    return 1u;
}

void
forge_policy_install_product_profile(
    const ForgeProductExposureProfile *profile)
{
    forge_policy_ensure_initialized();

    if (profile)
    {
        s_product_profile = *profile;
    }
}

void
forge_policy_install_session_profile(
    const ForgeSessionRestrictionProfile *profile)
{
    forge_policy_ensure_initialized();

    if (profile)
    {
        s_session_profile = *profile;
    }
}

uint32_t
forge_policy_backend_visible(
    const ForgeEffectiveProfile *profile,
    ForgeBackendId backend_id)
{
    uint32_t backend_mask;

    if (!profile)
    {
        return 0u;
    }

    backend_mask = forge_backend_mask_for_id(backend_id);
    return (profile->visible_backend_mask & backend_mask) != 0u;
}

uint32_t
forge_policy_extension_allowed(
    const ForgeEffectiveProfile *profile,
    ForgeExtensionFamily extension_family)
{
    uint32_t extension_mask;

    if (!profile)
    {
        return 0u;
    }

    extension_mask = forge_extension_mask_for_family(extension_family);
    return (profile->allowed_extension_mask & extension_mask) != 0u;
}

uint32_t
forge_policy_probe_visibility_allowed(
    const ForgeEffectiveProfile *profile,
    ForgeProbeVisibilityClass visibility_class)
{
    uint32_t visibility_mask;

    if (!profile)
    {
        return 0u;
    }

    switch (visibility_class)
    {
        case FORGE_PROBE_VISIBILITY_COMMON_PUBLIC:
            visibility_mask = FORGE_PROBE_VISIBILITY_MASK_COMMON_PUBLIC;
            break;
        case FORGE_PROBE_VISIBILITY_BACKEND_PUBLIC:
            visibility_mask = FORGE_PROBE_VISIBILITY_MASK_BACKEND_PUBLIC;
            break;
        case FORGE_PROBE_VISIBILITY_BACKEND_PRIVILEGED:
            visibility_mask = FORGE_PROBE_VISIBILITY_MASK_BACKEND_PRIVILEGED;
            break;
        case FORGE_PROBE_VISIBILITY_INTERNAL_ONLY:
            visibility_mask = FORGE_PROBE_VISIBILITY_MASK_INTERNAL_ONLY;
            break;
        default:
            return 0u;
    }

    return (profile->visible_probe_class_mask & visibility_mask) != 0u;
}
