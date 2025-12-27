#pragma once

#include "NovelMind/editor/qt/panels/nm_story_graph_panel.hpp"

namespace NovelMind::editor::qt::detail {

bool loadGraphLayout(QHash<QString, NMStoryGraphPanel::LayoutNode> &nodes,
                     QString &entryScene);
void saveGraphLayout(const QHash<QString, NMStoryGraphPanel::LayoutNode> &nodes,
                     const QString &entryScene);
QString resolveScriptPath(const NMGraphNodeItem *node);
bool updateSceneGraphBlock(const QString &sceneId, const QString &scriptPath,
                           const QStringList &targets);
bool updateSceneSayStatement(const QString &sceneId, const QString &scriptPath,
                             const QString &speaker, const QString &text);
QStringList splitChoiceLines(const QString &raw);
NMStoryGraphPanel::LayoutNode buildLayoutFromNode(const NMGraphNodeItem *node);

/// Validates if a speaker name is a valid NMScript identifier.
/// Valid identifiers must start with a Unicode letter or underscore,
/// followed by letters, digits, or underscores.
/// @param speaker The speaker name to validate
/// @return true if the speaker is a valid identifier
bool isValidSpeakerIdentifier(const QString &speaker);

/// Sanitizes a speaker name to be a valid NMScript identifier.
/// Replaces invalid characters with underscores and ensures the name
/// starts with a valid character.
/// @param speaker The speaker name to sanitize
/// @return A sanitized version of the speaker name, or "Narrator" if empty
QString sanitizeSpeakerIdentifier(const QString &speaker);

} // namespace NovelMind::editor::qt::detail
