# Scene Separation Design Document

## Issue Reference
- **Issue**: #345 - Design and implement clear scene separation in Story Graph workflow
- **Related Issues**: #321, #327, #344, #329

## Executive Summary

This document proposes a phased implementation of clear scene separation in the Story Graph workflow. The goal is to establish a cleaner mental model where:
- Scenes act as visual containers for narrative content
- Dialogues and choices are conceptually "inside" scenes
- Scene transitions represent visual/layout changes
- The workflow clearly distinguishes between navigation levels

## Current State Analysis

### Existing Architecture

The NovelMind editor already has foundational support for scene separation:

1. **Scene Nodes are visually distinct**:
   - Larger height (100px vs 80px for other nodes)
   - Green color scheme (border, header, text)
   - Scene-specific icon
   - Dialogue count badge
   - Embedded dialogue indicator

2. **Scene Dialogue Graph Panel exists**:
   - Separate panel for editing dialogue flow within a scene
   - Filtered node palette (no Scene nodes allowed)
   - Breadcrumb navigation back to Story Graph
   - Stores dialogue in `{sceneId}_dialogue.json`

3. **Scene Registry tracks cross-references**:
   - Maintains scene metadata
   - Tracks Story Graph node references
   - Supports auto-sync when scenes change

### Current Limitations

1. **Visual Hierarchy is Flat**: All nodes appear at the same visual "level" in the Story Graph
2. **No Scene Boundaries**: No visual indication of which nodes belong to which scene
3. **Navigation is Unclear**: Double-click on Scene nodes opens dialogue editor, but UX is not intuitive
4. **Transition Intent is Ambiguous**: No visual distinction between "stay in scene" vs "go to new scene" connections

## Proposed Design

### Approach: Enhanced Visualization with Progressive Disclosure

Based on research of similar tools (Twine, Yarn Spinner, Ink) and the existing codebase, we propose an approach that:
- Enhances the main Story Graph with scene container visualization
- Uses progressive disclosure to manage complexity
- Maintains backward compatibility with existing workflows

### Phase 1: Scene Container Visualization (Visual Grouping)

Add visual boundaries around scene nodes and their associated dialogue nodes.

#### 1.1 Scene Container Rectangle

Draw a rounded rectangle around each Scene node that:
- Contains the Scene node and all connected dialogue/choice nodes
- Uses the scene's green color scheme with low opacity
- Shows scene name in the top-left corner
- Can be collapsed to show only the Scene node

```cpp
// In nm_story_graph_scene.cpp - drawBackground()
void NMStoryGraphScene::drawSceneContainers(QPainter* painter) {
    for (auto* node : m_nodes) {
        if (node->isSceneNode()) {
            QRectF containerBounds = calculateSceneContainerBounds(node);
            drawSceneContainer(painter, containerBounds, node->sceneId());
        }
    }
}

QRectF NMStoryGraphScene::calculateSceneContainerBounds(NMGraphNodeItem* sceneNode) {
    QRectF bounds = sceneNode->boundingRect();
    bounds.translate(sceneNode->pos());

    // Include all dialogue nodes reachable before next Scene node
    for (auto* node : findDialogueNodesInScene(sceneNode)) {
        bounds = bounds.united(node->boundingRect().translated(node->pos()));
    }

    return bounds.adjusted(-20, -30, 20, 20); // Padding
}
```

#### 1.2 Scene Container Styling

```cpp
void NMStoryGraphScene::drawSceneContainer(QPainter* painter,
                                            const QRectF& bounds,
                                            const QString& sceneId) {
    // Semi-transparent fill
    QColor fillColor(100, 200, 150, 30); // Green with low opacity
    painter->setBrush(fillColor);

    // Dashed border
    QPen borderPen(QColor(100, 200, 150, 100), 2, Qt::DashLine);
    painter->setPen(borderPen);

    // Draw rounded rectangle
    painter->drawRoundedRect(bounds, 12, 12);

    // Draw scene label in top-left
    QFont labelFont = NMStyleManager::instance().defaultFont();
    labelFont.setPointSize(labelFont.pointSize() - 1);
    painter->setFont(labelFont);
    painter->setPen(QColor(100, 200, 150, 180));
    painter->drawText(bounds.topLeft() + QPointF(8, -4), sceneId);
}
```

### Phase 2: Scene Collapse/Expand

Allow users to collapse scenes to reduce visual complexity.

