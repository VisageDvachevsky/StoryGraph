/**
 * @file test_speaker_identifier_validation.cpp
 * @brief Test for issue #92 - validating and sanitizing speaker identifiers
 *
 * This test verifies that speaker names are properly validated and sanitized
 * before being written to NMScript files, preventing runtime errors like
 * "Undefined character 'rfsfsddsf' [E3001]".
 */

#include <QString>
#include <iostream>
#include <vector>

namespace detail {

/// Checks if a Unicode code point is a valid identifier start character.
/// Matches the same rules as the NMScript lexer for consistency.
bool isUnicodeIdentifierStart(uint codePoint) {
  // ASCII letters
  if ((codePoint >= 'A' && codePoint <= 'Z') ||
      (codePoint >= 'a' && codePoint <= 'z')) {
    return true;
  }
  // Latin Extended-A, Extended-B, Extended Additional
  if (codePoint >= 0x00C0 && codePoint <= 0x024F)
    return true;
  // Cyrillic (Russian, Ukrainian, etc.)
  if (codePoint >= 0x0400 && codePoint <= 0x04FF)
    return true;
  // Cyrillic Supplement
  if (codePoint >= 0x0500 && codePoint <= 0x052F)
    return true;
  // Greek
  if (codePoint >= 0x0370 && codePoint <= 0x03FF)
    return true;
  // CJK Unified Ideographs (Chinese, Japanese Kanji)
  if (codePoint >= 0x4E00 && codePoint <= 0x9FFF)
    return true;
  // Hiragana
  if (codePoint >= 0x3040 && codePoint <= 0x309F)
    return true;
  // Katakana
  if (codePoint >= 0x30A0 && codePoint <= 0x30FF)
    return true;
  // Korean Hangul
  if (codePoint >= 0xAC00 && codePoint <= 0xD7AF)
    return true;
  // Arabic
  if (codePoint >= 0x0600 && codePoint <= 0x06FF)
    return true;
  // Hebrew
  if (codePoint >= 0x0590 && codePoint <= 0x05FF)
    return true;

  return false;
}

/// Checks if a Unicode code point is valid within an identifier (after start).
bool isUnicodeIdentifierPart(uint codePoint) {
  // All identifier start chars are also valid parts
  if (isUnicodeIdentifierStart(codePoint))
    return true;
  // ASCII digits
  if (codePoint >= '0' && codePoint <= '9')
    return true;
  // Unicode combining marks (accents, etc.)
  if (codePoint >= 0x0300 && codePoint <= 0x036F)
    return true;

  return false;
}

bool isValidSpeakerIdentifier(const QString &speaker) {
  if (speaker.isEmpty()) {
    return false;
  }

  // Check first character: must be a letter or underscore
  const QChar first = speaker.at(0);
  if (first != '_' && !isUnicodeIdentifierStart(first.unicode())) {
    return false;
  }

  // Check remaining characters: must be letters, digits, or underscores
  for (int i = 1; i < speaker.length(); ++i) {
    const QChar ch = speaker.at(i);
    if (ch != '_' && !isUnicodeIdentifierPart(ch.unicode())) {
      return false;
    }
  }

  return true;
}

QString sanitizeSpeakerIdentifier(const QString &speaker) {
  if (speaker.isEmpty()) {
    return QStringLiteral("Narrator");
  }

  // If already valid, return as-is
  if (isValidSpeakerIdentifier(speaker)) {
    return speaker;
  }

  QString result;
  result.reserve(speaker.length() + 1);

  for (int i = 0; i < speaker.length(); ++i) {
    const QChar ch = speaker.at(i);

    if (i == 0) {
      // First character must be a letter or underscore
      if (ch == '_' || isUnicodeIdentifierStart(ch.unicode())) {
        result += ch;
      } else if (ch.isDigit()) {
        // Prepend underscore if starts with digit
        result += '_';
        result += ch;
      } else {
        // Replace invalid first character with underscore
        result += '_';
      }
    } else {
      // Subsequent characters can be letters, digits, or underscores
      if (ch == '_' || isUnicodeIdentifierPart(ch.unicode())) {
        result += ch;
      } else {
        // Replace invalid character with underscore
        result += '_';
      }
    }
  }

  // Ensure result is not empty after sanitization
  if (result.isEmpty() || result == "_") {
    return QStringLiteral("Narrator");
  }

  return result;
}

} // namespace detail

