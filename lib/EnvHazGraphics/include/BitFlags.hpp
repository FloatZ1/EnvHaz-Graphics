#ifndef BIT_FLAG_HPP
#define BIT_FLAG_HPP
#include <array>
#include <boost/serialization/serialization.hpp>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>
namespace eHazGraphics {

enum class SlotType : uint8_t {
  SLOT_NONE,
  DYNAMIC_SLOT_1,
  DYNAMIC_SLOT_2,
  DYNAMIC_SLOT_3,
  VERTEX_SLOT,
  INDEX_SLOT

};

enum class TypeFlags : uint32_t {

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
  BUFFER_ANIMATED_MESH_DATA = 1 << 17,
  BUFFER_DEBUG_DRAW_CALL_DATA = 1 << 18,
  BUFFER_DEBUG_SHAPE_DATA_UINT = 1 << 20,
  BUFFER_DEBUG_SHAPE_MATRIX_DATA = 1 << 19,
  BUFFER_GI_PROBE_DATA = 1 << 21,
  BUFFER_GI_GRID_DATA = 1 << 22,

  SHADER_TYPE_VERTEX_SHADER = 1 << 5,
  SHADER_TYPE_FRAGMENT_SHADER = 1 << 6,
  SHADER_TYPE_TESSALATION_SHADER = 1 << 8,
  SHADER_TYPE_GEOMETRY_SHADER = 1 << 9,
  SHADER_ERROR_NO_SHADER_ATTACHED = 1 << 10 // newest is 22

};

enum class ShaderManagerFlags : uint32_t {
  NONE = 0, // No special flags

  // Depth testing
  DEPTH_TEST_DISABLED = 1 << 0,  // glDisable(GL_DEPTH_TEST)
  DEPTH_TEST_ENABLED = 1 << 1,   // glEnable(GL_DEPTH_TEST)
  DEPTH_WRITE_DISABLED = 1 << 2, // glDepthMask(GL_FALSE)
  DEPTH_WRITE_ENABLED = 1 << 3,  // glDepthMask(GL_TRUE)
  DEPTH_LESS_EQUAL = 1 << 4,     // glDepthFunc(GL_LEQUAL)
  DEPTH_LESS = 1 << 5,           // glDepthFunc(GL_LESS)

  // Blending
  BLEND_DISABLED = 1 << 6, // glDisable(GL_BLEND)
  BLEND_ENABLED = 1 << 7,  // glEnable(GL_BLEND)
  BLEND_ALPHA = 1 << 8,    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
  BLEND_ADDITIVE = 1 << 9, // glBlendFunc(GL_ONE, GL_ONE)

  // Face culling
  CULL_FACE_DISABLED = 1 << 10, // glDisable(GL_CULL_FACE)
  CULL_FACE_ENABLED = 1 << 11,  // glEnable(GL_CULL_FACE)

  // Wireframe
  WIREFRAME_DISABLED = 1 << 12, // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)
  WIREFRAME_ENABLED = 1 << 13,  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)

  // Stencil testing
  STENCIL_TEST_DISABLED = 1 << 14, // glDisable(GL_STENCIL_TEST)
  STENCIL_TEST_ENABLED = 1 << 15   // glEnable(GL_STENCIL_TEST)
};

static const std::vector<std::string> ShaderManagerFlags_names = {
    "NONE",

    "DEPTH_TEST_DISABLED",
    "DEPTH_TEST_ENABLED",
    "DEPTH_WRITE_DISABLED",
    "DEPTH_WRITE_ENABLED",
    "DEPTH_LESS_EQUAL",
    "DEPTH_LESS",

    "BLEND_DISABLED",
    "BLEND_ENABLED",
    "BLEND_ALPHA",
    "BLEND_ADDITIVE",

    "CULL_FACE_DISABLED",
    "CULL_FACE_ENABLED",

    "WIREFRAME_DISABLED",
    "WIREFRAME_ENABLED",

    "STENCIL_TEST_DISABLED",
    "STENCIL_TEST_ENABLED"};

enum class SimpleShapes {

  SHAPE_CUBE,
  SHAPE_SPHERE,
  SHAPE_CAPSULE

};

template <typename Enum> constexpr auto to_underlying(Enum e) {
  return std::underlying_type_t<Enum>(e);
}

/*template <typename Enum> constexpr Enum operator|(Enum rhs, Enum lhs) {
  return static_cast<Enum>(to_underlying(rhs) | to_underlying(lhs));
}

template <typename Enum> constexpr Enum operator&(Enum rhs, Enum lhs) {
  return static_cast<Enum>(to_underlying(rhs) & to_underlying(lhs));
}

template <typename Enum> constexpr Enum operator~(Enum rhs) {
  return static_cast<Enum>(to_underlying(rhs));
}  */

// from  https://dietertack.medium.com/using-bit-flags-in-c-d39ec6e30f08
template <typename Enum> struct BitFlag {
  using Storage = std::underlying_type_t<Enum>;

  Storage FlagValue = 0;

  void FlipFlag(Enum flag) { FlagValue ^= to_underlying(flag); }

  void SetFlag(Enum flag) { FlagValue |= to_underlying(flag); }

  void UnsetFlag(Enum flag) { FlagValue &= ~((to_underlying(flag))); }
  bool HasFlag(Enum flag) {
    return (FlagValue & to_underlying(flag)) == to_underlying(flag);
  }
  bool HasAnyFlag(Enum multiFlag) {
    return (FlagValue & to_underlying(multiFlag)) != 0;
  }

  void SetFlagsFrom(const BitFlag &other) { FlagValue |= other.FlagValue; }

  void UnsetFlagsFrom(const BitFlag &other) { FlagValue &= ~other.FlagValue; }

  void CopyFlagsFrom(const BitFlag &other) { FlagValue = other.FlagValue; }

  void SetFlagsFromType(Storage p_Flags);

  void KeepCommonFlags(const BitFlag &other) { FlagValue &= other.FlagValue; }

private:
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & FlagValue;
  }
};

} // namespace eHazGraphics

#endif
