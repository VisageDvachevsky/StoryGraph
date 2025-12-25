#include "NovelMind/editor/project_manager.hpp"
#include "NovelMind/editor/qt/nm_icon_manager.hpp"
#include "NovelMind/editor/qt/nm_play_mode_controller.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"
#include "NovelMind/editor/qt/nm_undo_manager.hpp"
#include "NovelMind/editor/qt/panels/nm_story_graph_panel.hpp"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QLineF>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSet>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <filesystem>

namespace NovelMind::editor::qt {

// ============================================================================
// NMGraphConnectionItem
// ============================================================================

NMGraphConnectionItem::NMGraphConnectionItem(NMGraphNodeItem *startNode,
                                             NMGraphNodeItem *endNode)
    : QGraphicsItem(), m_startNode(startNode), m_endNode(endNode) {
  setZValue(-1); // Draw behind nodes
  // Don't call updatePath() in constructor - let the scene call it after adding
}

void NMGraphConnectionItem::updatePath() {
  if (!m_startNode || !m_endNode)
    return;

  // Safety check - ensure nodes are still in a scene
  if (!m_startNode->scene() || !m_endNode->scene())
    return;

  // Extra safety: ensure this connection is also in a scene
  if (!scene())
    return;

  QPointF start = m_startNode->outputPortPosition();
  QPointF end = m_endNode->inputPortPosition();

  // Notify Qt that geometry will change before modifying the path
  prepareGeometryChange();

  // Create bezier curve
  m_path = QPainterPath();
  m_path.moveTo(start);

  qreal dx = std::abs(end.x() - start.x()) * 0.5;
  m_path.cubicTo(start + QPointF(dx, 0), end + QPointF(-dx, 0), end);

  // Request redraw
  update();
}

QRectF NMGraphConnectionItem::boundingRect() const {
  return m_path.boundingRect().adjusted(-5, -5, 5, 5);
}

void NMGraphConnectionItem::paint(QPainter *painter,
                                  const QStyleOptionGraphicsItem * /*option*/,
                                  QWidget * /*widget*/) {
  // Save painter state to prevent state leakage to other items
  painter->save();

  const auto &palette = NMStyleManager::instance().palette();

  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(QPen(palette.connectionLine, 2));
  painter->setBrush(Qt::NoBrush);
  painter->drawPath(m_path);

  // Restore painter state
  painter->restore();
}

// ============================================================================

} // namespace NovelMind::editor::qt
