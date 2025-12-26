#include <catch2/catch_test_macros.hpp>

// Test for PlaybackSourceMode enum (Issue #82)
// Note: This tests the enum values without Qt dependencies.
// Full UI tests would require Qt Test framework.

#include "NovelMind/editor/project_manager.hpp"

using namespace NovelMind::editor;

TEST_CASE("PlaybackSourceMode enum values", "[playback_source][issue_82]") {
  SECTION("Enum has expected values") {
    // Verify enum values are distinct and can be cast to integers
    REQUIRE(static_cast<int>(PlaybackSourceMode::Script) == 0);
    REQUIRE(static_cast<int>(PlaybackSourceMode::Graph) == 1);
    REQUIRE(static_cast<int>(PlaybackSourceMode::Mixed) == 2);
  }

  SECTION("Enum values are distinct") {
    REQUIRE(PlaybackSourceMode::Script != PlaybackSourceMode::Graph);
    REQUIRE(PlaybackSourceMode::Graph != PlaybackSourceMode::Mixed);
    REQUIRE(PlaybackSourceMode::Script != PlaybackSourceMode::Mixed);
  }
}

TEST_CASE("ProjectMetadata default playback source mode",
          "[playback_source][issue_82]") {
  ProjectMetadata meta;

  SECTION("Default value is Script mode") {
    // Script mode is the default to maintain backward compatibility
    REQUIRE(meta.playbackSourceMode == PlaybackSourceMode::Script);
  }

  SECTION("Playback source mode can be changed") {
    meta.playbackSourceMode = PlaybackSourceMode::Graph;
    REQUIRE(meta.playbackSourceMode == PlaybackSourceMode::Graph);

    meta.playbackSourceMode = PlaybackSourceMode::Mixed;
    REQUIRE(meta.playbackSourceMode == PlaybackSourceMode::Mixed);

    meta.playbackSourceMode = PlaybackSourceMode::Script;
    REQUIRE(meta.playbackSourceMode == PlaybackSourceMode::Script);
  }
}

TEST_CASE("PlaybackSourceMode round-trip conversion",
          "[playback_source][issue_82]") {
  // Test that enum values can be safely converted to/from integers
  // This is important for serialization/deserialization

  SECTION("Script mode round-trip") {
    int value = static_cast<int>(PlaybackSourceMode::Script);
    auto mode = static_cast<PlaybackSourceMode>(value);
    REQUIRE(mode == PlaybackSourceMode::Script);
  }

  SECTION("Graph mode round-trip") {
    int value = static_cast<int>(PlaybackSourceMode::Graph);
    auto mode = static_cast<PlaybackSourceMode>(value);
    REQUIRE(mode == PlaybackSourceMode::Graph);
  }

  SECTION("Mixed mode round-trip") {
    int value = static_cast<int>(PlaybackSourceMode::Mixed);
    auto mode = static_cast<PlaybackSourceMode>(value);
    REQUIRE(mode == PlaybackSourceMode::Mixed);
  }
}
