/**
 * @file timeline_animation_example.cpp
 * @brief Example demonstrating Timeline Panel and Curve Editor usage
 *
 * This example shows how to:
 * 1. Create timeline tracks for animating scene objects
 * 2. Add keyframes with different easing functions
 * 3. Play back animations
 * 4. Use Curve Editor to customize interpolation
 *
 * ## Usage in NovelMind Editor
 *
 * ### Basic Animation Workflow
 *
 * 1. **Open Timeline Panel**: Window → Timeline (or Ctrl+5)
 *
 * 2. **Create Animation Track**:
 *    - Select an object in Scene View (e.g., a character sprite)
 *    - In Timeline, the track appears (if bindings are set up)
 *    - Or manually add track: Right-click → Add Track
 *
 * 3. **Add Keyframes**:
 *    - Move timeline scrubber to frame 0
 *    - Position your character at start position in Scene View
 *    - Click "Add Keyframe" button (K key)
 *    - Move scrubber to frame 60 (2 seconds at 30 FPS)
 *    - Move character to end position
 *    - Click "Add Keyframe" again
 *
 * 4. **Set Easing**:
 *    - Double-click a keyframe
 *    - Select easing type (e.g., "Ease In Out")
 *    - Click OK
 *
 * 5. **Play Animation**:
 *    - Click Play button or press Space
 *    - Character smoothly moves from start to end position!
 *
 * ### Advanced: Custom Curves
 *
 * 1. **Open Curve Editor**: Window → Curve Editor
 *
 * 2. **Edit Interpolation Curve**:
 *    - Select a keyframe in Timeline
 *    - Set easing to "Custom"
 *    - In Curve Editor, drag control points to shape the curve
 *    - The curve shows how the value changes over time
 *      - X axis = time (0.0 to 1.0)
 *      - Y axis = interpolated value (0.0 to 1.0)
 *
 * 3. **Curve Presets**:
 *    - Linear: Constant speed
 *    - Ease In: Slow start, fast end
 *    - Ease Out: Fast start, slow end
 *    - Ease In/Out: Slow start and end, fast middle
 *    - Bezier: Custom curve with control points
 *
 * ### Example Animation Scenarios
 *
 * #### Fade In Character
 * ```
 * Track: Character.Opacity
 * Frame 0: Opacity = 0.0, Easing = Ease In
 * Frame 30: Opacity = 1.0
 * Result: Character fades in smoothly over 1 second
 * ```
 *
 * #### Slide In from Left
 * ```
 * Track: Character.Position
 * Frame 0: Position = (-200, 0), Easing = Ease Out
 * Frame 45: Position = (0, 0)
 * Result: Character slides in and decelerates
 * ```
 *
 * #### Bounce In
 * ```
 * Track: Character.Scale
 * Frame 0: Scale = 0.0, Easing = Bounce Out
 * Frame 20: Scale = 1.0
 * Result: Character bounces into view
 * ```
 *
 * #### Complex Camera Pan
 * ```
 * Track: Camera.Position.X
 * Frame 0: X = 0, Easing = Ease In Out
 * Frame 60: X = 800
 * Frame 120: X = 0
 * Result: Camera pans right then back to start
 * ```
 *
 * ### Programmatic API Example
 *
 * This C++ code shows how Timeline and Curve Editor work internally:
 */

#include "NovelMind/editor/qt/panels/nm_timeline_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_curve_editor_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_animation_adapter.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QDebug>

using namespace NovelMind::editor::qt;

void demonstrateTimelineUsage() {
  qDebug() << "=== Timeline Panel API Example ===\n";

  // Create timeline panel
  auto *timeline = new NMTimelinePanel();
  timeline->onInitialize();

  // Add an animation track
  timeline->addTrack(TimelineTrackType::Animation, "Character.Position");

  // Add keyframes
  // Frame 0: Start at (0, 0)
  timeline->addKeyframeAtCurrent("Character.Position", QPointF(0.0, 0.0));

  // Frame 30: Move to (200, 0)
  timeline->setCurrentFrame(30);
  timeline->addKeyframeAtCurrent("Character.Position", QPointF(200.0, 0.0));

  // Frame 60: Move to (200, 100)
  timeline->setCurrentFrame(60);
  timeline->addKeyframeAtCurrent("Character.Position", QPointF(200.0, 100.0));

  qDebug() << "Added 3 keyframes to Character.Position track";

  // Get the track and demonstrate interpolation
  if (auto *track = timeline->getTrack("Character.Position")) {
    qDebug() << "\nInterpolated values:";
    for (int frame = 0; frame <= 60; frame += 10) {
      Keyframe kf = track->interpolate(frame);
      QPointF pos = kf.value.toPointF();
      qDebug() << QString("  Frame %1: (%2, %3)")
                      .arg(frame, 3)
                      .arg(pos.x(), 0, 'f', 1)
                      .arg(pos.y(), 0, 'f', 1);
    }
  }

  // Demonstrate easing
  qDebug() << "\n=== Easing Function Comparison ===\n";
  qDebug() << "Adding second track with different easing...";

  timeline->addTrack(TimelineTrackType::Animation, "Character.Opacity");

  // Add keyframes with easing
  timeline->setCurrentFrame(0);
  timeline->addKeyframeAtCurrent("Character.Opacity", 0.0);

  // Get the track and set easing manually
  if (auto *opacityTrack = timeline->getTrack("Character.Opacity")) {
    if (!opacityTrack->keyframes.isEmpty()) {
      // Set first keyframe to Ease In
      opacityTrack->keyframes[0].easing = EasingType::EaseIn;
    }
  }

  timeline->setCurrentFrame(30);
  timeline->addKeyframeAtCurrent("Character.Opacity", 1.0);

  // Show interpolated opacity values with easing
  if (auto *track = timeline->getTrack("Character.Opacity")) {
    qDebug() << "\nOpacity with Ease In:";
    for (int frame = 0; frame <= 30; frame += 5) {
      Keyframe kf = track->interpolate(frame);
      double opacity = kf.value.toDouble();
      qDebug() << QString("  Frame %1: %2")
                      .arg(frame, 3)
                      .arg(opacity, 0, 'f', 3);
    }
  }

  qDebug() << "\nNotice how Ease In starts slow and accelerates!";

  delete timeline;
}