#### 2.1 Collapse State Management

```cpp
// Add to NMGraphNodeItem
class NMGraphNodeItem : public QGraphicsItem {
    // ... existing members ...
    bool m_isCollapsed = false;
    QList<NMGraphNodeItem*> m_embeddedNodes; // Cached for collapsed state

public:
    void setCollapsed(bool collapsed);
    bool isCollapsed() const { return m_isCollapsed; }
};
```

#### 2.2 Collapse Button on Scene Nodes

Add a small collapse/expand button to Scene nodes:

```cpp
// In NMGraphNodeItem::paint() for Scene nodes
if (isScene) {
    // Draw collapse button in header
    QRectF collapseBtn(NODE_WIDTH - 24, 8, 14, 14);
    painter->setBrush(m_isCollapsed ? QColor(100, 180, 100) : QColor(80, 80, 80));
    painter->setPen(QPen(QColor(120, 120, 120), 1));
    painter->drawRoundedRect(collapseBtn, 2, 2);

    // Draw +/- icon
    painter->setPen(QPen(Qt::white, 2));
    painter->drawLine(collapseBtn.center() - QPointF(4, 0),
                     collapseBtn.center() + QPointF(4, 0));
    if (m_isCollapsed) {
        painter->drawLine(collapseBtn.center() - QPointF(0, 4),
                         collapseBtn.center() + QPointF(0, 4));
    }
}
```

#### 2.3 Collapsed State Rendering

When collapsed, show a summary on the Scene node:

```
┌────────────────────────────────────┐
│ 🎬 Scene                           │
├────────────────────────────────────┤
│ Opening Scene                      │
│                                    │
│ [5 dialogues, 2 choices]      [-]  │
│ Click to expand                    │
└────────────────────────────────────┘
```

### Phase 3: Transition Visual Indicators

Distinguish between same-scene and cross-scene connections.

#### 3.1 Connection Types

```cpp
enum class ConnectionType {
    SameScene,      // Dialogue to dialogue within scene
    SceneTransition, // Any connection from/to Scene node
    CrossScene      // Between different scene containers
};
```

#### 3.2 Visual Differentiation

```cpp
void NMGraphConnectionItem::paint(QPainter* painter, ...) {
    ConnectionType connType = determineConnectionType();

    switch (connType) {
        case ConnectionType::SameScene:
            // Standard line, subtle color
            painter->setPen(QPen(palette.connectionLine, 2));
            break;

        case ConnectionType::SceneTransition:
            // Thicker, emphasized line with scene color
            painter->setPen(QPen(QColor(100, 200, 150), 3));
            break;

        case ConnectionType::CrossScene:
            // Animated/dashed line indicating scene change
            QPen dashPen(QColor(255, 200, 100), 2, Qt::DashLine);
            painter->setPen(dashPen);
            break;
    }

    painter->drawPath(m_path);

    // Add transition icon for scene transitions
    if (connType == ConnectionType::SceneTransition) {
        drawTransitionIndicator(painter, m_path.pointAtPercent(0.5));
    }
}
```

#### 3.3 Transition Indicator Icon

```cpp
void NMGraphConnectionItem::drawTransitionIndicator(QPainter* painter,
                                                     const QPointF& pos) {
    // Draw small scene transition icon (two overlapping rectangles)
    const qreal size = 12;
    QRectF rect1(pos.x() - size/2, pos.y() - size/2, size*0.7, size*0.7);
    QRectF rect2 = rect1.translated(size*0.3, size*0.3);

    painter->setBrush(QColor(100, 200, 150, 200));
    painter->setPen(QPen(QColor(60, 160, 110), 1));
    painter->drawRoundedRect(rect1, 2, 2);
    painter->drawRoundedRect(rect2, 2, 2);
}
```

### Phase 4: Scene Focus Mode

Allow drilling into a single scene for focused editing.

#### 4.1 Focus Mode UI

```cpp
void NMStoryGraphPanel::enterSceneFocusMode(const QString& sceneId) {
    m_focusedSceneId = sceneId;
    m_focusModeBanner->setVisible(true);
    m_focusModeLabel->setText(tr("Editing: %1").arg(sceneId));

    // Dim/hide nodes outside the focused scene
    for (auto* node : m_scene->nodes()) {
        if (!isNodeInScene(node, sceneId)) {
            node->setOpacity(0.2);
            node->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }
    }

    // Center view on focused scene
    m_view->centerOnScene(sceneId);
}

void NMStoryGraphPanel::exitSceneFocusMode() {
    m_focusedSceneId.clear();
    m_focusModeBanner->setVisible(false);

    // Restore all nodes
    for (auto* node : m_scene->nodes()) {
        node->setOpacity(1.0);
        node->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
}
```

