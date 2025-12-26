/**
 * @file issue_80_repro.cpp
 * @brief Reproduction test for issue #80 - multiple say statements generated
 *
 * Issue: When user sets speaker="система" and text="Леха привет" in Inspector,
 * the script contains:
 *   say система "Леха привет\n"
 *   say система ""
 *   say "New scene"
 *
 * Expected: Only one say statement with the specified speaker and text.
 */

#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <iostream>

// Copy of the updateSceneSayStatement function
bool updateSceneSayStatement(const QString &sceneId, const QString &scriptPath,
                             const QString &speaker, const QString &text) {
  if (sceneId.isEmpty() || scriptPath.isEmpty()) {
    return false;
  }

  QFile file(scriptPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  QString content = QString::fromUtf8(file.readAll());
  file.close();

  // Find the scene block
  const QRegularExpression sceneRe(
      QString("\\bscene\\s+%1\\b").arg(QRegularExpression::escape(sceneId)));
  const QRegularExpressionMatch match = sceneRe.match(content);
  if (!match.hasMatch()) {
    return false;
  }

  const qsizetype bracePos = content.indexOf('{', match.capturedEnd());
  if (bracePos < 0) {
    return false;
  }

  // Find the scene end
  int depth = 0;
  bool inString = false;
  QChar stringDelimiter;
  bool inLineComment = false;
  bool inBlockComment = false;
  qsizetype sceneEnd = -1;

  for (qsizetype i = bracePos; i < content.size(); ++i) {
    const QChar c = content.at(i);
    const QChar next = (i + 1 < content.size()) ? content.at(i + 1) : QChar();

    if (inLineComment) {
      if (c == '\n') {
        inLineComment = false;
      }
      continue;
    }
    if (inBlockComment) {
      if (c == '*' && next == '/') {
        inBlockComment = false;
        ++i;
      }
      continue;
    }

    if (!inString && c == '/' && next == '/') {
      inLineComment = true;
      ++i;
      continue;
    }
    if (!inString && c == '/' && next == '*') {
      inBlockComment = true;
      ++i;
      continue;
    }

    if (c == '"' || c == '\'') {
      if (!inString) {
        inString = true;
        stringDelimiter = c;
      } else if (stringDelimiter == c && content.at(i - 1) != '\\') {
        inString = false;
      }
    }

    if (inString) {
      continue;
    }

    if (c == '{') {
      ++depth;
    } else if (c == '}') {
      --depth;
      if (depth == 0) {
        sceneEnd = i;
        break;
      }
    }
  }

  if (sceneEnd < 0) {
    return false;
  }

  const qsizetype bodyStart = bracePos + 1;
  const qsizetype bodyEnd = sceneEnd;
  QString body = content.mid(bodyStart, bodyEnd - bodyStart);

  // Find and replace the first say statement
  // Pattern: say <speaker> "<text>" OR say "<text>"
  // FIX for issue #80: Make speaker optional with (?:(\\w+)\\s+)?
  const QRegularExpression sayRe("\\bsay\\s+(?:(\\w+)\\s+)?\"([^\"]*)\"",
                                 QRegularExpression::DotMatchesEverythingOption);

  QRegularExpressionMatch sayMatch = sayRe.match(body);
  if (!sayMatch.hasMatch()) {
    // No say statement found in the scene - add one at the beginning
    QString escapedText = text;
    escapedText.replace("\\", "\\\\");
    escapedText.replace("\"", "\\\"");

    QString newSay = QString("\n    say %1 \"%2\"")
                         .arg(speaker.isEmpty() ? "Narrator" : speaker,
                              escapedText);
    body = newSay + body;
  } else {
    // Replace the existing say statement
    QString escapedText = text;
    escapedText.replace("\\", "\\\\");
    escapedText.replace("\"", "\\\"");

    QString newSay =
        QString("say %1 \"%2\"")
            .arg(speaker.isEmpty() ? "Narrator" : speaker, escapedText);
    body.replace(sayMatch.capturedStart(), sayMatch.capturedLength(), newSay);
  }

  QString updated = content.left(bodyStart);
  updated += body;
  updated += content.mid(bodyEnd);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text |
                 QIODevice::Truncate)) {
    return false;
  }
  file.write(updated.toUtf8());
  file.close();
  return true;
}

int main() {
  const QString testScriptPath = "/tmp/issue_80_test.nms";

  // Scenario 1: Start with default "New scene"
  std::cout << "=== Scenario 1: Default scene ===" << std::endl;
  const QString initialContent = R"(// test_scene
scene test_scene {
    say "New scene"
}
)";

  QFile file1(testScriptPath);
  if (!file1.open(QIODevice::WriteOnly | QIODevice::Text)) {
    std::cerr << "Failed to create test file" << std::endl;
    return 1;
  }
  file1.write(initialContent.toUtf8());
  file1.close();

  std::cout << "Initial content:\n" << initialContent.toStdString() << std::endl;

  // Simulate user setting speaker="система" in Inspector
  std::cout << "\n--- Setting speaker to 'система' ---" << std::endl;
  updateSceneSayStatement("test_scene", testScriptPath, "система", "New scene");

  QFile file2(testScriptPath);
  if (!file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::cerr << "Failed to read file" << std::endl;
    return 1;
  }
  QString afterSpeaker = QString::fromUtf8(file2.readAll());
  file2.close();
  std::cout << "After setting speaker:\n" << afterSpeaker.toStdString() << std::endl;

  // Simulate user setting text="Леха привет" in Inspector
  std::cout << "--- Setting text to 'Леха привет' ---" << std::endl;
  updateSceneSayStatement("test_scene", testScriptPath, "система", "Леха привет");

  QFile file3(testScriptPath);
  if (!file3.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::cerr << "Failed to read file" << std::endl;
    return 1;
  }
  QString afterText = QString::fromUtf8(file3.readAll());
  file3.close();
  std::cout << "After setting text:\n" << afterText.toStdString() << std::endl;

  // Count how many say statements exist
  QRegularExpression countSay("\\bsay\\b");
  QRegularExpressionMatchIterator it = countSay.globalMatch(afterText);
  int sayCount = 0;
  while (it.hasNext()) {
    it.next();
    sayCount++;
  }

  std::cout << "\n=== Results ===" << std::endl;
  std::cout << "Number of 'say' statements found: " << sayCount << std::endl;

  if (sayCount == 1) {
    std::cout << "✓ PASS: Only one say statement (as expected)" << std::endl;
  } else {
    std::cout << "✗ FAIL: Found " << sayCount << " say statements (expected 1)" << std::endl;
    std::cout << "\nThis confirms issue #80" << std::endl;
  }

  // Clean up
  QFile::remove(testScriptPath);

  return (sayCount == 1) ? 0 : 1;
}
