#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "MainComponent.h"

//==============================================================================

class GregTrainerApplication : public juce::JUCEApplication {
public:
  //==============================================================================
  GregTrainerApplication() {}

  const juce::String getApplicationName() override { return "GregTrainer"; }
  const juce::String getApplicationVersion() override { return "0.0.1"; }
  bool moreThanOneInstanceAllowed() override { return true; }

  //==============================================================================
  void initialise(const juce::String &commandLine) override {
    mainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override { mainWindow = nullptr; }

  //==============================================================================
  void systemRequestedQuit() override { quit(); }

  void anotherInstanceStarted(const juce::String &commandLine) override {}

  //==============================================================================
  /*
      This class implements the desktop window that contains an instance of
      our MainComponent class.
  */
  class MainWindow : public juce::DocumentWindow {
  public:
    MainWindow(juce::String name)
        : juce::DocumentWindow(
              name,
              juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                  ResizableWindow::backgroundColourId),
              DocumentWindow::allButtons),
          tree(IDs::GlobalRoot) {
      setUsingNativeTitleBar(true);
      setContentOwned(new MainComponent(tree), true);

#if JUCE_IOS || JUCE_ANDROID
      setFullScreen(true);
#else
      setResizable(true, true);
      centreWithSize(getWidth(), getHeight());
#endif

      setVisible(true);
    }

    void closeButtonPressed() override {
      JUCEApplication::getInstance()->systemRequestedQuit();
    }

  private:
    juce::ValueTree tree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  };

private:
  std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(GregTrainerApplication)