#### 4.2 Focus Mode Banner

```
┌────────────────────────────────────────────────────┐
│ ← Back to Story Graph │ Editing: Opening Scene     │
└────────────────────────────────────────────────────┘
```

### Data Model Considerations

#### Option A: Embedded Dialogue Nodes (Recommended)

Store dialogue node references within scene metadata:

```json
{
  "scenes": {
    "opening_scene": {
      "sceneId": "opening_scene",
      "title": "Opening Scene",
      "embeddedNodes": ["dialogue_1", "dialogue_2", "choice_1"],
      "position": { "x": 100, "y": 200 }
    }
  },
  "nodes": {
    "dialogue_1": { ... },
    "dialogue_2": { ... },
    "choice_1": { ... }
  }
}
```

**Advantages**:
- Clear ownership model
- Efficient rendering (know which nodes to draw in container)
- Supports collapse/expand naturally

**Migration**: Analyze existing connections to infer scene ownership.

#### Option B: Computed Scene Boundaries

Calculate scene boundaries at runtime based on graph structure:

```cpp
QList<NMGraphNodeItem*> findDialogueNodesInScene(NMGraphNodeItem* sceneNode) {
    QList<NMGraphNodeItem*> result;
    QSet<NMGraphNodeItem*> visited;
    QList<NMGraphNodeItem*> queue;

    // BFS from scene node, stop at other Scene nodes
    queue.append(sceneNode);
    while (!queue.isEmpty()) {
        auto* current = queue.takeFirst();
        if (visited.contains(current)) continue;
        visited.insert(current);

        for (auto* conn : findOutgoingConnections(current)) {
            auto* target = conn->endNode();
            if (!target->isSceneNode()) {
                result.append(target);
                queue.append(target);
            }
        }
    }
    return result;
}
```

**Advantages**:
- No migration needed
- Works with existing data

**Disadvantages**:
- Must recalculate on every change
- Ambiguous in complex graphs

### Recommended Implementation Order

1. **Phase 1** (Low risk, high impact): Scene container visualization
   - Can be toggled off if users don't like it
   - No data model changes required

2. **Phase 2** (Medium risk, high usability): Collapse/expand
   - Requires state management
   - Significant UX improvement for complex graphs

3. **Phase 3** (Low risk, medium impact): Transition indicators
   - Pure visual enhancement
   - Helps clarify workflow

4. **Phase 4** (Medium risk, high focus): Scene focus mode
   - More complex implementation
   - Excellent for detailed editing

### Implementation Files

| Feature | Files to Modify |
|---------|-----------------|
| Scene containers | `nm_story_graph_scene.cpp`, `nm_story_graph_panel.cpp` |
| Collapse/expand | `nm_story_graph_node.cpp`, `nm_story_graph_panel.hpp` |
| Transition indicators | `nm_story_graph_connection.cpp` |
| Focus mode | `nm_story_graph_panel.cpp`, `nm_story_graph_panel.hpp` |

### Testing Strategy

1. **Visual Tests**: Screenshot comparisons for rendering
2. **Unit Tests**: Scene boundary calculations
3. **Integration Tests**: Collapse/expand state persistence
4. **UX Tests**: User workflow validation

### Open Questions for Discussion

1. **Container Drawing Order**: Should containers be drawn behind or blend with the grid?
2. **Auto-grouping**: Should newly created dialogue nodes auto-attach to nearest Scene?
3. **Multi-scene Editing**: Should focus mode allow viewing multiple scenes?
4. **Performance**: For very large graphs, should we virtualize container rendering?

### References

- [Twine](https://twinery.org/) - Passage-based visual editor
- [Yarn Spinner](https://yarnspinner.dev/) - Node-based dialogue system
- [Ink by Inkle](https://www.inklestudios.com/ink/) - Hierarchical narrative structure
- [Ren'Py](https://www.renpy.org/) - Label-based scene organization

---

*Document Version: 1.0*
*Created: 2026-01-09*
*Author: AI Issue Solver*
