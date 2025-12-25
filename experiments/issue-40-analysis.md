# Issue #40 Analysis: Scene ID Not Set for Story Graph Nodes

## Problem Description
Right-clicking on a scene node in the Story Graph and selecting "Edit Dialogue Flow" resulted in a "Scene not selected" error instead of opening the dialogue editor.

## Root Cause
In `editor/src/qt/panels/nm_story_graph_panel.cpp`, the `ensureNode` lambda function creates scene nodes but only sets `m_sceneId` when layout data exists:

```cpp
// Original problematic code (lines 195-204)
if (layoutIt != m_layoutNodes.constEnd()) {
  node->setDialogueSpeaker(layoutIt->speaker);
  node->setDialogueText(layoutIt->dialogueText);
  node->setChoiceOptions(layoutIt->choices);
  // Scene Node specific properties
  node->setSceneId(layoutIt->sceneId);  // <-- Only called if layout exists!
  node->setHasEmbeddedDialogue(layoutIt->hasEmbeddedDialogue);
  node->setDialogueCount(layoutIt->dialogueCount);
  node->setThumbnailPath(layoutIt->thumbnailPath);
}
```

### Call Chain Analysis
1. User creates/loads a scene node without layout data
2. `ensureNode()` creates the node with `sceneId` parameter but doesn't set `m_sceneId`
3. User right-clicks the node → context menu appears
4. User selects "Edit Dialogue Flow"
5. `NMGraphNodeItem::contextMenuEvent()` emits `editDialogueFlowRequested(m_sceneId)` with empty string
6. `NMMainWindow` routes to `NMSceneDialogueGraphPanel::loadSceneDialogue("")`
7. Dialog panel shows "No Scene Loaded" error

## Solution
Always initialize `m_sceneId` immediately after node creation, allowing layout data to override if available:

```cpp
// Fixed code
auto *node = m_scene->addNode(title, type, pos, 0, sceneId);
if (node) {
  // Always set sceneId from the parameter to ensure it's never empty
  node->setSceneId(sceneId);

  // ... other initialization ...

  if (layoutIt != m_layoutNodes.constEnd()) {
    // ... other layout properties ...
    // Scene Node specific properties - override with layout data if available
    if (!layoutIt->sceneId.isEmpty()) {
      node->setSceneId(layoutIt->sceneId);
    }
    // ... remaining properties ...
  }
}
```

## Key Files Modified
- `editor/src/qt/panels/nm_story_graph_panel.cpp` (lines 189-205)

## Testing Strategy
1. Create a new scene node in Story Graph
2. Right-click on the node
3. Select "Edit Dialogue Flow"
4. Verify dialogue editor opens with correct scene ID
5. Test existing nodes with layout data still work correctly

## Related Code Locations
- Context menu: `editor/src/qt/panels/nm_story_graph_node.cpp:645-658`
- Signal connection: `editor/src/qt/nm_main_window_connections.cpp:967-985`
- Dialog panel: `editor/src/qt/panels/nm_scene_dialogue_graph_panel.cpp:154-174`
