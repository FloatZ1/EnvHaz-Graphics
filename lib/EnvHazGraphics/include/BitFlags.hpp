#ifndef BIT_FLAG_HPP
#define BIT_FLAG_HPP



#include <cstdint>
#include <type_traits>
namespace eHazGraphics
{


enum class TypeFlags : uint32_t
{

    BUFFER_ANIMATION_DATA = 1 << 0,
    BUFFER_MATRIX_DATA = 1 << 1,
    BUFFER_STATIC_DATA = 1 << 2,
    BUFFER_PARTICLE_DATA = 1 << 3,
    BUFFER_TEXTURE_DATA = 1 << 4,
    BUFFER_INSTANCE_DATA = 1 << 7,
    BUFFER_STATIC_TERRAIN_DATA = 1 << 11,
    BUFFER_STATIC_MESH_DATA = 1 << 12,
    BUFFER_DRAW_CALL_DATA = 1 << 13,
    BUFFER_CAMERA_DATA = 1 << 14,
    BUFFER_LIGHT_DATA = 1 << 15,
    BUFFER_STATIC_MATRIX_DATA = 1 << 16,


    SHADER_TYPE_VERTEX_SHADER = 1 << 5,
    SHADER_TYPE_FRAGMENT_SHADER = 1 << 6,
    SHADER_TYPE_TESSALATION_SHADER = 1 << 8,
    SHADER_TYPE_GEOMETRY_SHADER = 1 << 9,
    SHADER_ERROR_NO_SHADER_ATTACHED = 1 << 10 // newest is 16
};


enum class ShaderManagerFlags : uint32_t
{
    NONE = 0, // no special flags

    DISABLE_DEPTH_TEST = 1 << 0,  // glDisable(GL_DEPTH_TEST)
    ENABLE_BLEND = 1 << 1,        // glEnable(GL_BLEND)
    DISABLE_CULL_FACE = 1 << 2,   // glDisable(GL_CULL_FACE)
    ENABLE_WIREFRAME = 1 << 3,    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
    DISABLE_DEPTH_WRITE = 1 << 4, // glDepthMask(GL_FALSE)

    // Blending modes
    BLEND_ALPHA = 1 << 5,    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    BLEND_ADDITIVE = 1 << 6, // glBlendFunc(GL_ONE, GL_ONE)

    // Depth comparison tweaks
    DEPTH_LESS_EQUAL = 1 << 7, // glDepthFunc(GL_LEQUAL)

    // Stencil testing
    ENABLE_STENCIL_TEST = 1 << 8, // glEnable(GL_STENCIL_TEST)

    // Future flags (reserved for later use)
    RESERVED_1 = 1 << 9
};



template <typename Enum> constexpr auto to_underlying(Enum e)
{
    return std::underlying_type_t<Enum>(e);
}

template <typename Enum> constexpr Enum operator|(Enum rhs, Enum lhs)
{
    return static_cast<Enum>(to_underlying(rhs) | to_underlying(lhs));
}

template <typename Enum> constexpr Enum operator&(Enum rhs, Enum lhs)
{
    return static_cast<Enum>(to_underlying(rhs) & to_underlying(lhs));
}

template <typename Enum> constexpr Enum operator~(Enum rhs)
{
    return static_cast<Enum>(to_underlying(rhs));
}


// from  https://dietertack.medium.com/using-bit-flags-in-c-d39ec6e30f08
template <typename Enum> struct BitFlag
{
    using Storage = std::underlying_type_t<Enum>;

    Storage FlagValue = 0;

    void FlipFlag(Enum flag)
    {
        FlagValue ^= to_underlying(flag);
    }

    void SetFlag(Enum flag)
    {
        FlagValue |= to_underlying(flag);
    }

    void UnsetFlag(Enum flag)
    {
        FlagValue &= ~((to_underlying(flag)));
    }
    bool HasFlag(Enum flag)
    {
        return (FlagValue & to_underlying(flag)) == to_underlying(flag);
    }
    bool HasAnyFlag(Enum multiFlag)
    {
        return (FlagValue & to_underlying(multiFlag)) != 0;
    }

    void SetFlagsFrom(const BitFlag &other)
    {
        FlagValue |= other.FlagValue;
    }

    void UnsetFlagsFrom(const BitFlag &other)
    {
        FlagValue &= ~other.FlagValue;
    }

    void CopyFlagsFrom(const BitFlag &other)
    {
        FlagValue = other.FlagValue;
    }

    void KeepCommonFlags(const BitFlag &other)
    {
        FlagValue &= other.FlagValue;
    }
};





} // namespace eHazGraphics






#endif
