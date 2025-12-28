/**
 * @file game_launcher.cpp
 * @brief Game Launcher implementation
 */

#include "NovelMind/runtime/game_launcher.hpp"
#include "NovelMind/core/logger.hpp"
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

namespace fs = std::filesystem;

namespace NovelMind::runtime {

std::string LauncherError::format() const {
  std::string result = "[" + code + "] " + message;
  if (!details.empty()) {
    result += "\nDetails: " + details;
  }
  if (!suggestion.empty()) {
    result += "\nSuggestion: " + suggestion;
  }
  return result;
}

GameLauncher::GameLauncher() = default;

GameLauncher::~GameLauncher() {
  if (m_state == LauncherState::Running) {
    quit();
  }
}

Result<void> GameLauncher::initialize(int argc, char *argv[]) {
  m_options = parseArgs(argc, argv);

  if (m_options.help) {
    printHelp(argv[0]);
    m_state = LauncherState::Ready;
    return Result<void>::ok();
  }

  if (m_options.version) {
    printVersion();
    m_state = LauncherState::Ready;
    return Result<void>::ok();
  }

  // Determine base path
  std::string basePath = getExecutableDirectory();
  if (!m_options.configOverride.empty()) {
    fs::path configPath(m_options.configOverride);
    if (configPath.has_parent_path()) {
      basePath = configPath.parent_path().string();
    }
  }

  return initialize(basePath, m_options);
}

Result<void> GameLauncher::initialize(const std::string &basePath,
                                      const LaunchOptions &options) {
  setState(LauncherState::Initializing);
  m_basePath = basePath;
  m_options = options;

  // Normalize path
  if (!m_basePath.empty() && m_basePath.back() != '/' &&
      m_basePath.back() != '\\') {
    m_basePath += '/';
  }

  // Initialize in order
  auto result = initializeLogging();
  if (result.isError()) {
    setError("INIT_LOG", "Failed to initialize logging", result.error(),
             "Check write permissions in the logs directory");
    return result;
  }

  result = initializeConfig();
  if (result.isError()) {
    setError("INIT_CONFIG", "Failed to load configuration", result.error(),
             "Check that config/runtime_config.json exists and is valid JSON");
    return result;
  }

  result = initializeDirectories();
  if (result.isError()) {
    setError("INIT_DIRS", "Failed to create directories", result.error(),
             "Check write permissions in the game directory");
    return result;
  }

  result = initializePacks();
  if (result.isError()) {
    setError("INIT_PACKS", "Failed to load resource packs", result.error(),
             "Check that packs_index.json exists and pack files are present");
    return result;
  }

  result = initializeInput();
  if (result.isError()) {
    setError("INIT_INPUT", "Failed to initialize input system", result.error());
    return result;
  }

  result = initializeSaveSystem();
  if (result.isError()) {
    setError("INIT_SAVE", "Failed to initialize save system", result.error(),
             "Check write permissions in the saves directory");
    return result;
  }

  setState(LauncherState::Ready);
  logInfo("Game launcher initialized successfully");

  return Result<void>::ok();
}

i32 GameLauncher::run() {
  if (m_state != LauncherState::Ready) {
    logError("Cannot run: launcher not in Ready state");
    return 1;
  }

  // Check if help or version was requested
  if (m_options.help || m_options.version) {
    return 0;
  }

  setState(LauncherState::Running);
  m_running = true;

  logInfo("Starting game: " + m_configManager->getConfig().game.name);

  try {
    mainLoop();
  } catch (const std::exception &e) {
    setError("RUNTIME", "Runtime error", e.what());
    logError("Runtime exception: " + std::string(e.what()));
    return 1;
  }

  setState(LauncherState::ShuttingDown);
  logInfo("Game exited normally");

  return 0;
}

void GameLauncher::quit() {
  m_running = false;
}

bool GameLauncher::isRunning() const {
  return m_running;
}

void GameLauncher::showError(const LauncherError &error) {
  m_lastError = error;

  // Log the error
  logError(error.format());

  // Print to console
  std::cerr << "\n=== Error ===\n";
  std::cerr << error.format() << "\n";
  std::cerr << "=============\n\n";

  // In a full implementation, this would also show a GUI dialog
  if (m_onError) {
    m_onError(error);
  }
}

void GameLauncher::showError(const std::string &error) {
  LauncherError err;
  err.code = "ERROR";
  err.message = error;
  showError(err);
}

ConfigManager *GameLauncher::getConfigManager() {
  return m_configManager.get();
}

GameSettings *GameLauncher::getGameSettings() {
  return m_gameSettings.get();
}

const RuntimeConfig &GameLauncher::getConfig() const {
  static RuntimeConfig defaultConfig;
  if (m_configManager) {
    return m_configManager->getConfig();
  }
  return defaultConfig;
}

void GameLauncher::setOnError(OnLauncherError callback) {
  m_onError = std::move(callback);
}

void GameLauncher::setOnStateChanged(OnLauncherStateChanged callback) {
  m_onStateChanged = std::move(callback);
}

void GameLauncher::printVersion() {
  std::cout << "NovelMind Game Launcher version " << NOVELMIND_VERSION_MAJOR
            << "." << NOVELMIND_VERSION_MINOR << "." << NOVELMIND_VERSION_PATCH
            << "\n";
  std::cout << "A modern visual novel engine\n";
  std::cout << "Copyright (c) 2024 NovelMind Team\n";
}

void GameLauncher::printHelp(const char *programName) {
  std::cout << "Usage: " << programName << " [options]\n\n";
  std::cout << "NovelMind Game Launcher - Play visual novels.\n\n";
  std::cout << "Options:\n";
  std::cout << "  --config <path>   Override config file path\n";
  std::cout << "  --lang <locale>   Override language (e.g., en, ru)\n";
  std::cout << "  --scene <name>    Start from a specific scene\n";
  std::cout << "  --debug           Enable debug mode\n";
  std::cout << "  --verbose         Verbose logging\n";
  std::cout << "  --windowed        Disable fullscreen\n";
  std::cout << "  -h, --help        Show this help message\n";
  std::cout << "  --version         Show version information\n\n";
  std::cout << "The launcher automatically loads configuration from:\n";
  std::cout << "  config/runtime_config.json - Game settings\n";
  std::cout << "  config/runtime_user.json   - User preferences\n";
}

std::string GameLauncher::getExecutableDirectory() {
  try {
    // Try to get the executable path
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    fs::path exePath(buffer);
#else
    fs::path exePath = fs::read_symlink("/proc/self/exe");
#endif
    return exePath.parent_path().string() + "/";
  } catch (...) {
    // Fall back to current directory
    return fs::current_path().string() + "/";
  }
}

LaunchOptions GameLauncher::parseArgs(int argc, char *argv[]) {
  LaunchOptions opts;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      opts.help = true;
    } else if (arg == "--version") {
      opts.version = true;
    } else if (arg == "--config" && i + 1 < argc) {
      opts.configOverride = argv[++i];
    } else if (arg == "--lang" && i + 1 < argc) {
      opts.langOverride = argv[++i];
    } else if (arg == "--scene" && i + 1 < argc) {
      opts.sceneOverride = argv[++i];
    } else if (arg == "--debug") {
      opts.debugMode = true;
    } else if (arg == "--verbose" || arg == "-v") {
      opts.verbose = true;
    } else if (arg == "--windowed") {
      opts.noFullscreen = true;
    }
  }

  return opts;
}

