# Issue #80 Analysis and Fix

## Problem Summary

When editing dialogue in the StoryGraph Inspector:
- User sets speaker = "система"
- User sets text = "Леха привет"
- Expected: One `say` statement: `say система "Леха привет"`
- Actual: Multiple `say` statements generated:
  ```
  say система "Леха привет\n"
  say система ""
  say "New scene"
  ```

## Root Cause

The bug is in `editor/src/qt/panels/nm_story_graph_panel_detail.cpp` at line 514-516.

### The Buggy Code

```cpp
const QRegularExpression sayRe(
    "\\bsay\\s+(\\w+)\\s+\"([^\"]*)\"",
    QRegularExpression::DotMatchesEverythingOption);
```

This regex pattern **only matches** `say` statements WITH a speaker:
- ✅ Matches: `say Hero "Hello"`
- ❌ Does NOT match: `say "Hello"` (no speaker)

### Why This Causes the Bug

1. When a new scene node is created, the default script template includes:
   ```
   scene node_name {
       say "New scene"
   }
   ```
   (See `editor/src/qt/panels/nm_story_graph_scene.cpp:86`)

2. When user edits speaker/text in Inspector, `updateSceneSayStatement()` is called

3. The regex fails to match `say "New scene"` because there's no speaker

4. Code falls into the "No say statement found" branch (line 519-529)

5. It **ADDS** a new `say` statement instead of replacing the existing one

6. Result: Both statements exist in the scene

## The Fix

Change the regex to make the speaker optional using a non-capturing group:

```cpp
const QRegularExpression sayRe(
    "\\bsay\\s+(?:(\\w+)\\s+)?\"([^\"]*)\"",
    QRegularExpression::DotMatchesEverythingOption);
```

### Regex Explanation

- `\\bsay\\s+` - Match "say" keyword followed by whitespace
- `(?:(\\w+)\\s+)?` - **Optional** non-capturing group for speaker:
  - `(\\w+)` - Capture group 1: speaker name
  - `\\s+` - Whitespace after speaker
  - `?` - Makes entire speaker part optional
- `\"([^\"]*)\"` - Capture group 2: quoted text

Now the pattern matches both:
- ✅ `say Hero "Hello"` - speaker="Hero", text="Hello"
- ✅ `say "Hello"` - speaker=empty, text="Hello"

## Files Modified

1. `editor/src/qt/panels/nm_story_graph_panel_detail.cpp` - Main fix
2. `experiments/test_say_statement_sync.cpp` - Updated test
3. `experiments/issue_80_repro.cpp` - New reproduction test (created)

## Testing

The fix ensures that:
1. Default `say "New scene"` is properly detected and replaced
2. When user sets speaker + text, it replaces the default statement
3. No duplicate say statements are created
4. Existing functionality for `say speaker "text"` continues to work

## Related Code

- Scene creation: `editor/src/qt/panels/nm_story_graph_scene.cpp:86`
- Inspector property changes: `editor/src/qt/panels/nm_story_graph_panel_handlers.cpp:657-672`
- Say statement update function: `editor/src/qt/panels/nm_story_graph_panel_detail.cpp:416-553`