struct TestCase {
  QString input;
  bool expectedValid;
  QString expectedSanitized;
  QString description;
};

int main() {
  std::vector<TestCase> tests = {
      // Valid ASCII identifiers
      {"Hero", true, "Hero", "Simple ASCII identifier"},
      {"Narrator", true, "Narrator", "Standard narrator"},
      {"Character1", true, "Character1", "Identifier with digit"},
      {"_private", true, "_private", "Starts with underscore"},
      {"MainCharacter", true, "MainCharacter", "CamelCase identifier"},

      // Valid Unicode identifiers
      {"Персонаж", true, "Персонаж", "Cyrillic identifier"},
      {"Алексей", true, "Алексей", "Russian name"},
      {"Герой1", true, "Герой1", "Cyrillic with digit"},
      {"英雄", true, "英雄", "Chinese characters"},
      {"キャラ", true, "キャラ", "Japanese Katakana"},

      // Invalid identifiers that need sanitization
      {"rfsfsddsf", true, "rfsfsddsf",
       "Issue #92 repro - this IS valid (all letters)"},
      {"123scene", false, "_123scene", "Starts with digit"},
      {"my-scene", false, "my_scene", "Contains hyphen"},
      {"scene name", false, "scene_name", "Contains space"},
      {"test@user", false, "test_user", "Contains @ symbol"},
      {"user#1", false, "user_1", "Contains # symbol"},

      // Edge cases
      {"", false, "Narrator", "Empty string"},
      {"_", true, "_", "Single underscore"},
      {"123", false, "_123", "All digits"},
      {"@#$", false, "Narrator", "All special characters"},
      {"-start", false, "_start", "Starts with hyphen"},
      {"end-", false, "end_", "Ends with hyphen"},
  };

  int passed = 0;
  int failed = 0;

  std::cout << "=== Testing Speaker Identifier Validation (Issue #92) ==="
            << std::endl;
  std::cout << std::endl;

  for (const auto &test : tests) {
    bool isValid = detail::isValidSpeakerIdentifier(test.input);
    QString sanitized = detail::sanitizeSpeakerIdentifier(test.input);

    bool validOk = (isValid == test.expectedValid);
    bool sanitizedOk = (sanitized == test.expectedSanitized);

    if (validOk && sanitizedOk) {
      std::cout << "[PASS] " << test.description.toStdString() << std::endl;
      ++passed;
    } else {
      std::cout << "[FAIL] " << test.description.toStdString() << std::endl;
      std::cout << "       Input: \"" << test.input.toStdString() << "\""
                << std::endl;
      if (!validOk) {
        std::cout << "       isValid: expected " << test.expectedValid
                  << ", got " << isValid << std::endl;
      }
      if (!sanitizedOk) {
        std::cout << "       sanitized: expected \""
                  << test.expectedSanitized.toStdString() << "\", got \""
                  << sanitized.toStdString() << "\"" << std::endl;
      }
      ++failed;
    }
  }

  std::cout << std::endl;
  std::cout << "=== Results ===" << std::endl;
  std::cout << "Passed: " << passed << std::endl;
  std::cout << "Failed: " << failed << std::endl;

  if (failed > 0) {
    std::cout << std::endl;
    std::cout << "=== TESTS FAILED ===" << std::endl;
    return 1;
  }

  std::cout << std::endl;
  std::cout << "=== ALL TESTS PASSED ===" << std::endl;
  return 0;
}