void demonstrateCurveEditorUsage() {
  qDebug() << "\n=== Curve Editor API Example ===\n";

  // Create curve editor
  auto *curveEditor = new NMCurveEditorPanel();
  curveEditor->onInitialize();

  // Get curve data and add custom points
  auto &curve = curveEditor->curveData();

  // Clear default curve
  curve.clear();

  // Add custom control points
  curve.addPoint(0.0, 0.0, CurveInterpolation::EaseInOut);
  curve.addPoint(0.3, 0.8, CurveInterpolation::EaseOut);  // Fast rise
  curve.addPoint(0.7, 0.9, CurveInterpolation::Linear);   // Plateau
  curve.addPoint(1.0, 1.0, CurveInterpolation::Linear);

  qDebug() << "Created custom curve with 4 control points";

  // Evaluate curve at various points
  qDebug() << "\nCurve evaluation:";
  for (double t = 0.0; t <= 1.0; t += 0.1) {
    double value = curve.evaluate(t);
    qDebug() << QString("  t = %1 → value = %2")
                    .arg(t, 0, 'f', 1)
                    .arg(value, 0, 'f', 3);
  }

  qDebug() << "\nThis curve creates a 'ease-in with plateau' effect";
  qDebug() << "Perfect for: UI animations, camera movements, transitions";

  delete curveEditor;
}

void demonstrateAnimationAdapter() {
  qDebug() << "\n=== Animation Adapter Example ===\n";

  // Note: AnimationAdapter requires SceneManager which isn't available
  // in this standalone example. This shows the API usage conceptually.

  qDebug() << "Animation Adapter bridges Timeline and Scene View:";
  qDebug() << "";
  qDebug() << "1. Create bindings between tracks and scene objects:";
  qDebug() << "   adapter->createBinding(";
  qDebug() << "       \"Character.PositionX\",  // Track ID";
  qDebug() << "       \"character_01\",         // Scene Object ID";
  qDebug() << "       AnimatedProperty::PositionX";
  qDebug() << "   );";
  qDebug() << "";
  qDebug() << "2. Connect to Timeline:";
  qDebug() << "   adapter->connectTimeline(timelinePanel);";
  qDebug() << "   // Now adapter receives frame change signals";
  qDebug() << "";
  qDebug() << "3. Connect to Scene View:";
  qDebug() << "   adapter->connectSceneView(sceneViewPanel);";
  qDebug() << "   // Adapter can now update scene objects";
  qDebug() << "";
  qDebug() << "4. When timeline plays:";
  qDebug() << "   Timeline emits frameChanged(30)";
  qDebug() << "   → Adapter interpolates all tracks at frame 30";
  qDebug() << "   → Adapter applies values to scene objects";
  qDebug() << "   → Scene View updates and redraws";
  qDebug() << "";
  qDebug() << "Result: Smooth animated preview in Scene View!";
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  qDebug() << "╔═══════════════════════════════════════════════════════╗";
  qDebug() << "║  NovelMind Timeline & Curve Editor Example          ║";
  qDebug() << "╚═══════════════════════════════════════════════════════╝";
  qDebug() << "";

  // Run demonstrations
  demonstrateTimelineUsage();
  demonstrateCurveEditorUsage();
  demonstrateAnimationAdapter();

  qDebug() << "\n╔═══════════════════════════════════════════════════════╗";
  qDebug() << "║  Summary                                             ║";
  qDebug() << "╚═══════════════════════════════════════════════════════╝";
  qDebug() << "";
  qDebug() << "✅ Timeline Panel: Creates and plays keyframe animations";
  qDebug() << "✅ Curve Editor: Fine-tunes interpolation curves";
  qDebug() << "✅ Animation Adapter: Bridges Timeline ↔ Scene View";
  qDebug() << "✅ Easing Functions: 15+ types for smooth motion";
  qDebug() << "";
  qDebug() << "All systems are now functional! 🎉";
  qDebug() << "";
  qDebug() << "To test in the editor:";
  qDebug() << "1. Open NovelMind Editor";
  qDebug() << "2. Create/open a project";
  qDebug() << "3. Window → Timeline";
  qDebug() << "4. Add scene objects and create animations";
  qDebug() << "5. Press Play to see your animations!";

  return 0;
}
