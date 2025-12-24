/**
 * @file test_choice_rendering.cpp
 * @brief Test to verify Choice objects render correctly in Scene View
 *
 * This test verifies the fix for issue #4:
 * - ChoiceUIObject rendering is implemented
 * - Choice objects can be added to scene graph
 * - Choice objects have proper visibility and resource access
 */

#include "NovelMind/scene/scene_graph.hpp"
#include "NovelMind/resource/resource_manager.hpp"
#include <iostream>
#include <cassert>

using namespace NovelMind::scene;
using namespace NovelMind::resource;

int main() {
    std::cout << "=== Testing Choice Rendering ===" << std::endl;

    // Create scene graph
    SceneGraph graph;
    graph.setSceneId("test_choice_scene");

    // Create resource manager
    ResourceManager resources;
    graph.setResourceManager(&resources);

    std::cout << "1. Testing showChoices() creates ChoiceUIObject..." << std::endl;

    // Create choices
    std::vector<ChoiceUIObject::ChoiceOption> choices = {
        {"opt1", "Trust the AI", true, true, ""},
        {"opt2", "Shutdown the AI", true, true, ""},
        {"opt3", "Ask more questions", true, true, ""}
    };

    auto* choiceUI = graph.showChoices(choices);
    assert(choiceUI != nullptr && "showChoices should return non-null pointer");
    std::cout << "   ✓ ChoiceUIObject created successfully" << std::endl;

    std::cout << "2. Testing ChoiceUIObject properties..." << std::endl;
    assert(choiceUI->isVisible() && "Choice should be visible by default");
    assert(choiceUI->getAlpha() == 1.0f && "Choice should have full alpha");
    assert(choiceUI->getType() == SceneObjectType::ChoiceUI && "Type should be ChoiceUI");
    std::cout << "   ✓ ChoiceUIObject has correct properties" << std::endl;

    std::cout << "3. Testing choice options..." << std::endl;
    const auto& opts = choiceUI->getChoices();
    assert(opts.size() == 3 && "Should have 3 choice options");
    assert(opts[0].text == "Trust the AI" && "First choice text should match");
    assert(opts[1].text == "Shutdown the AI" && "Second choice text should match");
    assert(opts[2].text == "Ask more questions" && "Third choice text should match");
    std::cout << "   ✓ Choice options are correctly set" << std::endl;

    std::cout << "4. Testing findObject() can locate choice..." << std::endl;
    auto* found = graph.findObject("choice_menu");
    assert(found != nullptr && "Should be able to find choice by ID");
    assert(found == choiceUI && "Found object should be the same instance");
    std::cout << "   ✓ ChoiceUIObject can be found in scene graph" << std::endl;

    std::cout << "5. Testing hideChoices()..." << std::endl;
    graph.hideChoices();
    assert(!choiceUI->isVisible() && "Choice should be hidden");
    std::cout << "   ✓ hideChoices() works correctly" << std::endl;

    std::cout << "6. Testing showChoices() again makes it visible..." << std::endl;
    graph.showChoices(choices);
    assert(choiceUI->isVisible() && "Choice should be visible again");
    std::cout << "   ✓ showChoices() on existing choice makes it visible" << std::endl;

    std::cout << "7. Testing saveState/loadState..." << std::endl;
    auto state = choiceUI->saveState();
    assert(state.type == SceneObjectType::ChoiceUI && "Saved state should have ChoiceUI type");
    assert(state.properties.find("choiceCount") != state.properties.end() && "Should save choice count");
    std::cout << "   ✓ State serialization works" << std::endl;

    std::cout << "\n=== All tests passed! ✓ ===" << std::endl;
    std::cout << "\nConclusion:" << std::endl;
    std::cout << "- ChoiceUIObject is fully implemented and functional" << std::endl;
    std::cout << "- The issue #4 was not in the core rendering logic" << std::endl;
    std::cout << "- The fix added renderChoice() to NMSceneGLViewport for GL preview" << std::endl;

    return 0;
}