Result<void> GameLauncher::initializeLogging() {
  auto &logger = core::Logger::instance();

  // Set log level based on options
  if (m_options.verbose) {
    logger.setLevel(core::LogLevel::Debug);
  } else if (m_options.debugMode) {
    logger.setLevel(core::LogLevel::Info);
  } else {
    logger.setLevel(core::LogLevel::Warning);
  }

  // Create logs directory
  std::string logsDir = m_basePath + "logs/";
  try {
    fs::create_directories(logsDir);
  } catch (const std::exception &e) {
    return Result<void>::error(std::string("Failed to create logs dir: ") +
                               e.what());
  }

  // Set output file
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  char timeStr[64];
  std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S",
                std::localtime(&time));

  std::string logFile = logsDir + "game_" + timeStr + ".log";
  logger.setOutputFile(logFile);

  logInfo("Logging initialized: " + logFile);
  return Result<void>::ok();
}

Result<void> GameLauncher::initializeConfig() {
  m_configManager = std::make_unique<ConfigManager>();

  auto result = m_configManager->initialize(m_basePath);
  if (result.isError()) {
    return result;
  }

  result = m_configManager->loadConfig();
  if (result.isError()) {
    return result;
  }

  // Apply command-line overrides
  if (m_options.noFullscreen) {
    m_configManager->setFullscreen(false);
  }

  if (!m_options.langOverride.empty()) {
    m_configManager->setLocale(m_options.langOverride);
  }

  // Initialize game settings
  m_gameSettings = std::make_unique<GameSettings>(m_configManager.get());
  result = m_gameSettings->initialize();
  if (result.isError()) {
    return result;
  }

  logInfo("Configuration loaded: " + m_configManager->getConfig().game.name +
          " v" + m_configManager->getConfig().game.version);

  return Result<void>::ok();
}

Result<void> GameLauncher::initializeDirectories() {
  if (!m_configManager) {
    return Result<void>::error("ConfigManager not initialized");
  }

  return m_configManager->ensureDirectories();
}

Result<void> GameLauncher::initializePacks() {
  // In a full implementation, this would:
  // 1. Read packs_index.json
  // 2. Initialize the VFS with pack files
  // 3. Verify pack integrity

  const auto &config = m_configManager->getConfig();
  std::string packsDir = m_basePath + config.packs.directory + "/";
  std::string indexPath = packsDir + config.packs.indexFile;

  // Check if packs directory exists
  if (!fs::exists(packsDir)) {
    logInfo("No packs directory found, running in development mode");
    return Result<void>::ok();
  }

  // Check if index file exists
  if (!fs::exists(indexPath)) {
    logInfo("No packs_index.json found, running in development mode");
    return Result<void>::ok();
  }

  logInfo("Resource packs initialized from: " + packsDir);
  return Result<void>::ok();
}

Result<void> GameLauncher::initializeWindow() {
  // In a full implementation, this would create the game window
  // using the settings from config

  const auto &config = m_configManager->getConfig();
  logInfo("Window: " + std::to_string(config.window.width) + "x" +
          std::to_string(config.window.height) +
          (config.window.fullscreen ? " (fullscreen)" : " (windowed)"));

  return Result<void>::ok();
}

Result<void> GameLauncher::initializeAudio() {
  // In a full implementation, this would initialize the audio system
  // with volume settings from config

  const auto &config = m_configManager->getConfig();
  logInfo("Audio: Master=" + std::to_string(static_cast<int>(config.audio.master * 100)) +
          "%, Music=" + std::to_string(static_cast<int>(config.audio.music * 100)) + "%");

  return Result<void>::ok();
}

Result<void> GameLauncher::initializeLocalization() {
  // In a full implementation, this would load localization files
  // for the selected language

  const auto &config = m_configManager->getConfig();
  logInfo("Localization: " + config.localization.currentLocale);

  return Result<void>::ok();
}

Result<void> GameLauncher::initializeInput() {
  // Input system is already configured via RuntimeConfig
  logInfo("Input bindings configured");
  return Result<void>::ok();
}

Result<void> GameLauncher::initializeSaveSystem() {
  const auto &config = m_configManager->getConfig();
  std::string savesDir = m_basePath + config.saves.saveDirectory + "/";

  try {
    fs::create_directories(savesDir);
    logInfo("Save directory: " + savesDir);
    return Result<void>::ok();
  } catch (const std::exception &e) {
    return Result<void>::error(std::string("Failed to create saves dir: ") +
                               e.what());
  }
}

Result<void> GameLauncher::initializeScriptRuntime() {
  // In a full implementation, this would initialize the script runtime
  // and load the start scene

  const auto &config = m_configManager->getConfig();
  std::string startScene =
      m_options.sceneOverride.empty() ? "main" : m_options.sceneOverride;

  logInfo("Script runtime ready, start scene: " + startScene);
  return Result<void>::ok();
}

void GameLauncher::mainLoop() {
  using Clock = std::chrono::high_resolution_clock;

  auto lastTime = Clock::now();
  constexpr f64 targetFps = 60.0;
  constexpr f64 frameTime = 1.0 / targetFps;

  logInfo("Entering main loop");

  // For this implementation, we'll run a simplified demo loop
  // In a full implementation, this would integrate with the Application class

  std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
  std::cout << "║           " << m_configManager->getConfig().game.name;
  std::cout << std::string(49 - m_configManager->getConfig().game.name.length(), ' ') << "║\n";
  std::cout << "║           Version " << m_configManager->getConfig().game.version;
  std::cout << std::string(41 - m_configManager->getConfig().game.version.length(), ' ') << "║\n";
  std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";

  std::cout << "Game launcher is running.\n";
  std::cout << "Configuration loaded from: " << m_basePath << "config/\n";
  std::cout << "Saves will be stored in: " << m_basePath
            << m_configManager->getConfig().saves.saveDirectory << "/\n";
  std::cout << "Logs are stored in: " << m_basePath
            << m_configManager->getConfig().logging.logDirectory << "/\n\n";

  std::cout << "Current Settings:\n";
  std::cout << "  Resolution: " << m_configManager->getConfig().window.width
            << "x" << m_configManager->getConfig().window.height << "\n";
  std::cout << "  Fullscreen: "
            << (m_configManager->getConfig().window.fullscreen ? "Yes" : "No")
            << "\n";
  std::cout << "  Language: "
            << m_configManager->getConfig().localization.currentLocale << "\n";
  std::cout << "  Master Volume: "
            << static_cast<int>(m_configManager->getConfig().audio.master * 100)
            << "%\n\n";

  std::cout << "Press Enter to exit...\n";
  std::cin.get();

  m_running = false;
}

void GameLauncher::update(f64 deltaTime) {
  (void)deltaTime;
  // Update game logic
}

void GameLauncher::render() {
  // Render frame
}

void GameLauncher::setState(LauncherState state) {
  m_state = state;
  if (m_onStateChanged) {
    m_onStateChanged(state);
  }
}

void GameLauncher::setError(const std::string &code, const std::string &message,
                            const std::string &details,
                            const std::string &suggestion) {
  m_lastError.code = code;
  m_lastError.message = message;
  m_lastError.details = details;
  m_lastError.suggestion = suggestion;
  setState(LauncherState::Error);
}

void GameLauncher::logInfo(const std::string &message) {
  NOVELMIND_LOG_INFO("[Launcher] " + message);
}

void GameLauncher::logWarning(const std::string &message) {
  NOVELMIND_LOG_WARN("[Launcher] " + message);
}

void GameLauncher::logError(const std::string &message) {
  NOVELMIND_LOG_ERROR("[Launcher] " + message);
}

} // namespace NovelMind::runtime
